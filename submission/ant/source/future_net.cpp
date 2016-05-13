#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "future_net.h"


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
int g_resultRoute[MAX_VEX+1] = {-1};
/* 顶点信息数组 */
VexInfo **g_vexInfo;
/* 总变数 */
int g_edgeNum = 0;
/*  */
Map g_Map[MAX_VEX][MAX_VEX];


int main(int argc, char *argv[])
{
    FILE *fp_result = Initalize(argc, argv);
    DEBUG("after init\n");

    #ifndef _ANT_
    SearchRoute();
    #else
    project TSP;
    TSP.NewProject();
    TSP.GetAnt();
    TSP.StartSearch();
    #endif

    DEBUG("after search\n");

    #ifndef _TEST_RESULT_
    PrintResultFile(fp_result);
    #else
    PrintResultFile();
    #endif
    DEBUG("after result\n");

    #ifndef _TEST_RESULT_
    fclose(fp_result);
    #endif
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

    #ifndef _TEST_RESULT_
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
    #else
    FILE *fp_result = NULL;
    #endif

    InitDemandFile(fp_demand);
    InitTopoFile(fp_topo);

    #if (defined _JUDGE_MULTI_LAYER_ADJ_) && (defined _TEST_MULTI_LAYER_ADJ_)
    PrintAdjList();
    #endif
    #if (defined _JUDGE_MAX_INTER_) || (defined _JUDGE_DISCARD_)
    InitVexInfo();
    #endif
    #ifdef _JUDGE_MULTI_LAYER_ADJ_
    ReformMultiLayerAdjList(2, 2);
    #endif
    #if (defined _JUDGE_MULTI_LAYER_ADJ_) && (defined _TEST_MULTI_LAYER_ADJ_)
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
#ifndef _ANT_
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
#else
bool IsDupEdge(const int edgeID, const int srcVex, const int destVex, const int edgeCost)
{
    if (g_Map[srcVex][destVex].edgeID != -1)
    {
        if (g_Map[srcVex][destVex].edgeCost > edgeCost)
        {
            g_Map[srcVex][destVex].edgeID = edgeID;
            g_Map[srcVex][destVex].edgeCost = edgeCost;

        }
        return true;
    }
    return false;
}
#endif

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

    #ifndef _ANT_
    g_edgeList = (AdjListHead *)malloc(sizeof(AdjListHead)*MAX_VEX);
    #endif

    for (i = 0; i < MAX_VEX; i++)
    {
        #ifndef _ANT_
        g_edgeList[i].pFirstEdge = NULL;
        #endif

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

        #ifdef _ANT_
        for (int j = 0; j < MAX_VEX; j++)
        {
            g_Map[i][j].edgeID = -1;
        }
        #endif
    }

    int edgeID;
    int srcVex;
    int destVex;
    int edgeCost;

    #ifndef _ANT_
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
            g_edgeNum++;
        }
    }
    #else
    DEBUG("Read Topo");
    while (fscanf(fp_topo, "%d,%d,%d,%d\n", &edgeID, &srcVex, &destVex, &edgeCost) > 0)
    {
        if (!IsDupEdge(edgeID, srcVex, destVex, edgeCost))
        {
            g_Map[srcVex][destVex].edgeID     = edgeID;
            g_Map[srcVex][destVex].Trial      = 20.0;
            g_Map[srcVex][destVex].deltaTrial = 0.0;
            g_Map[srcVex][destVex].rawCost    = edgeCost;
            if (IsInterVex(destVex))
            {
                g_Map[srcVex][destVex].destInter = 1;
                g_Map[srcVex][destVex].edgeCost  = edgeCost;
            }
            else
            {
                g_Map[srcVex][destVex].destInter = 0;
                g_Map[srcVex][destVex].edgeCost  = edgeCost + PSEUDO_ARG;
            }
        }
    }
    DEBUG("Read Topo Over");
    #endif
}

