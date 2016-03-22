# -*- coding: utf-8 -*-

"""Voyager: find all possible routes in the universe(graph).

- input
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
    - o: output filename
    - '-s', '--shortest': only output shortest route
    - '-v', '--verbose': verbose printout
    - '-p', '--plot': topo figure printout `topo.png`
- output
    - all valid paths in csv format (weight, route)
    - shortest route (when `-s` is True)
"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX, matplotlib(for plot)

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


def check_input_and_plot(G, s, t, v1, plot_name, verbose):
    """Print input data."""
    from pprint import pprint
    import pygraphviz as pgv
    # import matplotlib.pyplot as plt

    if verbose:
        print('nodes:', G.nodes())
        print('edges:')
        pprint(G.edges(data=True))


    nx.drawing.nx_agraph.write_dot(G, plot_name + 'dot')
    A = pgv.AGraph(plot_name+ 'dot')

    # set graph, source and sink, edge labels
    A.graph_attr['label'] = str(s) + ' -> ' + str(t)

    # source: blue, sink: red
    source = A.get_node(s)
    source.attr['color'] = 'blue'
    sink = A.get_node(t)
    sink.attr['color'] = 'red'

    for edge in A.edges_iter():
        edge.attr['label'] = edge.attr['weight']
    # print(A.string())
    print(plot_name)


    A.draw(plot_name, prog='dot', args='-splines=spline')


    # # draw graph and edge labels in same layout
    # layout = nx.circular_layout(G)
    # nx.draw_networkx(G, pos=layout)
    # nx.draw_networkx_edge_labels(G, pos=layout, label_pos=0.5, font_size=8)

    # if verbose:
    #     print('s: {}, t: {}'.format(s, t))
    #     print('v1', v1)

    # plt.savefig(plot_name)


def voyager(G, s, t, v1, verbose):
    """Find best path.

    input: graph, s, t, v1
    output: valid path list `valid_paths`
    """
    set_v1 = set(v1)
    valid_paths = []
    num_paths = 0
    try:
        for path in nx.all_simple_paths(G, s, t):
            if verbose:
                print(path)
            if set_v1 <= set(path[1:-1]):
                # use set to find if all elements in V' are in path
                if verbose:
                    print(path)
                num_paths += 1
                valid_paths.append(path)
                if num_paths > 10:
                    break
    except nx.NetworkXNoPath:
        if verbose:
            print('no path')
        return "NA"
    # return
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
    parser.add_argument('-s', '--shortest',
                        help='only output shortest route',
                        action="store_true")
    parser.add_argument('-p', '--plot',
                        help='topo figure printout',
                        action="store_true")
    parser.add_argument('-v', '--verbose',
                        help='verbose printout',
                        action="store_true")
    args = parser.parse_args()
    topo = open(args.topo)
    demand = open(args.demand)
    o = open(args.o, 'w')
    verbose = args.verbose
    output_plot = args.plot
    if args.shortest is True:
        format = 'shortest'
    else:
        format = 'all'

    G, s, t, v1 = read_csv(topo, demand)
    if output_plot:
        import os.path
        directory = os.path.dirname(args.topo)
        plot_name = os.path.join(directory, 'topo.png')
        check_input_and_plot(G, s, t, v1, plot_name, verbose)
        return 0
    answer = voyager(G, s, t, v1, verbose)
    write_csv(o, answer, format, verbose)
    return 0

if __name__ == '__main__':
    main()
