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

    def add_link_to_links(p_out, p_in):
        """Add link to links_in, links_out."""
        v_out = p_out[0]
        v_in = p_in[-1]
        new_link_path = p_out + p_in
        if (v_out, v_in) not in links_in[v_in]:
            links_in[v_in][(v_out, v_in)] = [new_link_path]
            links_out[v_out][(v_out, v_in)] = [new_link_path]
        elif new_link_path not in links_in[v_in][(v_out, v_in)]:
            links_in[v_in][(v_out, v_in)] += [new_link_path]
            links_out[v_out][(v_out, v_in)] += [new_link_path]

    def iter_layer(v, depth, direction='in'):
        """Iterate one more layer of predecessors/successors BFS.

        add one more vertex to begin/end of paths."""
        if direction is 'in':
            if v not in preds:  # init first layer
                preds[v] = {}
                preds[v][1] = {}
                for v_pred in G.predecessors_iter(v):
                    if v_pred in set_sv1:
                        add_link_to_links([v_pred], [v])
                    else:
                        preds[v][1][v_pred] = [[v_pred, v]]
                return
            elif depth in preds[v]:  # if calculated, return
                return
            else:
                preds[v][depth] = {}  # else, init new layer
            for end_vertex in preds[v][depth - 1]:
                for end_path in preds[v][depth - 1][end_vertex]:
                    for v_pred in G.predecessors_iter(end_vertex):
                        if v_pred not in end_path:  # no cycle
                            if v_pred in set_sv1:  # link: add to links
                                add_link_to_links([v_pred], end_path)
                                continue
                            # not in v1, add to preds/succs for iteration
                            if v_pred not in preds[v][depth]:
                                preds[v][depth][v_pred] =\
                                    [[v_pred] + end_path]
                            else:
                                preds[v][depth][v_pred] +=\
                                    [[v_pred] + end_path]
        elif direction is 'out':  # out, successors
            if v not in succs:  # init first layer
                succs[v] = {}
                succs[v][1] = {}
                for v_succ in G.successors_iter(v):
                    if v_succ in set_v1t:
                        add_link_to_links([v], [v_succ])
                    else:
                        succs[v][1][v_succ] = [[v, v_succ]]
                return
            elif depth in succs[v]:  # if calculated, return
                return
            else:
                succs[v][depth] = {}  # else, init new layer
            for end_vertex in succs[v][depth - 1]:
                # from every end_vertex, add one layer
                for end_path in succs[v][depth - 1][end_vertex]:
                    for v_succ in G.successors_iter(end_vertex):
                        if v_succ not in end_path:
                            if v_succ in set_v1t:  # link: add to links
                                add_link_to_links(end_path, [v_succ])
                                continue
                            # not in v1, add to preds/succs for iteration
                            elif v_succ not in succs[v][depth]:
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
            return False
        for i_depth in range(depth, 0, -1):
            if v not in global_dict:
                iter_layer(v, 1, direction)
            if i_depth in global_dict[v]:
                layers_to_cal = list(range(i_depth + 1, depth + 1))
                for layer in layers_to_cal:
                    iter_layer(v, layer, direction)
                if global_dict[v][depth]:
                    return True
                else:
                    return False

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

    from collections import deque
    from pprint import pprint
    import time

    set_v1 = set(v1)
    set_sv1 = set([s] + v1)  # out
    set_v1t = set(v1 + [t])  # in
    valid_paths = []
    preds = {}
    succs = {}
    links = {}  # (vi, vj): [path1, path2, ...]
    links_out = {}
    links_in = {}
    for v in set_sv1:
        links_out[v] = {}
    for v in set_v1t:
        links_in[v] = {}

    wrong_set = set([])  # DP all wrong paths

    global num_paths, max_weight
    num_paths = 0
    BIG_WEIGHT = 4800
    max_weight = BIG_WEIGHT
    global i_searched  # num of paths searched
    i_searched = 0
    print("s:", s, "t:", t, "V':", v1)

    #init_globals(s, t, v1)
    #pprint(succs[3])
    INIT_DEPTH = 5
    for v in set_sv1:
        depth = 1
        while (not links_out[v]) or (depth < INIT_DEPTH):
            if get_v_layer(v, depth, 'out'):
                depth += 1
            else:
                # print('no way!')
                break
    for v in set_v1t:
        depth = 1
        while (not links_in[v]) or (depth < INIT_DEPTH):
            if get_v_layer(v, depth, 'in'):
                depth += 1
            else:
                # print('no way!')
                break
            
    # pprint(succs[3])
    # pprint(preds[3])
    # pprint(links)

    sv1_outdegree = {}
    v1t_indegree = {}
    for v in set_sv1:
        sv1_outdegree[v] = len(links_out[v])
        # print(v, 'out', len(links_out[v]))
    for v in set_v1t:
        v1t_indegree[v] = len(links_in[v])
        # print(v, 'in', len(links_in[v]))
    # print(sv1_outdegree)
    # print(v1t_indegree)
    # print(links)
    path_set_sv1 = set_sv1.copy()
    path_set_v1t = set_v1t.copy()
    # pprint(links_out)
    # pprint(links_in)
    global final_path
    final_path = []
    wrong_pair = {}

    def find_least_pair(links):
        """Find least merge pair."""
        # links: {link: (outdegree + indegree), outdegree, indegree, length, path}
        least = (100, 100, 100, 600, 0)
        for v in path_set_sv1:
            for pair in links_out[v]:
                v_in = pair[1]
                outdegree = sv1_outdegree[v]
                indegree = v1t_indegree[v_in]
                length = len(links_out[v][pair][0])  # MVP: shortest path
                links[pair] = (outdegree+indegree, outdegree, indegree, length, links_out[v][pair])
                # print(links[pair])
                # print(least)
                if outdegree <= least[1] and indegree <= least[2]:
                    least = links[pair]
                elif outdegree + indegree < least[0]:
                    least = links[pair]
                elif outdegree + indegree == least[0]:
                    if length < least[3]:  # least[length]
                        least = links[pair]
                elif (least[1] > 1 and least[2] > 1) and (outdegree is 1 or indegree is 1):  
                    # in and out > 1, but new one is (1,n)
                    least = links[pair]
        # pprint(links)
        # pprint(least)
        return least[4]  # all paths in pair

    def del_v1_pair(v, direction):
        """Del v1 pair from out or in."""
        if direction is 'out':
            path_set_sv1.remove(v)
            del links_out[v]
            for v_in in links_in:
                for key in links_in[v_in]:
                    if key[0] is v:
                        del links_in[v_in][key]
                        break
            # update v_outdegree
        else:
            path_set_v1t.remove(v)
            del links_in[v]
            for v_out in links_out:
                for key in links_out[v_out]:
                    if key[1] is v:
                        del links_out[v_out][key]
                        break

    def del_v1v2_pair(v_out, v_in):
        """Del v1v2 pair.

        if (2,3) is a pair, then 3->2 is not valid."""
        if v_in in links_out:
            if (v_in, v_out) in links_out[v_in]:
                del links_out[v_in][(v_in, v_out)]

    def merge_path(final_path, new_link):
        """Merge paths."""
        start, end = new_link[0], new_link[-1]
        for i in range(len(final_path)):
            if final_path[i][-1] == start:
                final_path[i] += new_link
            if final_path[i][0] == :
                final_path[i] = new_link + final_path[i]
        return final_path


    def one_move(final_path):
        least_paths = find_least_pair(links)
        best_path = least_paths[0]
        v_out, v_in = best_path[0], best_path[-1]
        print(best_path)
        final_path = merge_path(final_path, best_path)
        # final_path.append(best_path)
        del_v1_pair(v_out, 'out')
        del_v1_pair(v_in, 'in')
        del_v1v2_pair(v_out, v_in)
        print(final_path)

    while path_set_sv1:
        one_move(final_path)
        # pprint(links_out)
        # pprint(links_in)
        print(path_set_sv1)
    # pprint(preds)
    # pprint(succs)
    return



    def count_links():
        """Count link in links.

        links = {(vi, vj): [path1, path2, ...]}"""

    def check_path_between(v1_n, n_v2):
        """Check if path  of v1->n and n->v2 can be merged.

        input: v1_n, n_v2: list of path.
        output: return True if the path is valid.
        """
        if set(v1_n[1:] + n_v2[:-1]) & set_v1:  # i.e. v1 in path of v1->v2
            return False
        else:
            if set(v1_n) & set(n_v2) == set([n_v2[0]]):
                # intersection is only n
                return True
            else:
                return False

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
        return [p_out + p_in]

    def find_links(v_out, v_in, depth_out, depth_in):
        """Find demanded links of (vi, vj)."""
        if (v_out, v_in) in links:
            return links[(v_out, v_in)]
        if depth_out is 1 and depth_in is 1:
            # print(v_out, v_in)
            if v_in in succs[v_out][depth_out]:
                return add_link(v_out, v_in)
        if not (succs[v_out][depth_out] or preds[v_in][depth_in]):
            # print(succs[v_out], depth_out)
            # print(preds[v_in], depth_in)
            return 'X'
        for v_succ in succs[v_out][depth_out]:
            if v_succ in preds[v_in][depth_in]:
                paths_out = succs[v_out][depth_out][v_succ]
                paths_in = preds[v_in][depth_in][v_succ]
                for p_out in paths_out:
                    for p_in in paths_in:
                        if check_path_between(p_out, p_in):
                            return add_link(v_out, v_in, p_out, p_in)
        return []

    # # init 1-layer of links
    # for v in succs:  # find 1-layer links
    #     for v_succ in succs[v][1]:
    #         if v_succ in set_v1 or v_succ is t:
    #             # print('direct link of {0}->{1}.'.format(v, v_succ))
    #             add_link(v, v_succ)

    stack = deque()
    ll_v1 = [[v] for v in v1]
    path_begin = [[s], [t]] + ll_v1, set([])
    stack.appendleft(path_begin)
    # print(stack)
    # print(path_begin)

    def v1_merge_DFS(pack_path):
        """DFS algorithm to merge links between v1."""
        path, used = pack_path
        # print(path, used)
        tup_path = []
        for i in path:
            tup_path += [tuple(i)]
        tup_path = tuple(tup_path)
        if tup_path in wrong_set:
            # print('Wrong set: ', tup_path, wrong_set)
            return
        # print(tup_path)
        if len(path) is 1:  # find path
            print()
            print('find path.')
            print(path[0])
            print()
            return True

        def path_find_link(path_link, list_v_out, list_v_in, 
                           depth_out, depth_in):
            """Find link for path.

            Bugs may happen when no valid path."""
            while not path_link:
                for v in list_v_out:
                    get_v_layer(v, depth_out, 'out')
                for v in list_v_in:
                    get_v_layer(v, depth_in, 'in')
                for vi in list_v_out:
                    for vj in list_v_in:
                        # print(vi, vj, depth_out, depth_in)
                        if vi is not vj:
                            path_ij = find_links(vi, vj, depth_out, depth_in)
                            if path_ij is 'X':
                                return False
                            if path_ij:
                                path_link[(vi, vj)] = path_ij
                if depth_out == depth_in:
                    depth_out += 1
                else:
                    depth_in += 1
                # print('depth_out:{}, depth_in:{}.'.format(depth_out, depth_in))
                # print('path: link')
                # pprint(path_link)
                # input(s)
            return path_link

        # add link of first layer
        path_link = {}
        list_v_out = [v[-1] for v in path if v[-1] is not t]
        list_v_in = [v[0] for v in path if v[0] is not s]
        # print('list out', list_v_out)
        # print('list in', list_v_in)
        flag = path_find_link(path_link, list_v_out, list_v_in, 1, 1)
        # for link in links: init path_link
        #     if link[0] in list_v_out and link[1] in list_v_in:
        #         path_link[link] = links[link]
        if flag is False:
            # print('False')
            wrong_set.add(tup_path)


        # try to link
        for link in path_link:
            # print(link, path_link[link])
            for link_path in path_link[link]:
                vertices_to_add = set(link_path[1:-1])
                link_out, link_in = link[0], link[-1]
                if link_out in path[0] and link_in in path[1]:
                    if len(path) > 2:  # early s-t path without some of V1, invalid
                        continue
                if not(used & vertices_to_add):  # valid link
                    path_new = update_path(path, link_path)
                    if path_new:
                        stack.appendleft((path_new, used | vertices_to_add))
                    # return


            # finish iteration, add path to stack

    def update_path(path, link):
        """Merge path with link."""
        link_out, link_in = link[0], link[-1]
        # print(path, link)
        path_new = path[:]
        group_out, group_in = 0, 0
        for group in path_new:  # (s, link_out)
            if link_out in group:
                group_out = group
            if link_in in group:
                group_in = group
        # print(group_in, group_out)  # test
        if group_in == group_out:
            return False
        if link_in not in path[1]:
            path_new.remove(group_in)
            path_new[path_new.index(group_out)] = (group_out +
                                                   link[1:] + group_in[1:])
        else:  # change path[1] to not disturb t
            path_new.remove(group_out)
            # print(path_new)
            # print(group_out, link[1:], group_in[1:])
            if len(path_new) is 1: 
                path_new[0] = (group_out + link[1:] + group_in[1:])
            else:
                path_new[1] = (group_out + link[1:] + group_in[1:])
        return path_new

    i_searched = 0 
    while True:
        flag = v1_merge_DFS(stack.popleft())
        
        # pprint(stack[0])
        if flag:
            break
        i_searched += 1
        if i_searched % 10000 == 0:
            print('len of stack: {}'.format(len(stack)))
            # pprint(stack[0])
            # print(len(wrong_set))
            # input(s)
    print(i_searched)

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
    # write_csv(o, answer, 'all', verbose)
    t1 = time.time()
    print("time: {}".format(t1-t0))

    return 0


if __name__ == '__main__':
    main()
