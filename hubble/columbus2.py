# -*- coding: utf-8 -*-

"""Columbus: answer checker for Trevize.

Columbus: 检查结果是否正确. 给出适当的输出
- input
    - `python columbus.py topo.csv demand.csv answer.csv`
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
    - answer: `answer.csv`, contains answer
- process
    - check answer (only for cases that has valid answer?)
    - check answer set (duplicate of vertex in path)
    - check vertices in V'
    - check path start and end
- output
    - print to stdout
    - return answer json string (as output module in test framework Gaia)
        - format: "[AC/WA, length/WA reason]"
"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX

import networkx as nx
import json

def read_csv(g, stv1, path, verbose):
    """Read the csv input files.

    input: two csv input file
        detail of input format in http://codecraft.huawei.com/home/detail
        solve multigraph in input
    output: NetworkX-graph & path info
    """
    import re
    num = re.compile('\d+')

    # process G
    G = nx.MultiDiGraph()
    for line in g:
        nums = re.findall(num, line)
        for i in range(4):
            nums[i] = int(nums[i])  # re str to int
        G.add_edge(nums[1], nums[2], weight=nums[3], label=nums[0])

    # s, t, v1(may be blank)
    line = stv1.readline()
    stv1_list = re.findall(num, line)
    for i in range(len(stv1_list)):
        stv1_list[i] = int(stv1_list[i])
    v1 = stv1_list[3:]
    line = stv1.readline()
    stv1_list = re.findall(num, line)
    for i in range(len(stv1_list)):
        stv1_list[i] = int(stv1_list[i])
    s = stv1_list[1]
    t = stv1_list[2]
    v2 = stv1_list[3:]

    # parse answer to list of edges
    line = path.readline()
    answer1 = re.findall(num, line)
    for i in range(len(answer1)):
        answer1[i] = int(answer1[i])
    line = path.readline()
    answer2 = re.findall(num, line)
    for i in range(len(answer2)):
        answer2[i] = int(answer2[i])

    if verbose:
        print("s {}, t {}, v1 {}, v2 {}\nanswer_edges1 {}\n2 {}"\
              .format(s, t, v1, v2, answer1, answer2))
    return G, s, t, v1, v2, answer1, answer2


def columbus(G, s, t, v1, v2, answer1, answer2, verbose):
    """Check answer path.
    input: list of edges.
    process:
        - find all vertices in the path
        - check duplicate vertices
        - check all vertices in V'
    output: printout and return string"""
    vertices_in_path = [s]
    weight_in_path = 0
    # print(answer)
    for edge_label in answer1:  # process path1
        # print('edge label,', edge_label)
        v_out = vertices_in_path[-1]
        out_edges = G[v_out]
        # print(out_edges)
        for v_in in out_edges:
            # print(v_in, out_edges[v_in])
            for edge_num in out_edges[v_in]:
                if out_edges[v_in][edge_num]['label'] == edge_label:
                    # print('equal', out_edges[v_in][edge_num], edge_label)
                    weight_in_path += out_edges[v_in][edge_num]['weight']
                    vertices_in_path.append(v_in)
                    # print(vertices_in_path)
    else:  # finish input: successful got to t (sink)
        # print(vertices_in_path)
        if len(set(vertices_in_path)) != len(vertices_in_path):
            # check duplicates
            if verbose:
                print("WA, duplicate vertices.")
            return_list = ["WA", "duplicate vertices p1."]
        if set(v1) <= set(vertices_in_path):
            # check v in v1
            if verbose:
                print("vertices in V' is all included.")
            return_list = ["AC", weight_in_path]
            weight1 = weight_in_path
        else:
            return_list = ["WA", "V' not included."]

    vertices_in_path = [s]
    weight_in_path = 0
    for edge_label in answer2:  # process path2
        # print('edge label,', edge_label)
        v_out = vertices_in_path[-1]
        out_edges = G[v_out]
        # print(out_edges)
        for v_in in out_edges:
            # print(v_in, out_edges[v_in])
            for edge_num in out_edges[v_in]:
                if out_edges[v_in][edge_num]['label'] == edge_label:
                    # print('equal', out_edges[v_in][edge_num], edge_label)
                    weight_in_path += out_edges[v_in][edge_num]['weight']
                    vertices_in_path.append(v_in)
                    # print(vertices_in_path)
    else:  # finish input: successful got to t (sink)
        # print(vertices_in_path)
        if len(set(vertices_in_path)) != len(vertices_in_path):
            # check duplicates
            if verbose:
                print("WA, duplicate vertices.")
            if return_list[0] == 'AC':  # AC for path1, WA for path2
                return_list[0] = 'WA'
            return_list.append("duplicate vertices p2.")
        if set(v2) <= set(vertices_in_path):
            # check v in v2
            if verbose:
                print("vertices in V'' is all included.")
            if return_list[0] == 'AC':  # AC for path1/2
                weight2 = weight_in_path
                return_list.append(weight1 + weight2)
                return_list[1] = len(set(answer1) & set(answer2))
        else:
            if return_list[0] == 'AC':  # AC for path1, WA for path2
                return_list[0] = 'WA'
            return_list.append("V'' not included.")
    # sum_weight = weight1 + weight2
    dup_edges = set(answer1) & set(answer2)

    # output
    if verbose:
        print(sum_weight)
        print(len(dup_edges), dup_edges)
    # print(return_list)
    return json.dumps(return_list)


def main():
    """Parse arguments and main logic."""
    import argparse
    # argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('topo',
                        help='`topo.csv`, contains graph')
    parser.add_argument('demand',
                        help='`demand.csv`, contains s, t, v1.')
    parser.add_argument('answer',
                        help='`answer.csv`, contains path')
    parser.add_argument('-v', '--verbose',
                        help='verbose printout',
                        action="store_true")

    args = parser.parse_args()
    topo = open(args.topo)
    demand = open(args.demand)
    answer = open(args.answer)
    verbose = args.verbose
    # read file
    G, s, t, v1, v2, answer1, answer2 = read_csv(topo, demand, answer, verbose)

    result = columbus(G, s, t, v1, v2, answer1, answer2, verbose)
    print(result)

    return 0


if __name__ == '__main__':
    main()
