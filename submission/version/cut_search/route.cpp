#include "route.h"
#include "lib_record.h"
#include <stdio.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <queue>
using namespace std;


/* 全局常量 */
const int INF         = 0x7FFFFFFF;     // 极限值
const int MAX_VEX     = 2000;            // 最大顶点数
const int MAX_EDGE    = 40000;           // 最大边数
const int MAX_INTER   = 100;             // 最大v'中间点数
//const int MAX_STR     = 1024;           // 缺省字符串数
const int MAX_QUEUE   = 1;              // 最大BFS均搜层数
const double MAX_TIME = 4.5;            // 最大搜索时限


/* 全局变量 */
int g_headEdge[MAX_VEX];                // 各点出边邻接表的头节点边
int g_spfaLeastCost[MAX_EDGE];          // SPFA搜出的各点最短路径权重
Edge g_edges[MAX_EDGE];                 // 有向边数组

queue<int> g_vexQueu;                   // 顶点队列

int g_vexPath[MAX_VEX];                 // 用顶点表示的路径
int g_inEdgeOfVex[MAX_VEX];             // SPFA最短路径各点入边
int g_edgePath[MAX_EDGE];               // 用有向边表示的路径


AdjListHead *g_spfaEdgeList = NULL;     // 压缩图后,V'有向边邻接表
AdjListHead *g_edgeList = NULL;         // 有向边邻接表

int g_startVex;                         // 起点
int g_endVex;                           // 终点

int g_spfaInterVex[MAX_VEX];            // V'必经中间点集(SPFA用)
int g_interVex[MAX_INTER];              // V'必经中间点集

int g_edgeNum = 0;                      // 全图有向边数
int g_interNum = 0;                     // 全图V'必经中间点数
int g_totalCost = INF;                  // 总权重
int g_resultRoute[MAX_VEX+1];           // 搜索路径结果

clock_t g_startTime;                    // 计时器起始时间





//你要完成的功能总入口
void search_route(char *topo[MAX_EDGE_NUM], int edge_num, char *demand[MAX_DEMAND_NUM], int demand_num)
{
    int i = 0;

    int result1[MAX_VEX+1];
    int result2[MAX_VEX+1];
    int result_num1 = 0;
    int result_num2 = 0; 

    InitTopo(topo, edge_num);

    InitDemand(demand, 0);
    SearchRoute(0);
    if (-1 == g_resultRoute[0])
    {
        clear_result();
        return;
    }
    CopyResult(result1, &result_num1);

    #ifdef _TEST_RESULT_
    printf("Total Cost 1: %d\n\n", g_totalCost);
    #endif

    for (i = 0; i < MAX_VEX; i++)
    {
        DeleteEdgeNodeForSPFA(&(g_spfaEdgeList[i].pFirstEdge));
    }

    InitDemand(demand, 1);
    SearchRoute(1);
    if (-1 == g_resultRoute[0])
    {
        clear_result();
        return;
    }
    CopyResult(result2, &result_num2);

    #ifdef _TEST_RESULT_
    printf("Total Cost 2: %d\n\n", g_totalCost);
    #endif

    DupeExchange(result1, result2);

    #ifdef _TEST_RESULT_
    printf("Route 1: ");
    PrintResultFile(result1);
    printf("Route 2: ");
    PrintResultFile(result2);
    #endif

    i = 0;
    while (result1[i] != -1)
    {
        record_result(WORK_PATH, (unsigned short)result1[i]);
        i++;
    }
    i = 0;
    while (result2[i] != -1)
    {
        record_result(BACK_PATH, (unsigned short)result2[i]);
        i++;
    }
    
    #ifdef _TEST_RESULT_
    CheckResultDupeEdge(result1, result2);
    #endif

    /*
    #ifdef _TEST_RESULT_
    endTime = clock();
    duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
    cout<<endl<<"Time: "<<duration<<"s, Total Cost: "<<g_totalCost<<endl;
    #endif
    */
}



