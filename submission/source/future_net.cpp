//#include <alloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
//#define NULL 0
#define MAX 1000
#define bool int
#define true 1
#define false 0

struct edgenode
{
    int no;
    int eid;
    int cost;
    int level;
    struct edgenode *next;
};

struct vex
{
    struct edgenode *first;
};

typedef struct vex adjlist; //
//int n,e; //
adjlist *g;

int inter[50];
int internum = 0;
int costleast = 99999;
int count = 0;
int vexlevel[MAX];

bool isDup(struct edgenode *p, int id, int t, int c)
{
    while (NULL != p)
    {
        if (p->no == t)
        {
            if (p->cost > c)
            {
                p->cost = c;
                p->eid = id;
            }
            return true;
        }
        p = p->next;
    }
    return false;
}

inline bool isinter(int num)
{
    for (int i = 0; i < internum; i++)
    {
        if (num == inter[i])
            return true;
    }
    return false;
}

void initalize(FILE *fp_graph) //
{
    int i,s,t,c,id;
    struct edgenode *p,*q;
    //printf("Enter the n and e:");
    //scanf("%d %d",&n,&e);
    //g=(adjlist *)malloc(n*sizeof(adjlist));
    g = (adjlist *)malloc(MAX*sizeof(adjlist));
    //for(i=0;i<n;i++) 
    for(i=0; i<MAX; i++)
    {
        g[i].first = NULL;
        if (isinter(i))
            vexlevel[i] = 0;
        else
            vexlevel[i] = -1;
    }
    //for(i=1;i<=e;i++)
    while (fscanf(fp_graph, "%d,%d,%d,%d\n",&id,&s,&t,&c) > 0)
    {
        //printf("The %d edge’s id and start and end and cost:",i);
        //scanf("%d %d %d %d",&id,&s,&t,&c);
        if (!isDup(g[s].first, id, t ,c))
        {
            //p=(struct edgenode *)malloc(sizeof(struct edgenode));
            q = (struct edgenode *)malloc(sizeof(struct edgenode));

            edgenode *qtmp = g[s].first;
            edgenode *qtmp_pre = NULL;
            while (qtmp != NULL)
            {
                if (qtmp->cost < c)
                {
                    qtmp_pre = qtmp;
                    qtmp = qtmp->next;
                }
                else
                {
                    break;
                }
            }

            q->no = t;
            q->next = qtmp;
            if (qtmp == g[s].first)
            {
                g[s].first = q;
            }
            else
            {
                qtmp_pre->next = q;
            }
            q->eid = id;
            q->cost = c;

            /*
            qtmp = g[s].first;
            while (qtmp != NULL)
            {
                printf("%d ",qtmp->cost);
                qtmp = qtmp->next;
            }
            printf("\n");
            */

            /*
            //p->no=s;
            q->no = t;
            //p->next=g[t].first;g[t].first=p; 此句加上即可变为无向图。
            q->next = g[s].first;   // 链表往前插
            g[s].first = q;
            q->eid = id;
            q->cost = c;
            */

            if (isinter(s) && isinter(t))
            {
                q->level = 0;
            }
            else if (isinter(s) && !isinter(t))
            {
                q->level = 1;
                vexlevel[t] = 1;
            }
            else if (!isinter(s) && isinter(t))
            {
                q->level = 1;
                vexlevel[t] = 1;
            }
            else
            {
                q->level = 600;
            }
        }
    }
}

/*
void rmDup(struct edgenode *p)
{
    struct edgenode *ptmp = NULL;
    while (NULL !=p && NULL != p->next)
    {
        if (p->no == p->next->no)
        {
            ptmp = p->next;
            p->next = p->next->next;
            p->cost = (p->cost < ptmp->cost) ? p->cost : ptmp->cost;
            free(ptmp);
        }
        p = p->next;
    }
}
*/

