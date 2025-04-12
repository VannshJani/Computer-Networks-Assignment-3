#include <stdio.h>

extern struct rtpkt {
  int sourceid;       /* id of sending router sending this pkt */
  int destid;         /* id of router to which pkt being sent 
                         (must be an immediate neighbor) */
  int mincost[4];    /* min cost to node 0 ... 3 */
};

extern int TRACE;
extern int YES;
extern int NO;
extern float clocktime;

struct distance_table 
{
  int costs[4][4];
} dt0;

/* students to write the following two routines, and maybe some others */

void rtinit0() 
{
  int i, j;
  int mincost[4];
  struct rtpkt packet;
  
  /* Initialize the distance table with infinity */
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      dt0.costs[i][j] = 999;
    }
  }
  
  /* Set direct costs to neighbors */
  dt0.costs[0][0] = 0;    /* Cost to self is 0 */
  dt0.costs[1][1] = 1;    /* Direct cost to node 1 is 1 */
  dt0.costs[2][2] = 3;    /* Direct cost to node 2 is 3 */
  dt0.costs[3][3] = 7;    /* Direct cost to node 3 is 7 */
  
  /* Initialize minimum costs array */
  mincost[0] = 0;    /* Cost to self is 0 */
  mincost[1] = 1;    /* Minimum cost to node 1 is 1 */
  mincost[2] = 3;    /* Minimum cost to node 2 is 3 */
  mincost[3] = 7;    /* Minimum cost to node 3 is 7 */
  
  /* Create and send routing packets to all neighbors */
  
  /* Send to node 1 */
  creatertpkt(&packet, 0, 1, mincost);
  tolayer2(packet);
  
  /* Send to node 2 */
  creatertpkt(&packet, 0, 2, mincost);
  tolayer2(packet);
  
  /* Send to node 3 */
  creatertpkt(&packet, 0, 3, mincost);
  tolayer2(packet);
  
  printf("At time %f, rtinit0() called\n", clocktime);
  printdt0(&dt0);
}

void rtupdate0(struct rtpkt *rcvdpkt)
{
  int sourceid;
  int updated = 0;
  int dest, via;
  int newcost;
  int mincost[4];
  struct rtpkt packet;
  
  printf("At time %f, rtupdate0() called, received packet from %d\n", 
         clocktime, rcvdpkt->sourceid);
  
  sourceid = rcvdpkt->sourceid;
  
  /* For each destination node */
  for (dest = 0; dest < 4; dest++) {
    /* Skip updates to self */
    if (dest != 0) {
      /* Calculate the potential new cost to dest via the sourceid */
      newcost = dt0.costs[sourceid][sourceid] + rcvdpkt->mincost[dest];
      
      /* If new cost is better, update the distance table */
      if (newcost < dt0.costs[dest][sourceid]) {
        dt0.costs[dest][sourceid] = newcost;
        updated = 1;
      }
    }
  }
  
  /* If updates were made, recalculate minimum costs and send updates */
  if (updated) {
    /* Calculate current minimum costs to each destination */
    mincost[0] = 0;  /* Cost to self is always 0 */
    
    /* For each destination (except self), find minimum cost path */
    for (dest = 1; dest < 4; dest++) {
      mincost[dest] = 999;  /* Start with infinity */
      
      /* Check path via each neighbor */
      for (via = 1; via < 4; via++) {
        if (dt0.costs[dest][via] < mincost[dest]) {
          mincost[dest] = dt0.costs[dest][via];
        }
      }
    }
    
    /* Send updates to all neighbors */
    
    /* Send to node 1 */
    creatertpkt(&packet, 0, 1, mincost);
    tolayer2(packet);
    
    /* Send to node 2 */
    creatertpkt(&packet, 0, 2, mincost);
    tolayer2(packet);
    
    /* Send to node 3 */
    creatertpkt(&packet, 0, 3, mincost);
    tolayer2(packet);
    
    printf("At time %f, node 0 sends routing updates to neighbors\n", clocktime);
  }
  
  /* Print the updated distance table */
  printdt0(&dt0);
}


printdt0(dtptr)
  struct distance_table *dtptr;
  
