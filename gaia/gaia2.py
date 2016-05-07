# -*- coding: utf-8 -*-

"""Gaia: the test framework for Trevize

`$python gaia.py cases algorithm1 [algorithm2-n]`

- input:
    - test cases: a directory of test cases
        - a separated directory (`topo.csv` and `demand.csv`) for each case.
        - a ref.csv file for calculating score [not implemented]
    - algorithms: at least one
        - should meet run format of `$algorithm topo.csv demand.csv out.csv`
            - Python format available as
              `python algorithm topo.csv demand.csv out.csv`
            - duplicates of same algorithm is removed
        - note: algorithm may be single algorithm,
                or hybrid of different algorithms.
- process:
    - for each case, test every algorithm
- output:
    - make a directory of output
        - `test/Gaia_timestamp/`
    - tree: begin at `Gaia_timestamp/`
        - `parameters.txt`
        - `log.txt`
        - `/algorithm_x/case_y.csv`
    - features of the test case [not implemented]
    - result of algorithms
        - result: AC/WA
        - detail: dup_edges, weight/WA reason

"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependency: Python 3


def parse_input():
    """Parse arguments."""

    import argparse
    # argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('cases',
                        help='directory of cases for running Gaia')
    parser.add_argument('algorithms', nargs='*',
                        help='n algorithms (Python format in quote)' +
                             'for running Gaia')
    parser.add_argument('-p', '--plot',
                        help='topo figure printout',
                        action="store_true")

    args = parser.parse_args()
    algorithms_list = args.algorithms
    cases = args.cases
    plot = args.plot

    for i in range(len(algorithms_list)-1, 0, -1):
        if algorithms_list[i] in algorithms_list[:i]:
            del algorithms_list[i]  # remove duplicates

    print('cases directory:', cases)
    print('algorithms:', algorithms_list)

    return algorithms_list, cases, plot


def process(algorithms_list, cases, plot=False):
    """For each case, test every algorithm."""

    # make directory for the test, and write parameter file.
    import time
    import os

    timestamp = time.strftime("%Y%m%d_%H%M%S")
    output_dir = '{base}/Gaia_{time}'.format(base=cases, time=str(timestamp))
    os.mkdir(output_dir)
    parameter_file = open(output_dir + '/' + 'parameters.txt', 'w')
    parameter_file.write(timestamp + '\n' +
                         'cases: ' + cases + '\n' +
                         'algorithms list: ' + str(algorithms_list))
    parameter_file.close()
    log_file = open(output_dir + '/log.txt', 'w')
    log_file.write('algorithm, case, time, result, dup_edges, sum_weight\n')

    # get cases dir list
    list_cases = os.listdir(cases)
    checked_cases = []
    for directory in list_cases:
        path = os.path.join(cases, directory)
        if os.path.isdir(path):
            case_file_list = os.listdir(path)
            if ('topo.csv' in case_file_list and
                    'demand.csv' in case_file_list):
                checked_cases.append(path)

    # run algorithms on cases, and write log
    from subprocess import call, check_output
    import re
    import ast

    for algorithm in algorithms_list:
        is_python_algorithm = False
        algorithm_name = re.search('(?<=/)[^/]*', algorithm).group()
        # note of re: last '/' to end
        print(algorithm_name)
        if algorithm_name[-3:] == '.py':
            # an py algortithm, del '.py'
            is_python_algorithm = True
            algorithm_name = algorithm_name[:-3]
        algorithm_dir = output_dir + '/' + algorithm_name
        os.mkdir(algorithm_dir)

        for case in checked_cases:
            case_name = re.search('case[^/]*\d+', case).group()
            topo = os.path.join(case, 'topo.csv')
            demand = os.path.join(case, 'demand.csv')
            output = algorithm_dir + '/' + case_name + '_o.csv'
            print("processing: {}".format(case))

            # call algorithm and count time
            t0 = time.time()
            if is_python_algorithm:
                call(["python", algorithm, topo, demand, output])
            else:
                call([algorithm, topo, demand, output])
            t1 = time.time()
            runtime = t1 - t0
            # print(runtime)  # for debugging
            # printout result of Columbus module
            result = check_output(["python", "hubble/columbus2.py",
                                   topo, demand, output])
            result = ast.literal_eval(result.decode('utf-8')[:-1])
            # [:-1] to escape '\n', ast.literal_eval instead of json to parse
            print(str(result))
            log_file.write('{}, {}, {:.6f}, {}, {}, {}\n'.format(
                algorithm_name, case_name, runtime, result[0], result[1], result[2]))


if __name__ == '__main__':
    algorithms_list, cases, plot = parse_input()
    process(algorithms_list, cases, plot)