void levelUp(int level)
{
    struct edgenode *p;
    for (int te = 0; te < MAX; te++)
    {
        if (NULL == g[te].first)
            continue;
        //printf("Gt: ");
        p = g[te].first;
        while(p)
        {
            //printf("%d:%d:%d ",p->eid, p->no, p->cost);
            if (p->level == 600)
            {
                if (vexlevel[te] != -1 && vexlevel[p->no] != -1)
                {
                    p->level = ((vexlevel[te] < vexlevel[p->no]) ? vexlevel[te] : vexlevel[p->no]) + 1;
                }
                else if (vexlevel[te] == -1 && vexlevel[p->no] != -1)
                {
                    p->level = vexlevel[p->no] + 1;
                    vexlevel[te] = p->level;
                }
                else if (vexlevel[te] != -1 && vexlevel[p->no] == -1)
                {
                    p->level = vexlevel[te] + 1;
                    vexlevel[p->no] = p->level;
                }

            }
            p = p->next;
        }
        //printf("\n");
    }
}

int path[MAX+1];

bool searchPath(int st,int en, int level, clock_t beg_t)
{
    clock_t end_t;
    double duration;

    //int *visit,*stack;
    int visit[MAX];
    int stack[MAX+1];
    int top,v,head=1,i;
    struct edgenode *p;
    //visit=(int *)malloc(n*sizeof(int));
    //visit = (int *)malloc(MAX*sizeof(int));
    //stack=(int *)malloc((n+1)*sizeof(int));
    //stack = (int *)malloc((MAX+1)*sizeof(int));
    //int *costs=(int *)malloc((MAX+1)*sizeof(int));
    int costs[MAX+1];
    //int *edges=(int *)malloc((MAX+1)*sizeof(int));
    int edges[MAX+1];
    //int path[MAX+1];
    //for (i=0;i<n;i++) 
    for (i=0;i<MAX;i++) 
        visit[i]=0;
    v = st;
    visit[st] = 1;
    top = 1;
    stack[top] = v;
    costs[top] = 0;
    edges[top] = 0;

    p = g[v].first;
    int count = 0;
    while (p != NULL)
    {
        count++;
        p = p->next;
    }
    float time_inter = 9.9 / (float)count;
    //printf("Time_Inter: %f\n", time_inter);
    float time_limit = time_inter; 
    struct edgenode *pst = g[st].first;

    /*
    for (int te = 0; te < MAX; te++)
    {
        if (NULL == g[te].first)
            continue;
        printf("Gt: ");
        p = g[te].first;
        while(p)
        {
            printf("%d:%d:%d ",p->eid, p->no, p->cost);
            p = p->next;
        }
        printf("\n");
    }*/
    
    do
    {
        //printf("Least Cost: %d in %f second. %d\n", costleast, duration, count);
        if (head == 1) 
        {
            p = g[v].first;
            head = 0;

            /*
            edgenode *qtmp = p;
            while (qtmp != NULL)
            {
                printf("%d ",qtmp->cost);
                qtmp = qtmp->next;
            }
            printf("\n");
            */
        }
        else
        {
            p = p->next;
        }

        RENEW:

        if(p && p->level <= level)
        {
            //printf("p%d ",p->eid);
            if (visit[p->no] == 0)
            {
                visit[p->no] = 1;
                top++;
                stack[top] = p->no;

                costs[top] = p->cost;
                edges[top] = p->eid;
                if(p->no==en)
                {
                    //printf("p%d ",p->eid);
                    //printf("PATH: ");
                    int intertmp = internum;
                    int costsum = 0;
                    for(i=1;i<=top;i++)
                    {
                        if (0 != intertmp && isinter(stack[i]))
                            intertmp--;
                        //printf("%d ",stack[i]);
                        costsum += costs[i];
                    }
                    if (intertmp == 0)
                    {
                        //printf(" OK ");
                        if (costsum < costleast)
                        {
                            costleast = costsum;
                            memset(path, -1, sizeof(path));
                            for(i=1;i<=top;i++)
                            {
                                path[i-1] = edges[i];
                            }
                        }
                    }
                    //printf(" COST: %d\n",costsum);
            
                    visit[en] = 0;
                    top--;  //printf("TOP: %d\n",top);
                    v = stack[top];
                    head = 0;
                }
                else
                {
                    //printf("%d ",stack[top]);
                    v = stack[top];
                    head = 1;
                }

                end_t = clock();
                duration = (double)(end_t - beg_t) / CLOCKS_PER_SEC;
                //count++;
                //printf("Least Cost: %d in %f second. %d\n", costleast, duration, count);
                
                /*
                if (duration > 9.9)
                    break;
                */

                if (duration > time_limit)
                {
                    //printf("Least Cost: %d in %f second\n", costleast, duration);
                    if (duration > 9.9)
                    {
                        break;
                    }
                    time_limit += time_inter;

                    pst = pst->next;
                    if (pst == NULL)
                    {
                        break;
                    }
                    for (i=0;i<MAX;i++) 
                        visit[i]=0;
                    visit[en] = 0;
                    top = 1;  //printf("TOP: %d\n",top);
                    head = 0;
                    visit[st] = 1;
                    v = st;
                    p = pst;
                    stack[top] = p->no;
                    costs[top] = 0;
                    edges[top] = 0;

                    //printf("goto renew\n"); 
                    goto RENEW;
                }

            } //
        }
        else
        {
            //printf(" OKOKOK \n");
            visit[stack[top--]] = 0; //
            if(top)
            {
                p = g[stack[top]].first;
                while(p->no != v) 
                {
                    p = p->next;
                }
                v = stack[top];
                head = 0;
            }
        }
    }while(top);
/*
    if (costleast == 99999)
    {
        printf("NA\n");
        return false;
    }
    else
    {
        i = 1;
        printf("%d",path[i]);
        i++;
        while (path[i] != -1)
        {
            printf("|%d",path[i]);
            i++;
        }
        printf("\nCost: %d\n",costleast);
        return true;
    }*/
    return true;
}