{
  printf("                via     \n");
  printf("   D0 |    1     2    3 \n");
  printf("  ----|-----------------\n");
  printf("     1|  %3d   %3d   %3d\n",dtptr->costs[1][1],
	 dtptr->costs[1][2],dtptr->costs[1][3]);
  printf("dest 2|  %3d   %3d   %3d\n",dtptr->costs[2][1],
	 dtptr->costs[2][2],dtptr->costs[2][3]);
  printf("     3|  %3d   %3d   %3d\n",dtptr->costs[3][1],
	 dtptr->costs[3][2],dtptr->costs[3][3]);
}

linkhandler0(linkid, newcost)   
  int linkid, newcost;

/* called when cost from 0 to linkid changes from current value to newcost*/
{
  int i;
  int updated = 0;
  int mincost[4];
  struct rtpkt packet;
  
  printf("At time %f, linkhandler0() called for link to %d with newcost %d\n", 
         clocktime, linkid, newcost);
  
  /* Update the direct cost to the neighbor */
  dt0.costs[linkid][linkid] = newcost;
  
  /* Recalculate minimum costs to all destinations */
  mincost[0] = 0;  /* Cost to self is always 0 */
  
  /* For each destination (except self), find minimum cost path */
  for (i = 1; i < 4; i++) {
    mincost[i] = 999;  /* Start with infinity */
    
    /* Check path via each neighbor */
    if (dt0.costs[i][1] < mincost[i]) mincost[i] = dt0.costs[i][1];
    if (dt0.costs[i][2] < mincost[i]) mincost[i] = dt0.costs[i][2];
    if (dt0.costs[i][3] < mincost[i]) mincost[i] = dt0.costs[i][3];
  }
  
  /* Check if any minimum costs have changed */
  if (mincost[1] != dt0.costs[1][1] || 
      mincost[2] != dt0.costs[2][2] || 
      mincost[3] != dt0.costs[3][3]) {
    updated = 1;
  }
  
  /* If updates were made, send updates to all neighbors */
  if (updated) {
    /* Send to node 1 */
    creatertpkt(&packet, 0, 1, mincost);
    tolayer2(packet);
    
    /* Send to node 2 */
    creatertpkt(&packet, 0, 2, mincost);
    tolayer2(packet);
    
    /* Send to node 3 */
    creatertpkt(&packet, 0, 3, mincost);
    tolayer2(packet);
    
    printf("At time %f, node 0 sends routing updates to neighbors due to link change\n", clocktime);
  }
  
  /* Print the updated distance table */
  printdt0(&dt0);
}


// #include <stdio.h>

// extern struct rtpkt
// {
//     int sourceid;   /* id of sending router sending this pkt */
//     int destid;     /* id of router to which pkt being sent
//                        (must be an immediate neighbor) */
//     int mincost[4]; /* min cost to node 0 ... 3 */
// };

// extern int TRACE;
// extern int YES;
// extern int NO;
// extern float clocktime;

// /* Forward declarations for functions in distance_vector.c */
// extern void creatertpkt(struct rtpkt *initrtpkt, int srcid, int destid, int mincosts[]);
// extern void tolayer2(struct rtpkt packet);

// struct distance_table
// {
//     int costs[4][4];
// } dt0;

// /* Function prototypes */
// void rtinit0(void);
// void rtupdate0(struct rtpkt *rcvdpkt);
// int printdt0(struct distance_table *dtptr);
// int linkhandler0(int linkid, int newcost);

// void rtinit0()
// {
//     int i, j;
//     int mincost[4];
//     struct rtpkt packet;

//     /* Initialize the distance table with infinity */
//     for (i = 0; i < 4; i++)
//     {
//         for (j = 0; j < 4; j++)
//         {
//             dt0.costs[i][j] = 999;
//         }
//     }

//     /* Set direct costs to neighbors */
//     dt0.costs[0][0] = 0; /* Cost to self is 0 */
//     dt0.costs[1][1] = 1; /* Direct cost to node 1 is 1 */
//     dt0.costs[2][2] = 3; /* Direct cost to node 2 is 3 */
//     dt0.costs[3][3] = 7; /* Direct cost to node 3 is 7 */

