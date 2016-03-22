# -*- coding: utf-8 -*-

"""Star chart: take a snapshot of the universe(graph).

- input
    - `python star_chart.py topo.csv demand.csv`
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
- output
    - all valid paths in csv format (weight, route)
    - shortest route (when `-s` is True)
"""


# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX, graphviz, pygraphviz


import networkx as nx
import pygraphviz as pgv


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


def star_chart(G, s, t, v1, plot_name):
    """Print input data."""
    from pprint import pprint

    nx.drawing.nx_agraph.write_dot(G, plot_name + 'dot')
    A = pgv.AGraph(plot_name + 'dot')

    # set graph, source and sink, edge labels
    A.graph_attr['label'] = str(s) + ' -> ' + str(t)

    # set nodes for source, sink and v1
    source = A.get_node(s)
    source.attr['color'] = 'blue'
    source.attr['fontcolor'] = 'blue'
    source.attr['shape'] = 'star'
    sink = A.get_node(t)
    sink.attr['color'] = 'red'
    sink.attr['fontcolor'] = 'red'
    sink.attr['shape'] = 'star'
    for node in v1:
        node = A.get_node(node)
        node.attr['color'] = 'orange'
        node.attr['fontcolor'] = 'orange'
        node.attr['shape'] = 'box'

    # set edge label as weight
    num_edges = 0
    for edge in A.edges_iter():
        edge.attr['label'] = edge.attr['weight']
        num_edges += 1

    # http://pygraphviz.github.io/documentation/pygraphviz-1.3rc1/reference/agraph.html#pygraphviz.AGraph.draw
    # dot graph is slow when the graph is big (>50 edges)
    if num_edges < 100:
        A.draw(plot_name, prog='dot', args='-splines=spline')
    elif num_edges < 500:
        A.draw(plot_name, prog='fdp')
    else:
        A.draw(plot_name, prog='neato')


    print('Star chart generated as {}.'.format(plot_name))


def main():
    """Parse arguments and main logic."""
    import argparse
    # argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('directory',
                        help='directory contains `topo.csv` and `demand.csv`.')
    args = parser.parse_args()
    topo = open(args.directory + '/topo.csv')
    demand = open(args.directory + '/demand.csv')

    G, s, t, v1 = read_csv(topo, demand)  # read file

    import os.path
    directory = os.path.dirname(args.directory)
    plot_name = os.path.join(directory, 'topo.png')
    star_chart(G, s, t, v1, plot_name)
    
    return 0


if __name__ == '__main__':
    main()
