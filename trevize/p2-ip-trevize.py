# -*- coding: utf-8 -*-

"""Trevize: find path using heuristic algorithm.

Phase II version.
- input
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')` and `s, t, v2(V'')`
    - o: output filename
- process: integer-programming based optimization
    - IP to find P1 and P2
    - try edge-disjoint P1 and P2
- output
    - path

Todo:
    - support multigraph(add support of multi-edges)
"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX

import networkx as nx
import time  # get benchmark data
from pulp import *


def read_csv(g, stv1):
    """Read the two cs v input files.

    input: two csv input file
        detail of input format in http://codecraft.huawei.com/home/detail
        solve multigraph in input
    output: NetworkX-graph
    """
    import re
    num = re.compile('\d+')

    # read demand.csv
    # read line 1
    line1 = stv1.readline()
    stv1_list = re.findall(num, line1)
    for i in range(len(stv1_list)):
        stv1_list[i] = int(stv1_list[i])
    s = stv1_list[0]
    t = stv1_list[1]
    v1 = stv1_list[2:]
    # read line 2
    line2 = stv1.readline()
    stv2_list = re.findall(num, line2)
    for i in range(len(stv2_list)):
        stv2_list[i] = int(stv2_list[i])
    s = stv2_list[0]
    t = stv2_list[1]
    v2 = stv2_list[2:]

    # read topo.csv
    G = nx.DiGraph()
    for line in g:
        nums = re.findall(num, line)

        for i in range(4):
            nums[i] = int(nums[i])  # re str to int

        if nums[1] is t or nums[2] is s:
            # print("s/t ", line)
            continue

        # solve multigraph: find best edge and apply
        try:
            exist_weight = G[nums[1]][nums[2]]['weight']
        except KeyError:
            G.add_edge(nums[1], nums[2], weight=nums[3], label=nums[0])
        else:
            if exist_weight > nums[3]:  # need update
                G[nums[1]][nums[2]]['weight'] = nums[3]
                G[nums[1]][nums[2]]['label'] = nums[0]

    for node in G.nodes():
        if (node is not s) and (node is not t):
            if (G.in_degree(node) is 0) or (G.out_degree(node) is 0):
                # print(node)
                G.remove_node(node)
                # dead node

    return G, s, t, v1, v2


def trevize(G, s, t, v1, verbose):
    """Find path.

    input: graph, s, t, v1
    process:
        - init IP problem
            - 0/1 edges
            - s/t/v1
            - other nodes
        - call solver
        - check and output
    output: valid path list `valid_paths`
    """
    prob = LpProblem("cc problem", LpMinimize)  # minimize path weight

    x = {}
    # init variable list(edges 0/1)
    edge_dict = {}
    for edge in G.edges_iter():
        # print(edge)
        # print(G[edge[0]][edge[1]]["label"])
        edge_dict[G[edge[0]][edge[1]]["label"]] = edge
        label = G[edge[0]][edge[1]]["label"]
        x[label] = LpVariable("edges{}".format(label), 0, 1, LpInteger)
    for node in G.nodes_iter():
        x['pi'+str(node)] = LpVariable("pi{}".format(node), 0, None, LpInteger)
    # print(x) # test variable list
    # s/t/v1 constraints
    prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
                  for edge in G.out_edges(s)) == 1, "s out"
    # prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
    #               for edge in G.in_edges(s)) == 0, "s in"
    prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
                  for edge in G.in_edges(t)) == 1, "t in"
    # prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
    #               for edge in G.out_edges(t)) == 0, "t out is 0"
    for v in v1:
        prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
                      for edge in G.out_edges(v)) == 1, ""
        prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
                      for edge in G.in_edges(v)) == 1, ""
    # other nodes constraints: in - out = 0; in <= 1
    for v in G.nodes_iter():
        if v not in v1 + [s, t]:
            prob += (lpSum(x[G[edge[0]][edge[1]]["label"]]
                          for edge in G.out_edges(v))
                    -
                    lpSum(x[G[edge[0]][edge[1]]["label"]]
                          for edge in G.in_edges(v))) == 0, ""
            prob += lpSum(x[G[edge[0]][edge[1]]["label"]]
                          for edge in G.out_edges(v)) <= 1, ""
    # pi constraints (Andrade 2013)
    prob += x['pi'+str(s)] == 0, "pi(s) = 0"
    M = 10000  # enough big num
    for edge in G.edges_iter():
        prob += x['pi'+str(edge[1])] - x['pi'+str(edge[0])] <=\
            G[edge[0]][edge[1]]["weight"] + M*(1-x[G[edge[0]][edge[1]]["label"]])
        prob += x['pi'+str(edge[1])] - x['pi'+str(edge[0])] >=\
            G[edge[0]][edge[1]]["weight"] - M*(1-x[G[edge[0]][edge[1]]["label"]])

    # the optimize target: total weight
    prob += lpSum(x[G[edge[0]][edge[1]]["label"]] *
                  G[edge[0]][edge[1]]["weight"]
                  for edge in G.edges_iter()), "weight for MVP"

    # prob.writeLP("ip-trevize.lp")  # output LP

    def check_IP_path(path_edge_list, edge_dict, s, t):
        """Check if the path is valid path.

        - input: edge list from IP.
        - process: check cycles in path.
            i.e. check length of s-t. == len(edge_list) means no cycle.
        - output: valid path route or return invalid path from s to t."""
        # print(s, t)
        # print(path_edge_list)
        begin_v = s
        path = []
        edge_list = path_edge_list[:]
        while begin_v != t:
            # print(path_edge_list)
            # print(len(edge_list))
            for i in range(len(edge_list)):
                e = edge_list[i]
                if edge_dict[e][0] == begin_v:
                    begin_v = edge_dict[e][1]
                    path.append(str(e))
                    # print(path, begin_v, t)
                    del edge_list[i]
                    break
            # print(path)
        if len(path) == len(path_edge_list):
            return "|".join(path)
        else:  # forming cycle in path
            return path

    num_iter = 0  # printout number of iterations
    while True:
        num_iter += 1
        prob.solve(GUROBI_CMD(msg=False))  # Gurobi interface
        # prob.solve(PULP_CBC_CMD())  # open source solver
        # ref: https://pythonhosted.org/PuLP/solvers.html#pulp.solvers.COIN_CMD
        # output
        if verbose:
            print(LpStatus[prob.status])
        path_edge_list = []
        for e in prob.variables():
            if e.varValue == 1 and 'edge' in e.name:
                # print(e.name, '=', e.varValue)
                path_edge_list.append(int(e.name[5:]))
        # print(path_edge_list)
        check_result = check_IP_path(path_edge_list, edge_dict, s, t)
        # print(type(check_result))
        if type(check_result) is list:
            # print(check_result)
            prob += lpSum(x[int(edge)] for edge in check_result) <=\
                    len(check_result) - 1, ""
            # if num_iter % 10 == 0:
            #     print(num_iter)
        else:
            if verbose:
                print("iter: {}".format(num_iter))
                print(check_result)
            return(check_result)
    return


def main():
    """Parse arguments and main logic."""
    import argparse
    # argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('topo',
                        help='`topo.csv`, contains graph')
    parser.add_argument('demand',
                        help='`demand.csv`, contains s, t, v1.')
    parser.add_argument('o',
                        help='output filename')
    parser.add_argument('-v', '--verbose',
                        help='verbose printout',
                        action="store_true")
    args = parser.parse_args()
    topo = open(args.topo)
    demand = open(args.demand)
    o = open(args.o, 'w')
    verbose = args.verbose

    G, s, t, v1, v2 = read_csv(topo, demand)

    global t0
    t0 = time.time()
    path1 = trevize(G, s, t, v1, verbose)
    path2 = trevize(G, s, t, v2, verbose)
    o.write(path1)
    o.write('\n')
    o.write(path2)
    t1 = time.time()
    if verbose:
        print("time: {}".format(t1-t0))

    return 0


if __name__ == '__main__':
    main()

    # import cProfile
    # cProfile.run('main()', 'restats')

    # import pstats
    # p = pstats.Stats('restats')
    # p.sort_stats('cumulative').print_stats(20)