/****************************************
函数名称：
函数功能：双层DFS
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/

void PrintStack(int vexStack[], int stackTop)
{
    int i = 0;
    for (i = 1 ;i <= stackTop; i++)
    {
        TEST("%d ", vexStack[i]);
    }
    TEST("\n");
}

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
/****************************************
函数名称：
函数功能：蚁群
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
//获得指定范围内的一个随机数
double rnd(int low,double uper)
{
    double p=(rand()/(double)RAND_MAX)*((uper)-(low))+(low);
    return (p);
};

//获得指定上限范围内的一个随机数
int rnd(int uper)
{
    return (rand()%uper);
};

//清空数据，蚂蚁周游完各个城市后，要重新开始周游各个城市时调用
void ant::Clear()
{
    over = 0;
    m_nInterNum = g_interNum;
    m_dLength = 0.0;
    m_dRowLength = 0;
    int i = 0;
    for(i = 0; i < 8; i++)
    {
        prob[i] = 0.0;
        nextCity[i] = -1;
    }
    for (i = 0; i < MAX_VEX; i++)
    {
        visitCity[i] = 0;
        tabu[i] = -1;
    }

    //i = tabu[N_CITY_COUNT-1]; //用最后一个城市作为出发城市
	i = g_startVex;
    m_nCityCount = 0;
    AddCity(i);
}

//初始化
ant::ant()
{
    m_dLength=m_dShortest=0;
    m_dRowLength = 0;
    m_nCityCount=0;

    int i = 0;
    for(i = 0; i < 8; i++)
    {
        nextCity[i] = -1;
        prob[i] = 0.0;
    }
    for (i = 0; i < MAX_VEX; i++)
    {
        visitCity[i] = 0;
        tabu[i] = -1;
    }

    over = 0;
}

void ant::NewAnt()
{
    m_dLength = m_dShortest = 0;
    m_dRowLength = 0;
    m_nCityCount = 0;

    int i = 0;
    for(i = 0; i < 8; i++)
    {
        nextCity[i] = -1;
        prob[i] = 0.0;
    }
    for (i = 0; i < MAX_VEX; i++)
    {
        visitCity[i] = 0;
        tabu[i] = -1;
    }

    m_nInterNum = g_interNum;
    over = 0;
}

//增加一个城市到走过的城市数组中，并改变没走过的城市数组中的标志
void ant::AddCity(int city)
{
    //add city to tabu;
    tabu[m_nCityCount] = city;
    m_nEndCity = city;
    m_nCityCount++;
    visitCity[city] = 1;
    int i = 0;
    int n = 0;
    for (i = 0; i < MAX_VEX; i ++)
    {
        if (-1 != g_Map[city][i].edgeID && 0 == visitCity[i])
        {
            nextCity[n] = i;
            prob[n] = 0.0;
            n++;
        }
    }
    if (0 == n || g_endVex == city)
    {
        over = 1;
    }
    else
    {
        for (; n < 8; n++)
        {
            nextCity[n] = -1;
            prob[i] = 0.0;
        }
    }
}

int ant::ChooseNextCity()
{

    //Update the probability of path selection
    //select a path from tabu[m_nCityCount-1] to next

    int j = INF;
    double temp = 0.0;

    int curCity = tabu[m_nCityCount-1]; //当前走到那个城市了

    #ifdef _TEST_ANT_VERBOSE_
    TEST("Cur City: %d, m_nCityCount: %d\n", curCity, m_nCityCount);
    #endif

    //先计算当前城市和没有走过的城市，两两之间的信息素的总和
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (nextCity[i] != -1)
        {
            prob[i] = pow((100.0 / g_Map[curCity][nextCity[i]].edgeCost), DB_BETA) * pow((g_Map[curCity][nextCity[i]].Trial), DB_ALPHA);
            temp = temp + prob[i];

            #ifdef _TEST_ANT_VERBOSE_
            TEST("Prob: %d %f %f\n", nextCity[i], prob[i], temp);
            #endif
        }
    }

    //计算没有走过的城市被选中的概率
    double sel = 0.0;
    for (i = 0; i < 8; i++)
    {
        if (nextCity[i] != -1)
        {
            prob[i] = prob[i] / temp;
            sel = sel + prob[i];

            #ifdef _TEST_ANT_VERBOSE_
            TEST("Prob: %d %f %f\n", nextCity[i], prob[i], sel);
            #endif
        }
        /*
        else
        {
            prob[i] = 0;
        }
        */
    }

    //下面的操作实际上就是遗传算法中的轮盘选择
    double mRate = rnd(0, sel);
    #ifdef _TEST_ANT_VERBOSE_
    TEST("mRate %f\n", mRate);
    #endif
    //double mRate=rnd(0,1);
    double mSelect = 0;
    for (i = 0; i < 8; i++)
    {
        if (nextCity[i] != -1)
        {
            mSelect += prob[i];
            if (mSelect >= mRate)
            {
                j = nextCity[i];
                break;
            }
        }
    }

    //这种情况只有在temp=0.0的时候才会出现
    if (j == INF)
    {
        for (i = 0; i < 8; i++)
        {
            if (nextCity[i] != -1)
            {
                j = nextCity[i];
                break;
            }
        }
    }

    if (1 == g_Map[curCity][j].destInter)
    {
        m_nInterNum--;
    }
    g_Map[curCity][j].Trial *= DB_SUBROU;
    m_dRowLength += g_Map[curCity][j].rawCost;
    if (m_dRowLength > g_totalCost)
    {
        over = 1;
    }

    #ifdef _TEST_ANT_VERBOSE_
    TEST("Next City: %d\n", j);
    #endif

    return j;
}

