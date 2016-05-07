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
    s = stv1_list[1]
    t = stv1_list[2]
    v1 = stv1_list[3:]
    # read line 2
    line2 = stv1.readline()
    stv2_list = re.findall(num, line2)
    for i in range(len(stv2_list)):
        stv2_list[i] = int(stv2_list[i])
    s = stv2_list[1]
    t = stv2_list[2]
    v2 = stv2_list[3:]

    # read topo.csv
    G = nx.MultiDiGraph()
    for line in g:
        nums = re.findall(num, line)

        for i in range(4):
            nums[i] = int(nums[i])  # re str to int

        if nums[1] is t or nums[2] is s:
            # print("s/t ", line)
            continue

        G.add_edge(nums[1], nums[2], weight=nums[3], label=nums[0])

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
    edge_dict = {}  # {label: in, out, weight}
    for u, v, edata in G.edges_iter(data=True):
        # print(u, v, edata['label'], edata['weight'])
        label = edata['label']
        weight = edata['weight']
        edge_dict[label] = (u, v, weight)
        x[label] = LpVariable("edges{}".format(label), 0, 1, LpInteger)
    for node in G.nodes_iter():
        x['pi'+str(node)] = LpVariable("pi{}".format(node), 0, None, LpInteger)

    # s/t/v1 constraints
    prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                  for edge in G.out_edges(s)
                  for e in G[edge[0]][edge[1]]
                  ) == 1, "s out"
    prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                  for edge in G.in_edges(t)
                  for e in G[edge[0]][edge[1]]
                  ) == 1, "t in"
    for v in v1:
        prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                    for edge in G.out_edges(v)
                    for e in G[edge[0]][edge[1]]
                    ) == 1, ""
        prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                    for edge in G.in_edges(v)
                    for e in G[edge[0]][edge[1]]
                    ) == 1, ""

    # other nodes constraints: in - out = 0; in <= 1
    for v in G.nodes_iter():
        if v not in v1 + [s, t]:
            prob += (lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.out_edges(v)
                            for e in G[edge[0]][edge[1]])
                    -
                    lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.in_edges(v)
                            for e in G[edge[0]][edge[1]])) == 0, ""
            prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.in_edges(v)
                            for e in G[edge[0]][edge[1]]) <= 1, ""
    # pi constraints (Andrade 2013)
    prob += x['pi'+str(s)] == 0, "pi(s) = 0"
    M = 10000  # enough big num
    for u, v, edata in G.edges_iter(data=True):
        prob += x['pi'+str(v)] - x['pi'+str(u)] <=\
            edata["weight"] + M*(1-x[edata["label"]])
        prob += x['pi'+str(v)] - x['pi'+str(u)] >=\
            edata["weight"] - M*(1-x[edata["label"]])

    # the optimize target: total weight
    prob += lpSum(x[label] * edge_dict[label][2]
                  for label in edge_dict), "weight for MVP"

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


