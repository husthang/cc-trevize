#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"



/****************************************
 * 宏 控制编译版本
 ****************************************/
/* debug信息打印 */
//#define _DEBUG_
#ifdef  _DEBUG_
//#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
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

/* 起始点出边DFS均搜 */
//#define _STARTVEX_BALANCE_DFS_

/* 两层DFS均搜 */
//#define _TWO_LAYER_DFS_
//#define _TEST_TWO_LAYER_DFS_


/****************************************
 * 结构体
 ****************************************/
typedef struct Edge
{
    int srcVex;
    int destVex;
    int nextEdge;
    int edgeCost;
} Edge;


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


/****************************************
 * 函数声明
 ****************************************/
void search_route(char *graph[MAX_EDGE_NUM], int edge_num, char *condition[MAX_DEMAND_NUM], int demand_num);

void InitTopo(char *topo[MAX_EDGE_NUM], int edge_num);

void InitDemand(char *demand[MAX_DEMAND_NUM], int demand_id);

void CopyResult(int *result, int *result_num);

void SearchRoute(int demand_id);

void PrintResultFile(int resultRoute[]);

int CheckResultDupeEdge(int result1[], int result2[]);

void DupeExchange(int result1[], int result2[]);

int GetExchange(int edgeID);

void PrintSPFALeastCost(int End);

void PrintPathByVex(int k);

void PrintPathByEdge(int k);

void InsertEdgeNodeForSPFA(EdgeNode **pFirstEdge, EdgeInfo *pInfo);

void DeleteIntLinkedList(IntNode **pHead);

void DeleteEdgeNodeForSPFA(EdgeNode **pFirstEdge);

void PrintList();

void GetVexSegment(int k, IntNode **pHead, IntNode **pTail);

void GetEdgeSegment(int k, IntNode **pHead, IntNode **pTail, int *pCost);

void PrintIntNode(IntNode *pHead);

void SPFA(int Start, int End);

void PrintStack(int vexStack[], int stackTop);

void PrintVisit(int visit[]);

bool EnVisit(int visit[], IntNode *pHead);

void DeVisit(int visit[], IntNode *pHead);

void SearchRouteBySPFA();

inline bool IsInterVex(const int vexID);

bool IsDupEdge(EdgeNode *pNode, const int edgeId, const int destVex, const int edgeCost);

int GetPseudoCost(EdgeInfo *pEdgeInfo);

void InsertEdgeNode(EdgeNode **pFirstEdge, EdgeInfo *pInfo);

bool UpdateEdges(EdgeNode *pEdges[], int QueueLayer);

EdgeNode *UpdateStackInfo(EdgeNode *pEdges[], int vexStack[], int visit[], EdgeInfo *edgeStack[], int *stackTop, int *stackTopVex, bool *isFirstEdge, int *costSum);

bool CountEdges(EdgeNode *pEdges[], int QueueLayer, int *count);

void SearchRouteByDFS();

double GetTimeInterval(EdgeNode *pEdge, double remainTime);

int MultiLayerPseudoCost(EdgeInfo *pEdgeInfo, int layer);

void ReformMultiLayerAdjList(int loop, int layer);

void SearchRouteByBalanceSPFA(int demand_id);

double GetTimeIntervalForBalanceSPFA(EdgeNode *pEdge, double remainTime);



#endif
