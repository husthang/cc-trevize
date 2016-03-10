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

    import os



    pass

def output():
    pass


if __name__ == '__main__':
    algorithms_list, cases = parse_input()
    process(algorithms_list, cases)
    output()