def trevize2(G, s, t, v1, v2, verbose):
    """Find path.

    input: graph, s, t, v1, v2
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
    edge_dict = {}  # {label: in, out, weight}
    N = 100000
    for u, v, edata in G.edges_iter(data=True):
        # print(u, v, edata['label'], edata['weight'])
        label = edata['label']
        weight = edata['weight']
        edge_dict[label] = (u, v, weight)
        x[label] = LpVariable("edge1_{}".format(label), 0, 1, LpInteger)
        x[N+label] = LpVariable("edge2_{}".format(label), 0, 1, LpInteger)
        # big number to remove duplicate label, may lead to bug in small
    for node in G.nodes_iter():
        x['pi1_'+str(node)] = LpVariable("pi1_{}".format(node),
                                        0, None, LpInteger)
        x['pi2_'+str(node)] = LpVariable("pi2_{}".format(node),
                                        0, None, LpInteger)

    # s/t/v1 constraints
    prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                  for edge in G.out_edges(s)
                  for e in G[edge[0]][edge[1]]
                  ) == 1, "s out"
    prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                  for edge in G.in_edges(t)
                  for e in G[edge[0]][edge[1]]
                  ) == 1, "t in"
    for v in v1:
        prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                      for edge in G.out_edges(v)
                      for e in G[edge[0]][edge[1]]
                     ) == 1, "v1 {} out".format(v)
        prob += lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                      for edge in G.in_edges(v)
                      for e in G[edge[0]][edge[1]]
                     ) == 1, "v1 {} in".format(v)

    # s/t/v2 constraints
    prob += lpSum(x[N+G[edge[0]][edge[1]][e]["label"]]
                  for edge in G.out_edges(s)
                  for e in G[edge[0]][edge[1]]
                  ) == 1, "s out 2"
    prob += lpSum(x[N+G[edge[0]][edge[1]][e]["label"]]
                  for edge in G.in_edges(t)
                  for e in G[edge[0]][edge[1]]
                  ) == 1, "t in 2"
    for v in v2:
        prob += lpSum(x[N+G[edge[0]][edge[1]][e]["label"]]
                      for edge in G.out_edges(v)
                      for e in G[edge[0]][edge[1]]
                     ) == 1, "v2 {} out".format(v)
        prob += lpSum(x[N+G[edge[0]][edge[1]][e]["label"]]
                      for edge in G.in_edges(v)
                      for e in G[edge[0]][edge[1]]
                     ) == 1, "v2 {} in".format(v)

    for v in G.nodes_iter():
        if v not in v1 + [s, t]:
            # other nodes constraints: in - out = 0; in <= 1
            prob += (lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.out_edges(v)
                            for e in G[edge[0]][edge[1]])
                    -
                    lpSum(x[G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.in_edges(v)
                            for e in G[edge[0]][edge[1]])) == 0,\
                    "p1 pass {}".format(v)
        if v not in v2 + [s, t]:
            prob += (lpSum(x[N+G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.out_edges(v)
                            for e in G[edge[0]][edge[1]])
                    -
                    lpSum(x[N+G[edge[0]][edge[1]][e]["label"]]
                            for edge in G.in_edges(v)
                            for e in G[edge[0]][edge[1]])) == 0,\
                    "p2 pass {}".format(v)

    for label in edge_dict:  # all nodes: no duplicate edges
        prob += x[label] + x[N+label] <= 1, "no_dup_edge{}".format(label)

    # no duplicate edges in path1 and path2
    # pi constraints path1 (Andrade 2013)
    prob += x['pi1_'+str(s)] == 0, "pi1_(s)=0"
    prob += x['pi2_'+str(s)] == 0, "pi2_(s)=0"
    M = 100000  # enough big num
    for u, v, edata in G.edges_iter(data=True):
        prob += x['pi1_'+str(v)] - x['pi1_'+str(u)] <=\
            edata["weight"] + M*(1-x[edata["label"]])
        prob += x['pi1_'+str(v)] - x['pi1_'+str(u)] >=\
            edata["weight"] - M*(1-x[edata["label"]])
        prob += x['pi2_'+str(v)] - x['pi2_'+str(u)] <=\
            edata["weight"] + M*(1-x[N+edata["label"]])
        prob += x['pi2_'+str(v)] - x['pi2_'+str(u)] >=\
            edata["weight"] - M*(1-x[N+edata["label"]])


    # the optimize target: total weight
    prob += lpSum((x[label] + x[N+label]) * edge_dict[label][2]
                  for label in edge_dict), "weight for MVP p2"

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
            length = len(edge_list)
            for i in range(len(edge_list)):
                e = edge_list[i]
                if edge_dict[e][0] == begin_v:
                    begin_v = edge_dict[e][1]
                    path.append(str(e))
                    # print(path, begin_v, t)
                    del edge_list[i]
                    break
            if len(edge_dict) == length:
                print("wrong path")
                return
        if len(path) == len(path_edge_list):
            return "|".join(path)
        else:  # forming cycle in path
            return path

    num_iter = 0  # printout number of iterations
    while True:
        num_iter += 1
        prob.solve(GUROBI_CMD())  # Gurobi interface
        # prob.solve(PULP_CBC_CMD())  # open source solver
        # ref: https://pythonhosted.org/PuLP/solvers.html#pulp.solvers.COIN_CMD
        # output
        if verbose:
            print(LpStatus[prob.status])
        path1_edge_list = []
        path2_edge_list = []
        for e in prob.variables():
            if e.varValue == 1 and 'edge1' in e.name:
                # print(e.name, '=', e.varValue)
                path1_edge_list.append(int(e.name[6:]))
            elif e.varValue == 1 and 'edge2' in e.name:
                # print(e.name, '=', e.varValue)
                path2_edge_list.append(int(e.name[6:]))
            elif e.varValue != 0 and e.varValue < 90000:
                # print(e.name, e.varValue)
                pass

        # print(path1_edge_list, path2_edge_list)

        # print(edge_dict)
        check_result1 = check_IP_path(path1_edge_list, edge_dict, s, t)
        check_result2 = check_IP_path(path2_edge_list, edge_dict, s, t)
        # print(type(check_result))
        if type(check_result1) is str and type(check_result2) is str:
            if verbose:
                print("iter: {}".format(num_iter))
                print(check_result1)
                print(check_result2)
            return (check_result1, check_result2)
        else:
            print('WA')
            return (check_result1, check_result2)
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
    # try best path (no duplicate edges)
    path1, path2 = trevize2(G, s, t, v1, v2, verbose)
    # if no answer, try p1 + p2 and p2 + p1, and find best
    # path1 = trevize(G, s, t, v1, verbose)
    # path2 = trevize(G, s, t, v2, verbose)
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
