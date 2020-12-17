#include <stdio.h>
#include "project3.h"
#include <stdlib.h>

extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt1;
struct NeighborCosts   *neighbor1;

int neighbors1[MAX_NODES];

/* students to write the following two routines, and maybe some others */

void sendRoutePackets1() {
    // Create and initialize route packet structure, except for destid.
    struct RoutePacket *routeStruct = (struct RoutePacket *)malloc( sizeof (struct RoutePacket));
    routeStruct->sourceid = 1;
    
    int i;
    for (i = 0; i < MAX_NODES; i++) {
        routeStruct->mincost[i]=dt1.costs[1][i];
    }
    
    // Send RoutePackets to all neighbors
    for (i = 0; i < MAX_NODES; i++) {
        if (neighbors1[i] == NO) continue;
        printf("At time t=%f, node 1 sends packet to node %d with: %d %d %d %d\n", clocktime, i, dt1.costs[1][0], dt1.costs[1][1], dt1.costs[1][2], dt1.costs[1][3]);
        routeStruct->destid = i;
        toLayer2(*routeStruct);
    }
}

void rtinit1() {
    printf("At time t=%f, rtinit1() called.\n", clocktime);
    // Initialize distance table to INFINITY and neighbors array to NO for all edges
    int i, j;
    for (i = 0; i < MAX_NODES; i++) {
        for (j = 0; j < MAX_NODES; j++) {
            dt1.costs[i][j] = INFINITY;
        }
        neighbors1[i] = NO;
    }
    // Get costs to neighbors
    struct NeighborCosts *costs = getNeighborCosts(1);
    
    // Update distance table for neighbors
    for (i = 0; i < MAX_NODES; i++) {
        if (costs->NodeCosts[i] < INFINITY) {
            dt1.costs[1][i] = costs->NodeCosts[i];
            neighbors1[i] = YES;
        }
    }
    // Remove self from neighbors array
    neighbors1[1] = NO;
    
    
    printf("At time t=%f, node 1 initial distance vector: %d %d %d %d\n", clocktime, dt1.costs[1][0], dt1.costs[1][1], dt1.costs[1][2], dt1.costs[1][3]);
    
    // Update neighbors
    sendRoutePackets1();
}


void rtupdate1( struct RoutePacket *rcvdpkt ) {
    printf("At time t=%f, rtupdate1() called, by a pkt received from Sender id: %d.\n", clocktime, rcvdpkt->sourceid);
    // Current shortest distance to node of origin
    int destDistance = dt1.costs[1][rcvdpkt->sourceid];
    
    // Only want to send out new packets once, so wait until loop is finished
    int changedFlag = NO;
    
    int i;
    for (i = 0; i < MAX_NODES; i++) {
        int oldDistance = dt1.costs[1][i];
        int newDistance = destDistance + rcvdpkt->mincost[i];
        if (newDistance < oldDistance) {
            changedFlag = YES;
            dt1.costs[1][i] = newDistance;
        }
        printf("At time t=%f, node 1 current distance vector: %d %d %d %d\n", clocktime, dt1.costs[1][0], dt1.costs[1][1], dt1.costs[1][2], dt1.costs[1][3]);
    }
    if (changedFlag) {
        sendRoutePackets1();
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
void printdt1( int MyNodeNumber, struct NeighborCosts *neighbor, 
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
}    // End of printdt1

