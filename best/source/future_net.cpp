#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <queue>
#include <time.h>
#include "future_net.h"
using namespace std;


const int MAXV = 600;
const int MAXE = 4800;
const int INF  = 0x7FFFFFFF;

typedef struct Edge
{  
    int v;  
    int next;  
    int cost;  
}Edge;

Edge e[MAXE];
int p[MAXE];
int Dis[MAXE];
bool vist[MAXV];
queue<int> q;  

int pathv[MAXV];
int vpree[MAXV];
int pathe[MAXE];

/*
typedef struct EdgeInfo
{
    int destVex;    // 目标顶点
    int srcVex;     // 起始定点
    int edgeID;     // 有向边编号
    int edgeCost;   // 有向边权重
    IntNode *vexSegHead;
    IntNode *vexSegTail;
    IntNode *edgeSegHead;
    IntNode *edgeSegTail;
} EdgeInfo;
*/

/* 有向边链表节点 */
/*
typedef struct EdgeNode
{
    struct EdgeInfo edgeInfo;   // 有向边信息
    struct EdgeNode *pNext;
} EdgeNode;
*/

/* 邻接表 各链表头节点 存储第一条边 */
/*
typedef struct AdjListHead
{
    struct EdgeNode *pFirstEdge;
} AdjListHead;
*/

AdjListHead *g_spfaEdgeList = NULL;
AdjListHead *g_edgeList = NULL;

int g_startVex;
int g_endVex;

int g_spfaInterVex[MAXV];
int g_interVex[MAX_INTER];

int g_interNum = 0;
int g_totalCost = INF;
int g_resultRoute[MAX_VEX+1];
clock_t g_startTime;

VexInfo **g_vexInfo;
int g_edgeNum = 0;

void init(FILE *fp_topo, FILE *fp_demand)
{
    int eid = 0;
    int from,to,cost;
    memset(p, -1, sizeof(p));
    memset(g_resultRoute, -1, sizeof(g_resultRoute));

    while (fscanf(fp_topo, "%d,%d,%d,%d\n", &eid, &from, &to, &cost) > 0)
    {
        e[eid].next = p[from];  
        e[eid].v    = to;  
        e[eid].cost = cost;  
        p[from]     = eid;
        //printf("## %d %d %d %d\n", eid, e[eid].next, e[eid].v, e[eid].cost);
    }

    fscanf(fp_demand, "%d,%d,", &g_startVex, &g_endVex);
    memset(g_spfaInterVex, 0, sizeof(g_spfaInterVex));
    int num;
    while (fscanf(fp_demand, "%d|", &num) > 0)
    {
        g_spfaInterVex[num] = 1;
        g_interVex[g_interNum] = num;
        g_interNum++;
    }

    g_spfaEdgeList = (AdjListHead *)malloc(sizeof(AdjListHead)*(MAXV));
    for (int i = 0; i < MAXV; i++)
    {
        g_spfaEdgeList[i].pFirstEdge = NULL;
        vist[i] = false;
    }
}

void print(int End)  
{  
    if (Dis[End] == INF)
        printf("NA\n");
    else
        printf("%d\n", Dis[End]);      
}

void PrintPathV(int k)
{
    if(pathv[k] != -1) 
        PrintPathV(pathv[k]);
    cout<<k<<' ';
}

void PrintPathE(int k)
{
    if(pathe[k] != -1) 
    {
        PrintPathE(pathe[k]);
        cout<<'|';
    }
    cout<<k;
}


void spfa_InsertEdgeNode(EdgeNode **pFirstEdge, EdgeInfo *pInfo)
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


