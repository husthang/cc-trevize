#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "future_net.h"

/* 宏 控制编译版本 */
/* debug信息打印 */
//#define _DEBUG_
#ifdef  _DEBUG_
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif
/* 测试信息打印 */
#define _TEST_
#ifdef  _TEST_
#define TEST(format,...) printf(format, ##__VA_ARGS__)
#else
#define TEST(format,...)
#endif
/* 从interVex往外搜索 */
//#define _VEX_LEVEL_
/* 按是否为中间点v'和权重大小两项排序邻接表 */
#define _PSEUDO_COST_

//#define _JUDGE_MAX_INTER_

//#define _JUDGE_DISCARD_

#define _JUDGE_MULTI_LAYER_ADJ_


/* 全局变量 */
/* 邻接表 存储有向边 */
AdjListHead *g_edgeList = NULL;
/* 中间点集v' */
int g_interVex[MAX_INTER];
/* 中间点数目 */
int g_interNum = 0;
/* 起始点编号 */
int g_startVex;
/* 终止点编号 */
int g_endVex;
/* 总权重最小值 */
int g_totalCost = INF;
/* 计时器 起始时间 */
clock_t g_startTime;
/* 输出路径 */
int g_resultRoute[MAX_VEX+1];
/* 顶点信息数组 */
VexInfo **g_vexInfo;


int main(int argc, char *argv[])
{
    FILE *fp_result = Initalize(argc, argv); DEBUG("after init\n");
    SearchRoute(); DEBUG("after search\n");
    PrintResultFile(fp_result); DEBUG("after result\n");
    fclose(fp_result);
    TEST("\nTotal Cost: %d\n", g_totalCost);
    return 0;
}

/****************************************
函数名称：
函数功能：打印状态信息
输入参数：状态信息返回码
输出参数：
返回值：
修改情况：
*****************************************/
void PrintStatusInfo(StatusInfo statusInfo)
{
    switch (statusInfo)
    {
        case SYS_ERROR:
            DEBUG("SYS_ERROR: System Error\n");
            break;
        default:
            break;
    }
}

/****************************************
函数名称：
函数功能：程序初始化
输入参数：
输出参数：
返回值：  result.csv文件地址
修改情况：
*****************************************/
FILE *Initalize(int argc, char *argv[])
{
    g_startTime = clock();

    char *str = (char*)malloc(MAX_STR);
    if (NULL == str)
    {
        PrintStatusInfo(SYS_ERROR);
        exit(1);
    }

    memset(str, 0, MAX_STR);
    strcpy(str, "topo.csv");
    if (argv[1] != NULL)
    {
        strcpy(str, argv[1]);
    }
    FILE *fp_topo = fopen(str, "r");
    if (NULL == fp_topo)
    {
        PrintStatusInfo(SYS_ERROR);
        exit(1);
    }

    memset(str, 0, MAX_STR);
    strcpy(str, "demand.csv");
    if (argv[1] != NULL && argv[2] != NULL)
    {
        strcpy(str, argv[2]);
    }
    FILE *fp_demand = fopen(str, "r");
    if (NULL == fp_demand)
    {
        PrintStatusInfo(SYS_ERROR);
        exit(1);
    }
    
    memset(str, 0, MAX_STR);
    strcpy(str, "result.csv");
    if (argv[1] != NULL && argv[2] != NULL && argv[3] != NULL)
    {
        strcpy(str, argv[3]);
    }
    FILE *fp_result = fopen(str, "w");
    if (NULL == fp_result)
    {
        PrintStatusInfo(SYS_ERROR);
        exit(1);
    }

    InitDemandFile(fp_demand);
    InitTopoFile(fp_topo);
    #if (defined _JUDGE_MULTI_LAYER_ADJ_) && (defined _TEST_)
    PrintAdjList();
    #endif
    #if (defined _JUDGE_MAX_INTER_) || (defined _JUDGE_DISCARD_)
    InitVexInfo();
    #endif
    #ifdef _JUDGE_MULTI_LAYER_ADJ_
    ReformMultiLayerAdjList(2, 2);
    #endif
    #if (defined _JUDGE_MULTI_LAYER_ADJ_) && (defined _TEST_)
    PrintAdjList();
    #endif

    free(str);
    fclose(fp_topo);
    fclose(fp_demand);

    return fp_result;
}

