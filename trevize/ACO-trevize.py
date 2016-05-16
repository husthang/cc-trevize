# -*- coding: utf-8 -*-

"""Trevize: find path using ants algorithm

- input
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
    - o: output filename
- process: V'-based search
    - find links between vertice in V'
    - greedy search between these paths
    - merge into the path
- output
    - path

Notes:
    - as we use greedy search between paths, we cannot always find the path.
    (e.g. std case 3 is a counter example. while 1, 2, 4 can be easily solved.)
    - this algorithm can be fixed to
        - find more pairs first
        - use DFS or random search (instead of greedy based on in/out degrees)
    - however, as we are working in Phase II and integer programming is
    proposed as priority, V'-based search is not so important now.
"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependencies: Python 3, NetworkX

import networkx as nx
import time  # get benchmark data
import pants
from collections import deque


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


def trevize(G, s, t, v1, verbose, first_pairs=[]):
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
        return z  # for py < 3.5
        # return {**x, **y}

    def check_path(path):
        """Check if path con tains all vertices in v1."""
        if set_v1 <= set(path[1:-1]):
            if path not in valid_paths:
                return True
        else:
            return False

    def sort_path(vertex, next_list):
        """Sort next vertex list by V' first, then by weight."""
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
                weight_list[next_v] = (next_list[next_v]['weight'] + LARGE_NUM)
        return sorted(weight_list, key=weight_list.get)

    def find_least_pair(links, pair_to_find=False):
        """Find least merge pair."""
        # links: {link:(outdegree+indegree), outdegree, indegree, length, path}
        if pair_to_find:
            print(pair_to_find)
            path_pair = links_out[pair_to_find[0]][tuple(pair_to_find)][0]
            return path_pair
        least = (100, 100, 100, 600, 0)
        for v in path_set_sv1:
            if v not in links_out:  # a vertex is not in links_out
                print('no way for vertex:', v)
                print(path_set_sv1)
                pprint(links_out)
                return 0
        for v in path_set_sv1:
            for pair in links_out[v]:  # iterate all link in links_out
                v_in = pair[1]
                outdegree = sv1_outdegree[v]
                # print(links_out[v])
                # print(pair)
                indegree = v1t_indegree[v_in]
                length = len(links_out[v][pair][0])  # MVP: shortest path
                links[pair] = (outdegree+indegree, outdegree, indegree,
                               length, links_out[v][pair])
                # print(links[pair])
                # print(least)
                if outdegree < least[1] and indegree < least[2]:
                    least = links[pair]
                elif outdegree + indegree < least[0]:
                    least = links[pair]
                elif outdegree + indegree == least[0]:
                    if length < least[3]:  # least[length]
                        least = links[pair]
                elif ((least[1] > 1 and least[2] > 1) and
                        (outdegree is 1 or indegree is 1)):
                    # in and out > 1, but new one is (1,n)
                    least = links[pair]
        # pprint(links)
        # pprint(least)
        return least[4]  # all paths in pair

    def del_v1_pair(v, direction):
        """Delete v1 pair from out or in."""
        if direction is 'out':
            path_set_sv1.remove(v)
            del sv1_outdegree[v]
            if v in links_out:
                del links_out[v]
            for v_in in links_in:
                for key in links_in[v_in]:
                    if key[0] == v and key in links_in[v_in]:
                        del links_in[v_in][key]
                        break
            # update v_outdegree
        else:
            path_set_v1t.remove(v)
            del v1t_indegree[v]
            if v in links_in:
                del links_in[v]
            for v_out in links_out:
                for key in links_out[v_out]:
                    if key[1] == v and key in links_out[v_out]:
                        del links_out[v_out][key]
                        break

    def del_v1v2_pair(v_out, v_in):
        """Del v1v2 pair.

        if (2,3) is the new merged pair, then 3->2 is not valid."""
        if v_out in links_out:
            if (v_out, v_in) in links_out[v_out]:
                del links_out[v_out][(v_out, v_in)]
        if v_in in links_in:
            if (v_out, v_in) in links_in[v_in]:
                del links_in[v_in][(v_out, v_in)]

    def merge_path(final_path, new_link):
        """Merge paths."""
        global used
        start, end = new_link[0], new_link[-1]
        if len(new_link) > 2:  # add v not in v1 to used
            used += new_link[1:-1]
        # print(final_path, new_link)
        for i in range(len(final_path)):
            if final_path[i][-1] == start:
                del_start = final_path[i]
            if final_path[i][0] == end:
                del_end = final_path[i]
        new_ele = del_start + new_link[1:-1] + del_end
        final_path.remove(del_start)
        final_path.remove(del_end)
        final_path.append(new_ele)
        new_out = new_ele[-1]
        new_in = new_ele[0]
        # print(final_path)
        return final_path, new_out, new_in

    def one_move(final_path, first_pairs):
        if first_pairs:
            best_path = find_least_pair(links, first_pairs[0])
            del first_pairs[0]
        else:
            least_paths = find_least_pair(links)
            # print('least_paths', least_paths)
            if not least_paths:  # no paths
                return False
            for best_path in least_paths:
                if len(best_path) > 2:  # if new v not in V' is added to path
                    new_used_v = set(best_path[1:-1])
                    if verbose:
                        print('new added to v:', new_used_v)
                    for v in new_used_v:
                        del_v_between(v)
                    if not (new_used_v & set(used)):
                        break
                else:
                    break
            else:
                print(least_paths)
                first_pairs0.append([best_path[0], best_path[-1]])
                print(first_pairs0)
                print('No right path.\n')
                return False
        v_out, v_in = best_path[0], best_path[-1]
        if verbose:
            print("new merge:", best_path)
        final_path, new_out, new_in = merge_path(final_path, best_path)

        del_v1_pair(v_out, 'out')
        del_v1_pair(v_in, 'in')
        del_v1v2_pair(new_out, new_in)
        for v in path_set_sv1:
            if v in links_out:
                sv1_outdegree[v] = len(links_out[v])
        for v in path_set_v1t:
            if v in links_in:
                v1t_indegree[v] = len(links_in[v])
        if verbose:
            print(final_path)
            input('pause')
        return True

    def del_v_between(v):
        """Delete all paths contains certain vertex.

        Delete from links_out and links_in.

        links_out[v_out][(v_out, v_in)] = [new_link_path]
        """
        # pprint(links_out)
        # print('del v:', v)
        for v_out in links_out:
            for link in links_out[v_out]:
                for path in links_out[v_out][link]:
                    # iterate all available paths
                    if v in path:
                        # print(path, v)
                        links_out[v_out][link].remove(path)

        for v_in in links_in:
            for link in links_in[v_in]:
                for path in links_in[v_in][link]:
                    # iterate all available paths
                    if v in path:
                        # print(path, v)
                        links_in[v_in][link].remove(path)

        # pprint(links_out)
        clean_in_out()
        # pprint(links_out)

    def clean_in_out():
        """Clean links_in and links_out after delete some v."""
        for v in links_out:
            for link in list(links_out[v].keys()):
                # list(dict.keys()) to safely iterate dict and del keys
                if not links_out[v][link]:
                    del links_out[v][link]
        for v in list(links_out.keys()):
            if not links_out[v]:
                del links_out[v]
        for v in links_in:
            for link in list(links_in[v].keys()):
                if not links_in[v][link]:
                    del links_in[v][link]
        for v in list(links_in.keys()):
            if not links_in[v]:
                del links_in[v]


    def generate_seq(v1):
        """Generate sequence of V'."""
        import random
        v1_copy = v1[:]
        random.shuffle(v1_copy)
        return v1_copy

