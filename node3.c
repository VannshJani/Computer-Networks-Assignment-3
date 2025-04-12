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
} dt3;

/* Function prototypes */
void rtinit3(void);
void rtupdate3(struct rtpkt *rcvdpkt);
int printdt3(struct distance_table *dtptr);

void rtinit3()
{
    int i, j;
    int mincost[4];
    struct rtpkt packet;

    /* Initialize the distance table with infinity */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            dt3.costs[i][j] = 999;
        }
    }

    /* Set direct costs to neighbors */
    dt3.costs[0][0] = 7;   /* Direct cost to node 0 is 7 */
    dt3.costs[1][0] = 999; /* No direct connection to node 1 (infinity) */
    dt3.costs[2][2] = 2;   /* Direct cost to node 2 is 2 */
    dt3.costs[3][0] = 0;   /* Cost to self is 0 */

    /* Initialize minimum costs array */
    mincost[0] = 7;   /* Minimum cost to node 0 is 7 */
    mincost[1] = 999; /* No path to node 1 yet (infinity) */
    mincost[2] = 2;   /* Minimum cost to node 2 is 2 */
    mincost[3] = 0;   /* Cost to self is 0 */

    /* Create and send routing packets to neighbors */

    /* Send to node 0 */
    creatertpkt(&packet, 3, 0, mincost);
    tolayer2(packet);

    /* Send to node 2 */
    creatertpkt(&packet, 3, 2, mincost);
    tolayer2(packet);

    printf("At time %f, rtinit3() called\n", clocktime);
    printdt3(&dt3);
}

void rtupdate3(struct rtpkt *rcvdpkt)
{
    int sourceid;
    int updated = 0;
    int dest;
    int direct_cost;
    int newcost;
    int mincost[4];
    struct rtpkt packet;

    printf("At time %f, rtupdate3() called, received packet from %d\n",
           clocktime, rcvdpkt->sourceid);

    sourceid = rcvdpkt->sourceid;

    /* Determine direct cost to the sender */
    direct_cost = (sourceid == 0) ? 7 : 2; /* 7 to node 0, 2 to node 2 */

    /* For each destination node */
    for (dest = 0; dest < 4; dest++)
    {
        /* Skip updates to self */
        if (dest != 3)
        {
            /* Calculate the potential new cost to dest via the sourceid */
            newcost = direct_cost + rcvdpkt->mincost[dest];

            /* If new cost is better, update the distance table */
            if (newcost < dt3.costs[dest][sourceid])
            {
                dt3.costs[dest][sourceid] = newcost;
                updated = 1;
            }
        }
    }

    /* If updates were made, recalculate minimum costs and send updates */
    if (updated)
    {
        /* Calculate current minimum costs to each destination */

        /* For each destination, find minimum cost path */
        mincost[0] = dt3.costs[0][0]; /* Start with via node 0 */
        mincost[1] = dt3.costs[1][0]; /* Start with via node 0 */
        mincost[2] = dt3.costs[2][2]; /* Start with via node 2 */
        mincost[3] = 0;               /* Cost to self is always 0 */

        /* Check if there's a better path via the other neighbor */
        if (dt3.costs[0][2] < mincost[0])
            mincost[0] = dt3.costs[0][2];
        if (dt3.costs[1][2] < mincost[1])
            mincost[1] = dt3.costs[1][2];
        if (dt3.costs[2][0] < mincost[2])
            mincost[2] = dt3.costs[2][0];

        /* Send updates to neighbors */

        /* Send to node 0 */
        creatertpkt(&packet, 3, 0, mincost);
        tolayer2(packet);

        /* Send to node 2 */
        creatertpkt(&packet, 3, 2, mincost);
        tolayer2(packet);

        printf("At time %f, node 3 sends routing updates to neighbors\n", clocktime);
    }

    /* Print the updated distance table */
    printdt3(&dt3);
}

int printdt3(struct distance_table *dtptr)
{
    printf("             via     \n");
    printf("   D3 |    0     2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d   %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
    printf("dest 1|  %3d   %3d\n", dtptr->costs[1][0], dtptr->costs[1][2]);
    printf("     2|  %3d   %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
    return 0;
}