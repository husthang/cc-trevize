/*
 * graph_builder v1.0
 * Copyright (c) 2016. GloomyMouse (Chaofei XU). All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int getCost()
{
    int cost = rand() % 20;
    if (0 == cost)
        cost = 20;
    return cost;
}

int getStart(int vex, int *outdegree)
{
    int start = rand() % vex;
    while (8 == outdegree[start])
        start = rand() % vex;
    outdegree[start]++;
    return start;
}

int getEnd(int vex, int start)
{
    int end = rand() % vex;
    while (end == start)
        end = rand() % vex;
    return end;
}

int getInter(int vex, int *isinter)
{
    int inter = rand() % vex;
    if (rand() % 2)
    {
        while (1 == isinter[inter])
            inter = rand() % vex;
        isinter[inter]++;
    }
    else
    {
        inter = -1;
    }
    return inter;
}

int main(int argc, char *argv[])
{
    int vex = 100;
    int edge = 500;
    int interset = 50;
    if (argv[1] != NULL)
    {
        vex = atoi(argv[1]);
    }
    if (argv[1] != NULL && argv[2] != NULL)
    {
        edge = atoi(argv[2]);
    }

    int outdegree[vex];
    memset(outdegree, 0, sizeof(outdegree));
    int isinter[vex];
    memset(isinter, 0, sizeof(isinter));

    srand((unsigned)time(NULL));
    int i = rand() % vex;

    FILE *fp = fopen("topo.csv", "w");
    if (NULL == fp)
        exit(1);
    int start;
    int end;
    int cost;

    printf("%d %d\n",vex,edge);
    for (i=0; i<edge; i++)
    {
        start = getStart(vex, outdegree);
        end = getEnd(vex, start);
        cost = getCost();
        fprintf(fp, "%d,%d,%d,%d\n", i, start, end, cost);
    }
    fclose(fp);

    fp = fopen("demand.csv", "w");
    if (NULL == fp)
        exit(1);

    start = rand() % vex;
    isinter[start]++;
    end = getEnd(vex, start);
    isinter[end]++;
    fprintf(fp, "%d,%d", start, end);

    int internum = 0;
    int inter = getInter(vex, isinter);
    while (internum < interset && -1 != inter)
    {
        if (internum == 0)
        {
            fprintf(fp, ",%d", inter);
        }
        else
        {
            fprintf(fp, "|%d", inter);
        }
        internum++;
        inter = getInter(vex, isinter);
    }
    fclose(fp);

    return 0;
}
