# -*- coding: utf-8 -*-
# py-trevize
# Related to Trevize project of Organization Laboratory
# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Enviroment: Python 3, NetworkX

import networkx as nx
from sys import argv


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


def check_input(G, s, t, v1):
    """Print input data."""
    from pprint import pprint
    import matplotlib.pyplot as plt

    print('nodes:', G.nodes())
    print('edges:')
    pprint(G.edges(data=True))

    # draw graph and edge labels in same layout
    layout=nx.circular_layout(G)
    nx.draw_networkx(G,pos=layout)
    nx.draw_networkx_edge_labels(G, pos=layout, label_pos=0.5, font_size=8)

    print('s: {}, t: {}'.format(s, t))
    print('v1', v1)

    plt.show()


def trevize(G, s, t, v1, verbose=False):
    """Find best path.

    input: graph, s, t, v1
    output: best path
    """
    all_paths = list(nx.shortest_simple_paths(G, s, t, weight='weight'))
    set_v1 = set(v1)
    for path in all_paths:
        if verbose:
            print(path)
        if set_v1 <= set(path[1:-1]):
            # use set to find if all elements in v' are in path
            if verbose:
                print('find path')
            return path, G
    if verbose:
        print('no path')
    return "NA"


def write_csv(f, answer, verbose=False):
    """Write the answer to csv file.

    proposed format: 
        - 'NA' for no answer; 
        - 'e[1]|e[2]|..|e[n]' for shortest path, e[i] is label of edge
    """
    if answer is "NA":
        print(answer)
        f.write(answer)
        return 0
    # else
    path, G = answer
    answer = ""
    for i in range(len(path) - 1):
        # as used i+1
        if verbose:
            print(G[path[i]][path[i+1]]['label'])
        answer += str(G.edge[path[i]][path[i+1]]['label']) + '|'
    print(answer[:-1])
    f.write(answer[:-1])
    return 0


def main():
    # file i/o:
    # g.csv: graph data; stv1.csv: path data
    # o.csv: file to write
    script, g, stv1, o = argv
    g = open(g)
    stv1 = open(stv1)
    o = open(o, 'w')

    G, s, t, v1 = read_csv(g,stv1)
    # check_input(G, s, t, v1)
    # set verbose to True to get more output infomation
    answer = trevize(G, s, t, v1, verbose=False)
    write_csv(o, answer, verbose=False)


if __name__ == '__main__':
    main()