int main(int argc, char *argv[])
{
    clock_t beg_t, end_t;
    beg_t = clock();

    char *str = (char*)malloc(1024);
    memset(str, 0, 1024);
    strcpy(str, "topo.csv");
    if (argv[1] != NULL)
    {
        strcpy(str, argv[1]);
    }
    FILE *fp_graph = fopen(str, "r");
    if (fp_graph == NULL)
        exit(1);

    memset(str, 0, 1024);
    strcpy(str, "demand.csv");
    if (argv[1] != NULL && argv[2] != NULL)
    {
        strcpy(str, argv[2]);
    }
    FILE *fp_path = fopen(str, "r");
    if (fp_path == NULL)
        exit(1);

    
    memset(str, 0, 1024);
    strcpy(str, "result.csv");
    if (argv[1] != NULL && argv[2] != NULL && argv[3] != NULL)
    {
        strcpy(str, argv[3]);
    }
    FILE *fp_result = freopen(str, "w", stdout);
    if (fp_result == NULL)
        exit(1);
    

    int st,en; //
    //initalize(fp_graph);

    //printf("Enter the start and the end:");
    //scanf("%d %d",&st,&en);
    fscanf(fp_path,"%d,%d,",&st,&en);
    //printf("Enter the inter vex:");
    memset(inter, -1, sizeof(inter));
    //scanf("%d%d",&inter[0],&inter[1]);
    internum = 0;
    while (fscanf(fp_path, "%d|", &inter[internum]) > 0)
    {
        internum++;
    }

    initalize(fp_graph);

    int flag;
    double duration;
    int level = 0;/*
    do
    {
        flag = searchPath(st,en,level,beg_t);
        levelUp(level);
        level++;
        end_t = clock();
        duration = (double)(end_t - beg_t) / CLOCKS_PER_SEC;
    } while (level <= 600 && duration < 9.9);*/

    searchPath(st, en, 601,beg_t);

    if (costleast == 99999)
    {
        printf("NA\n");
        return false;
    }
    else
    {
        int i = 1;
        printf("%d",path[i]);
        i++;
        while (path[i] != -1)
        {
            printf("|%d",path[i]);
            i++;
        }
        //printf("\nCost: %d\n",costleast);
    }

    /*
    end_t = clock();
    duration = (double)(end_t - beg_t) / CLOCKS_PER_SEC;
    printf("\n%f second\n", duration);
    */
    fclose(fp_graph);
    fclose(fp_path);
    free(str);
    return 0;
} 