//计算周游完城市后，走过的路径长度
void ant::UpdateResult()
{
    // Update the length of tour

    #ifdef _TEST_RESULT_
    m_dRowLength = 0;
    #endif

    for(int i = 0; i < MAX_VEX - 1; i++)
    {
        if (-1 == g_Map[tabu[i]][tabu[i+1]].edgeID)
        {
            break;
        }
        m_dLength += g_Map[tabu[i]][tabu[i+1]].edgeCost;

        #ifdef _TEST_RESULT_
        m_dRowLength += g_Map[tabu[i]][tabu[i+1]].rawCost;
        #endif
    }
}

void ant::Move()
{
    //the ant move to next town and add town ID to tabu.
    int n = ChooseNextCity();
    AddCity(n);
    #ifdef _TEST_ANT_VERBOSE_
    PrintTabu();
    #endif
}

void ant::PrintTabu()
{
    int i = 0;
    TEST("Tabu: ");
    while (1)
    {
        if (tabu[i] != -1)
        {
            TEST("%d ", tabu[i]);
            i++;
        }
        else
        {
            TEST("\n");
            break;
        }
    }
}

void ant::PrintAnt()
{
    TEST("%d %f %f %d %d %d\n", m_nCityCount,m_dLength,m_dRowLength,m_nInterNum,m_nEndCity,over);
}

void project::PrintMap()
{
    int i, j;
    for (i = 0; i < MAX_VEX; i++)
    {
        for (j = 0; j < MAX_VEX; j++)
        {
            if (g_Map[i][j].edgeID != -1)
            {
                TEST("%d %d %d %d %f %f %d\n", i, j,
                     g_Map[i][j].edgeID, g_Map[i][j].edgeCost,
                     g_Map[i][j].Trial, g_Map[i][j].deltaTrial,
                     g_Map[i][j].destInter);
            }
        }
    }
}

void project::InitMap()
{
    for(int i = 0; i < MAX_VEX; i++)
    {
        for (int j = 0; j < MAX_VEX; j++)
        {
            if (0 == g_Map[i][j].destInter)
            {
                g_Map[i][j].Trial = 20;
            }
            else
            {
                g_Map[i][j].Trial = 40;
            }
            g_Map[i][j].deltaTrial = 0.0;
        }
    }
}