/****************************************
函数名称：
函数功能：读取输入文件,初始化
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void InitTopo(char *topo[MAX_EDGE_NUM], int edge_num)
{
    int i = 0;
    g_edgeList     = (AdjListHead *)malloc(sizeof(AdjListHead)*MAX_VEX);
    g_spfaEdgeList = (AdjListHead *)malloc(sizeof(AdjListHead)*(MAX_VEX));

    for (i = 0; i < MAX_VEX; i++)
    {
        g_edgeList[i].pFirstEdge     = NULL;
        g_spfaEdgeList[i].pFirstEdge = NULL;
    }

    memset(g_headEdge, -1, sizeof(g_headEdge));
    memset(g_resultRoute, -1, sizeof(g_resultRoute));

    int edgeID;
    int srcVex;
    int destVex;
    int edgeCost;
    EdgeInfo *pInfo;

    for (i = 0; i < edge_num; i++)
    {
        sscanf(topo[i], "%d,%d,%d,%d\n", &edgeID, &srcVex, &destVex, &edgeCost);
        g_edges[edgeID].srcVex   = srcVex;
        g_edges[edgeID].nextEdge = g_headEdge[srcVex];
        g_edges[edgeID].destVex  = destVex;
        g_edges[edgeID].edgeCost = edgeCost;
        g_headEdge[srcVex]       = edgeID;

        //cout<<edgeID<<" "<<g_edges[edgeID].srcVex<<" "<<g_edges[edgeID].destVex<<" "<<g_edges[edgeID].edgeCost<<endl;

        //if (!IsDupEdge(g_edgeList[srcVex].pFirstEdge, edgeID, destVex, edgeCost))
        //{
            pInfo = (EdgeInfo *)malloc(sizeof(EdgeInfo));
            pInfo->edgeID      = edgeID;
            pInfo->srcVex      = srcVex;
            pInfo->destVex     = destVex;
            pInfo->edgeCost    = edgeCost;
            pInfo->vexSegHead  = NULL;
            pInfo->vexSegTail  = NULL;
            pInfo->edgeSegHead = NULL;
            pInfo->edgeSegTail = NULL;
            InsertEdgeNode(&(g_edgeList[srcVex].pFirstEdge), pInfo);
            g_edgeNum++;
        //}
    }

    #ifdef _JUDGE_MULTI_LAYER_ADJ_
    ReformMultiLayerAdjList(2, 2);
    #endif

}


void InitDemand(char *demand[MAX_DEMAND_NUM], int demand_id)
{
    g_interNum = 0;
    g_totalCost = INF; 
    memset(g_resultRoute, -1, sizeof(g_resultRoute));
    memset(g_spfaInterVex, 0, sizeof(g_spfaInterVex));
    memset(g_interVex, 0, sizeof(g_interVex));

    int len = 0;
    int vexID;
    char buf1[6], buf2[6], ch;
    
    len = 2;
    sscanf(demand[demand_id]+len, "%[^,|],%[^,|]", buf1, buf2);
    g_startVex = atoi(buf1);
    g_endVex   = atoi(buf2);

    //cout<<demand[demand_id]<<endl;
    //cout<<buf1<<" "<<buf2<<" ";

    len = len + strlen(buf1) + strlen(buf2) + 1;
    while (demand[demand_id][len] != '\n' && demand[demand_id][len] != '\0')
    {
        memset(buf1, 0, 6);
        sscanf(demand[demand_id]+len, "%[,|]%[^,|]", &ch, buf1);
        if (buf1[0] != 'N')
        {
            vexID = atoi(buf1);
            g_spfaInterVex[vexID]  = 1;
            g_interVex[g_interNum] = vexID;
            g_interNum++;
        }
        len += strlen(buf1) + 1; 

        //cout<<vexID<<" ";
    }//cout<<endl;
}


void CopyResult(int *result, int *result_num)
{
    if (-1 == g_resultRoute[0])
    {
        *result_num = 0;
        return;
    }
    else
    {
        int i = 0;
        while (g_resultRoute[i] != -1)
        {
            result[i] = g_resultRoute[i];
            g_edges[result[i]].edgeCost = -1;
            i++;
        }
        result[i] = -1;
        *result_num = i;
    }
}


/****************************************
函数名称：
函数功能：搜索路径
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void SearchRoute(int demand_id)
{
    g_startTime = clock();

    for (int i = 0; i < MAX_VEX; i++)
    {
        if (1 == g_spfaInterVex[i] || i == g_startVex)
        {
            SPFA(i, 0);
        }
    }

    SearchRouteByBalanceSPFA(demand_id);
}


/****************************************
函数名称：
函数功能：输出结果至 result文件 或 标准输出
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void PrintResultFile(int resultRoute[])
{
    if (-1 == resultRoute[0])
    {
        printf("NA\n");
    }
    else
    {
        int i = 0;
        printf("%d", resultRoute[i]);
        i++;
        while (resultRoute[i] != -1)
        {
            printf("|%d", resultRoute[i]);
            i++;
        }
    }
    printf("\n");
    //printf("Total Cost: %d\n", g_totalCost);
}


int CheckResultDupeEdge(int result1[], int result2[])
{
    int i = 0;
    int j = 0;
    int dupeNum = 0;
    while (result1[i] != -1)
    {
        j = 0;
        while (result2[j] != -1)
        {
            if (result1[i] == result2[j])
            {
                dupeNum++;
                break;
            }
            j++;
        }
        i++;
    }
    printf("\nDupeEdgeNum: %d\n", dupeNum);
    return dupeNum;
}


void DupeExchange(int result1[], int result2[])
{
    int i = 0;
    int j = 0;
    int newEdge = -1;
    while (result1[i] != -1)
    {
        j = 0;
        while (result2[j] != -1)
        {
            if (result1[i] == result2[j])
            {
                newEdge = GetExchange(result1[i]);
                if (newEdge != -1)
                {
                    result1[i] = newEdge;
                }
                break;
            }
            j++;
        }
        i++;
    }
}


int GetExchange(int edgeID)
{
    EdgeNode *pNode = g_edgeList[g_edges[edgeID].srcVex].pFirstEdge;
    int minCost = 0;
    int newID   = -1;
    while (pNode != NULL)
    {
        if ((pNode->edgeInfo.edgeID != edgeID)
            && (pNode->edgeInfo.srcVex == g_edges[edgeID].srcVex) 
            && (pNode->edgeInfo.destVex == g_edges[edgeID].destVex))
        {
            if (minCost < g_edges[edgeID].edgeCost)
            {
                minCost = g_edges[edgeID].edgeCost;
                newID   = pNode->edgeInfo.edgeID;
            }
        }
        pNode = pNode->pNext;
    }
    return newID;
}


/****************************************
函数名称：
函数功能：在拓扑压缩后的邻接表中插入节点
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void InsertEdgeNodeForSPFA(EdgeNode **pFirstEdge, EdgeInfo *pInfo)
{
    if (NULL == pFirstEdge || NULL == pInfo)
    {
        exit(1);
    }

    EdgeNode *pHead = *pFirstEdge;
    EdgeNode *pNode = pHead;
    EdgeNode *pNew  = (EdgeNode *)malloc(sizeof(EdgeNode));
    
    while (NULL != pNode)
    {
        if (pNode->edgeInfo.edgeCost < pInfo->edgeCost)
        {
            pHead = pNode;
            pNode = pNode->pNext;
        }
        else
        {
            break;
        }
    }

    memcpy(&(pNew->edgeInfo), pInfo, sizeof(EdgeInfo));
    pNew->pNext = pNode;
    if (pNode == pHead)
    {
        *pFirstEdge = pNew;
    }
    else
    {
        pHead->pNext = pNew;
    }
}


void DeleteIntLinkedList(IntNode **pHead)
{
    IntNode *pNode = *pHead;
    IntNode *pNext = pNode;
    *pHead = NULL;
    while (pNode != NULL)
    {
        pNext = pNode->pNext;
        free(pNode);
        pNode = pNext;
    }
}


void DeleteEdgeNodeForSPFA(EdgeNode **pFirstEdge)
{
    EdgeNode *pNode = *pFirstEdge;
    EdgeNode *pNext = pNode;
    *pFirstEdge = NULL;
    while (pNode != NULL)
    {
        pNext = pNode->pNext;
        DeleteIntLinkedList(&(pNode->edgeInfo.vexSegHead));
        DeleteIntLinkedList(&(pNode->edgeInfo.edgeSegHead));
        free(pNode);
        pNode = pNext;
    }
}


/****************************************
函数名称：
函数功能：获得V'顶点间最短路径(以顶点存储)
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void GetVexSegment(int k, IntNode **pHead, IntNode **pTail)
{
    if(g_vexPath[k] != -1)
    {
        GetVexSegment(g_vexPath[k], pHead, pTail);
    }
    IntNode *pNew = (IntNode *)malloc(sizeof(IntNode));
    pNew->data = k;
    pNew->pNext = NULL;
    if (*pHead == NULL)
    {
        *pHead = pNew;
    }
    else
    {
        IntNode *pNode = *pHead;
        while (pNode->pNext != NULL)
        {
            pNode = pNode->pNext;
        }
        pNode->pNext = pNew;
    }
    *pTail = pNew;
}


/****************************************
函数名称：
函数功能：获得V'顶点间最短路径(以有向边存储)
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void GetEdgeSegment(int k, IntNode **pHead, IntNode **pTail, int *pCost)
{
    if(g_edgePath[k] != -1)
    {
        GetEdgeSegment(g_edgePath[k], pHead, pTail, pCost);
    }
    IntNode *pNew = (IntNode *)malloc(sizeof(IntNode));
    pNew->data = k;
    pNew->pNext = NULL;
    *pCost += g_edges[k].edgeCost;
    if (*pHead == NULL)
    {
        *pHead = pNew;
    }
    else
    {
        IntNode *pNode = *pHead;
        while (pNode->pNext != NULL)
        {
            pNode = pNode->pNext;
        }
        pNode->pNext = pNew;
    }
    *pTail = pNew;
}


/****************************************
函数名称：
函数功能：SPFA算法搜索最短路
输入参数：
输出参数：
返回值：
修改情况：
算法引用: Duan, Fanding (1994), "关于最短路径的SPFA快速算法", 西南交通大学学报 29 (2): 207–212
*****************************************/
void SPFA(int startVex, int endVex)
{
    memset(g_vexPath, -1, sizeof(g_vexPath));
    memset(g_inEdgeOfVex, -1, sizeof(g_inEdgeOfVex));
    memset(g_edgePath, -1, sizeof(g_edgePath));

    while (!g_vexQueu.empty())
    {
        g_vexQueu.pop();
    }

    fill(g_spfaLeastCost, g_spfaLeastCost+MAX_EDGE, INF);
    g_spfaLeastCost[startVex]  = 0;

    int visit[MAX_VEX] = {0};
    visit[startVex] = 1;

    g_vexQueu.push(startVex);
    while (!g_vexQueu.empty())
    {  
        int queueFront = g_vexQueu.front();
        g_vexQueu.pop();
        visit[queueFront] = 0;
        int i;

        for (i = g_headEdge[queueFront]; i != -1; i = g_edges[i].nextEdge)
        {
            if (-1 == g_edges[i].edgeCost)
            {
                continue;
            }

            int cost = g_edges[i].edgeCost;
            if (cost + g_spfaLeastCost[queueFront] < g_spfaLeastCost[g_edges[i].destVex])
            {
                g_spfaLeastCost[g_edges[i].destVex] = cost + g_spfaLeastCost[queueFront];
                g_vexPath[g_edges[i].destVex]       = queueFront;
                g_inEdgeOfVex[g_edges[i].destVex]   = i;
                g_edgePath[i]                       = g_inEdgeOfVex[queueFront];

                if (!visit[g_edges[i].destVex]
                    && (g_edges[i].destVex != g_startVex && g_edges[i].destVex != g_endVex)
                    && (g_spfaInterVex[g_edges[i].destVex] == 0 || g_spfaInterVex[g_edges[i].destVex] == endVex))
                {
                    visit[g_edges[i].destVex] = 1;
                    g_vexQueu.push(g_edges[i].destVex);
                }  
            }  
        }
    }

    EdgeInfo *pInfo;
    for (int i = 0; i < MAX_VEX; i++)
    {
        if ((1 == g_spfaInterVex[i] || i == g_endVex) && i != startVex)
        {
            if (g_spfaLeastCost[i] != INF)
            {
                //PrintPathByVex(i);
                //printf("\n");
                //PrintPathByEdge(g_inEdgeOfVex[i]);
                //printf("\n");

                pInfo = (EdgeInfo *) malloc(sizeof(EdgeInfo));
                pInfo->edgeID   = -1;
                pInfo->srcVex   = startVex;
                pInfo->destVex  = i;
                pInfo->edgeCost = 0;
                pInfo->vexSegHead = NULL;
                pInfo->vexSegTail = NULL;
                GetVexSegment(i, &(pInfo->vexSegHead), &(pInfo->vexSegTail));
                //PrintIntNode(pInfo->vexSegHead);
                pInfo->edgeSegHead = NULL;
                pInfo->edgeSegTail = NULL;
                GetEdgeSegment(g_inEdgeOfVex[i], &(pInfo->edgeSegHead), &(pInfo->edgeSegTail), &(pInfo->edgeCost));
                //PrintIntNode(pInfo->edgeSegHead);
                InsertEdgeNodeForSPFA(&(g_spfaEdgeList[startVex].pFirstEdge), pInfo);
            }
        }
    }
}


