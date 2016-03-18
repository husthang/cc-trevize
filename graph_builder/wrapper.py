# -*- coding: utf-8 -*-

"""Graph builder wrapper."""

import os
from subprocess import call
from shutil import move


num_cases = 20
vex = 30
edge = 80

for i in range(60, 80):
    print(vex, edge)
    call(["./graph_builder", str(vex), str(edge)])
    dst = "../test/case_random" + str(i)
    os.mkdir(dst)
    move("topo.csv", dst)
    move("demand.csv", dst)
    edge += 4
    if edge % 12 == 0:
        vex += 1
