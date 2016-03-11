# -*- coding: utf-8 -*-

"""Gaia: the test framework for Trevize

- input:
    - algorithms
        - should meet run format of `algorithm topo.csv demand.csv out.csv`
        - note: algorithm may be single algorithm,
                or hybrid of different algorithms.
    - test cases
- process:
    - for each case, test every algorithm
- output: to a log file
    - features of the test case
    - result of algorithms
        - time
        - route
        - weight

"""

# Author: Mo Frank Hu (mofrankhu@gmail.com)
# Dependency: Python 3


import argparse


def parse_input():
    """Parse arguments."""

    # argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('algorithms',
                        help='list of algorithms (Python format in quote)' +
                             'for running Gaia')
    parser.add_argument('cases',
                        help='directory of cases for running Gaia')
    args = parser.parse_args()
    algorithms = args.algorithms
    cases = args.cases

    # parse algorithms
    import re
    t = re.compile('[\[\],\ ]')  # process '[', ']', <space>

    algorithms_list = t.split(algorithms)
    for i in range(len(algorithms_list)-1, -1, -1):
        if algorithms_list[i] is '':
            del algorithms_list[i]

    return algorithms_list, cases

def process(algorithms_list, cases):
    """For each case, test every algorithm."""

    # get cases dir list
    import os
    list_cases = os.listdir(cases)
    checked_cases = []
    for directory in list_cases:
        path = os.path.join(cases, directory)
        if os.path.isdir(path):
            case_file_list = os.listdir(path)
            if 'topo.csv' in case_file_list and 'demand.csv' in case_file_list:
                checked_cases.append(path)

    print(checked_cases)  # for debugging

    import time
    import re

    timestamp = time.strftime("%Y%m%d%H%M%S", time.gmtime(time.time()))
    output_dir = '../test/gaia-' + timestamp
    os.mkdir(output_dir)
    from subprocess import call
    import time
    for algorithm in algorithms_list:
        for case in checked_cases:

            case_name = re.search('case.*', case).group()
            print(case_name)
            algorithm = '../py-trevize/py-trevize.py'
            topo = os.path.join(case, 'topo.csv')
            demand = os.path.join(case, 'demand.csv')
            output = output_dir + '/' + case_name + '_o.csv'

            t0 = time.time()
            call(["python", algorithm, topo, demand, output])
            t1 = time.time()
            runtime = t1 - t0
            print(runtime)
            # get return code and time



if __name__ == '__main__':
    algorithms_list, cases = parse_input()
    process(algorithms_list, cases)