/****************************************
函数名称：
函数功能：拓扑展开拼接最短路时,判断非V'点是否成环
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
bool EnVisit(int visit[], IntNode *pHead)
{
    IntNode *pNode = pHead;
    int stop = 0;
    while (pNode != NULL)
    {
        if (0 == visit[pNode->data])
        {
            visit[pNode->data] = 1;
            pNode = pNode->pNext;
        }
        else
        {
            stop = pNode->data;
            pNode = pHead;
            while (pNode->data != stop)
            {
                visit[pNode->data] = 0;
                pNode = pNode->pNext;
            }
            return false;
        }
    }
    return true;
}


/****************************************
函数名称：
函数功能：拓扑展开拼接最短路时,对于成环路段,回溯visit判断
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void DeVisit(int visit[], IntNode *pHead)
{
    IntNode *pNode = pHead;
    while (pNode != NULL)
    {
        visit[pNode->data] = 0;
        pNode = pNode->pNext;
    }
}


/****************************************
函数名称：
函数功能：回溯拼接SPFA最短路,寻中符合要求的路径
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void SearchRouteBySPFA()
{
    clock_t endTime;
    double duration;

    int visit[MAX_VEX] = {0};
    int vexStack[MAX_VEX+1];
    int stackTop = 1;
    int stackTopVex = g_startVex;
    bool isFirstEdge = true;

    EdgeInfo *edgeStack[MAX_VEX];     // 由于每个点只经过一次,所以最多走过MAX_VEX-1条边. 且stack[0]不压入数据.
    EdgeNode *pEdge = g_spfaEdgeList[g_startVex].pFirstEdge;

    visit[g_startVex] = 1;
    vexStack[stackTop] = g_startVex;    // vexStack[0]不压入数据,用于判断栈空. vexStack[1]存放起始点.
    edgeStack[0] = NULL;                // edgeStack[0]不压入数据. edgeStack[1]用于存放其实边,即起始点的出边.

    int i = 0;

    double remainTime = MAX_TIME;
    int costSum = 0;

    int visitInt[MAX_VEX] = {0};
    visitInt[g_startVex] = 1;

    do
    {
        if (isFirstEdge)
        {
            pEdge = g_spfaEdgeList[stackTopVex].pFirstEdge;
            isFirstEdge = false;
        }
        else
        {
            pEdge = pEdge->pNext;
        }

        if (pEdge != NULL)
        {
            if (0 == visit[pEdge->edgeInfo.destVex])
            {
                visit[pEdge->edgeInfo.destVex] = 1;
                stackTop++;
                /* vexStack[stackTop-1]  是 edgeStack[stackTop-1] 的起始点,
                 * vexStack[stackTop]    是 edgeStack[stackTop-1] 的指向点.
                 * edgeStack[stackTop-1] 是 vexStack[stackTop]    的入边,
                 * edgeStack[stackTop]   是 vexStack[stackTop]    的出边    */
                vexStack[stackTop]  = pEdge->edgeInfo.destVex;
                edgeStack[stackTop-1] = &(pEdge->edgeInfo);
                costSum += pEdge->edgeInfo.edgeCost;

                if (!EnVisit(visitInt, edgeStack[stackTop-1]->vexSegHead->pNext))
                {
                    goto BACKWARD;
                }

                if (pEdge->edgeInfo.destVex == g_endVex)
                {

                    if (g_interNum + 2 == stackTop)
                    {
                        if (costSum < g_totalCost)
                        {
                            int n = 0;
                            IntNode *pIntVex = NULL;
                            IntNode *pIntEdge = NULL;
                            for (i = 1; i < stackTop; i++)
                            {
                                pIntVex = edgeStack[i]->vexSegHead->pNext;
                                pIntEdge = edgeStack[i]->edgeSegHead;
                                while (pIntEdge != NULL)
                                {
                                    g_resultRoute[n] = pIntEdge->data;
                                    pIntEdge = pIntEdge->pNext;
                                    n++;
                                }
                            }
                            g_resultRoute[n] = -1;
                            g_totalCost = costSum;
                            #ifdef _TEST_RESULT_
                            cout << "New Least Cost  " << g_totalCost << endl;
                            #endif
                        }
                    }

                    visit[g_endVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;

                    if (stackTop > 0)
                    {
                        DeVisit(visitInt, edgeStack[stackTop]->vexSegHead->pNext);
                    }
                }
                else if (costSum >= g_totalCost)
                {
                    DeVisit(visitInt, edgeStack[stackTop-1]->vexSegHead->pNext);

                    BACKWARD:

                    visit[pEdge->edgeInfo.destVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;
                }
                else
                {
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = true;
                }

            }

            endTime = clock();
            duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
            if (duration > remainTime)
            {
                break;
            }
        }
        else
        {
            visit[vexStack[stackTop]] = 0;
            stackTop--;

            if (stackTop != 0)
            {
                DeVisit(visitInt, edgeStack[stackTop]->vexSegHead->pNext);

                pEdge = g_spfaEdgeList[vexStack[stackTop]].pFirstEdge;
                while (pEdge->edgeInfo.destVex != stackTopVex)
                {
                    pEdge = pEdge->pNext;
                }
                stackTopVex = vexStack[stackTop];
                isFirstEdge = false;
                costSum -= pEdge->edgeInfo.edgeCost;
            }
        }
    } while (stackTop);
}


