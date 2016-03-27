#ifndef _FUTURE_NET_H_
#define _FUTURE_NET_H_


/* 最大顶点数 */
#define MAX_VEX     (600)
/* 最大有向边数 */
#define MAX_EDGE    (4800)
/* 最大中间点集v'数 */
#define MAX_INTER   (50)
/* 权重极限值 */
#define INF         (999999)
/* 缺省字符串长度 */
#define MAX_STR     (1024)


/* 有向边信息 */
typedef struct EdgeInfo
{
    int destVex;    // 目标顶点
    int srcVex;     // 起始定点
    int edgeID;     // 有向边编号
    int edgeCost;   // 有向边权重
} EdgeInfo;

/* 有向边链表节点 */
typedef struct EdgeNode
{
    struct EdgeInfo edgeInfo;   // 有向边信息
    struct EdgeNode *pNext;
} EdgeNode;

/* 邻接表 各链表头节点 存储第一条边 */
typedef struct AdjListHead
{
    struct EdgeNode *pFirstEdge;
} AdjListHead;

/* 程序状态返回码 */
typedef enum StatusInfo
{
    SYS_ERROR = 0
} StatusInfo;




void PrintStatusInfo(StatusInfo statusInfo);

bool IsDupEdge(EdgeNode *pNode, const int edgeId, const int destVex, const int edgeCost);

inline bool IsInterVex(const int vexID);

FILE *Initalize(int argc, char *argv[]);

void InitDemandFile(FILE *fp_demand);

int GetPseudoCost(EdgeInfo *pEdgeInfo);

void InsertEdgeNode(EdgeNode **pFirstEdge, EdgeInfo *pInfo);

void InitTopoFile(FILE *fp_topo);

void SearchRoute();

double GetTimeInterval(EdgeNode *pEdge, double remainTime);

void PrintResultFile(FILE *fp_result);


#endif 
