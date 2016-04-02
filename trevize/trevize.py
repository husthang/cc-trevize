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
import time  # get benchmark data


def read_csv(g, stv1):
    """Read the two csv input files.

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
    process: DFS & check path when completed.
        - using deque for a first implementation
        - sort edges before adding to deque as a stack
    output: valid path list `valid_paths`
    """
    from collections import deque
    from pprint import pprint

    paths = deque()  # use as stack for DFS
    set_v1 = set(v1)
    dict_v1 = {}
    for i in v1:
        dict_v1[i] = True
    valid_paths = []

    # Y, N dict for edge pair DP
    # format: edge1: {(edge, '')}
    Y = {}
    N = {}

    # global wrong edges list
    N_global = {}  # format: {edge: ''}

    # found path for DP
    # vertex:value dict
    found_path = {}
    cycle = {}

    global num_paths, max_weight
    num_paths = 0
    BIG_WEIGHT = 4800
    max_weight = BIG_WEIGHT
    global i_searched  # num of paths searched
    i_searched = 0

    def init_N_list(G, N):
        """Initialize N list by iterating nodes."""
        for node in G.nodes_iter():
            in_degree = G.in_degree(node)
            if in_degree > 1:
                pre_node_list = G.predecessors(node)
                for i in range(len(pre_node_list)):
                    N[(pre_node_list[i], node)] = {}
                    for j in range(len(pre_node_list)):
                        if j is not i:
                            N[(pre_node_list[i], node)]\
                             [(pre_node_list[j], node)] = ""
            elif in_degree is 1:  # in_degree is 1
                pre_node = G.predecessors(node)[0]
                N[pre_node, node] = {}

    def dfs(G, paths):
        """DFS search algorithm.

        element in the stack: (path, weight)
            - path: list of vertices in the path
            - weight: DP for weight
            - Y_path: dict, edges must in the path
            - N_path: dict, edges mustn't in the path
        process:
            - init end vertex, end edge
            - next vertices:
                - outdegree == 0: dead end
                - outdegree == 1: add to Y_list
        """
        global num_paths, max_weight, i_searched

        path, weight, Y_path, N_path = paths.popleft()

        # end vertex and new edge
        end_vertex = path[-1]
        if end_vertex is not s:
            # must have >= 1 edge
            end_edge = (path[-2], path[-1])
            if end_edge not in Y:
                Y[end_edge] = {}
            if end_edge not in N:
                N[end_edge] = {}

        next_v_list = G[end_vertex]
        if len(next_v_list) is 0:  # dead end
            # del edge and end vertex
            G.remove_node(end_vertex)
            if len(path) >= 2:
                G.remove_edge((path[-2],path[-1]))
        elif len(next_v_list) is 1:
            # only one outdegree: add to Y-pair list
            # print(next_v_list)
            for key in next_v_list:
                next_v = key
            next_edge = (end_vertex, next_v)  # the only out-edge
            if next_edge in N_global:
            # this vertex will always lead to the wrong end, delete it.
                G.remove_node(end_vertex)
                return
            if end_vertex is not s:
                if next_edge in N[end_edge]:  # forming cycle
                    add_cycle(path + [end_vertex], cycle)
                    return
                if next_edge not in Y[end_edge]:
                    # end_edge -> next_edge
                    # merge N_edge
                    N[end_edge] = merge_dicts(N[end_edge], N[next_edge])
                    # adding edge to Y list (by extending inverse list)
                    Y[end_edge][next_edge] = ""
                    if next_edge in Y:
                        Y[end_edge] = merge_dicts(Y[end_edge], Y[next_edge])
                        for e in Y[next_edge]:
                        # path must extend to these edges
                            if e in N[end_edge]:
                            # end edge will always lead to wrong
                                print("del end edge in searching.")
                                G.remove_edge(end_edge)
                                N_global[end_edge] = ""
                                return
                            if e in N_path:
                            # will find wrong path
                                print("find wrong!")
                                return

        for next_v in sort_path(end_vertex, next_v_list):
            weight_1 = weight + G[end_vertex][next_v]['weight']
            if weight_1 < max_weight:  # weight overflow
                next_edge = (end_vertex, next_v)

                if next_edge in Y:  # check early cycle
                    for e in Y[next_edge]:
                        if (e in N_global) or (e in N_path):  # WA
                            continue

                if next_v is t:  # reach sink, finish
                    if check_path(path + [next_v]):
                        if verbose:
                            print(max_weight, weight_1,
                                  i_searched, time.time() - t0)
                        max_weight = weight_1
                        num_paths += 1
                        valid_paths.append(path + [next_v])
                    else:  # add DP information if not AC
                        continue
                        for i in range(len(path)):
                            if i not in cycle:
                                v = path[i]
                                num_v_in_v1 = num_ele_in_dict(path[i:], dict_v1)
                                if v in found_path:
                                    if num_v_in_v1 > found_path[v]:
                                        found_path[v] = num_v_in_v1
                                else:
                                    found_path[v] = num_v_in_v1

                elif next_v in path:  # next_v in the path: forming cycle
                    for vertex in path:
                        cycle[vertex] = True
                    cycle[next_v] = True

                else:  # new vertex in the path, add to stack
                    next_edge = (end_vertex, next_v)
                    # if next edge in N_global and N_path, continue
                    if next_edge in N_global:
                        continue
                    if next_edge in N_path:  # wrong edge: forming cycle
                        add_cycle(path + [next_v], cycle)
                        continue

                    # merge next edge to path Y/N dict
                    flag_Y = False
                    flag_N = False
                    if next_edge in Y:  # Y_path merge Y[next_edge]
                        flag_Y = True
                        Y_path1 = merge_dicts(Y_path, Y[next_edge])
                        for edge in Y[next_edge]:
                            N_path1 = merge_dicts(N_path, N[edge])
                    if next_edge in N:  # N_path merge
                        flag_N = True
                        N_path1 = merge_dicts(N_path, N[next_edge])

                    i_searched += 1  # add to stack
                    if flag_Y and flag_N:
                        paths.appendleft([path + [next_v], weight_1,
                                         Y_path1, N_path1])
                    elif flag_Y:
                        paths.appendleft([path + [next_v], weight_1,
                                         Y_path1, N_path1])
                    elif flag_N:
                        paths.appendleft([path + [next_v], weight_1,
                                         Y_path, N_path1])
                    else:
                        paths.appendleft([path + [next_v], weight_1,
                                         Y_path, N_path])


    def merge_dicts(x, y):
        """Given two dicts, merge them into a new dict as a shallow copy."""
        # z = x.copy()
        # z.update(y)
        # return z  u# for py < 3.5
        return {**x, **y}

    def check_path(path):
        """Check if path contains all vertices in v1."""
        if set_v1 <= set(path[1:-1]):
            # print('find path:', path)
            return True
        else:
            return False

    def sort_path(vertex, next_list, iter_depth=0):
        """Sort next vertex list by V' first, then by weight.

        iter_depth:
            - 0: iterating for one more layer and find best
            - 1: find weight
        """

        weight_list = {}
        if len(next_list) is 1:
            return list(next_list)

        for next_v in next_list:
            # generate value list: add large num to vertices not in v1
            LARGE_NUM = 20
            if next_v is t:
                weight_list[next_v] = 0
            if next_v in dict_v1:
                weight_list[next_v] = next_list[next_v]['weight']
                if iter_depth is 0:
                    nn_v_list = G[next_v]
                    if nn_v_list:
                        nn_weight = sort_path(next_v, nn_v_list, 1)[0]
                        # least weight from nn_v_list
                        weight_list[next_v] = (next_list[next_v]['weight'] +
                                               nn_weight)
                else:
                    weight_list[next_v] = (next_list[next_v]['weight'])
            else:
                if iter_depth is 0:
                    nn_v_list = G[next_v]
                    if nn_v_list:
                        # print(next_v, nn_v_list)
                        nn_weight = sort_path(next_v, nn_v_list, 1)[0]
                        weight_list[next_v] = (next_list[next_v]['weight'] +
                                               nn_weight + LARGE_NUM)
                else:
                    weight_list[next_v] = (next_list[next_v]['weight'] +
                                           LARGE_NUM)

        return sorted(weight_list, key=weight_list.get)

    def num_ele_in_dict(elements, d):
        """How many elements are in the dict."""
        num = 0
        for i in elements:
            if i in d:
                num += 1
        return num

    def add_cycle(path, d_add):
        """Add vertices in path dict d_add."""
        for v in path:
            d_add[v] = True
        return

    init_N_list(G, N)
    paths.appendleft([[s], 0, {}, {}])

    while paths:  # DFS
        dfs(G, paths)

    if verbose:  # verbose printout
        print("added route:", i_searched)
        print("num of paths: {}".format(num_paths))
        print("Y_edge_pair:")
        pprint(Y)
        print("N_edge_pair:")
        pprint(N)
        print("N_global:")
        pprint(N_global)
        pprint(valid_paths)
        print("found_path:")
        print(found_path)
        print("cycle:")
        pprint(cycle)

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
    global t0
    t0 = time.time()
    answer = trevize(G, s, t, v1, verbose)
    # print(answer)
    write_csv(o, answer, 'all', verbose)
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