//更新环境信息素
//这里采用的是 ANT-CYCLE 模式，即每只蚂蚁周游完城市后才更新
//还有其他方式为蚂蚁每走一个城市就更新一次，经过试验表明，周游完后才更新比较好
void project::UpdateTrial()
{
    //calculate the changes of trial information
    int m = 0;
    int n = 0;
    double temp = 0.0;
    int i = 0;

    /*
    if (iter_bestant == INF)
    {
        return;
    }

    temp = DB_Q / ants[iter_bestant].m_dRowLength;

    if (temp > 40)
    {
        temp = 40;
    }
    else if (temp < 0.00001)
    {
        temp = 0.00001;
    }

    for (i = 0; i < MAX_VEX - 1; i++)	// 只计算迭代最优蚂蚁的信息素（较大改动）
    {
        if (-1 == ants[iter_bestant].tabu[i+1])
        {
            break;
        }

        m = ants[iter_bestant].tabu[i];
        n = ants[iter_bestant].tabu[i+1];
        g_Map[m][n].deltaTrial += temp;
    }
    */

	for(i = 0; i < MAX_ANT; i++) //计算每只蚂蚁在两两城市间留下的信息素，蚂蚁走过的路径越短，留下的信息素数值越大（废除）
	{
        //if (iter_bestant == i)
        if (0 == ants[i].m_nInterNum)
        {
            temp = DB_Q / ants[i].m_dRowLength;
            if (temp > 4)
            {
                temp = 4;
            }
            else if (temp < 0.00001)
            {
                temp = 0.00001;
            }

            for (int j = 0; j < MAX_VEX - 1; j++) //计算两两城市间的信息素
            {
                if (-1 == ants[i].tabu[j+1])
                {
                    break;
                }
                /*
                m = ants[iter_bestant].tabu[j];
                n = ants[iter_bestant].tabu[j + 1];
                g_Map[m][n].deltaTrial += temp;
                */
                g_Map[ants[i].tabu[j]][ants[i].tabu[j+1]].deltaTrial += temp;
            }
        }/*
        else if (iter_maxintant == ants[i].m_nInterNum)
        {
            temp = DB_Q / ants[i].m_dRowLength;
            if (temp > 20)
            {
                temp = 20;
            }
            else if (temp < 0.00001)
            {
                temp = 0.00001;
            }

            for (int j = 0; j < MAX_VEX - 1; j++) //计算两两城市间的信息素
            {
                if (-1 == ants[i].tabu[j+1])
                {
                    break;
                }

                //m = ants[i].tabu[j];
                //n = ants[i].tabu[j + 1];
                //g_Map[m][n].deltaTrial -= temp;

                g_Map[ants[i].tabu[j]][ants[i].tabu[j+1]].deltaTrial -= temp;
            }
        }*/
        else if (iter_minintant == ants[i].m_nInterNum)
        {
            temp = DB_Q / ants[i].m_dRowLength;
            if (temp > 2)
            {
                temp = 2;
            }
            else if (temp < 0.00001)
            {
                temp = 0.00001;
            }

            for (int j = 0; j < MAX_VEX - 1; j++) //计算两两城市间的信息素
            {
                if (-1 == ants[i].tabu[j+1])
                {
                    break;
                }
                /*
                m = ants[i].tabu[j];
                n = ants[i].tabu[j + 1];
                g_Map[m][n].deltaTrial += temp;
                */
                g_Map[ants[i].tabu[j]][ants[i].tabu[j+1]].deltaTrial += temp;
            }
        }
	}

    //最新的环境信息素 = 消失掉的信息素 +  新留下的信息素
    for (i = 0; i < MAX_VEX; i++)
    {
        for (int j=0;j < MAX_VEX; j++)
        {
            g_Map[i][j].Trial = DB_ROU * g_Map[i][j].Trial + g_Map[i][j].deltaTrial;
            g_Map[i][j].deltaTrial = 0.0;
            /*
            if (g_Map[i][j].Trial > 60)
            {
                g_Map[i][j].Trial = 60;
            }*/
        }
    }
}

project::project()
{
    InitMap();
    m_dLength = INF;
    for (int i = 0; i < MAX_ANT; i++)
    {
        ants[i].NewAnt();
    }
}

void project::NewProject()
{
    InitMap();
    m_dLength = INF;
    for (int i = 0; i < MAX_ANT; i++)
    {
        ants[i].NewAnt();
    }
}

void project::GetAnt()
{

    //初始化随机种子
    srand((unsigned)time(NULL));
    rand();

    //为每只蚂蚁随机分配一个出发城市
    int city = g_startVex;
    for (int i = 0; i < MAX_ANT; i++)
    {
        //city=rnd(N_CITY_COUNT);
        ants[i].AddCity(city);
    }
}