#     print(generate_seq(v1))
#     conten_path(v1)
#     print(generate_seq(v1))
#     print(generate_seq(v1))
#     print(generate_seq(v1))

    def conten_path(seq_v1):
        """Generate path(possibly with cycle)."""
        seq_path = [s] + seq_v1 + [t]
        seq_path2 = [s] + seq_v1 + [t]
        seq_v1 = []
        path = [s]
        weight = 0
        while len(seq_path) > 1:
            pair = (seq_path[0], seq_path[1])
            try:
                new_conten = links_out[seq_path[0]][pair][0][1:]
            except KeyError:
                new_conten = [-1, seq_path[1]]
                seq_v1.append([seq_path[0], -1, seq_path[1]])
                weight += 10000  # punish no edge
            else:
                seq_v1.append([seq_path[0]] + new_conten)
                weight += cal_path_weight([seq_path[0]] + new_conten)
            finally:
                path += new_conten
            del seq_path[0]
            # print(new_conten)
        # print(path)
        return path, seq_path2, weight

    def cal_v1_weight(seq_v1):
        """Generate path(possibly with cycle)."""
        weight = 0
        for i in range(len(seq_v1)-1):
            weight += distance_v1(seq_v1[i], seq_v1[i+1])
            # print(weight)
        # print(seq_v1, weight)
        return weight

    def cal_path_weight(path):
        """Calculate path weight.
            - input: path node list.
            - output: path weight.
        """
        weight = 0
        for i in range(len(path) - 1):
            # as used i+1
            weight += G[path[i]][path[i+1]]['weight']
        return weight

    def distance_v1(a, b):
        """Calculate distance of vi and vj."""
        if a == t:
            if b == s:
                return 0
            else:
                return 100000  # test for big
        elif a == s and b == t:
                return 100000
        else:
            try:
                new_conten = links_out[a][(a,b)][0][:]
            except KeyError:
                return 10000  # punish no path
            else:
                return cal_path_weight(new_conten)

    from pprint import pprint

    set_v1 = set(v1)
    set_sv1 = set([s] + v1)  # out
    set_v1t = set(v1 + [t])  # in
    valid_paths = []
    preds = {}  # two dict to store values
    succs = {}
    links = {}  # (vi, vj): [path1, path2, ...]
    links_out = {}
    links_in = {}
    for v in set_sv1:
        links_out[v] = {}
    for v in set_v1t:
        links_in[v] = {}

    global num_paths, max_weight
    num_paths = 0
    BIG_WEIGHT = 4800
    max_weight = BIG_WEIGHT
    global i_searched  # num of paths searched
    i_searched = 0
    print("s:", s, "t:", t, "V':", v1)  # printout for debugging

    INIT_DEPTH = 9  # init depth: 5 works for std case 1, 2, 4
    for v in set_sv1:
        depth = 1
        while (not links_out[v]) or (depth < INIT_DEPTH):
            if get_v_layer(v, depth, 'out'):
                depth += 1
            else:
                break
    for v in set_v1t:
        depth = 1
        while (not links_in[v]) or (depth < INIT_DEPTH):
            if get_v_layer(v, depth, 'in'):
                depth += 1
            else:
                break

    sv1_outdegree = {}
    v1t_indegree = {}
    for v in set_sv1:
        sv1_outdegree[v] = len(links_out[v])
        # print(v, 'out', len(links_out[v]))
    for v in set_v1t:
        v1t_indegree[v] = len(links_in[v])
        # print(v, 'in', len(links_in[v]))
    path_set_sv1 = set_sv1.copy()
    path_set_v1t = set_v1t.copy()

    global final_path
    final_path = [[s], [t]] + [[v] for v in v1]
    global used
    used = []
    global first_pairs0
    first_pairs0 = []
    for i in first_pairs:
        first_pairs0.append(i[:])

    pij = {}
    for v in set_sv1:
        for v2 in set_v1t:
            if v2 != v:
                pair = (v, v2)
                pij[(v,v2)] = distance_v1(v, v2)
    # pprint(pij)

    nodes = []
    node_dict = {}
    for v in [s,t] + v1:
        nodes.append(v)
        node_dict[len(nodes)] = v
    # print(node_dict)

    world = pants.World(nodes, distance_v1)
    solver = pants.Solver(start=s,
                          alpha=1,
                          beta=3,
                          limit=100,
                          links_out=links_out)
    solution = solver.solve(world)
    # solutions = solver.solutions(world)
    # print('distance: {}'.format(solution.distance))
    # print('tour: {}'.format(solution.tour))

    i = 0


    def local_DFS(seq, depth=6):
        """local DFS to reduce weight.

        1. generate sequence for local DFS.
        2. DFS to find least local weight."""
        for i in range(len(seq) - 1):
            if distance_v1(seq[i], seq[i+1]) >= 10000:
                # no path between
                # print(seq[i])
                begin = max(0, i-depth)
                end = min(i+depth, len(seq))
                # print('begin {}, end{}'.format(begin,end))
                # print(seq[begin:end])
                weight0 = cal_v1_weight(seq[begin:end])  # init weight for DFS
                DFS(seq[begin:end], weight0)

    def DFS(seq, begin_weight=1000000):
        print(seq, begin_weight)
        weight0 = begin_weight
        best_path = []
        stack = deque()
        stack.append([[seq[0]], 0])
        while stack:  # is not blank
            # print(stack)
            # input()
            t = stack.popleft()
            # print(t)
            path, weight = t[0], t[1]
            if weight >= weight0:
                continue
            if len(path) == len(seq):  # come to end
                print(weight0, weight)
                weight0 = weight
                best_path = path
            for i in seq:  # seq of random sequence
                if i not in path:
                    stack.appendleft((path+[i],
                                     weight + distance_v1(path[-1], i)))
        print(begin_weight, weight0)
        return best_path

    def MC_DFS(seq, begin_weight=1000000):
        print(seq, begin_weight)
        weight0 = begin_weight
        best_path = []
        stack = deque()
        stack.append([[seq[0]], 0])  # MC for first layer sequence
        t = stack.popleft()
        path, weight = t[0], t[1]
        if weight >= weight0:
            continue
        if len(path) == len(seq):  # come to end
            print(weight0, weight)
            weight0 = weight
            best_path = path
        num_coins = 500
        seq = MC_path(seq, num_coins)
        for i in seq:
            if i not in path:
                stack.appendleft((path+[i],
                                    weight + distance_v1(path[-1], i)))
        while stack:
            t = stack.pop()
            path, weight = t[0], t[1]
            if weight >= weight0:
                continue
            if len(path) == len(seq):  # come to end
                print(weight0, weight)
                weight0 = weight
                best_path = path
            for i in seq:
                if i not in path:
                    stack.append((path+[i],
                                  weight + distance_v1(path[-1], i)))
        print(begin_weight, weight0)
        return best_path

    def MC_path(seq, num_coins):
        """MC方法排序DFS, 启发式决定DFS栈中元素的先后顺序."""
        paths_for_MC = []  # path 列表, 注意控制总量
        weight_paths = []  # weight 列表
        for node1 in seq:
            if distance_v1(s, node1) <= 100000:
                paths_for_MC.append([s,node1])
                weight_paths.append(0)  # 初始化, 加入s附近的路径
        # print('len of MC ', len(paths_for_MC))
        coin_path = num_coins / len(paths_for_MC)  # 每个路径的MC次数
        for path in paths_for_MC:
            for i in range(int(coin_path)):
                try_path = generate_seq(list(set(v1)-set([node1])))
                weight = cal_v1_weight(path + try_path + [t])  # 加入pseudo-weight计算
                # should add cycle weight in later version
                # print(path+try_path +[t])
                # print(weight)
                weight_paths[paths_for_MC.index(path)] += weight / int(coin_path)
        for i in range(len(paths_for_MC)):
            print(paths_for_MC[i], weight_paths[i])
        sorted_paths = []
        sorted_index = sorted(weight_paths)
        for i in range(len(paths_for_MC)):
            sorted_paths.append(paths_for_MC[weight_paths.index(sorted_index[i])])
        # print(sorted_paths)
        return sorted_paths

    MC_path(v1, 5000)
    input()


    while True:
        i += 1
        seq_v1 = solution.tour
        j = seq_v1.index(s)
        seq_v1 = seq_v1[j:] + seq_v1[:j]
        print('seq of v1:', seq_v1)
        path, seq, weight = conten_path(seq_v1[1:-1])
        print(path, 'weight', weight)

        if path:
            if -1 not in path and len(path) == len(set(path)):
                print('valid path.')
                return path
            else:
                print('not valid')
                print(path)
                print(seq)
                print('gaps:', len(path)-len(set(path)))
                local_DFS(seq)
                input()


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

    global t0
    t0 = time.time()
    note = []
    path = trevize(G, s, t, v1, verbose, note)
    # print(path)
    # heuristic write_csv(o, (path, G), 'all', verbose)
    t1 = time.time()
    print("time: {}".format(t1-t0))

    return 0


