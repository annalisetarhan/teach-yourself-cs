#include <stdio.h>
#include "project3.h"
#include <stdlib.h>

extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt0;
struct NeighborCosts   *neighbor0;

int neighbors0[MAX_NODES];

/* students to write the following two routines, and maybe some others */

void sendRoutePackets0() {
    // Create and initialize route packet structure, except for destid.
    struct RoutePacket *routeStruct = (struct RoutePacket *)malloc( sizeof (struct RoutePacket));
    routeStruct->sourceid=0;
    
    int i;
    for (i = 0; i < MAX_NODES; i++) {
        routeStruct->mincost[i]=dt0.costs[0][i];
    }
    
    // Send RoutePackets to all neighbors
    for (i = 0; i < MAX_NODES; i++) {
        if (neighbors0[i] == NO) continue;
        printf("At time t=%f, node 0 sends packet to node %d with: %d %d %d %d\n", clocktime, i, dt0.costs[0][0], dt0.costs[0][1], dt0.costs[0][2], dt0.costs[0][3]);
        routeStruct->destid = i;
        toLayer2(*routeStruct);
    }
}

void rtinit0() {
    printf("At time t=%f, rtinit0() called.\n", clocktime);
    // Initialize distance table to INFINITY and neighbors array to NO for all edges
    int i, j;
    for (i = 0; i < MAX_NODES; i++) {
        for (j = 0; j < MAX_NODES; j++) {
            dt0.costs[i][j] = INFINITY;
        }
        neighbors0[i] = NO;
    }
    // Get costs to neighbors
    struct NeighborCosts *costs = getNeighborCosts(0);
    
    // Update distance table for neighbors
    for (i = 0; i < MAX_NODES; i++) {
        if (costs->NodeCosts[i] < INFINITY) {
            dt0.costs[0][i] = costs->NodeCosts[i];
            neighbors0[i] = YES;
        }
    }
    // Remove self from neighbors array
    neighbors0[0] = NO;
    
    printf("At time t=%f, node 0 initial distance vector: %d %d %d %d\n", clocktime, dt0.costs[0][0], dt0.costs[0][1], dt0.costs[0][2], dt0.costs[0][3]);
    
    // Update neighbors
    sendRoutePackets0();
}


void rtupdate0( struct RoutePacket *rcvdpkt ) {
    printf("At time t=%f, rtupdate0() called, by a pkt received from Sender id: %d.\n", clocktime, rcvdpkt->sourceid);
    // Current shortest distance to node of origin
    int destDistance = dt0.costs[0][rcvdpkt->sourceid];
    
    // Only want to send out new packets once, so wait until loop is finished
    int changedFlag = NO;
    
    int i;
    for (i = 0; i < MAX_NODES; i++) {
        int oldDistance = dt0.costs[0][i];
        int newDistance = destDistance + rcvdpkt->mincost[i];
        if (newDistance < oldDistance) {
            changedFlag = YES;
            dt0.costs[0][i] = newDistance;
        }
        printf("At time t=%f, node 0 current distance vector: %d %d %d %d\n", clocktime, dt0.costs[0][0], dt0.costs[0][1], dt0.costs[0][2], dt0.costs[0][3]);
    }
    if (changedFlag) {
        sendRoutePackets0();
    } 
}



/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt0

