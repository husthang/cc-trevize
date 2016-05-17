# Trevize

Trevize, an algorithm for Codecraft 2016

Trevize, [2016 华为软件精英挑战赛（HUAWEI Code Craft 2016)](http://codecraft.huawei.com/) 的参赛算法仓库.

Author: [Organization Laboratory](https://bitbucket.org/organization-lab/)

License: MIT

## Repository structure

- `submission/`: main algorithm 主算法
- `trevize`: python algorithm prototype 测试算法(Python)
- `gaia/`: test framework 判卷和批量测试框架
- `hubble/`: facilities 辅助脚本
- `wiki/`: 一些比赛相关资源

## Main algorithm

V'-zip algorithm then a depth-first search with balanced time control.

基于必经点的压缩算法把原图转化为以所有必经点和 s, t 构成的稠密图; 再用深度优先搜索寻路.

具体 i/o 参见 wiki 或 http://codecraft.huawei.com/home/detail

## Facilities

### IP-trevize, integer programming-based algorithms

`trevize/ip-trevize.py` and `trevize/p2-ip-trevize.py`

基于整数规划和求解器的算法, 可给出 phase 1 and phase 2 的**最优解**.

具体算法主要参考文献 (andrade 2013 and RNDM 2015)

`Elementary shortest-paths visiting a given set of nodes, by Rafael Castro de Andrade`

`Protected shortest path visiting specified nodes, by Teresa Gomes, Sofia Marques, Lucia Martins, Marta Pascoal and David Tipper`

### Gaia, the testing framework

`$python gaia/gaia2.py cases_folder algorithm1 [algorithm2-n]`

可批量对多个算法进行 Benchmark, 并输出结果.

测试文件夹会输出到 `cases_folder/Gaia_timestamp/` 下.

建议使用方法: 把想测的 cases 都放在 test/ 下面, 然后运行 Gaia.
(可避免 Git 跟踪这些文件)

注意: 目前的版本只能识别 `topo.csv` 和`demand.csv`,
如果 case 文件名不对需要调整.

`log.txt` 中按照 csv format 输出 log:

`algorithm, case, time, result, dup_edges, sum_weight`

### Columbus, the judge

Columbus: answer checker for Trevize.

`python hubble/columbus2.py topo.csv demand.csv answer.csv`

Columbus: 检查结果是否正确. 给出适当的输出

- input
    - `python columbus.py topo.csv demand.csv answer.csv`
    - topo: `topo.csv`, contains graph
    - demand: `demand.csv`, contains `s, t, v1(V')`
    - answer: `answer.csv`, contains answer
- process
    - check answer (only for cases that has valid answer?)
    - check answer set (duplicate of vertex in path)
    - check vertices in V'
    - check path start and end
- output
    - print to stdout
    - return answer json string (as output module in test framework Gaia)
        - format: "[AC/WA, length/WA reason]"

