# -*- coding: utf-8 -*-

"""Star chart: take a snapshot of the universe(graph).

Star chart 对题目中的图进行可视化.
- input
    - `python star_chart.py topo.csv demand.csv [-a answer.csv]`
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
- output
    - `topo.png` in same directory
        - weight of edges labeled
        - source: blue star
        - sink: red star
        - vertrices in V': orange box
        - [if -a/--answer,] path: violet
"""


# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX, graphviz, pygraphviz


import networkx as nx
import pygraphviz as pgv
import os


def read_csv(g, stv1, path):
    """Read the csv input files.

    input: two csv input file
        detail of input format in http://codecraft.huawei.com/home/detail
        solve multigraph in input
    output: NetworkX-graph & path info
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

    if path:  # provided answer list
        answer = re.findall(num, path.read())
    else:
        answer = False

    return G, s, t, v1, answer


def star_chart(G, s, t, v1, answer, plot_name):
    """Print input data."""

    nx.drawing.nx_agraph.write_dot(G, plot_name + 'dot')
    A = pgv.AGraph(plot_name + 'dot')
    os.remove(plot_name + 'dot')

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
        if answer:
            if edge.attr['label'] in answer:
                edge.attr['color'] = 'violet'
        edge.attr['label'] = edge.attr['weight']
        num_edges += 1

    # http://pygraphviz.github.io/documentation/pygraphviz-1.3rc1/reference/agraph.html#pygraphviz.AGraph.draw
    # http://www.graphviz.org/doc/info/attrs.html
    # dot graph is slow when the graph is big (>50 edges)
    if num_edges < 100:
        A.draw(plot_name, prog='dot', args='-splines=spline')
    elif num_edges < 500:
        for edge in A.edges_iter():
            edge.attr['penwidth'] = '0.5'
            edge.attr['arrowsize'] = '0.5'
        A.draw(plot_name, prog='fdp')
    else:
        for edge in A.edges_iter():
            edge.attr['penwidth'] = '0.5'
            edge.attr['arrowsize'] = '0.5'
        A.draw(plot_name, prog='twopi')

    print('Star chart generated as {}.'.format(plot_name))


def main():
    """Parse arguments and main logic."""
    import argparse
    # argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('topo',
                        help='`topo.csv`, contains graph')
    parser.add_argument('demand',
                        help='`demand.csv`, contains s, t, v1.')
    parser.add_argument('-a', '--answer', nargs='?',
                        help='`answer.csv`, contains path')
    args = parser.parse_args()
    topo = open(args.topo)
    demand = open(args.demand)
    if args.answer:
        answer = open(args.answer)
    else:
        answer = False
    G, s, t, v1, answer = read_csv(topo, demand, answer)  # read file

    # generate output filename and plot
    directory = os.path.dirname(args.topo)
    plot_name = os.path.join(directory, 'topo.png')
    star_chart(G, s, t, v1, answer, plot_name)

    return 0


if __name__ == '__main__':
    main()