/****************************************
函数名称：
函数功能：判断顶点是否属于中间点集v'
输入参数：顶点编号
输出参数：
返回值：
修改情况：
*****************************************/
inline bool IsInterVex(const int vexID)
{
    for (int i = 0; i < g_interNum; i++)
    {
        if (vexID == g_interVex[i])
            return true;
    }
    return false;
}


/****************************************
函数名称：
函数功能：判断两顶点间是否有多条同向边.若有,留下权重更小的.
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
bool IsDupEdge(EdgeNode *pNode, const int edgeId, const int destVex, const int edgeCost)
{
    while (NULL != pNode)
    {
        if (destVex == pNode->edgeInfo.destVex)
        {
            if (pNode->edgeInfo.edgeCost > edgeCost)
            {
                pNode->edgeInfo.edgeCost = edgeCost;
                pNode->edgeInfo.edgeID   = edgeId;
            }
            return true;
        }
        pNode = pNode->pNext;
    }
    return false;
}


/****************************************
函数名称：
函数功能：在插入节点时,非中间点集v'内的点,权重按+20计算,排在后面
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
int GetPseudoCost(EdgeInfo *pEdgeInfo)
{
    if (NULL == pEdgeInfo)
    {
        return 0;
    }

    if (IsInterVex(pEdgeInfo->destVex))
    {
        return pEdgeInfo->edgeCost;
    }
    else
    {
        return (pEdgeInfo->edgeCost + 20);
    }
}


/****************************************
函数名称：
函数功能：在邻接表中按权重大小顺序插入有向边节点
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void InsertEdgeNode(EdgeNode **pFirstEdge, EdgeInfo *pInfo)
{
    if (NULL == pFirstEdge || NULL == pInfo)
    {
        exit(1);
    }

    EdgeNode *pHead = *pFirstEdge;
    EdgeNode *pNode = pHead;
    EdgeNode *pNew  = (EdgeNode *)malloc(sizeof(EdgeNode));
    while (NULL != pNode)
    {
        #ifdef _PSEUDO_COST_
        if (GetPseudoCost(&(pNode->edgeInfo)) < GetPseudoCost(pInfo))
        #else
        if (pNode->edgeInfo.edgeCost < pInfo->edgeCost)
        #endif
        {
            pHead = pNode;
            pNode = pNode->pNext;
        }
        else
        {
            break;
        }
    }
    memcpy(&(pNew->edgeInfo), pInfo, sizeof(EdgeInfo));
    pNew->pNext = pNode;
    if (pNode == pHead)
    {
        *pFirstEdge = pNew;
    }
    else
    {
        pHead->pNext = pNew;
    }
}


/****************************************
函数名称：
函数功能：更新浅层DFS边栈元素
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
bool UpdateEdges(EdgeNode *pEdges[], int QueueLayer)
{
    if (-1 == QueueLayer)
    {
        return false;
    }
    pEdges[QueueLayer] = pEdges[QueueLayer]->pNext;
    while (NULL == pEdges[QueueLayer])
    {
        if(!UpdateEdges(pEdges, QueueLayer-1))
        {
            return false;
        }
        pEdges[QueueLayer] = g_edgeList[pEdges[QueueLayer-1]->edgeInfo.destVex].pFirstEdge;
    }
    return true;
}

/****************************************
函数名称：
函数功能：更新浅层DFS边栈信息
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
EdgeNode *UpdateStackInfo(EdgeNode *pEdges[], int vexStack[], int visit[], EdgeInfo *edgeStack[MAX_VEX], int *stackTop, int *stackTopVex, bool *isFirstEdge, int *costSum)
{
    int i = 0;
    EdgeNode *pNode = NULL;

    UPDATE:

    for (i = 0; i < MAX_VEX; i++)
    {
        visit[i] = 0;
    }
    visit[g_startVex] = 1;

    do
    {
        if (false == UpdateEdges(pEdges, MAX_QUEUE - 1))
        {
            return NULL;
        }
        pNode = g_edgeList[pEdges[MAX_QUEUE-1]->edgeInfo.destVex].pFirstEdge;
    } while (NULL == pNode);

    *costSum = 0;
    for (i = 0; i < MAX_QUEUE; i++)
    {
        vexStack[i+2] = pEdges[i]->edgeInfo.destVex;
        edgeStack[i+1] = &(pEdges[i]->edgeInfo);
        if (1 == visit[vexStack[i+2]])
        {
            TEST("visit %d\n", vexStack[i+2]);
            goto UPDATE;
        }
        visit[vexStack[i+2]]= 1;
        *costSum += pEdges[i]->edgeInfo.edgeCost;
    }
    *stackTop = MAX_QUEUE + 1;
    *stackTopVex = vexStack[*stackTop];
    *isFirstEdge = false;
    return pNode;
}


/****************************************
函数名称：
函数功能：计算浅层DFS路径数
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
bool CountEdges(EdgeNode *pEdges[], int QueueLayer, int *count)
{
    if (-1 == QueueLayer)
    {
        return false;
    }
    pEdges[QueueLayer] = pEdges[QueueLayer]->pNext;
    while (NULL == pEdges[QueueLayer])
    {
        if(!UpdateEdges(pEdges, QueueLayer-1))
        {
            return false;
        }
        pEdges[QueueLayer] = g_edgeList[pEdges[QueueLayer-1]->edgeInfo.destVex].pFirstEdge;
    }

    for (int i = 0; i < MAX_QUEUE; i++)
    {
        for (int j = i + 1; j < MAX_QUEUE; j ++)
        {
            if (pEdges[i]->edgeInfo.destVex == pEdges[j]->edgeInfo.destVex
                || pEdges[i]->edgeInfo.destVex == g_startVex)
            {
                return true;
            }
        }
    }
    (*count)++;
    return true;
}


/****************************************
函数名称：
函数功能：DFS回溯均搜
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void SearchRouteByDFS()
{
    clock_t endTime;
    double duration;

    int i = 0;
    int visit[MAX_VEX] = {0};
    int vexStack[MAX_VEX+1];
    int stackTop = 1;
    int stackTopVex = g_startVex;
    bool isFirstEdge = true;

    EdgeInfo *edgeStack[MAX_VEX];     // 由于每个点只经过一次,所以最多走过MAX_VEX-1条边. 且stack[0]不压入数据.
    EdgeNode *pEdge = g_edgeList[g_startVex].pFirstEdge;

    #ifdef _STARTVEX_BALANCE_DFS_
    EdgeNode *pStartEdge = pEdge;
    #endif

    visit[g_startVex] = 1;
    vexStack[stackTop] = g_startVex;    // vexStack[0]不压入数据,用于判断栈空. vexStack[1]存放起始点.
    edgeStack[0] = NULL;                // edgeStack[0]不压入数据. edgeStack[1]用于存放其实边,即起始点的出边.

    double remainTime = MAX_TIME;

    #ifdef _STARTVEX_BALANCE_DFS_
    double timeInterval = GetTimeInterval(pEdge, remainTime);
    double timeLimit = timeInterval;
    #endif

    int costSum = 0;
    memset(g_resultRoute, -1, sizeof(g_resultRoute));


    #ifdef _TWO_LAYER_DFS_
    EdgeNode *pEdges[MAX_QUEUE];
    EdgeNode *pEdgesCopy[MAX_QUEUE];
    int timeCount = 1;
    if (g_edgeNum > 400)
    {
        pEdges[0] = g_edgeList[g_startVex].pFirstEdge;
        pEdgesCopy[0] = g_edgeList[g_startVex].pFirstEdge;
        vexStack[2] = pEdges[0]->edgeInfo.destVex;
        edgeStack[1] = &(pEdges[0]->edgeInfo);
        visit[vexStack[2]] = 1;
        for (i = 1; i < MAX_QUEUE; i++)
        {
            pEdges[i] = g_edgeList[pEdges[i - 1]->edgeInfo.destVex].pFirstEdge;
            pEdgesCopy[i] = g_edgeList[pEdges[i - 1]->edgeInfo.destVex].pFirstEdge;
            vexStack[i + 2] = pEdges[i]->edgeInfo.destVex;
            edgeStack[i + 1] = &(pEdges[i]->edgeInfo);
            visit[vexStack[i + 2]] = 1;
        }
        stackTop = MAX_QUEUE + 1;
        stackTopVex = vexStack[stackTop];
        isFirstEdge = false;
        while (1)
        {
            if (!CountEdges(pEdgesCopy, MAX_QUEUE - 1, &timeCount))
            {
                break;
            }
        }
        timeInterval = remainTime / (double) timeCount;
        timeLimit = timeInterval;

        #ifdef _TEST_TWO_LAYER_DFS_
        TEST("%d %f\n", timeCount, timeInterval);
        PrintStack(vexStack, stackTop);
        #endif

        goto RENEW;
    }
    #endif

    DEBUG("after search: definition\n");
    do
    {
        if (isFirstEdge)    /* 如果是每个顶点邻接表的第一条边,则取出第一条边 */
        {
            pEdge = g_edgeList[stackTopVex].pFirstEdge;
            isFirstEdge = false;
        }
        else    /* 如果不是每个顶点的第一条边,则沿着链表往下 */
        {
            pEdge = pEdge->pNext;
        }

        #if (defined _TWO_LAYER_DFS_) || (defined _STARTVEX_BALANCE_DFS_)
        RENEW:
        #endif

        if (pEdge != NULL)
        {


            if (0 == visit[pEdge->edgeInfo.destVex])    /* 如果该边指向的点从未走过,即没有成环 */
            {
                DEBUG("No Circle\n");

                visit[pEdge->edgeInfo.destVex] = 1;
                stackTop++;
                /* vexStack[stackTop-1]  是 edgeStack[stackTop-1] 的起始点,
                 * vexStack[stackTop]    是 edgeStack[stackTop-1] 的指向点.
                 * edgeStack[stackTop-1] 是 vexStack[stackTop]    的入边,
                 * edgeStack[stackTop]   是 vexStack[stackTop]    的出边.   */
                vexStack[stackTop]  = pEdge->edgeInfo.destVex;
                edgeStack[stackTop-1] = &(pEdge->edgeInfo);
                stackTopVex = vexStack[stackTop];
                costSum += pEdge->edgeInfo.edgeCost;


                DEBUG("stackTop++\n");
                if (pEdge->edgeInfo.destVex == g_endVex)    /* 走到终点 */
                {
                    DEBUG("endVex\n");
                    int interNumCount = g_interNum;

                    for (i = 1; i < stackTop; i++)  // edgeStack[]的下标范围是[1]~[stackTop-1]
                    {
                        if (interNumCount != 0 && IsInterVex(vexStack[i]))      /* 是否经过所有V' */
                        {
                            interNumCount--;
                        }
                    }

                    if (0 == interNumCount)     /* 更新最小总权重 */
                    {
                        DEBUG("Right Route\n");
                        if (costSum < g_totalCost)
                        {
                            g_totalCost = costSum;
                            memset(g_resultRoute, -1, sizeof(g_resultRoute));
                            for (i = 1; i < stackTop; i++)
                            {
                                g_resultRoute[i-1] = edgeStack[i]->edgeID;
                            }

                            #ifdef _TEST_RESULT_
                            printf("Right Route. Least totalCost: %d\n", g_totalCost);
                            #endif
                        }
                    }

                    visit[g_endVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;

                }
                else if (costSum >= g_totalCost)
                {
                    DEBUG("greater costSum\n");

                    visit[pEdge->edgeInfo.destVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;
                }
                else
                {
                    DEBUG("Not End, Go On.\n");
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = true;
                }

                #if (defined _TWO_LAYER_DFS_) || (defined _STARTVEX_BALANCE_DFS_)
                if (1 == stackTop)
                {
                    timeLimit += timeInterval;
                    pStartEdge = pStartEdge->pNext;
                }
                #endif
            }
            endTime = clock();
            duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;

            #if (!defined _TWO_LAYER_DFS_) && (!defined _STARTVEX_BALANCE_DFS_)
            if (duration > remainTime)
            {
                break;
            }
            #else
            if (duration > timeLimit)
            {
                if (duration > remainTime)
                {
                    break;
                }
                timeLimit += timeInterval;

                #ifdef _TWO_LAYER_DFS_
                if (g_edgeNum > 400)
                {
                    for (i = 0; i < MAX_VEX; i++)
                    {
                    visit[i] = 0;
                    }
                    pEdge = UpdateStackInfo(pEdges, vexStack, visit, edgeStack, &stackTop, &stackTopVex, &isFirstEdge, &costSum);

                    #ifdef _TEST_TWO_LAYER_DFS_
                    PrintStack(vexStack, stackTop);
                    #endif

                    if (pEdge == NULL)
                    {
                        break;
                    }

                    goto RENEW;
                }
                #endif

                #ifdef _STARTVEX_BALANCE_DFS_
                pStartEdge = pStartEdge->pNext;
                if (NULL == pStartEdge)
                {
                    break;
                }

                isFirstEdge = false;
                stackTop = 1;
                stackTopVex = g_startVex;
                visit[g_startVex] = 1;
                pEdge = pStartEdge;

                costSum = 0;
                goto RENEW;
                #endif
            }
            #endif
        }
        else
        {
            DEBUG("pNode Null\n");

            visit[vexStack[stackTop]] = 0;
            stackTop--;

            if (stackTop != 0)
            {
                pEdge = g_edgeList[vexStack[stackTop]].pFirstEdge;
                while (pEdge->edgeInfo.destVex != stackTopVex)
                {
                    pEdge = pEdge->pNext;
                }
                stackTopVex = vexStack[stackTop];
                isFirstEdge = false;
                costSum -= pEdge->edgeInfo.edgeCost;
            }
        }
    } while (stackTop);
}


