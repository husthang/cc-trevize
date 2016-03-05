# -*- coding: utf-8 -*-
# py-trevize
# Related to Trevize project of Organization Laboratory
# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Enviroment: Python 3, NetworkX

import networkx as nx
import matplotlib.pyplot as plt

g = open('../test/g.csv')
stv1 = open('../test/stv1.csv')
o = open('../test/o.csv', 'w')

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
    print('nodes:', G.nodes())
    from pprint import pprint
    print('edges:')
    pprint(G.edges(data=True))

    layout=nx.circular_layout(G)
    nx.draw_networkx(G,pos=layout)
    nx.draw_networkx_edge_labels(G, pos=layout, label_pos=0.5, font_size=8)

    print('s: {}, t: {}'.format(s, t))
    print('v1', v1)

    plt.show()


def trevize(G, s, t, v1):
    """Find best path.

    input: graph, s, t, v1
    output: best path
    """
    all_paths = list(nx.shortest_simple_paths(G, s, t))
    set_v1 = set(v1)
    for path in all_paths:
        print(path)
        if set_v1 <= set(path[1:-1]):
            print('find path')
            return path, G
    print('no path')
    return "NA"


def write_csv(f, answer):
    """Write the answer to csv file.

    format: 
        - "NA" for no answer; 
        - e[1]|e[2]|..|e[n] for shortest path, e[i] is label of edge
    """
    if answer is "NA":
        print(answer)
        f.write(answer)
        return 0
    # else
    path, G = answer
    print(G.edges())
    answer = ""
    for i in range(len(path) - 1):
        # as used i+1
        print(G[path[i]][path[i+1]]['label'])
        answer += str(G.edge[path[i]][path[i+1]]['label']) + '|'
    print(answer[:-1])
    f.write(answer[:-1])
    return 0


def main():
    G, s, t, v1 = read_csv(g,stv1)
    #check_input(G, s, t, v1)
    answer = trevize(G, s, t, v1)
    write_csv(o, answer)


if __name__ == '__main__':
    main()