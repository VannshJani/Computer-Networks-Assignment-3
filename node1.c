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

int connectcosts1[4] = {1, 0, 1, 999};

struct distance_table
{
    int costs[4][4];
} dt1;

/* Function prototypes */
int rtinit1(void);
int rtupdate1(struct rtpkt *rcvdpkt);
int printdt1(struct distance_table *dtptr);
int linkhandler1(int linkid, int newcost);

int rtinit1()
{
    int i, j;
    int mincost[4];
    struct rtpkt packet;

    /* Initialize the distance table with infinity */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            dt1.costs[i][j] = 999;
        }
    }

    /* Set direct costs from the connectcosts1 array */
    dt1.costs[0][0] = connectcosts1[0]; /* Direct cost to node 0 is 1 */
    dt1.costs[1][0] = 0;                /* Cost to self is 0 */
    dt1.costs[2][2] = connectcosts1[2]; /* Direct cost to node 2 is 1 */
    dt1.costs[3][0] = connectcosts1[3]; /* No direct connection to node 3 (infinity) */

    /* Initialize minimum costs array */
    mincost[0] = connectcosts1[0]; /* Minimum cost to node 0 is 1 */
    mincost[1] = 0;                /* Cost to self is 0 */
    mincost[2] = connectcosts1[2]; /* Minimum cost to node 2 is 1 */
    mincost[3] = connectcosts1[3]; /* No path to node 3 yet (infinity) */

    /* Create and send routing packets to neighbors */

    /* Send to node 0 */
    creatertpkt(&packet, 1, 0, mincost);
    tolayer2(packet);

    /* Send to node 2 */
    creatertpkt(&packet, 1, 2, mincost);
    tolayer2(packet);

    printf("At time %f, rtinit1() called\n", clocktime);
    printdt1(&dt1);
    return 0;
}

int rtupdate1(struct rtpkt *rcvdpkt)
{
    int sourceid;
    int updated = 0;
    int dest;
    int direct_cost;
    int newcost;
    int mincost[4];
    struct rtpkt packet;

    printf("At time %f, rtupdate1() called, received packet from %d\n",
           clocktime, rcvdpkt->sourceid);

    sourceid = rcvdpkt->sourceid;

    /* Calculate direct cost to the sender */
    direct_cost = (sourceid == 0) ? connectcosts1[0] : connectcosts1[2];

    /* For each destination node */
    for (dest = 0; dest < 4; dest++)
    {
        /* Skip updates to self */
        if (dest != 1)
        {
            /* Calculate the potential new cost to dest via the sourceid */
            newcost = direct_cost + rcvdpkt->mincost[dest];

            /* If new cost is better, update the distance table */
            if (newcost < dt1.costs[dest][sourceid])
            {
                dt1.costs[dest][sourceid] = newcost;
                updated = 1;
            }
        }
    }

    /* If updates were made, recalculate minimum costs and send updates */
    if (updated)
    {
        /* Calculate current minimum costs to each destination */

        /* For each destination, find minimum cost path */
        mincost[0] = dt1.costs[0][0]; /* Start with via node 0 */
        mincost[1] = 0;               /* Cost to self is always 0 */
        mincost[2] = dt1.costs[2][2]; /* Start with via node 2 */
        mincost[3] = 999;             /* Start with infinity */

        /* Check if there's a better path via the other neighbor */
        if (dt1.costs[0][2] < mincost[0])
            mincost[0] = dt1.costs[0][2];
        if (dt1.costs[2][0] < mincost[2])
            mincost[2] = dt1.costs[2][0];
        if (dt1.costs[3][0] < mincost[3])
            mincost[3] = dt1.costs[3][0];
        if (dt1.costs[3][2] < mincost[3])
            mincost[3] = dt1.costs[3][2];

        /* Send updates to neighbors */

        /* Send to node 0 */
        creatertpkt(&packet, 1, 0, mincost);
        tolayer2(packet);

        /* Send to node 2 */
        creatertpkt(&packet, 1, 2, mincost);
        tolayer2(packet);

        printf("At time %f, node 1 sends routing updates to neighbors\n", clocktime);
    }

    /* Print the updated distance table */
    printdt1(&dt1);
    return 0;
}

int printdt1(struct distance_table *dtptr)
{
    printf("             via   \n");
    printf("   D1 |    0     2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d   %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
    printf("dest 2|  %3d   %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
    printf("     3|  %3d   %3d\n", dtptr->costs[3][0], dtptr->costs[3][2]);
    return 0;
}

int linkhandler1(int linkid, int newcost)
{
    /* called when cost from 1 to linkid changes from current value to newcost*/
    /* You can leave this routine empty if you're an undergrad. If you want */
    /* to use this routine, you'll need to change the value of the LINKCHANGE */
    /* constant definition in prog3.c from 0 to 1 */
    return 0;
}