/****************************************
函数名称：
函数功能：计算均搜时间间隔
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
double GetTimeInterval(EdgeNode *pEdge, double remainTime)
{
    int count = 0;
    while (pEdge != NULL)
    {
        count++;
        pEdge = pEdge->pNext;
    }
    double timeInterval = remainTime / (double)count;
    return timeInterval;
}


/****************************************
函数名称：
函数功能：多层权重排序
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
int MultiLayerPseudoCost(EdgeInfo *pEdgeInfo, int layer)
{
    if (NULL == pEdgeInfo || 0 == layer)
    {
        return 0;
    }

    int ret = MultiLayerPseudoCost(&(g_edgeList[pEdgeInfo->destVex].pFirstEdge->edgeInfo), layer - 1);
    return GetPseudoCost(pEdgeInfo) + ret;
}


/****************************************
函数名称：
函数功能：多次多层权重排序
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void ReformMultiLayerAdjList(int loop, int layer)
{
    int i = 0;
    while (loop--)
    {
        for (i = 0; i < MAX_VEX; i++)
        {
            EdgeNode *pNode = g_edgeList[i].pFirstEdge;
            if (NULL == pNode)
            {
                continue;
            }
            while (pNode != NULL)
            {
                pNode->edgeInfo.edgePseudoCost = MultiLayerPseudoCost(&(pNode->edgeInfo), layer);
                //GetPseudoCost(&(pNode->edgeInfo))
                //+ GetPseudoCost(&(g_edgeList[pNode->edgeInfo.destVex].pFirstEdge->edgeInfo));
                pNode = pNode->pNext;
            }
        }
    }

    for (i = 0; i < MAX_VEX; i++)
    {
        if (NULL == g_edgeList[i].pFirstEdge || NULL == g_edgeList[i].pFirstEdge->pNext)
        {
            continue;
        }
        EdgeNode *pHead = g_edgeList[i].pFirstEdge;
        EdgeNode *pDumpHead = (EdgeNode *)malloc(sizeof(EdgeNode));
        pDumpHead->pNext = pHead;
        memset(&(pDumpHead->edgeInfo), 0, sizeof(EdgeInfo));
        EdgeNode *pNode = pHead->pNext;
        EdgeNode *pPre  = pHead;
        EdgeNode *pSubNode;
        EdgeNode *pSubPre;
        while (pNode != NULL)
        {
            pSubNode = pDumpHead->pNext;
            pSubPre  = pDumpHead;
            while (pSubNode != pNode)
            {
                if (pSubNode->edgeInfo.edgePseudoCost <= pNode->edgeInfo.edgePseudoCost)
                {
                    pSubNode = pSubNode->pNext;
                    pSubPre  = pSubPre->pNext;
                }
                else
                {
                    pSubPre->pNext = pNode;
                    pPre->pNext    = pNode->pNext;
                    pNode->pNext   = pSubNode;
                    break;
                }
            }
            if (pSubNode == pNode)
            {
                pNode = pNode->pNext;
                pPre = pPre->pNext;
            }
            else
            {
                pNode = pPre->pNext;
            }
        }
        g_edgeList[i].pFirstEdge = pDumpHead->pNext;
        free(pDumpHead);
    }
}


/****************************************
函数名称：
函数功能：均搜回溯拼接SPFA最短路,寻中符合要求的路径
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void SearchRouteByBalanceSPFA(int demand_id)
{
    clock_t endTime;
    double duration;

    int visit[MAX_VEX] = {0};
    int vexStack[MAX_VEX+1];
    int stackTop = 1;
    int stackTopVex = g_startVex;
    bool isFirstEdge = true;

    EdgeInfo *edgeStack[MAX_VEX];     // 由于每个点只经过一次,所以最多走过MAX_VEX-1条边. 且stack[0]不压入数据.
    EdgeNode *pEdge = g_spfaEdgeList[g_startVex].pFirstEdge;
    EdgeNode *pStartEdge = pEdge;

    visit[g_startVex] = 1;
    vexStack[stackTop] = g_startVex;    // vexStack[0]不压入数据,用于判断栈空. vexStack[1]存放起始点.
    edgeStack[0] = NULL;                // edgeStack[0]不压入数据. edgeStack[1]用于存放其实边,即起始点的出边.


    int i = 0;

    double remainTime = MAX_TIME;
    double timeInterval = GetTimeIntervalForBalanceSPFA(pEdge, remainTime);
    double timeLimit = timeInterval;

    int costSum = 0;

    int visitInt[MAX_VEX] = {0};
    visitInt[g_startVex] = 1;

    do
    {
        if (isFirstEdge)
        {
            pEdge = g_spfaEdgeList[stackTopVex].pFirstEdge;
            isFirstEdge = false;
        }
        else
        {
            pEdge = pEdge->pNext;
        }

        /*
        while (pEdge != NULL && g_edges[pEdge->edgeInfo.edgeID].edgeCost == -1)
        {
            pEdge = pEdge->pNext;
        }
        */

        RENEW:

        if (pEdge != NULL)
        {
            if (0 == visit[pEdge->edgeInfo.destVex])
            {
                visit[pEdge->edgeInfo.destVex] = 1;
                stackTop++;
                /* vexStack[stackTop-1]  是 edgeStack[stackTop-1] 的起始点,
                 * vexStack[stackTop]    是 edgeStack[stackTop-1] 的指向点.
                 * edgeStack[stackTop-1] 是 vexStack[stackTop]    的入边,
                 * edgeStack[stackTop]   是 vexStack[stackTop]    的出边    */
                vexStack[stackTop]  = pEdge->edgeInfo.destVex;
                edgeStack[stackTop-1] = &(pEdge->edgeInfo);
                costSum += pEdge->edgeInfo.edgeCost;

                if (!EnVisit(visitInt, edgeStack[stackTop-1]->vexSegHead->pNext))
                {
                    goto BACKWARD;
                }

                if (pEdge->edgeInfo.destVex == g_endVex)
                {

                    if (g_interNum + 2 == stackTop)
                    {
                        if (costSum < g_totalCost)
                        {
                            int n = 0;
                            IntNode *pIntVex = NULL;
                            IntNode *pIntEdge = NULL;
                            for (i = 1; i < stackTop; i++)
                            {
                                pIntVex = edgeStack[i]->vexSegHead->pNext;
                                pIntEdge = edgeStack[i]->edgeSegHead;
                                while (pIntEdge != NULL)
                                {
                                    g_resultRoute[n] = pIntEdge->data;
                                    pIntEdge = pIntEdge->pNext;
                                    n++;
                                }
                            }
                            g_resultRoute[n] = -1;
                            g_totalCost = costSum;
                            #ifdef _TEST_RESULT_
                            cout << "New Least Cost " << g_totalCost << endl;
                            #endif
                            if (demand_id == 0)
                            {
                                return;
                            }
                        }
                    }

                    visit[g_endVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;

                    if (stackTop > 0)
                    {
                        DeVisit(visitInt, edgeStack[stackTop]->vexSegHead->pNext);
                    }
                }
                else if (costSum >= g_totalCost)
                {
                    DeVisit(visitInt, edgeStack[stackTop-1]->vexSegHead->pNext);

                    BACKWARD:

                    visit[pEdge->edgeInfo.destVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;
                }
                else
                {
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = true;
                }


                if (1 == stackTop)
                {
                    timeLimit += timeInterval;
                    pStartEdge = pStartEdge->pNext;
                }

            }

            endTime = clock();
            duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;

            if (duration > timeLimit)
            {
                if (duration > remainTime)
                {
                    break;
                }
                timeLimit += timeInterval;
                pStartEdge = pStartEdge->pNext;
                if (NULL == pStartEdge)
                {
                    break;
                }
                for (i = 0; i < MAX_VEX; i++)
                {
                    visit[i] = 0;
                    visitInt[i] = 0;
                }
                isFirstEdge = false;
                stackTop = 1;
                stackTopVex = g_startVex;
                visit[g_startVex] = 1;
                visitInt[g_startVex] = 0;
                pEdge = pStartEdge;
                costSum = 0;

                goto RENEW;
            }

        }
        else
        {
            visit[vexStack[stackTop]] = 0;
            stackTop--;

            if (stackTop != 0)
            {
                DeVisit(visitInt, edgeStack[stackTop]->vexSegHead->pNext);

                pEdge = g_spfaEdgeList[vexStack[stackTop]].pFirstEdge;
                while (pEdge->edgeInfo.destVex != stackTopVex)
                {
                    pEdge = pEdge->pNext;
                }
                stackTopVex = vexStack[stackTop];
                isFirstEdge = false;
                costSum -= pEdge->edgeInfo.edgeCost;
            }
        }
    } while (stackTop);
}


