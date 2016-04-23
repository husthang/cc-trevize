# -*- coding: utf-8 -*-

"""Trevize: find path using heuristic algorithm

- input
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
    - o: output filename
- process: V'-based search
    - find links between vertice in V'
    - greedy search between these paths
    - merge into the path
- output
    - path

Notes:
    - as we use greedy search between paths, we cannot always find the path.
    (e.g. std case 3 is a counter example. while 1, 2, 4 can be easily solved.)
    - this algorithm can be fixed to
        - find more pairs first
        - use DFS or random search (instead of greedy based on in/out degrees)
    - however, as we are working in Phase II and integer programming is
    proposed as priority, V'-based search is not so important now.
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

    # s, t, v1(may be blank)
    for line in stv1:
        stv1_list = re.findall(num, line)
        for i in range(len(stv1_list)):
            stv1_list[i] = int(stv1_list[i])
        s = stv1_list[0]
        t = stv1_list[1]
        v1 = stv1_list[2:]

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

    return G, s, t, v1


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
    prob = LpProblem("cc problem", LpMinimize)

    x = {}

    prob += 0, "no weight for MVP"

    # init variable list(edges 0/1)
    for edge in G.edges_iter():
        # print(edge)
        # print(G[edge[0]][edge[1]]["label"])
        x[G[edge[0]][edge[1]]["label"]] = LpVariable("edges{}".format(G[edge[0]][edge[1]]["label"]), 0, 1, LpInteger)

    print(x)
    # s/t/v1 constaints
    prob += lpSum(x[G[edge[0]][edge[1]]["label"]] for edge in G.out_edges(s)) == 1, "s"
    prob += lpSum(x[G[edge[0]][edge[1]]["label"]] for edge in G.in_edges(t)) == 1, "t"
    for v in v1:
        prob += lpSum(x[G[edge[0]][edge[1]]["label"]] for edge in G.out_edges(v)) == 1, ""
        prob += lpSum(x[G[edge[0]][edge[1]]["label"]] for edge in G.in_edges(v)) == 1, ""

    for v in G.nodes_iter():
        if v not in v1 + [s, t]:
            prob += lpSum(x[G[edge[0]][edge[1]]["label"]] for edge in G.out_edges(v)) <= 1, ""
            # prob += lpSum(x[G[edge[0]][edge[1]]["label"]] for edge in G.out_edges(v)) <= 1, ""


    prob.writeLP("ip-trevize.lp")
    prob.solve()
    print(LpStatus[prob.status])
    for e in prob.variables():
        print(e.name, '=', e.varValue)
    return

def write_csv(f, answer, format, verbose):
    """Write the answer to csv file.

    proposed format:
        - 'NA' for no answer;
        - 'e[1]|e[2]|..|e[n]' for shortest path, e[i] is label of edge
    parameters:
        - f: file output
        - answer: NA or path list `paths`
        - format:
            - 'all' to output all paths;
            - 'shortest' to output shortest route (proposed `result.csv`)
        - verbose: set to True to printout more information
    """
    if answer is "NA":
        if verbose:
            print(answer)
        f.write(answer)
        return 0
    # else
    paths, G = answer
    for path in paths:
        route = ""
        weight = 0
        for i in range(len(path) - 1):
            # as used i+1
            route += str(G.edge[path[i]][path[i+1]]['label']) + '|'
            weight += G[path[i]][path[i+1]]['weight']
        if verbose:
            print("{}, {}".format(str(weight), route[:-1]))
        if format is 'shortest':
            # only write first result, no weight output
            f.write("{}".format(route[:-1]))
            return 0
        f.write("{}, {}\n".format(str(weight), route[:-1]))

    return 0


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

    G, s, t, v1 = read_csv(topo, demand)

    global t0
    t0 = time.time()
    answer = trevize(G, s, t, v1, verbose)
    # write_csv(o, answer, 'all', verbose)
    t1 = time.time()
    print("time: {}".format(t1-t0))

    return 0


if __name__ == '__main__':
    main()

    # import cProfile
    # cProfile.run('main()', 'restats')

    # import pstats
    # p = pstats.Stats('restats')
    # p.sort_stats('cumulative').print_stats(20)