if __name__ == '__main__':
    '''
    # demo pants start
    nodes = []
    for _ in range(5):
        x = random.uniform(-10, 10)
        y = random.uniform(-10, 10)
        nodes.append((x, y))
    s = (100, 100)
    t = (-100, -100)
    nodes.append(s)
    nodes.append(t)

    def euclidean(a, b):
        """Test for s-t path."""
        if a == t:
            if b == s:
                return 0
            else:
                return 10000  # test for big
        if a == s:
            if b == t:
                return 10000
        else:
            return math.sqrt(pow(a[1] - b[1], 2) + pow(a[0] - b[0], 2))

    world = pants.World(nodes, euclidean)
    solver = pants.Solver()
    solution = solver.solve(world)
    # solutions = solver.solutions(world)
    print('distance')
    print(solution.distance)
    print('tour')
    print(solution.tour)    # Nodes visited in order
    print('path')
    print(solution.path)    # Edges taken in order
    # or
    # best = float("inf")
    # for solution in solutions:
    #     assert solution.distance < best
    #     best = solution.distance


    # demo pants end
    '''
    main()

    # import cProfile
    # cProfile.run('main()', 'restats')

    # import pstats
    # p = pstats.Stats('restats')
    # p.sort_stats('cumulative').print_stats(20)
