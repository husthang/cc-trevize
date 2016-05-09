# Trevize

Trevize, an algorithm for Codecraft 2016

Author: [Organization Laboratory](https://bitbucket.org/organization-lab/)

## Facilities

### Gaia, the testing framework

`$python gaia/gaia2.py cases_folder algorithm1 [algorithm2-n]`

可同时给多个算法进行 Benchmark, 并输出结果.

测试文件夹会输出到 `cases_folder/Gaia_timestamp/` 下.

建议使用方法: 把想测的 cases 都放在 test/ 下面, 然后运行 Gaia.
(可避免 Git 跟踪这些文件)

注意: 目前的版本只能识别 `topo.csv` 和`demand.csv`,
如果 case 文件名不对需要调整.

`log.txt` 中按照 csv format 输出 log:

`algorithm, case, time, result, dup_edges, sum_weight`