void project::StartSearch()
{
    clock_t endTime;
    double duration = 0.0;
    memset(g_resultRoute, -1, sizeof(g_resultRoute));
    g_totalCost = MAX_COST;

    //begin to find best solution
    int max = 0;    //every ant tours times
    double temp;
    int temp_min;
    int temp_max;
    int temptour[MAX_VEX];

    double dbMin = 0.0;
    int iters_no_renew = 0;		//无效迭代持续次数

    int j = 0;
    int t = 0;
    //while (max < MAX_IT)
    while (duration < MAX_TIME)
    {
        #ifdef _TEST_ANT_
        fflush(stdout);
        #endif

        //PrintMap();
        //getchar();

        dbMin = INF; //本次叠迭中的最小路径长度

        for(j = 0; j < MAX_ANT; j++)
        {
            while (1 != ants[j].over)
            {
                ants[j].Move();
            }
            ants[j].UpdateResult(); //计算路径总长度

            #ifdef _TEST_ANT_VERBOSE_
            TEST("\nAnt: %d, Result Length: %f, InterNum: %d\n", j, ants[j].m_dLength, ants[j].m_nInterNum);
            TEST("Next Ant!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
            #endif
        }

        DEBUG("All Ants Seach Over\n");

        //find out the best solution of the step and put it into temp
        temp = INF;				//temp存放本次迭代各蚂蚁路径的最短长度
        temp_min = INF;
        temp_max = 0;
        iter_bestant = INF;					//最优蚂蚁序号
        //iter_maxintant = INF;
        iter_minintant = INF;

        for (t = 0; t < MAX_VEX; t++)
        {
            temptour[t] = ants[0].tabu[t];	//temptour存放本次迭代的最短路径
            if (-1 == ants[0].tabu[t])
            {
                break;
            }
        }

        DEBUG("Begin Get Best Ant\n");

        for(j = 0; j < MAX_ANT; j++)
        {
            if (ants[j].m_nEndCity == g_endVex && 0 == ants[j].m_nInterNum && temp > ants[j].m_dLength)
            {
                iter_bestant = j;
                temp = ants[j].m_dLength;

                g_totalCost = ants[j].m_dRowLength;

                for (t = 0; t < MAX_VEX; t++)
                {
                    temptour[t] = ants[j].tabu[t];
                    if (-1 == ants[j].tabu[t])
                    {
                        break;
                    }
                }
            }

            /*
            if (temp_max < ants[j].m_nInterNum)
            {
                temp_max = ants[j].m_nInterNum;
            }*/
            if (temp_min > ants[j].m_nInterNum)
            {
                temp_min = ants[j].m_nInterNum;
            }

            if (ants[j].m_nEndCity == g_endVex && 0 == ants[j].m_nInterNum && dbMin > ants[j].m_dLength)
            {
                dbMin = ants[j].m_dLength;
            }

        }

        #ifdef _TEST_ANT_VERBOSE_
        TEST("Best Ant: %d\n", iter_bestant);
        #endif

        DEBUG("Get Best Ant Over\n");

        if (temp < m_dLength)
        {
            iters_no_renew = 0;
            m_dLength = temp;
            for (t = 0; t < MAX_VEX - 1; t++)
            {
                if (-1 == temptour[t+1])
                {
                    g_resultRoute[t] = -1;
                    break;
                }
                g_resultRoute[t] = g_Map[temptour[t]][temptour[t+1]].edgeID;

                DEBUG("t: %d, temptour[t]: %d", t, temptour[t]);
            }
            g_resultRoute[t] = -1;

            #ifdef _TEST_RESULT_
            endTime = clock();
            duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
            TEST("Time: %f, m_dLength: %.0f\n", duration, m_dLength);
            #endif
        }
        else
        {
            iters_no_renew++;
            if (iters_no_renew > MAX_NO_RENEW)
            {
                InitMap();
                GetAnt();
                iters_no_renew = 0;

                TEST("Renewed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }
        }

        //iter_maxintant = temp_max;
        iter_minintant = temp_min;

        DEBUG("Begin UpdateTrial\n");

        UpdateTrial(); //全部蚂蚁遍历各个城市后，更新环境信息素

        DEBUG("Begin Next Search Loop\n");

        for(j = 0; j < MAX_ANT;j ++)  //再搜索一次
        {
            ants[j].Clear();
        }


        max++;
/*
        if (temp != INF)
        {
            break;
        }
*/
        endTime = clock();
        duration = (double)(endTime - g_startTime) / CLOCKS_PER_SEC;
    }

    TEST("The shortest toure is : %f\n", m_dLength);
    TEST("Time: %fs\n\n", duration);

    /*
    for (int t = 0; t < MAX_VEX; t++)
    {
        //TEST(" %d ",besttour[t]);
    }
    */
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

    double remainTime = 9.9;
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
函数功能：输出result.csv文件
输入参数：
输出参数：
返回值：
修改情况：
*****************************************/
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
        if (g_vexInfo[vexStack[i]]->maxInterNum < g_vexInfo[vexStack[i]]->maxInterTmp)
        {
            g_vexInfo[vexStack[i]]->maxInterNum = g_vexInfo[vexStack[i]]->maxInterTmp;
            g_vexInfo[vexStack[i]]->isDone = true;
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
