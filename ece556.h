// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#ifndef ECE556_H
#define ECE556_H

#include <stdio.h>
#include <sys/time.h>

#define MAX_ALLOWED_RUNTIME 900 // time in seconds

/**
 * A structure to represent a 2D Point.
 */
typedef struct
{
  int x; /* x coordinate ( >=0 in the routing grid)*/
  int y; /* y coordinate ( >=0 in the routing grid)*/

} point;

/**
 * A structure to represent a segment
 */
typedef struct
{
  point p1; /* start point of a segment */
  point p2; /* end point of a segment */

  int numEdges; /* number of edges in the segment*/
  int *edges;   /* array of edges representing the segment*/

} segment;

/**
 * A structure to represent a route
 */
typedef struct
{
  int numSegs;       /* number of segments in a route*/
  segment *segments; /* an array of segments (note, a segment may be flat, L-shaped or any other shape, based on your preference */
  long int cost;

} route;

/**
 * A structure to represent nets
 */
typedef struct
{

  int id;       /* ID of the net */
  int numPins;  /* number of pins (or terminals) of the net */
  point *pins;  /* array of pins (or terminals) of the net. */
  route nroute; /* stored route for the net. */

} net;

/**
 * A structure to represent the routing instance
 */
typedef struct
{
  int gx; /* x dimension of the global routing grid */
  int gy; /* y dimension of the global routing grid */

  int cap;

  int numNets; /* number of nets */
  net *nets;   /* array of nets */

  int numEdges;   /* number of edges of the grid */
  int *edgeCaps;  /* array of the actual edge capacities after considering for blockages */
  int *edgeUtils; /* array of edge utilizations */

} routingInst;

/* int readBenchmark(const char *fileName, routingInst *rst)
   Read in the benchmark file and initialize the routing instance.
   This function needs to populate all fields of the routingInst structure.
   input1: fileName: Name of the benchmark input file
   input2: pointer to the routing instance
   output: 1 if successful
*/
int readBenchmark(const char *fileName, routingInst *rst);

/*
Generate two-terminal subnets for multi-terminal nets
This step is essentially reordering of the entries in
the pins array of the net data structure, assuming each pair
of consecutive pins in the array make a subnet

We use RSMT which is a generalization of RMST where in addition to
the original nodes in the graph, new nodes called
“Steiner Points” may be added to the graph which may help further reduce the
total edge cost

input: pointer to the routing instance
output: 1 if successful, 0 otherwise
*/
int subnetGen(routingInst *rst);

/*
Updates shortest path to MBB with the new distToSteinerPt
*/
void update_shortestPath(int &shortestPath, int &distToSteinerPt, point &shortestPath_pC, point &pC);

/*
RSMT is a generalization of Rectilinear minimum spanning tree where in addition to
the original nodes in the graph, new nodes called “Steiner Points” may be
added to the graph which may help further reduce the total edge cost

The function takes in the coordinates of the two current points of the current MBB acc. to pin_itr, and finds the RSMT distance to pC in consideration.
*/
void RSMT(int &MBB_x1, int &MBB_x2, int &MBB_y1, int &MBB_y2, int &distToSteinerPt, int &shortestPath, point &pC, point &shortestPath_pC, int &pin_itr);

/* int solveRouting(routingInst *rst)
   This function creates a routing solution
   input: pointer to the routing instance
   output: 1 if successful, 0 otherwise
*/
int solveRouting(routingInst *rst);

/*
1>We first update edge weights
2>We then calculate net ordering
3>Given the updated edge weights at the beginning of each RRR
iteration, for each net n, we calculate a cost
4>For each net in the new order, we remove the existing route stored for the net by
decreasing the edge utilizations corresponding to the route and then reroute to generate a better route (of lower cost) for the net

   input: pointer to the routing instance, starttime
   output: 1 if successful, 0 otherwise
*/
int rrr(routingInst *rst, timeval startTime);

/* int writeOutput(const char *outRouteFile, routingInst *rst)
   Write the routing solution obtained from solveRouting().
   Refer to the project link for the required output format.

   Finally, make sure your generated output file passes the evaluation script to make sure
   it is in the correct format and the nets have been correctly routed. The script also reports
   the total wirelength and overflow of your routing solution.

   input1: name of the output file
   input2: pointer to the routing instance
   output: 1 if successful, 0 otherwise
  */
int writeOutput_rrr(const char *outRouteFile, routingInst *rst);
int writeOutput_sub(const char *outRouteFile, routingInst *rst);

/* int release(routingInst *rst)
   Release the memory for all the allocated data structures.
   Failure to release may cause memory problems after multiple runs of your program.
   Need to recursively delete all memory allocations from bottom to top
   (starting from segments then routes then individual fields within a net struct,
   then nets, then the fields in a routing instance, and finally the routing instance)

   output: 1 if successful, 0 otherwise
*/
int release(routingInst *rst);

#endif // ECE556_H