//     /* Initialize minimum costs array */
//     mincost[0] = 0; /* Cost to self is 0 */
//     mincost[1] = 1; /* Minimum cost to node 1 is 1 */
//     mincost[2] = 3; /* Minimum cost to node 2 is 3 */
//     mincost[3] = 7; /* Minimum cost to node 3 is 7 */

//     /* Create and send routing packets to all neighbors */

//     /* Send to node 1 */
//     creatertpkt(&packet, 0, 1, mincost);
//     tolayer2(packet);

//     /* Send to node 2 */
//     creatertpkt(&packet, 0, 2, mincost);
//     tolayer2(packet);

//     /* Send to node 3 */
//     creatertpkt(&packet, 0, 3, mincost);
//     tolayer2(packet);

//     printf("At time %f, rtinit0() called\n", clocktime);
//     printdt0(&dt0);
// }

// void rtupdate0(struct rtpkt *rcvdpkt)
// {
//     int sourceid;
//     int updated = 0;
//     int dest, via;
//     int newcost;
//     int mincost[4];
//     struct rtpkt packet;

//     printf("At time %f, rtupdate0() called, received packet from %d\n",
//            clocktime, rcvdpkt->sourceid);

//     sourceid = rcvdpkt->sourceid;

//     /* For each destination node */
//     for (dest = 0; dest < 4; dest++)
//     {
//         /* Skip updates to self */
//         if (dest != 0)
//         {
//             /* Calculate the potential new cost to dest via the sourceid */
//             newcost = dt0.costs[sourceid][sourceid] + rcvdpkt->mincost[dest];

//             /* If new cost is better, update the distance table */
//             if (newcost < dt0.costs[dest][sourceid])
//             {
//                 dt0.costs[dest][sourceid] = newcost;
//                 updated = 1;
//             }
//         }
//     }

//     /* If updates were made, recalculate minimum costs and send updates */
//     if (updated)
//     {
//         /* Calculate current minimum costs to each destination */
//         mincost[0] = 0; /* Cost to self is always 0 */

//         /* For each destination (except self), find minimum cost path */
//         for (dest = 1; dest < 4; dest++)
//         {
//             mincost[dest] = 999; /* Start with infinity */

//             /* Check path via each neighbor */
//             for (via = 1; via < 4; via++)
//             {
//                 if (dt0.costs[dest][via] < mincost[dest])
//                 {
//                     mincost[dest] = dt0.costs[dest][via];
//                 }
//             }
//         }

//         /* Send updates to all neighbors */

//         /* Send to node 1 */
//         creatertpkt(&packet, 0, 1, mincost);
//         tolayer2(packet);

//         /* Send to node 2 */
//         creatertpkt(&packet, 0, 2, mincost);
//         tolayer2(packet);

//         /* Send to node 3 */
//         creatertpkt(&packet, 0, 3, mincost);
//         tolayer2(packet);

//         printf("At time %f, node 0 sends routing updates to neighbors\n", clocktime);
//     }

//     /* Print the updated distance table */
//     printdt0(&dt0);
// }

// int printdt0(struct distance_table *dtptr)
// {
//     printf("                via     \n");
//     printf("   D0 |    1     2    3 \n");
//     printf("  ----|-----------------\n");
//     printf("     1|  %3d   %3d   %3d\n", dtptr->costs[1][1],
//            dtptr->costs[1][2], dtptr->costs[1][3]);
//     printf("dest 2|  %3d   %3d   %3d\n", dtptr->costs[2][1],
//            dtptr->costs[2][2], dtptr->costs[2][3]);
//     printf("     3|  %3d   %3d   %3d\n", dtptr->costs[3][1],
//            dtptr->costs[3][2], dtptr->costs[3][3]);
//     return 0;
// }

// int linkhandler0(int linkid, int newcost)
// {
//     /* called when cost from 0 to linkid changes from current value to newcost*/
//     /* You can leave this routine empty if you're an undergrad. If you want */
//     /* to use this routine, you'll need to change the value of the LINKCHANGE */
//     /* constant definition in prog3.c from 0 to 1 */
//     return 0;
// }