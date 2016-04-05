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
    def init_globals(s, t, v1):
        """Init preds, succs of first layer.

        store as vertex: {depth: {end_vertex: [list of paths]}}
        vertices in preds: v in v1 and t.
        vertices in succs: v in v1 and s."""
        # init s, t
        succs[s] = {1: {}}
        preds[t] = {1: {}}
        for v_pred in G.predecessors_iter(t):
            preds[t][1][v_pred] = [[v_pred, t]]
        for v_succ in G.successors_iter(s):
            succs[s][1][v_succ] = [[s, v_succ]]
        # init v in v1
        for v in v1:
            preds[v] = {1: {}}
            succs[v] = {1: {}}
            for v_pred in G.predecessors_iter(v):
                preds[v][1][v_pred] = [[v_pred, v]]
            for v_succ in G.successors_iter(v):
                succs[v][1][v_succ] = [[v, v_succ]]

    def iter_layer(v, depth, direction='in'):
        """Iterate one more layer of predecessors/successors BFS.

        add one more vertex to begin/end of paths."""
        if direction is 'in':
            preds[v][depth] = {}
            for end_vertex in preds[v][depth - 1]:
                if end_vertex in set_v1:  # set is hashable, faster than list
                    continue
                for end_path in preds[v][depth - 1][end_vertex]:
                    for v_pred in G.predecessors_iter(end_vertex):
                        if v_pred not in end_path:
                            if v_pred not in preds[v][depth]:
                                preds[v][depth][v_pred] =\
                                    [[v_pred] + end_path]
                            else:
                                preds[v][depth][v_pred] +=\
                                    [[v_pred] + end_path]
        elif direction is 'out':  # out, successors
            succs[v][depth] = {}
            for end_vertex in succs[v][depth - 1]:
                if end_vertex in set_v1:
                    continue
                for end_path in succs[v][depth - 1][end_vertex]:
                    for v_succ in G.successors_iter(end_vertex):
                        if v_succ not in end_path:
                            if v_succ not in succs[v][depth]:
                                succs[v][depth][v_succ] =\
                                    [end_path + [v_succ]]
                            else:
                                succs[v][depth][v_succ] +=\
                                    [end_path + [v_succ]]
        else:
            return 1

    def get_v_layer(v, depth, direction):
        """Wrapper to get demanded layer of in/out BFS.

        - direction: 'in', 'out'
        """
        if direction is 'in':
            global_dict = preds
        elif direction is 'out':
            global_dict = succs
        else:  # wrong direction
            return 1
        for i_depth in range(depth, 0, -1):
            if i_depth in global_dict[v]:
                layers_to_cal = list(range(i_depth + 1, depth + 1))
                for layer in layers_to_cal:
                    iter_layer(v, layer, direction)
                return

    def merge_dicts(x, y):
        """Given two dicts, merge them into a new dict as a shallow copy."""
        z = x.copy()
        z.update(y)
        return z

    def check_path(path):
        """Check if path con tains all vertices in v1."""
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

    # from collections import deque
    from pprint import pprint

    set_v1 = set(v1)
    valid_paths = []
    preds = {}
    succs = {}
    links = {}  # (vi, vj): [path1, path2, ...]

    global num_paths, max_weight
    num_paths = 0
    BIG_WEIGHT = 4800
    max_weight = BIG_WEIGHT
    global i_searched  # num of paths searched
    i_searched = 0
    print("V':", v1)

    init_globals(s, t, v1)
    pprint(preds)
    pprint(succs)

    def check_path_between(v1_n, n_v2):
        """Check if path  of v1->n and n->v2 can be merged.

        input: v1_n, n_v2: list of path.
        output: return True if the path is valid.
        """
        if set(v1_n) & set(n_v2) == set([n_v2[0]]):  # intersection is only n
            return True
        else:
            return False

    def merge_v1():
        """Merge (vi, vj)."""
        pass

    def add_link(v_out, v_in, p_out=[], p_in=[]):
        if not p_out:
            p_out = [v_out]
        if p_in:
            p_in = p_in[1:]
        else:
            p_in = [v_in]
        if (v_out, v_in) in links:
            links[(v_out, v_in)] += [p_out + p_in]
        else:
            links[(v_out, v_in)] = [p_out + p_in]


    def find_links(v_out, v_in, depth_out, depth_in):
        """Find demanded links of (vi, vj)."""
        for v_succ in succs[v_out][depth_out]:
            if v_succ in preds[v_in][depth_in]:
                paths_out = succs[v_out][depth_out][v_succ]
                paths_in = preds[v_in][depth_in][v_succ]
                for p_out in paths_out:
                    for p_in in paths_in:
                        if check_path_between(p_out, p_in):
                            add_link(v_out, v_in, p_out, p_in)


    for v in succs:  # find 1-layer links
        for v_succ in succs[v][1]:
            if v_succ in set_v1 or v_succ is t:
                print('direct link of {0}->{1}.'.format(v, v_succ))
                add_link(v, v_succ)

    pprint(links)

    get_v_layer(s, 3, 'out')
    get_v_layer(t, 3, 'in')
    for v in v1:
        get_v_layer(v, 3, 'in')
        get_v_layer(v, 3, 'out')

    pprint(succs)
    pprint(preds)


    for vi in preds:
        for vj in succs:
            find_links(vj, vi, 3, 2)

    pprint(links)

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
