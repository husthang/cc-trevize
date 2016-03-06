功能：
生成拓扑结构文件topo.csv和起止点与中间点集文件demand.csv

编译：
g++ graph_builder.cpp -o graph_builder

使用：
./graph_builder [vex edge]
生成含有节点数为vex和定向边数为edge的拓扑结构。
./graph_builder
默认vex=100, edge=500。

