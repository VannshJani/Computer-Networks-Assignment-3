#include <stdio.h>

extern struct rtpkt
{
    int sourceid;   /* id of sending router sending this pkt */
    int destid;     /* id of router to which pkt being sent
                       (must be an immediate neighbor) */
    int mincost[4]; /* min cost to node 0 ... 3 */
};

extern int TRACE;
extern int YES;
extern int NO;
extern float clocktime;

/* Forward declarations for functions in distance_vector.c */
extern void creatertpkt(struct rtpkt *initrtpkt, int srcid, int destid, int mincosts[]);
extern void tolayer2(struct rtpkt packet);

struct distance_table
{
    int costs[4][4];
} dt2;

/* Function prototypes */
void rtinit2(void);
void rtupdate2(struct rtpkt *rcvdpkt);
int printdt2(struct distance_table *dtptr);

void rtinit2()
{
    int i, j;
    int mincost[4];
    struct rtpkt packet;

    /* Initialize the distance table with infinity */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            dt2.costs[i][j] = 999;
        }
    }

    /* Set direct costs to neighbors */
    dt2.costs[0][0] = 3; /* Direct cost to node 0 is 3 */
    dt2.costs[1][1] = 1; /* Direct cost to node 1 is 1 */
    dt2.costs[2][0] = 0; /* Cost to self is 0 */
    dt2.costs[3][3] = 2; /* Direct cost to node 3 is 2 */

    /* Initialize minimum costs array */
    mincost[0] = 3; /* Minimum cost to node 0 is 3 */
    mincost[1] = 1; /* Minimum cost to node 1 is 1 */
    mincost[2] = 0; /* Cost to self is 0 */
    mincost[3] = 2; /* Minimum cost to node 3 is 2 */

    /* Create and send routing packets to all neighbors */

    /* Send to node 0 */
    creatertpkt(&packet, 2, 0, mincost);
    tolayer2(packet);

    /* Send to node 1 */
    creatertpkt(&packet, 2, 1, mincost);
    tolayer2(packet);

    /* Send to node 3 */
    creatertpkt(&packet, 2, 3, mincost);
    tolayer2(packet);

    printf("At time %f, rtinit2() called\n", clocktime);
    printdt2(&dt2);
}

void rtupdate2(struct rtpkt *rcvdpkt)
{
    int sourceid;
    int updated = 0;
    int dest;
    int direct_cost;
    int newcost;
    int mincost[4];
    struct rtpkt packet;

    printf("At time %f, rtupdate2() called, received packet from %d\n",
           clocktime, rcvdpkt->sourceid);

    sourceid = rcvdpkt->sourceid;

    /* Determine direct cost to the sender */
    direct_cost = 999;
    if (sourceid == 0)
        direct_cost = 3;
    else if (sourceid == 1)
        direct_cost = 1;
    else if (sourceid == 3)
        direct_cost = 2;

    /* For each destination node */
    for (dest = 0; dest < 4; dest++)
    {
        /* Skip updates to self */
        if (dest != 2)
        {
            /* Calculate the potential new cost to dest via the sourceid */
            newcost = direct_cost + rcvdpkt->mincost[dest];

            /* If new cost is better, update the distance table */
            if (newcost < dt2.costs[dest][sourceid])
            {
                dt2.costs[dest][sourceid] = newcost;
                updated = 1;
            }
        }
    }

    /* If updates were made, recalculate minimum costs and send updates */
    if (updated)
    {
        /* Calculate current minimum costs to each destination */

        /* For each destination, find minimum cost path */
        for (dest = 0; dest < 4; dest++)
        {
            if (dest == 2)
            {
                mincost[dest] = 0; /* Cost to self is always 0 */
            }
            else
            {
                /* Start with via first neighbor (node 0) */
                mincost[dest] = dt2.costs[dest][0];

                /* Check if there's a better path via other neighbors */
                if (dt2.costs[dest][1] < mincost[dest])
                    mincost[dest] = dt2.costs[dest][1];
                if (dt2.costs[dest][3] < mincost[dest])
                    mincost[dest] = dt2.costs[dest][3];
            }
        }

        /* Send updates to all neighbors */

        /* Send to node 0 */
        creatertpkt(&packet, 2, 0, mincost);
        tolayer2(packet);

        /* Send to node 1 */
        creatertpkt(&packet, 2, 1, mincost);
        tolayer2(packet);

        /* Send to node 3 */
        creatertpkt(&packet, 2, 3, mincost);
        tolayer2(packet);

        printf("At time %f, node 2 sends routing updates to neighbors\n", clocktime);
    }

    /* Print the updated distance table */
    printdt2(&dt2);
}

int printdt2(struct distance_table *dtptr)
{
    printf("                via     \n");
    printf("   D2 |    0     1    3 \n");
    printf("  ----|-----------------\n");
    printf("     0|  %3d   %3d   %3d\n", dtptr->costs[0][0],
           dtptr->costs[0][1], dtptr->costs[0][3]);
    printf("dest 1|  %3d   %3d   %3d\n", dtptr->costs[1][0],
           dtptr->costs[1][1], dtptr->costs[1][3]);
    printf("     3|  %3d   %3d   %3d\n", dtptr->costs[3][0],
           dtptr->costs[3][1], dtptr->costs[3][3]);
    return 0;
}