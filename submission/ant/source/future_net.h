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
#define _TEST_
#ifdef  _TEST_
#define TEST(format,...) printf(format, ##__VA_ARGS__)
#else
#define TEST(format,...)
#endif

/* 搜索结果信息打印 */
#define _TEST_RESULT_

/* 按是否为中间点v'和权重大小两项 排序邻接表 */
#define _PSEUDO_COST_
/* 搜索多层邻接表 按是否为中间点v'和权重大小两项 重新排序邻接表 */
//#define _JUDGE_MULTI_LAYER_ADJ_
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

/* ANT */
#define _ANT_
//#define _TEST_ANT_
//#define _TEST_ANT_VERBOSE_

/****************************************
 * 宏 常量
 ****************************************/
/* 最大顶点数 */
#define MAX_VEX      (600)
/* 最大有向边数 */
#define MAX_EDGE     (4800)
/* 最大中间点集v'数 */
#define MAX_INTER    (50)
/* 权重极限值 */
#define INF          (999999)

#define MAX_COST     (1200)

/* 缺省字符串长度 */
#define MAX_STR      (1024)

#define MAX_QUEUE    (3)

#define MAX_ANT      (500)

#define MAX_IT       (2000)

#define MAX_NO_RENEW (80)

#define PSEUDO_ARG   (20)

#define MAX_TIME     (9.0)


const double DB_Q      = 500; //总的信息素
const double DB_ALPHA  = 1; //信息素重要程度
const double DB_BETA   = 5; //这个数越大，则蚂蚁往信息素大的地方走的概率就越大
const double DB_ROU    = 0.9; //环境信息素挥发速度
const double DB_SUBROU = 0.99;


/****************************************
 * 结构体
 ****************************************/
/* 有向边信息 */
typedef struct EdgeInfo
{
    int destVex;    // 目标顶点
    int srcVex;     // 起始定点
    int edgeID;     // 有向边编号
    int edgeCost;   // 有向边权重
    int edgePseudoCost;
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

typedef struct Map
{
    char   destInter;
    int    edgeID;
    int    edgeCost;
    int    rawCost;
    double Trial;
    double deltaTrial;
} Map;

class ant
{
private:
    int ChooseNextCity(); //选择下一个城市
    double prob[8]; //临时变量数组，选择下一个城市的时候，保存各个城市被选中的概率值
    int m_nCityCount; //记录蚂蚁已经走过的城市数目
    int nextCity[8];//没有走过的城市，数组索引对应城市编号
    int visitCity[MAX_VEX];

public:
    double m_dLength;
    double m_dRowLength;
    double m_dShortest;
    int tabu[MAX_VEX]; //记录走过的城市，里面存的是城市的编号，数组索引表示走的顺序。
    int m_nInterNum;
    int m_nEndCity;
    int over;

public:
    ant();
    void NewAnt();
    void AddCity(int city);
    void Clear();
    void UpdateResult();
    void Move();
    void PrintTabu();
    void PrintAnt();
};

class project
{
public:
    double m_dLength;
    int iter_bestant;
    int iter_maxintant;
    int iter_minintant;
    ant ants[MAX_ANT];

public:
    project();
    void NewProject();
    void InitMap();
    void UpdateTrial();
    void GetAnt();
    void StartSearch();
    void PrintMap();
};


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
void PrintStatusInfo(StatusInfo statusInfo);

#ifndef _ANT_
bool IsDupEdge(EdgeNode *pNode, const int edgeId, const int destVex, const int edgeCost);
#else
bool IsDupEdge(const int edgeID, const int srcVex, const int destVex, const int edgeCost);
#endif

inline bool IsInterVex(const int vexID);

FILE *Initalize(int argc, char *argv[]);

void InitDemandFile(FILE *fp_demand);

int GetPseudoCost(EdgeInfo *pEdgeInfo);

void InsertEdgeNode(EdgeNode **pFirstEdge, EdgeInfo *pInfo);

void InitTopoFile(FILE *fp_topo);

void SearchRoute();

double GetTimeInterval(EdgeNode *pEdge, double remainTime);

void PrintResultFile(FILE *fp_result);

void PrintResultFile();

void InitVexInfo();

void AddOnePassToVexInfo(int vexStack[], int stackTop);

void AddMaxInterToVexInfo(int vexStack[], int stackTop);

void SubMaxInterToVexInfo(int vexStack[], int stackTop);

void UpdateMaxInterNum(int vex);

void ClearMaxInterToVexInfo(int vexStack[], int stackTop);

void UpdateAllMaxInterNum(int vexStack[], int stackTop);

void PrintMaxInterNum();

int MultiLayerPseudoCost(EdgeInfo *pEdgeInfo, int layer);

void ReformMultiLayerAdjList(int loop, int layer);

void PrintAdjList();


#endif 
