# -*- coding: utf-8 -*-

"""Trevize: find path using heuristic algorithm

- input
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
    - o: output filename
- process
    - DFS + DP
- output
    - path
"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX

import networkx as nx


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
    process: BFS & check path when completed.
        - using deque for a first implementation
        - sort edges before adding to deque as a stack
    output: valid path list `valid_paths`
    """
    # from collections import deque
    from pprint import pprint

    global preds, succs
    set_v1 = set(v1)
    valid_paths = []
    preds = {}
    succs = {}
    # store as vertex: {depth: {end_vertex: [list of paths]}}

    print(v1)
    global num_paths, max_weight
    num_paths = 0
    BIG_WEIGHT = 4800
    max_weight = BIG_WEIGHT
    global i_searched  # num of paths searched
    i_searched = 0

    for v in v1:
        preds[v] = {0: {}}
        succs[v] = {0: {}}

        for v_pred in G.predecessors_iter(v):
            preds[v][0][v_pred] = [[v_pred]]
        for v_succ in G.successors_iter(v):
            succs[v][0][v_succ] = [[v_succ]]
    pprint(preds)
    pprint(succs)

    def iter_layer(v, depth, direction='in'):
        """Iterate one more layer of predecessors/successors.

        add one more vertex to begin/end of paths."""
        if direction is 'in':
            preds[v][depth] = {}
            for end_vertex in preds[v][depth - 1]:
                for end_path in preds[v][depth - 1][end_vertex]:
                    for v_pred in G.predecessors_iter(end_vertex):
                        if v_pred not in preds[v][depth]:
                            preds[v][depth][v_pred] =\
                                [[v_pred] + end_path]
                        else:
                            preds[v][depth][v_pred] +=\
                                [[v_pred] + end_path]
        else:  # out, successors
            succs[v][depth] = {}
            for end_vertex in succs[v][depth - 1]:
                for end_path in succs[v][depth - 1][end_vertex]:
                    for v_succ in G.successors_iter(end_vertex):
                        if v_succ not in succs[v][depth]:
                            succs[v][depth][v_succ] =\
                                [end_path + [v_succ]]
                        else:
                            succs[v][depth][v_succ] +=\
                                [end_path + [v_succ]]

    for v in v1:
        iter_layer(v, 1, 'in')
        iter_layer(v, 1, 'out')
    pprint(preds)
    pprint(succs)

    def merge_dicts(x, y):
        """Given two dicts, merge them into a new dict as a shallow copy."""
        z = x.copy()
        z.update(y)
        return z

    def check_path(path):
        """Check if path contains all vertices in v1."""
        if set_v1 <= set(path[1:-1]):
            # print('find path:', path)
            return True
        else:
            return False

    def sort_path(vertex, next_list):
        """Sort next vertex list by V' first, then by weight."""

        weight_list = {}
        for next_v in next_list:
            # generate value list: add large num to vertices not in v1
            LARGE_NUM = 21
            if next_v in v1:
                weight_list[next_v] = next_list[next_v]['weight']
            else:
                weight_list[next_v] = (next_list[next_v]['weight'] + LARGE_NUM)

        return sorted(weight_list, key=weight_list.get)



    if verbose:  # verbose printout
        print("added route:", i_searched)
        print("num of paths: {}".format(num_paths))
        pprint(valid_paths)

    if valid_paths:  # output
        return valid_paths, G
    else:
        if verbose:
            print('no path')
        return "NA"


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

    import time
    t0 = time.time()
    answer = trevize(G, s, t, v1, verbose)
    # print(answer)
    write_csv(o, answer, 'all', verbose)
    t1 = time.time()
    print("time: {}".format(t1-t0))

    return 0

if __name__ == '__main__':
    main()
