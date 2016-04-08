#ifndef _FUTURE_NET_H_
#define _FUTURE_NET_H_


/****************************************
 * 宏 控制编译版本
 ****************************************/
/* debug信息打印 */
//#define _DEBUG_
#ifdef  _DEBUG_
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif

/* 测试信息打印 */
//#define _TEST_
#ifdef  _TEST_
#define TEST(format,...) printf(format, ##__VA_ARGS__)
#else
#define TEST(format,...)
#endif

/* 搜索结果信息打印 */
//#define _TEST_RESULT_

/* 按是否为中间点v'和权重大小两项 排序邻接表 */
#define _PSEUDO_COST_
/* 搜索多层邻接表 按是否为中间点v'和权重大小两项 重新排序邻接表 */
#define _JUDGE_MULTI_LAYER_ADJ_
//#define _TEST_MULTI_LAYER_ADJ_

/* 从interVex往外搜索 */
//#define _VEX_LEVEL_

/* 统计每个顶点在之后的路径中最多经过多少v'顶点,并据此判断是否继续搜索 */
/* 有不明bug */
//#define _JUDGE_MAX_INTER_

/* 统计每个顶点在之后的路径中是否能通向终点endVex,并据此判断是否继续搜索 */
/* 对成环情况考虑不周 */
//#define _JUDGE_DISCARD_

/* BFS */
#define _BFS_
//#define _TEST_BFS_


/****************************************
 * 宏 常量
 ****************************************/
/* 最大顶点数 */
#define MAX_VEX     (600)
/* 最大有向边数 */
#define MAX_EDGE    (4800)
/* 最大中间点集v'数 */
#define MAX_INTER   (50)
/* 权重极限值 */
//#define INF         (999999)
/* 缺省字符串长度 */
#define MAX_STR     (1024)

#define MAX_QUEUE   (3)


/****************************************
 * 结构体
 ****************************************/
typedef struct IntNode
{
    int data;
    struct IntNode *pNext;
} IntNode;


/* 有向边信息 */
typedef struct EdgeInfo
{
    int destVex;    // 目标顶点
    int srcVex;     // 起始定点
    int edgeID;     // 有向边编号
    int edgeCost;   // 有向边权重
    int edgePseudoCost;

    IntNode *vexSegHead;
    IntNode *vexSegTail;
    IntNode *edgeSegHead;
    IntNode *edgeSegTail;
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

typedef struct VexInfo
{
    bool isDone;
    bool isDiscard;
    int passNum;
    int minInterNum;
    int maxInterNum;
    int maxInterTmp;
} VexInfo;


/****************************************
 * 枚举
 ****************************************/
/* 程序状态返回码 */
typedef enum StatusInfo
{
    SYS_ERROR = 0
} StatusInfo;


/****************************************
 * 函数声明
 ****************************************/
inline bool IsInterVex(const int vexID);

bool IsDupEdge(EdgeNode *pNode, const int edgeId, const int destVex, const int edgeCost);

int GetPseudoCost(EdgeInfo *pEdgeInfo);

void InsertEdgeNode(EdgeNode **pFirstEdge, EdgeInfo *pInfo);

void InitTopoFile(FILE *fp_topo);

bool UpdateEdges(EdgeNode *pEdges[], int QueueLayer);

EdgeNode *UpdateStackInfo(EdgeNode *pEdges[], int vexStack[], int visit[], EdgeInfo *edgeStack[MAX_VEX], int *stackTop, int *stackTopVex, bool *isFirstEdge, int *costSum);

bool CountEdges(EdgeNode *pEdges[], int QueueLayer, int *count);

void SearchRoute();

double GetTimeInterval(EdgeNode *pEdge, double remainTime);

int MultiLayerPseudoCost(EdgeInfo *pEdgeInfo, int layer);

void ReformMultiLayerAdjList(int loop, int layer);


void spfa_SearchRoute_11();

double GetTimeInterval_11(EdgeNode *pEdge, double remainTime);


#endif 