void PrintList()
{
    EdgeNode *pNode = NULL;
    for (int i = 0; i < MAXV; i++)
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


void GetVexSegment(int k, IntNode **pHead, IntNode **pTail)
{
    if(pathv[k] != -1)
    {
        GetVexSegment(pathv[k], pHead, pTail);
    }
    //cout<<k<<' ';
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

void GetEdgeSegment(int k, IntNode **pHead, IntNode **pTail, int *pCost)
{
    if(pathe[k] != -1)
    {
        GetEdgeSegment(pathe[k], pHead, pTail, pCost);
        //cout<<'|';
    }
    //cout<<k;
    IntNode *pNew = (IntNode *)malloc(sizeof(IntNode));
    pNew->data = k;
    pNew->pNext = NULL;
    *pCost += e[k].cost;
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

void SPF(int Start, int End)
{
    memset(pathv, -1, sizeof(pathv));
    memset(vpree, -1, sizeof(vpree));
    memset(pathe, -1, sizeof(pathe));
    fill(Dis, Dis+MAXE, INF);
    while (!q.empty())
    {
        q.pop();
    }


    Dis[Start]  = 0;  
    vist[Start] = true;
    q.push(Start);  
    while (!q.empty())  
    {  
        int t = q.front();  
        q.pop();  
        vist[t] = false;  
        int j;

        for (j=p[t]; j!=-1; j=e[j].next)
        {
            int w = e[j].cost;
            if (w + Dis[t] < Dis[e[j].v])  
            {
                Dis[e[j].v] = w + Dis[t];
                pathv[e[j].v] = t;
                vpree[e[j].v] = j;
                pathe[j] = vpree[t];
                //printf("E: %d, V: %d %d\n", j, e[j].v, t);
                if (!vist[e[j].v]
                    && (e[j].v != g_startVex && e[j].v != g_endVex)
                    && (g_spfaInterVex[e[j].v] == 0 || g_spfaInterVex[e[j].v] == End))
                {  
                    vist[e[j].v] = true;  
                    q.push(e[j].v);  
                }  
            }  
        }
    }
    //print(End);
    //PrintPathV(End);
    //printf("\n");
    //PrintPathE(vpree[End]);
    //printf("\n");
    EdgeInfo *pInfo;
    for (int i = 0; i < MAXV; i++)
    {
        if ((1 == g_spfaInterVex[i] || i == g_endVex) && i != Start)
        {
            if (Dis[i] != INF)
            {
                //PrintPathV(i);
                //printf("\n");
                //PrintPathE(vpree[i]);
                //printf("\n");

                pInfo = (EdgeInfo *) malloc(sizeof(EdgeInfo));
                pInfo->edgeID   = -1;
                pInfo->srcVex   = Start;
                pInfo->destVex  = i;
                pInfo->edgeCost = 0;
                pInfo->vexSegHead = NULL;
                pInfo->vexSegTail = NULL;
                GetVexSegment(i, &(pInfo->vexSegHead), &(pInfo->vexSegTail));
                //PrintIntNode(pInfo->vexSegHead);
                pInfo->edgeSegHead = NULL;
                pInfo->edgeSegTail = NULL;
                GetEdgeSegment(vpree[i], &(pInfo->edgeSegHead), &(pInfo->edgeSegTail), &(pInfo->edgeCost));
                //PrintIntNode(pInfo->edgeSegHead);
                spfa_InsertEdgeNode(&(g_spfaEdgeList[Start].pFirstEdge), pInfo);

            }
        }
    }
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



#ifndef _TEST_RESULT_
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
        i++;
        while (g_resultRoute[i] != -1)
        {
            fprintf(fp_result, "|%d", g_resultRoute[i]);
            i++;
        }
    }
}
#else
void PrintResultFile()
{
    if (-1 == g_resultRoute[0])
    {
        printf("NA\n");
    }
    else
    {
        int i = 0;
        printf("%d", g_resultRoute[i]);
        i++;
        while (g_resultRoute[i] != -1)
        {
            printf("|%d", g_resultRoute[i]);
            i++;
        }
    }
    printf("\nTotal Cost: %d\n", g_totalCost);
}
#endif


void PrintVisit(int visit[])
{
    for (int i = 0; i < 50; i++)
    {
        cout<<visit[i];
    }
    cout<<endl;
}


bool EnVisit(int visit[], IntNode *pHead)
{
    visit[599] = 1;
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


void DeVisit(int visit[], IntNode *pHead)
{
    IntNode *pNode = pHead;
    while (pNode != NULL)
    {
        visit[pNode->data] = 0;
        pNode = pNode->pNext;
    }
}




void spfa_SearchRoute()
{
    clock_t endTime;
    double duration;


    char visit[MAX_VEX] = {0};
    int vexStack[MAX_VEX+1];
    int stackTop = 1;
    int stackTopVex = g_startVex;
    bool isFirstEdge = true;

    EdgeInfo *edgeStack[MAX_VEX];     // 由于每个点只经过一次,所以最多走过MAX_VEX-1条边. 且stack[0]不压入数据.
    EdgeNode *pEdge = g_spfaEdgeList[g_startVex].pFirstEdge;
    //EdgeNode *pStartEdge = pEdge;

    visit[g_startVex] = 1;
    vexStack[stackTop] = g_startVex;    // vexStack[0]不压入数据,用于判断栈空. vexStack[1]存放起始点.
    edgeStack[0] = NULL;                // edgeStack[0]不压入数据. edgeStack[1]用于存放其实边,即起始点的出边.


    int i = 0;

    //double remainTime = 9.9;
    //double timeInterval = GetTimeInterval(pEdge, remainTime);
    //double timeLimit = timeInterval;

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

        RENEW:

        if (pEdge != NULL)
        {
            //sPrintStack(vexStack, stackTop);

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
                    /*
                    int interNumCount = g_interNum;
                    //costSum = 0;
                    for (i = 1; i < stackTop; i++)  // edgeStack[]的下标范围是[1]~[stackTop-1]
                    {
                        if (interNumCount != 0 && IsInterVex(vexStack[i]))
                        {
                            interNumCount--;
                        }
                        //costSum += edgeStack[i]->edgeCost;
                    }

                    if (0 == interNumCount)
                    {
                        if (costSum < g_totalCost)
                        {
                            g_totalCost = costSum;
                            memset(g_resultRoute, -1, sizeof(g_resultRoute));
                            for (i = 1; i < stackTop; i++)
                            {
                                g_resultRoute[i-1] = edgeStack[i]->edgeID;
                            }
                        }
                    }
                    */

                    /*
                    if (g_interNum + 2 == stackTop)
                    {
                        g_totalCost = costSum;
                        memset(g_resultRoute, -1, sizeof(g_resultRoute));
                        for (i = 1; i < stackTop; i++)
                        {
                            g_resultRoute[i - 1] = edgeStack[i]->edgeID;
                        }
                    }
                    */

                    if (g_interNum + 2 == stackTop)
                    {
                        if (costSum < g_totalCost)
                        {
                            //memset(visitInt, 0, sizeof(visitInt));
                            int n = 0;
                            IntNode *pIntVex = NULL;
                            IntNode *pIntEdge = NULL;
                            for (i = 1; i < stackTop; i++)
                            {
                                pIntVex = edgeStack[i]->vexSegHead->pNext;
                                pIntEdge = edgeStack[i]->edgeSegHead;
                                while (pIntEdge != NULL)
                                {
                                   // if (visitInt[pIntVex->data] == 0)
                                   // {
                                        g_resultRoute[n] = pIntEdge->data;
                                        pIntEdge = pIntEdge->pNext;
                                        n++;
                                        //visitInt[pIntVex->data] = 1;
                                        //pIntVex = pIntVex->pNext;
                                    //}
                                    //else
                                    //{
                                    //    goto WRONG_ANSWER;
                                    //}
                                }
                            }
                            g_resultRoute[n] = -1;
                            g_totalCost = costSum;
                            #ifdef _TEST_RESULT_
                            cout << "OKOKOKOKOK " << g_totalCost << endl;
                            #endif
                            //PrintResultFile(NULL);
                        }
                    }
                    WRONG_ANSWER:

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

                /*
                if (1 == stackTop)
                {
                    timeLimit += timeInterval;
                    pStartEdge = pStartEdge->pNext;
                }
                */
            }

            endTime = clock();
            duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
            if (duration > 9.9)
            {
                break;
            }
            /*
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
                isFirstEdge = false;
                stackTop = 1;
                stackTopVex = g_startVex;
                visit[g_startVex] = 1;
                pEdge = pStartEdge;
                costSum = 0;

                goto RENEW;
            }
            */
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





int main(int argc, char *argv[])
{  
    char *str = (char*)malloc(1024);
    memset(str, 0, 1024);
    strcpy(str, argv[1]);
    FILE *fp_topo = fopen(str, "r");
    if (fp_topo == NULL)
        exit(1);

    memset(str, 0, 1024);
    strcpy(str, argv[2]);
    FILE *fp_demand = fopen(str, "r");
    if (fp_demand == NULL)
        exit(1);

    #ifndef _TEST_RESULT_
    memset(str, 0, 1024);
    strcpy(str, argv[3]);
    FILE *fp_result = fopen(str, "w");
    if (fp_result == NULL)
        exit(1);
    #endif

    init(fp_topo, fp_demand);
    InitTopoFile(fp_topo);

    if (g_interNum > 40)
    {
        for (int i = 0; i < MAXV; i++)
        {
            if (1 == g_spfaInterVex[i] || i == g_startVex)
            {
                SPF(i, 0);
            }
        }

        spfa_SearchRoute_11();
    }
    else
    {
        SearchRoute();

        #ifdef _TEST_RESULT_
        clock_t endTime;
        double duration;
        endTime = clock();
        duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
        cout<<endl<<"Time: "<<duration<<"s, Total Cost: "<<g_totalCost<<endl;
        #endif

        for (int i = 0; i < MAXV; i++)
        {
            if (1 == g_spfaInterVex[i] || i == g_startVex)
            {
                SPF(i, 0);
            }
        }

        if (g_edgeNum > 1200)
        {
            spfa_SearchRoute_11();
        }
        else
        {
            spfa_SearchRoute();
        }
    }


    #ifdef _TEST_RESULT_
    PrintResultFile();
    #else
    PrintResultFile(fp_result);
    #endif

    #ifdef _TEST_RESULT_
    endTime = clock();
    duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
    cout<<endl<<"Time: "<<duration<<"s, Total Cost: "<<g_totalCost<<endl;
    #endif


    fclose(fp_topo);
    fclose(fp_demand);
    #ifndef _TEST_RESULT_
    fclose(fp_result);
    #endif

    free(str);
    return 0;
}




///////////////////////


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
    fseek(fp_topo, 0, SEEK_SET);
    while (fscanf(fp_topo, "%d,%d,%d,%d\n", &edgeID, &srcVex, &destVex, &edgeCost) > 0)
    {
        if (!IsDupEdge(g_edgeList[srcVex].pFirstEdge, edgeID, destVex, edgeCost))
        {
            pInfo = (EdgeInfo *)malloc(sizeof(EdgeInfo));
            pInfo->edgeID   = edgeID;
            pInfo->srcVex   = srcVex;
            pInfo->destVex  = destVex;
            pInfo->edgeCost = edgeCost;
            pInfo->vexSegHead  = NULL;
            pInfo->vexSegTail  = NULL;
            pInfo->edgeSegHead = NULL;
            pInfo->edgeSegTail = NULL;
            InsertEdgeNode(&(g_edgeList[srcVex].pFirstEdge), pInfo);
            g_edgeNum++;
        }
    }

#ifdef _JUDGE_MULTI_LAYER_ADJ_
    ReformMultiLayerAdjList(2, 2);
#endif

}




/****************************************
函数名称：
函数功能：
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
        if(false == UpdateEdges(pEdges, QueueLayer-1))
        {
            return false;
        }
        pEdges[QueueLayer] = g_edgeList[pEdges[QueueLayer-1]->edgeInfo.destVex].pFirstEdge;
    }
    return true;
}

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

bool CountEdges(EdgeNode *pEdges[], int QueueLayer, int *count)
{
    if (-1 == QueueLayer)
    {
        return false;
    }
    pEdges[QueueLayer] = pEdges[QueueLayer]->pNext;
    while (NULL == pEdges[QueueLayer])
    {
        if(false == UpdateEdges(pEdges, QueueLayer-1))
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

void SearchRoute()
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
    EdgeNode *pStartEdge = pEdge;

    visit[g_startVex] = 1;
    vexStack[stackTop] = g_startVex;    // vexStack[0]不压入数据,用于判断栈空. vexStack[1]存放起始点.
    edgeStack[0] = NULL;                // edgeStack[0]不压入数据. edgeStack[1]用于存放其实边,即起始点的出边.

    double remainTime = 2.9;
    double timeInterval = GetTimeInterval(pEdge, remainTime);
    double timeLimit = timeInterval;

    int costSum = 0;
    g_totalCost = 1200;
    memset(g_resultRoute, -1, sizeof(g_resultRoute));

#ifdef _JUDGE_MAX_INTER_
    int interSum = 0;
#endif

#ifdef _BFS_
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
#ifdef _TEST_BFS_
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

                            if (g_edgeNum > 400)
                            {
                                break;
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

                for (i = 0; i < MAX_VEX; i++)
                {
                    visit[i] = 0;
                }

#ifdef _JUDGE_MAX_INTER_
                interSum = 0;
                ClearMaxInterToVexInfo(vexStack, stackTop);
#endif

                if (g_edgeNum > 400)
                {
                    pEdge = UpdateStackInfo(pEdges, vexStack, visit, edgeStack, &stackTop, &stackTopVex, &isFirstEdge, &costSum);
#ifdef _TEST_BFS_
                    PrintStack(vexStack, stackTop);
#endif
                    if (pEdge == NULL)
                    {
                        break;
                    }

                    goto RENEW;
                }

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



// ======================
void spfa_SearchRoute_11()
{
    clock_t endTime;
    double duration;


    char visit[MAX_VEX] = {0};
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

    double remainTime = 9.9;
    double timeInterval = GetTimeInterval_11(pEdge, remainTime);
    double timeLimit = timeInterval;
    //cout<<timeInterval<<endl;

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

        RENEW:

        if (pEdge != NULL)
        {
            //PrintStack(vexStack, stackTop);

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
                            //memset(visitInt, 0, sizeof(visitInt));
                            int n = 0;
                            IntNode *pIntVex = NULL;
                            IntNode *pIntEdge = NULL;
                            for (i = 1; i < stackTop; i++)
                            {
                                pIntVex = edgeStack[i]->vexSegHead->pNext;
                                pIntEdge = edgeStack[i]->edgeSegHead;
                                while (pIntEdge != NULL)
                                {
                                    // if (visitInt[pIntVex->data] == 0)
                                    // {
                                    g_resultRoute[n] = pIntEdge->data;
                                    pIntEdge = pIntEdge->pNext;
                                    n++;
                                    //visitInt[pIntVex->data] = 1;
                                    //pIntVex = pIntVex->pNext;
                                    //}
                                    //else
                                    //{
                                    //    goto WRONG_ANSWER;
                                    //}
                                }
                            }
                            g_resultRoute[n] = -1;
                            g_totalCost = costSum;
                            #ifdef _TEST_RESULT_
                            cout << "OKOKOKOKOK " << g_totalCost << endl;
                            #endif
                            //PrintResultFile(NULL);
                        }
                    }
                    WRONG_ANSWER:

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



double GetTimeInterval_11(EdgeNode *pEdge, double remainTime)
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