/****************************************
函数名称：
函数功能：计算<均搜回溯拼接SPFA最短路>的均搜时间间隔
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
double GetTimeIntervalForBalanceSPFA(EdgeNode *pEdge, double remainTime)
{
    int count = 0;
    while (pEdge != NULL)
    {
        count++;
        pEdge = pEdge->pNext;
    }
    double timeInterval = remainTime / (double)count;
    return timeInterval;
}


/****************************************
函数名称：
函数功能：测试打印
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void PrintSPFALeastCost(int endVex)
{
    if (g_spfaLeastCost[endVex] == INF)
        printf("NA\n");
    else
        printf("%d\n", g_spfaLeastCost[endVex]);
}


void PrintPathByVex(int i)
{
    if(g_vexPath[i] != -1)
        PrintPathByVex(g_vexPath[i]);
    cout<<i<<' ';
}


void PrintPathByEdge(int i)
{
    if(g_edgePath[i] != -1)
    {
        PrintPathByEdge(g_edgePath[i]);
        cout<<'|';
    }
    cout<<i;
}


void PrintStack(int vexStack[], int stackTop)
{
    int i = 1;
    for (i = 1; i <= stackTop; i++)
    {
        cout<<vexStack[i]<<" ";
    }
    cout<<endl;
}


void PrintList()
{
    EdgeNode *pNode = NULL;
    for (int i = 0; i < MAX_VEX; i++)
    {
        pNode = g_spfaEdgeList[i].pFirstEdge;
        if (pNode == NULL)
        {
            continue;
        }
        while (pNode != NULL)
        {
            cout << pNode->edgeInfo.srcVex << "->" << pNode->edgeInfo.destVex << endl;
            pNode = pNode->pNext;
        }
        cout << endl;
    }
}


void PrintIntNode(IntNode *pHead)
{
    IntNode *pNode = pHead;
    while (pNode != NULL)
    {
        cout<<pNode->data<<" ";
        pNode = pNode->pNext;
    }
    cout<<endl;
}


void PrintVisit(int visit[])
{
    for (int i = 0; i < 50; i++)
    {
        cout<<visit[i];
    }
    cout<<endl;
}