/****************************************
函数名称：
函数功能：demand.csv文件初始化
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void InitDemandFile(FILE *fp_demand)
{
    fscanf(fp_demand, "%d,%d,", &g_startVex, &g_endVex);
    memset(g_interVex, -1, sizeof(g_interVex));
    g_interNum = 0;
    while (fscanf(fp_demand, "%d|", &g_interVex[g_interNum]) > 0)
    {
        g_interNum++;
    }
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
        PrintStatusInfo(SYS_ERROR);
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
函数功能：topo.csv文件初始化
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void InitTopoFile(FILE *fp_topo)
{
    int i = 0;
    g_edgeList = (AdjListHead *)malloc(sizeof(AdjListHead)*MAX_VEX);

    for (i = 0; i < MAX_VEX; i++)
    {
        g_edgeList[i].pFirstEdge = NULL;

        #ifdef _VEX_LEVEL_
        if (IsInterVex(i))
        {
            //vexlevel[i] = 0;
        }
        else
        {
            //vexlevel[i] = -1;
        }
        #endif
    }

    int edgeID;
    int srcVex;
    int destVex;
    int edgeCost;
    EdgeInfo *pInfo;
    while (fscanf(fp_topo, "%d,%d,%d,%d\n", &edgeID, &srcVex, &destVex, &edgeCost) > 0)
    {
        if (!IsDupEdge(g_edgeList[srcVex].pFirstEdge, edgeID, destVex, edgeCost))
        {
            pInfo = (EdgeInfo *)malloc(sizeof(EdgeInfo));
            pInfo->edgeID   = edgeID;
            pInfo->srcVex   = srcVex;
            pInfo->destVex  = destVex;
            pInfo->edgeCost = edgeCost;
            InsertEdgeNode(&(g_edgeList[srcVex].pFirstEdge), pInfo);
        }
    }
}

/****************************************
函数名称：
函数功能：寻找符合条件的通路
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void SearchRoute()
{
    clock_t endTime;
    double duration;


    char visit[MAX_VEX] = {0};
    int vexStack[MAX_VEX+1];
    int stackTop = 1;
    int stackTopVex = g_startVex;
    bool isFirstEdge = true;

    EdgeInfo *edgeStack[MAX_VEX];     // 由于每个点只经过一次,所以最多走过MAX_VEX-1条边. 且stack[0]不压入数据.
    EdgeNode *pEdge = g_edgeList[g_startVex].pFirstEdge;
    EdgeNode *pStartEdge = pEdge;

    visit[g_startVex] = 1;
    vexStack[stackTop] = g_startVex;    // vexStack[0]不压入数据,用于判断栈空. vexStack[1]存放起始点.
    edgeStack[0] = NULL;                // edgeStack[0]不压入数据. edgeStack[1]用于存放其实边,即起始点的出边.


    int i = 0;

    double remainTime = 9.9;
    double timeInterval = GetTimeInterval(pEdge, remainTime);
    double timeLimit = timeInterval;

    int costSum = 0;
    g_totalCost = 1200;
    memset(g_resultRoute, -1, sizeof(g_resultRoute));
    int interSum = 0;

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

        RENEW:

        if (pEdge != NULL)
        {
            #ifdef _JUDGE_MAX_INTER_
            TEST("InterSum: %d & MaxNum: %d\n", interSum, g_vexInfo[pEdge->edgeInfo.destVex]->maxInterNum);
            #endif

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

                #ifdef _JUDGE_MAX_INTER_
                if (IsInterVex(stackTopVex))
                {
                    interSum++;
                    AddMaxInterToVexInfo(vexStack, stackTop);
                }
                #endif

                DEBUG("stackTop++\n");
                if (pEdge->edgeInfo.destVex == g_endVex)    /* 走到终点 */
                {
                    DEBUG("endVex\n");
                    int interNumCount = g_interNum;
                    //costSum = 0;

                    #ifdef _JUDGE_DISCARD_
                    AddOnePassToVexInfo(vexStack, stackTop);
                    #endif

                    for (i = 1; i < stackTop; i++)  // edgeStack[]的下标范围是[1]~[stackTop-1]
                    {
                        if (interNumCount != 0 && IsInterVex(vexStack[i]))      /* 是否经过所有V' */
                        {
                            interNumCount--;
                        }
                        //costSum += edgeStack[i]->edgeCost;
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
                            TEST("Right Route. Least totalCost: %d\n", g_totalCost);
                        }
                    }

                    visit[g_endVex] = 0;
                    stackTop--;
                    stackTopVex = vexStack[stackTop];
                    isFirstEdge = false;
                    costSum -= pEdge->edgeInfo.edgeCost;

                    #ifdef _JUDGE_MAX_INTER_
                    UpdateAllMaxInterNum(vexStack, stackTop);
                    #endif
                }
                #ifdef _JUDGE_MAX_INTER_
                else if (costSum >= g_totalCost
                         || (g_vexInfo[stackTopVex]->isDone
                             && interSum + g_vexInfo[stackTopVex]->maxInterNum < g_interNum))
                #else
                else if (costSum >= g_totalCost)
                #endif
                {
                    DEBUG("greater costSum\n");

                    #ifdef _JUDGE_MAX_INTER_
                    UpdateMaxInterNum(stackTopVex);
                    if (IsInterVex(stackTopVex))
                    {
                        interSum--;
                        SubMaxInterToVexInfo(vexStack, stackTop);
                    }
                    g_vexInfo[stackTopVex]->maxInterTmp = 0;
                    #endif

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
                }

                #ifdef _JUDGE_MAX_INTER_
                interSum = 0;
                ClearMaxInterToVexInfo(vexStack, stackTop);
                #endif

                isFirstEdge = false;
                stackTop = 1;
                stackTopVex = g_startVex;
                visit[g_startVex] = 1;
                pEdge = pStartEdge;
                costSum = 0;

                goto RENEW;
            }
        }
        else
        {
            DEBUG("pNode Null\n");

            #ifdef _JUDGE_MAX_INTER_
            UpdateMaxInterNum(stackTopVex);
            if (IsInterVex(stackTopVex))
            {
                interSum--;
                SubMaxInterToVexInfo(vexStack, stackTop);
            }
            g_vexInfo[stackTopVex]->isDone = true;
            g_vexInfo[stackTopVex]->maxInterTmp = 0;
            #endif

            visit[vexStack[stackTop]] = 0;
            stackTop--;

            #ifdef _JUDGE_DISCARD_
            if (0 == g_vexInfo[stackTopVex]->passNum)
            {
                g_vexInfo[stackTopVex]->isDiscard = true;
                TEST("Discard %d\n",stackTopVex);
            }
            #endif

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
函数功能：计算时间间隔
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
函数功能：输出result.csv文件
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void PrintResultFile(FILE *fp_result)
{
    if (-1 == g_resultRoute[0])
    {
        fprintf(fp_result, "NA\n");
    }
    else
    {
        int i = 0;
        fprintf(fp_result, "%d", g_resultRoute[i]);
        TEST("%d", g_resultRoute[i]);
        i++;
        while (g_resultRoute[i] != -1)
        {
            fprintf(fp_result, "|%d", g_resultRoute[i]);
            TEST("|%d", g_resultRoute[i]);
            i++;
        }
    }
}

/****************************************
函数名称：
函数功能：
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void InitVexInfo()
{
    g_vexInfo = (VexInfo **)malloc(sizeof(VexInfo *) * MAX_VEX);
    for (int i = 0; i < MAX_VEX; i++)
    {
        g_vexInfo[i] = (VexInfo *)malloc(sizeof(VexInfo));
        g_vexInfo[i]->isDone      = false;
        g_vexInfo[i]->isDiscard   = false;
        g_vexInfo[i]->passNum     = 0;
        g_vexInfo[i]->minInterNum = 0;
        g_vexInfo[i]->maxInterNum = 0;
        g_vexInfo[i]->maxInterTmp = 0;
    }
}

/****************************************
函数名称：
函数功能：
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void AddOnePassToVexInfo(int vexStack[], int stackTop)
{
    int i = 0;
    for (i = 1; i <= stackTop; i++)
    {
        g_vexInfo[vexStack[i]]->passNum++;
    }
}

/****************************************
函数名称：
函数功能：
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
void AddMaxInterToVexInfo(int vexStack[], int stackTop)
{
    int i = 0;
    for (i = 1; i <= stackTop; i++)
    {
        g_vexInfo[vexStack[i]]->maxInterTmp++;
    }
}

void SubMaxInterToVexInfo(int vexStack[], int stackTop)
{
    int i = 0;
    for (i = 1; i <= stackTop; i++)
    {
        g_vexInfo[vexStack[i]]->maxInterTmp--;
    }
}

void ClearMaxInterToVexInfo(int vexStack[], int stackTop)
{
    int i = 0;
    for (i = 1; i <= stackTop; i++)
    {
        g_vexInfo[vexStack[i]]->maxInterTmp = 0;
    }
}

void UpdateMaxInterNum(int vex)
{
    if (g_vexInfo[vex]->maxInterNum < g_vexInfo[vex]->maxInterTmp)
    {
        g_vexInfo[vex]->maxInterNum = g_vexInfo[vex]->maxInterTmp;
        g_vexInfo[vex]->isDone = true;
    }
}

void UpdateAllMaxInterNum(int vexStack[], int stackTop)
{
    int i = 0;
    for (i = 1; i <= stackTop; i++)
    {
        if (g_vexInfo[i]->maxInterNum < g_vexInfo[i]->maxInterTmp)
        {
            g_vexInfo[i]->maxInterNum = g_vexInfo[i]->maxInterTmp;
            g_vexInfo[i]->isDone = true;
        }
    }
}

void PrintMaxInterNum()
{
    for (int i = 0; i < MAX_VEX; i++)
    {
        printf("%d %d\n", i, g_vexInfo[i]->maxInterNum);
    }
}

/****************************************
函数名称：
函数功能：
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

void PrintAdjList()
{
    int i = 0;
    EdgeNode *pNode = NULL;
    for (i = 0; i < MAX_VEX; i++)
    {
        pNode = g_edgeList[i].pFirstEdge;
        if (NULL == pNode)
        {
            continue;
        }
        while (pNode != NULL)
        {
            pNode->edgeInfo.edgePseudoCost =
                    GetPseudoCost(&(pNode->edgeInfo))
                    + GetPseudoCost(&(g_edgeList[pNode->edgeInfo.destVex].pFirstEdge->edgeInfo));
            printf("%d:%d ", pNode->edgeInfo.edgeID, pNode->edgeInfo.edgePseudoCost);
            pNode = pNode->pNext;
        }
        printf("\n");
    }
    printf("AdjList Over\n");
}
