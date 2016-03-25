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
    """Read the two csv input files.

    input: two csv input file
        detail of input format in http://codecraft.huawei.com/home/detail
        solve multigraph in input
    output: NetworkX-graph
    """
    import re
    num = re.compile('\d+')

    G = nx.DiGraph()
    for line in g:
        nums = re.findall(num, line)

        for i in range(4):
            nums[i] = int(nums[i])  # re str to int

        # solve multigraph: find best edge and apply
        try:
            exist_weight = G[nums[1]][nums[2]]['weight']
        except KeyError:
            G.add_edge(nums[1], nums[2], weight=nums[3], label=nums[0])
        else:
            if exist_weight > nums[3]:  # need update
                G[nums[1]][nums[2]]['weight'] = nums[3]
                G[nums[1]][nums[2]]['label'] = nums[0]

    # s, t, v1(may be blank)
    for line in stv1:
        stv1_list = re.findall(num, line)
        for i in range(len(stv1_list)):
            stv1_list[i] = int(stv1_list[i])
        s = stv1_list[0]
        t = stv1_list[1]
        v1 = stv1_list[2:]

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
    paths = deque()
    set_v1 = set(v1)
    global num_paths, max_weight
    valid_paths = []
    num_paths = 0
    BIG_WEIGHT = 4800
    max_weight = BIG_WEIGHT

    def dfs(G, paths):
        """DFS search algorithm."""
        global num_paths, max_weight

        path, weight = paths.popleft()
        end_vertex = path[-1]
        next_vertex_list = G[end_vertex]
        
        for vertex in sort_path(end_vertex, next_vertex_list):
            # print(vertex, G[end_vertex][vertex])
            weight_1 = weight + G[end_vertex][vertex]['weight']
            if weight_1 < max_weight:
                if vertex is t:  # got sink
                    if check_path(path + [vertex]):
                        print(max_weight, weight_1)
                        max_weight = weight_1
                        num_paths += 1
                        valid_paths.append(path + [vertex])
                elif vertex not in path:  # new vertex
                    paths.appendleft([path + [vertex], weight_1])
            # else: forming cycle or dead

    def check_path(path):
        """Check if path contains all vertices in v1."""
        if set_v1 <= set(path[1:-1]):
            # print('find path:', path)
            return True
        else:
            return False

    def sort_path(vertex, next_list):
        """Sort next vertex list by V' first, then by weight."""

        sorted_list = {}
        for next_vertex in next_list:
        # generate value list: add large num to vertices not in v1
            LARGE_NUM = 21
            if next_vertex in v1:
                sorted_list[next_vertex] = next_list[next_vertex]['weight']
            else:
                sorted_list[next_vertex] = LARGE_NUM + next_list[next_vertex]['weight']

        return sorted(sorted_list, key=sorted_list.get)

    paths.appendleft([[s], 0])
    while paths:
        dfs(G, paths)
    print("num of paths: {}".format(num_paths))

    from pprint import pprint
    pprint(valid_paths)
    if valid_paths:
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
    # o = open(args.o, 'w')
    verbose = args.verbose

    G, s, t, v1 = read_csv(topo, demand)

    import time 
    t0 = time.time()
    answer = trevize(G, s, t, v1, verbose)
    #print(answer)
    # write_csv(o, answer, format, verbose)
    t1 = time.time()
    print("time: {}".format(t1-t0))

    return 0

if __name__ == '__main__':
    main()
