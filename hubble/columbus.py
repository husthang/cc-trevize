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
"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX

import networkx as nx
import json

def read_csv(g, stv1, path):
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
    for line in stv1:
        stv1_list = re.findall(num, line)
        for i in range(len(stv1_list)):
            stv1_list[i] = int(stv1_list[i])
        s = stv1_list[0]
        t = stv1_list[1]
        v1 = stv1_list[2:]

    # parse answer to list of edges
    for line in path:
        answer = re.findall(num, line)
    for i in range(len(answer)):
        answer[i] = int(answer[i])

    print("s {}, t {}, v1 {}, answer_edges {}".format(s, t, v1, answer))
    return G, s, t, v1, answer


def columbus(G, s, t, v1, answer):
    """Check answer path.
    input: list of edges.
    process:
        - find all vertices in the path
        - check duplicate vertices
        - check all vertices in V'
    output: printout and return string"""
    vertices_in_path = [s]
    weight_in_path = 0
    for edge_label in answer:
        # print('edge label,', edge_label)
        v_out = vertices_in_path[-1]
        out_edges = G[v_out]
        # print(out_edges)
        for v_in in out_edges:
            # print(v_in, out_edges[v_in])
            for edge_num in out_edges[v_in]:
                if out_edges[v_in][edge_num]['label'] is edge_label:
                    # print('equal', out_edges[v_in][edge_num], edge_label)
                    weight_in_path += out_edges[v_in][edge_num]['weight']
                    vertices_in_path.append(v_in)
                    # print(vertices_in_path)
    else:  # finish input: successful got to t (sink)
        if len(set(vertices_in_path)) is not len(vertices_in_path):
            # check duplicates
            print("WA, duplicate vertices.")
            return_str = json.dumps(["WA", "duplicate vertices."])
        else:
            print("length:", len(set(vertices_in_path)), len(vertices_in_path))
        if set(v1) <= set(vertices_in_path):
            # check v in v1
            print("vertices in V' is all included.")
            return_str = json.dumps(["AC", weight_in_path])
        else:
            return_str = json.dumps(["WA", "V' not included."])
    # print(return_str)
    return return_str


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
    args = parser.parse_args()
    topo = open(args.topo)
    demand = open(args.demand)
    answer = open(args.answer)
    G, s, t, v1, answer = read_csv(topo, demand, answer)  # read file

    result = json.loads(columbus(G, s, t, v1, answer))
    if result[0] == "AC":
        print("AC, weight:", result[1])
    else:
        print(result[0], result[1])

    return result


if __name__ == '__main__':
    main()
