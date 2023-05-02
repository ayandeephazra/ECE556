// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <climits>
using namespace std;

int readNets(FILE *fp, routingInst *rst);
void netDecompose(routingInst *rst);
int readBlockages(FILE *fp, routingInst *rst);
void getEdgePts(routingInst *rst, int edgeID, point *pt1, point *pt2);
int getEdgeID(routingInst *rst, int x1, int y1, int x2, int y2);

int readBenchmark(const char *fileName, routingInst *rst)
{
  /* Try to open file */
  FILE *fp;

  if (fileName == NULL || rst == NULL)
  {
    fprintf(stderr, "Passed null arg to readBenchmark.\n");
    return EXIT_FAILURE;
  }

  fp = fopen(fileName, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "Couldn't open input file.\n");
    return EXIT_FAILURE;
  }

  /* Parse file and fill in rst */
  /* grid x, y */
  if (fscanf(fp, "%*s %d %d ", &(rst->gx), &(rst->gy)) != 2)
  {
    fprintf(stderr, "Failed reading grid arg.\n");
    return EXIT_FAILURE;
  }

  /* max edge capacity */
  if (fscanf(fp, "%*s %d ", &(rst->cap)) != 1)
  {
    fprintf(stderr, "Failed to read capacity.\n");
    return EXIT_FAILURE;
  }

  /* number of nets */
  if (fscanf(fp, "%*s %*s %d ", &(rst->numNets)) != 1)
  {
    fprintf(stderr, "Failed to read numer of nets.\n");
    return EXIT_FAILURE;
  }

  /* read all nets */
  if (readNets(fp, rst) == EXIT_FAILURE)
  {
    fprintf(stderr, "Failed to read nets from input file.\n");
    return EXIT_FAILURE;
  }

  /* calculate total number of edges, and use this
   * to mallocate enough space for edgeCaps and edgeUtils
   */
  rst->numEdges = rst->gy * (rst->gx - 1) + rst->gx * (rst->gy - 1);

  rst->edgeCaps = (int *)malloc(rst->numEdges * sizeof(int));
  if (rst->edgeCaps == NULL)
  {
    fprintf(stderr, "Couldn't malloc rst->edgeCaps.\n");
    return EXIT_FAILURE;
  }

  /* initialize edgeUtils to 0, edgeCaps to cap */
  rst->edgeUtils = (int *)malloc(rst->numEdges * sizeof(int));
  if (rst->edgeUtils == NULL)
  {
    fprintf(stderr, "couldn't allocate edgeUtils.\n");
    return EXIT_FAILURE;
  }

  for (int i = 0; i < rst->numEdges; i++)
  {
    rst->edgeCaps[i] = rst->cap;
    rst->edgeUtils[i] = 0;
  }

  /* read in blockages */
  if (readBlockages(fp, rst) == EXIT_FAILURE)
  {
    fprintf(stderr, "Failed to read blockages from input file.\n");
    return EXIT_FAILURE;
  }

  if (fclose(fp) != 0)
  {
    fprintf(stderr, "There was an error on closing the input file.\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* parses input file for nets, and allocates them into the
 * routingInst.
 *
 * returns:
 * EXIT_FAILURE - on fail
 * EXIT_SUCCESS - on successfull read
 */
int readNets(FILE *fp, routingInst *rst)
{

  int i, j;

  /* parameter check */
  if (fp == NULL || rst == NULL)
  {
    fprintf(stderr, "Passed null arg to readNets.\n");
    return EXIT_FAILURE;
  }

  /* allocate memory for nets array */
  rst->nets = (net *)malloc(rst->numNets * sizeof(net));
  if (rst->nets == NULL)
  {
    fprintf(stderr, "Failed to allocate nets.\n");
    return EXIT_FAILURE;
  }

  for (i = 0; i < rst->numNets; i++)
  {
    /* initialize nroute.segments to NULL */
    rst->nets[i].nroute.segments = NULL;

    /* get net name and  number of pins of net */
    /*if(fscanf(fp, "%*c"); */
    if (fscanf(fp, "%*c%d %d ", &(rst->nets[i].id), &(rst->nets[i].numPins)) != 2)
    {
      fprintf(stderr, "Failed to get net or num pins on iter:  %d.\n", i);
      return EXIT_FAILURE;
    }

    /* create array of pins */
    rst->nets[i].pins = (point *)malloc(rst->nets[i].numPins * sizeof(point));
    if (rst->nets[i].pins == NULL)
    {
      fprintf(stderr, "Failed to allocate pin array for net %d.\n", rst->nets[i].id);
      return EXIT_FAILURE;
    }

    /* populate array */
    for (j = 0; j < rst->nets[i].numPins; j++)
    {
      if (fscanf(fp, "%d %d ", &(rst->nets[i].pins[j].x), &(rst->nets[i].pins[j].y)) != 2)
      {
        fprintf(stderr, "Failed to read pin coordinates of net %d.\n", rst->nets[i].id);
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}

/* Takes original routing instance after all nets are read from input file
 * and decomposes each net. New net format: (pin0, pin1, pin2, etc.) where
 * pin0 and pin1 are the closest pair of points and the following point pin2
 * is the closest point to the Minimum Bounding Box of pin0 and pin 1
 */
void netDecompose(routingInst *rst)
{
  int i, j, k, l, m;
  point start0, start1, test0, test1, pt, closestPt;
  int recDistance, minDistance;
  int x, x1, x2, y, y1, y2;
  bool contains;
  point *newList = NULL;

  /** Loop through all nets in the routing instance and decompose each **/
  for (i = 0; i < rst->numNets; i++)
  {
    /**find closest pair of points to form initial MBB - set first two points as initial solution**/
    minDistance = abs(rst->nets[i].pins[0].x - rst->nets[i].pins[1].x) + abs(rst->nets[i].pins[0].y - rst->nets[i].pins[1].y);
    start0 = rst->nets[i].pins[0];
    start1 = rst->nets[i].pins[1];
    for (j = 0; j < rst->nets[i].numPins; j++)
    {
      for (k = 0; k < rst->nets[i].numPins; k++)
      {
        recDistance = abs(rst->nets[i].pins[j].x - rst->nets[i].pins[k].x) + abs(rst->nets[i].pins[j].y - rst->nets[i].pins[k].y);
        /**if not same point and rect. dist. less than minimum tested, update start MBB points**/
        if ((recDistance != 0) && (recDistance < minDistance))
        {
          minDistance = recDistance;
          start0 = rst->nets[i].pins[j];
          start1 = rst->nets[i].pins[k];
        }
      }
    }

    /**Place closest points first in temporary list and continue decomposition**/
    newList = (point *)realloc(newList, rst->nets[i].numPins * sizeof(point));
    newList[0] = start0;
    newList[1] = start1;
    for (k = 0; k < (rst->nets[i].numPins - 2); k++)
    {
      /** MBB to test against is MBB of last two poinst in temp list**/
      test0 = newList[k];
      test1 = newList[k + 1];
      closestPt.x = INT_MAX;
      closestPt.y = INT_MAX;
      minDistance = INT_MAX;
      /**Loop through all pins in net to determine closest to MBB**/
      for (l = 0; l < rst->nets[i].numPins; l++)
      {
        pt = rst->nets[i].pins[l];
        // Check if temp list already contains the test point before running
        contains = false;
        for (m = 0; m < k + 2; m++)
        {
          if ((pt.x == newList[m].x) && (pt.y == newList[m].y))
          {
            contains = true;
          }
        }
        // if point not mapped, find its rectilinear distance to the MBB
        if (!contains)
        {
          x = pt.x;
          y = pt.y;
          x1 = test0.x;
          x2 = test1.x;
          y1 = test0.y;
          y2 = test1.y;
          //
          if (((x1 <= x) && (x <= x2)) || ((x2 <= x) && (x <= x1)))
          {
            if (abs(y - y1) < abs(y - y2))
            {
              recDistance = abs(y - y1);
            }
            else
            {
              recDistance = abs(y - y2);
            }

            if (recDistance < minDistance)
            {
              minDistance = recDistance;
              closestPt = pt;
            }
          }
          else if (((y1 <= y) && (y <= y2)) || ((y2 <= y) && (y <= y1)))
          {
            if (abs(x - x1) < abs(x - x2))
            {
              recDistance = abs(x - x1);
            }
            else
            {
              recDistance = abs(x - x2);
            }

            if (recDistance < minDistance)
            {
              minDistance = recDistance;
              closestPt = pt;
            }
          }
          else if ((((x < x1) && (x1 <= x2)) && ((y > y1) && (y1 >= y2))) ||
                   (((x < x1) && (x1 <= x2)) && ((y < y1) && (y1 <= y2))) ||
                   (((x > x1) && (x1 >= x2)) && ((y > y1) && (y1 >= y2))) ||
                   (((x > x1) && (x1 >= x2)) && ((y < y1) && (y1 <= y2))))
          {
            recDistance = abs(x - x1) + abs(y - y1);

            if (recDistance < minDistance)
            {
              minDistance = recDistance;
              closestPt = pt;
            }
          }
          else if ((((x < x2) && (x2 <= x1)) && ((y > y2) && (y2 >= y1))) ||
                   (((x < x2) && (x2 <= x1)) && ((y < y2) && (y2 <= y1))) ||
                   (((x > x2) && (x2 >= x1)) && ((y > y2) && (y2 >= y1))) ||
                   (((x > x2) && (x2 >= x1)) && ((y < y2) && (y2 <= y1))))
          {
            recDistance = abs(x - x2) + abs(y - y2);

            if (recDistance < minDistance)
            {
              minDistance = recDistance;
              closestPt = pt;
            }
          }
          else if ((((x < x2) && (x2 <= x1)) && ((y > y1) && (y1 >= y2))) ||
                   (((x < x2) && (x2 <= x1)) && ((y < y1) && (y1 <= y2))) ||
                   (((x > x2) && (x2 >= x1)) && ((y > y1) && (y1 >= y2))) ||
                   (((x > x2) && (x2 >= x1)) && ((y < y1) && (y1 <= y2))))
          {
            recDistance = abs(x - x2) + abs(y - y1);

            if (recDistance < minDistance)
            {
              minDistance = recDistance;
              closestPt = pt;
            }
          }
          else if ((((x < x1) && (x1 <= x2)) && ((y > y2) && (y2 >= y1))) ||
                   (((x < x1) && (x1 <= x2)) && ((y < y2) && (y2 <= y1))) ||
                   (((x > x1) && (x1 >= x2)) && ((y > y2) && (y2 >= y1))) ||
                   (((x > x1) && (x1 >= x2)) && ((y < y2) && (y2 <= y1))))
          {
            recDistance = abs(x - x1) + abs(y - y2);

            if (recDistance < minDistance)
            {
              minDistance = recDistance;
              closestPt = pt;
            }
          }
        }
      }
      // add closest point to MBB to the end of the temp list
      newList[k + 2] = closestPt;
    }
    // once temp list is complete, copy it pins list for the net in the main routing instance
    for (m = 0; m < rst->nets[i].numPins; m++)
    {
      rst->nets[i].pins[m].x = newList[m].x;
      rst->nets[i].pins[m].y = newList[m].y;
    }
  }
  free(newList);
}

/* reads in the two endpoints of an edge
 * and updates edgeCaps. sets other
 * capacities to default value read in
 *
 * mallocs edgeCaps
 *
 * returns:
 * EXIT_FAILURE on fail
 * EXIT_SUCCESS on pass
 */
int readBlockages(FILE *fp, routingInst *rst)
{
  int i;
  int numBlock;
  int newCap;

  point p1, p2;

  if (fp == NULL || rst == NULL)
  {
    fprintf(stderr, "Null args passed to readBlockages.\n");
    return EXIT_FAILURE;
  }

  /* number of Blockages */
  if (fscanf(fp, "%d ", &numBlock) != 1)
  {
    fprintf(stderr, "Failed to read # of blockages.\n");
    return EXIT_FAILURE;
  }

  /* read blockages from file, set appropiate edge to 0 */
  for (i = 0; i < numBlock; i++)
  {
    /* read terminal locations */
    if (fscanf(fp, "%d %d %d %d ", &p1.x, &p1.y, &p2.x, &p2.y) != 4)
    {
      fprintf(stderr, "Couldn't read blockage endpoints.\n");
      return EXIT_FAILURE;
    }

    if (fscanf(fp, "%d ", &newCap) != 1)
    {
      fprintf(stderr, "Couldn't read blockage capacity.\n");
      return EXIT_FAILURE;
    }

    int ID = getEdgeID(rst, p1.x, p1.y, p2.x, p2.y);
    rst->edgeCaps[ID - 1] = newCap;
  }

  return EXIT_SUCCESS;
}

void printRoutingInst(routingInst rst)
{
  int i, j;
  printf("*******routingInst info************\n");
  printf("numNets: %d\n", rst.numNets);
  for (i = 0; i < rst.numNets; i++)
  {
    printf("\n>n%d\n", rst.nets[i].id);
    printf("->numPins: %d\n", rst.nets[i].numPins);
    for (j = 0; j < rst.nets[i].numPins; j++)
    {
      printf("--> (%d, %d)\n", rst.nets[i].pins[j].x, rst.nets[i].pins[j].y);
    }
  }
}

/* This function calculates the integer edge ID for an edge.
 * the edge id is determined as follows for a 3x3 grid
 *
 *  + 5 + 6 +
 *  8   10  12
 *  + 3 + 4 +
 *  7   9   11
 *  + 1 + 2 +
 */
int getEdgeID(routingInst *rst, int x1, int y1, int x2, int y2)
{
  /* determine if horiz or vertical */
  if (y2 > y1)
  {
    /* vertical, (x2,y2) on top */
    return (y2 + (rst->gx - 1) * (rst->gy) + x1 * (rst->gy - 1));
  }
  else if (y2 < y1)
  {
    /* vertical, (x1,y1) on top */
    return (y1 + (rst->gx - 1) * (rst->gy) + x1 * (rst->gy - 1));
  }
  else if (x2 > x1)
  {
    /* horiz, (x2, y2) on right */
    return (x2 + y1 * (rst->gx - 1));
  }
  else
  {
    /* horiz, (x1, y1) on right */
    return (x1 + y1 * (rst->gx - 1));
  }
}

int getEdgeID(routingInst *rst, point p1, point p2)
{
  return getEdgeID(rst, p1.x, p1.y, p2.x, p2.y);
}

/* this function takes in an edge id, and fills in the passed
 * in points with the edge points.  It follows the same numbering
 * described above for getEdgeID
 */
void getEdgePts(routingInst *rst, int edgeID, point *pt1, point *pt2)
{
  int vEdgeID;

  vEdgeID = edgeID - (rst->gx - 1) * (rst->gy);
  /* first determine the direction of the edge by its magnitude */
  if (vEdgeID > 0)
  {
    /* Vertical segment */
    /* vEdgeId  = larger Y +  X * (rst->gy - 1) */
    /* X = (int) vEdgeId / (rst->gy) */
    pt1->x = (vEdgeID - 1) / (rst->gy - 1);
    pt2->x = pt1->x;

    pt2->y = vEdgeID - pt2->x * (rst->gy - 1);
    pt1->y = pt2->y - 1;
  }
  else
  {
    /* horizontal segment */
    /* edgeID = larger X + Y * (rst->gx - 1)  */
    pt1->y = (edgeID - 1) / (rst->gx - 1);
    pt2->y = pt1->y;

    pt2->x = edgeID - pt2->y * (rst->gx - 1);
    pt1->x = pt2->x - 1;
  }
}

void routeHorizontal(routingInst *rst, int i, int j, int dx, int dy)
{
  int k;
  int id;
  int y;

  /* adjust y coord if the vertical part was routed first */
  y = rst->nets[i].pins[j].y + dy;

  if (dx < 0)
  {
    /* -dx means p1 is right of p2, route left */
    for (k = 0; k < -dx; k++)
    {
      id = getEdgeID(rst,
                     rst->nets[i].pins[j].x - k,
                     y,
                     rst->nets[i].pins[j].x - k - 1,
                     y);
      rst->nets[i].nroute.segments[j].edges[k + abs(dy)] = id;
    }
  }
  else
  {
    /* +dx means p1 is left of p2, route right */
    for (k = 0; k < dx; k++)
    {
      id = getEdgeID(rst,
                     rst->nets[i].pins[j].x + k,
                     y,
                     rst->nets[i].pins[j].x + k + 1,
                     y);
      rst->nets[i].nroute.segments[j].edges[k + abs(dy)] = id;
    }
  }
}

/* this function routes the vertical edges between
 * pin[j] and pin[j+1] (using dy)
 */
void routeVertical(routingInst *rst, int i, int j, int dx, int dy)
{

  int k, id;
  int x;

  /* adjust x coord of vert route if horiz is already done */
  x = rst->nets[i].pins[j].x + dx;

  if (dy < 0)
  {
    /* -dy means p1 above p2, must go down */
    for (k = 0; k < -dy; k++)
    {
      id = getEdgeID(rst,
                     x,
                     rst->nets[i].pins[j].y - k,
                     x,
                     rst->nets[i].pins[j].y - k - 1);
      rst->nets[i].nroute.segments[j].edges[k + abs(dx)] = id;
    }
  }
  else
  {
    /* +dy means p1 is below p2, route up */
    for (k = 0; k < dy; k++)
    {
      id = getEdgeID(rst,
                     x,
                     rst->nets[i].pins[j].y + k,
                     x,
                     rst->nets[i].pins[j].y + k + 1);
      rst->nets[i].nroute.segments[j].edges[k + abs(dx)] = id;
    }
  }
}

int NumEdges(int p1_x, int p1_y, int p2_x, int p2_y)
{
  int num_edges = abs(p1_x - p2_x) + abs(p1_y - p2_y);

  return num_edges;
}

int solveRouting1(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/
  int x, y;
  point p1, p2;
  int x_diff_abs, y_diff_abs;

  for (auto net_itr = 0; net_itr != rst->numNets; net_itr++)
  {
    /* Number of segments = Number of pins - 1 */
    rst->nets[net_itr].nroute.numSegs = rst->nets[net_itr].numPins - 1;
    printf("For net%d, number of pins is %d, and number of segments is %d \n", net_itr, rst->nets[net_itr].numPins, rst->nets[net_itr].nroute.numSegs);
    printf("1\n");
    rst->nets[net_itr].nroute.segments = (segment *)malloc(rst->nets[net_itr].nroute.numSegs * sizeof(segment));
    printf("2\n");
    /* Marking start-end pins and it's segments for different types of route
    |                     --------                                   |
    |                     |                                          |
    |                     |                                          |
    |________      or     |            OR     _________    OR        |
    */

    int pin_num = 0;
    for (auto seg_itr = 0; seg_itr != rst->nets[net_itr].nroute.numSegs; seg_itr++)
    {
      printf("3\n");
      p1.x = rst->nets[net_itr].pins[pin_num].x;   /* x coordinate of start point p1( >=0 in the routing grid)*/
      p1.y = rst->nets[net_itr].pins[pin_num++].y; /* y coordinate of end point p1  ( >=0 in the routing grid)*/
      p2.x = rst->nets[net_itr].pins[pin_num].x;   /* x coordinate of start point p2( >=0 in the routing grid)*/
      p2.y = rst->nets[net_itr].pins[pin_num].y;   /* y coordinate of end point p2  ( >=0 in the routing grid)*/

      rst->nets[net_itr].nroute.segments[seg_itr].p1 = p1; /* start point of current segment */
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = p2; /* end point of current segment */
      printf("4\n");
      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = NumEdges(p1.x, p2.x, p1.y, p2.y); /* number of edges in the segment*/

      rst->nets[net_itr].nroute.segments[seg_itr].edges = new int[abs(p1.x - p2.x) + abs(p1.y - p2.y)]; /* array of edges representing the segment*/

      printf("5\n");
      /*
      P2 is above P1 and P2 is to the right of P1
      */
      int x_diff = p2.x - p1.x;
      int y_diff = p2.y - p1.y;
      int dx = rst->nets[net_itr].pins[seg_itr + 1].x - rst->nets[net_itr].pins[seg_itr].x;
      int dy = rst->nets[net_itr].pins[seg_itr + 1].y - rst->nets[net_itr].pins[seg_itr].y;
      /*
      P2 is below P1 and P2 is to the left of P1
      */
      if (p1.x > p2.x)
      {
        x_diff_abs = abs(x_diff);
        y_diff_abs = abs(y_diff);
      }

      /*
      For numbering the edge ids, we first number the horizontal edges from left to right starting from the bottom most one. We then number the vertical edges starting from the left most one then moving up.
        ____e5_______e6__
      e8|     e10|       |e12
        |___e3___|___e4__|
      e7|      e9|       |e11
        |___e1___|___e2__|
      */

      if (p1.x > p2.x)
      {
        routeHorizontal(rst, net_itr, seg_itr, dx, 0);
        routeVertical(rst, net_itr, seg_itr, dx, dy);
        /*
         Routing Horizontally towards left side
        */
        /*
         y = rst->nets[net_itr].pins[seg_itr].y;
         for (auto itr = 0; itr != x_diff_abs; itr++)
         {
           rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x - itr) + y * (rst->gx - 1);
         }
         /*
         Routing Vertically downwards
         */
        /*
         x = rst->nets[net_itr].pins[seg_itr].x + x_diff;
         for (auto itr = 0; itr != y_diff_abs; itr++)
         {
           rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(x_diff)] = (rst->nets[net_itr].pins[seg_itr].y - itr) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
         }
         */
      }
      else
      {
        routeVertical(rst, net_itr, seg_itr, 0, dy);
        routeHorizontal(rst, net_itr, seg_itr, dx, dy);
        /*
        Routing Vertically upwards
        */
        /*
         x = rst->nets[net_itr].pins[seg_itr].x;
         for (auto itr = 0; itr != abs(y_diff); itr++)
         {
           rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(x_diff)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
         }
         /*
          Routing Horizontally towards right side
         */
        /*
         y = rst->nets[net_itr].pins[seg_itr].y + y_diff;
         for (auto itr = 0; itr != abs(x_diff); itr++)
         {
           rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
         }
         */
      }
    }
  }

  return EXIT_SUCCESS;
}

int solveRouting(routingInst *rst)
{
  /* Go through all nets in routing instance.
   * Connect pairs of pins via shortest route */

  /* for part one, time and overflow don't matter,
   * but for later parts it might? */
  int i, j;
  int dx, dy;
  int x, y;

  for (i = 0; i < rst->numNets; i++)
  {
    /* route all pins of a net */
    /* assume 1 segment between every 2 pts, so
     * numpins-1 segments */
    rst->nets[i].nroute.numSegs = rst->nets[i].numPins - 1;
    rst->nets[i].nroute.segments = (segment *)malloc(rst->nets[i].nroute.numSegs * sizeof(segment));
    if (rst->nets[i].nroute.segments == NULL)
    {
      fprintf(stderr, "Failed to allocate memory for route segments.\n");
      return EXIT_FAILURE;
    }

    for (j = 0; j < rst->nets[i].nroute.numSegs; j++)
    {
      /* use L or 7  (or straight line) segments,
       * so set starting and ending points from pins */
      rst->nets[i].nroute.segments[j].p1 = rst->nets[i].pins[j];
      rst->nets[i].nroute.segments[j].p2 = rst->nets[i].pins[j + 1];

      /*route between pin[j] and pin [j+1] */
      /* find delta x and delta y */
      dx = rst->nets[i].pins[j + 1].x - rst->nets[i].pins[j].x;
      dy = rst->nets[i].pins[j + 1].y - rst->nets[i].pins[j].y;

      /* calc number of edges using dx and dy */
      rst->nets[i].nroute.segments[j].numEdges =
          abs(dx) + abs(dy);

      rst->nets[i].nroute.segments[j].edges =
          (int *)malloc(rst->nets[i].nroute.segments[j].numEdges * sizeof(int));
      if (rst->nets[i].nroute.segments[j].edges == NULL)
      {
        fprintf(stderr, "Failed to allocate memory for edges array.\n");
        return EXIT_FAILURE;
      }

      /* use sign of dx to determine if route goes horiz. or
       * vert. from p1 first */
      if (dx < 0)
      {

         routeHorizontal(rst, i, j, dx, 0);

        routeVertical(rst, i, j, dx, dy);
      }
      else
      {
        routeVertical(rst, i, j, 0, dy);
        routeHorizontal(rst, i, j, dx, dy);
      }
    }
  }

  return EXIT_SUCCESS;
}

int writeOutput(const char *outRouteFile, routingInst *rst)
{
  int i, j, k;
  point p1, p2;
  int dxprev, dyprev, dx, dy;
  point p1prev, p2prev;
  FILE *fp;

  /* check input parameters */
  if (outRouteFile == NULL || rst == NULL)
  {
    fprintf(stderr, "Passed null arg into writeOutput.\n");
    return EXIT_FAILURE;
  }

  fp = fopen(outRouteFile, "w");
  if (fp == NULL)
  {
    fprintf(stderr, "Output file could not be created.\n");
    return EXIT_FAILURE;
  }

  /* initialize stored points */
  p1prev.x = 0;
  p1prev.y = 0;
  p2prev.x = 0;
  p2prev.y = 0;

  /* for all nets in rst,
   * go through route  segments - for all segments,
   * go through all edges, printing out the corresponding pts */
  for (i = 0; i < rst->numNets; i++)
  {
    fprintf(fp, "n%d\n", rst->nets[i].id);

    for (j = 0; j < rst->nets[i].nroute.numSegs; j++)
    {

      for (k = 0; k < rst->nets[i].nroute.segments[j].numEdges; k++)
      {
        getEdgePts(rst, rst->nets[i].nroute.segments[j].edges[k], &p1, &p2);

        /*special case where only one edge - print edge*/
        if (rst->nets[i].nroute.segments[j].numEdges == 1)
        {
          fprintf(fp, "(%d,%d)-(%d,%d)\n", p1.x, p1.y, p2.x, p2.y);
        }

        /*if first edge in segment, set this edge as the longest flat path*/
        if (k == 0)
        {
          p1prev = p1;
          p2prev = p2;
          dxprev = abs(p2.x - p1.x);
          dyprev = abs(p2.y - p1.y);
        }
        else
        {
          dx = abs(p2.x - p1.x);
          dy = abs(p2.y - p1.y);

          /* horizontal path, new edge also horizontal
           * update path end point and continue */
          if ((dxprev > 0) && (dx > 0))
          {
            if (p1prev.x == p2.x)
            {
              p1prev = p1;
            }
            else if (p1prev.x == p1.x)
            {
              p1prev = p2;
            }
            else if (p2prev.x == p2.x)
            {
              p2prev = p1;
            }
            else if (p2prev.x == p1.x)
            {
              p2prev = p2;
            }
            /*if last edge in segment, print*/
            if (k == (rst->nets[i].nroute.segments[j].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
          /* horizontal path, new edge vertical (bend)
           * print previous path and set edge as new path */
          else if ((dxprev > 0) && (dy > 0))
          {
            fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            p1prev = p1;
            p2prev = p2;
            dxprev = abs(p2.x - p1.x);
            dyprev = abs(p2.y - p1.y);

            /* if last edge in segment, print */
            if (k == (rst->nets[i].nroute.segments[j].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
          /* vertical path, new edge vetical */
          else if ((dyprev > 0) && (dy > 0))
          {
            if (p1prev.y == p2.y)
            {
              p1prev = p1;
            }
            else if (p1prev.y == p1.y)
            {
              p1prev = p2;
            }
            else if (p2prev.y == p2.y)
            {
              p2prev = p1;
            }
            else if (p2prev.y == p1.y)
            {
              p2prev = p2;
            }
            /* if last edge in segment, print */
            if (k == (rst->nets[i].nroute.segments[j].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
          /* vertical path, new edge horizontal(bend) */
          else if ((dyprev > 0) && (dx > 0))
          {
            fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            p1prev = p1;
            p2prev = p2;
            dxprev = abs(p2.x - p1.x);
            dyprev = abs(p2.y - p1.y);

            /* if last edge in segment, print */
            if (k == (rst->nets[i].nroute.segments[j].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
        }
      }
    }

    fprintf(fp, "!\n");
  }

  return EXIT_SUCCESS;
}

int release(routingInst *rst)
{

  int i, j;

  /* Must free all dynamically allocated memory from
   * routingInst.  This could include the following:
   * rst->edgeUtils
   * rst->edgeCaps
   * rst->nets
   *  nets[i].pins
   *  nets[i].route.segments
   *    nets[i].route.segments[j].edges
   */

  if (rst == NULL)
  {
    fprintf(stderr, "Null arg passed into release.\n");
    return EXIT_FAILURE;
  }

  /* free bottom up, starting with edges */
  if (rst->nets != NULL)
  {
    for (i = 0; i < rst->numNets; i++)
    {
      if (rst->nets[i].nroute.segments != NULL)
      {
        for (j = 0; j < rst->nets[i].nroute.numSegs; j++)
        {
          free(rst->nets[i].nroute.segments[j].edges);
        }

        free(rst->nets[i].nroute.segments);
      }
      free(rst->nets[i].pins);
    }
  }
  free(rst->nets);

  /* other memory in routingInst */
  free(rst->edgeCaps);
  free(rst->edgeUtils);

  return EXIT_SUCCESS;
}

int releaseSegsAndEdges(routingInst *rst)
{
  int i, j;

  if (rst == NULL)
  {
    fprintf(stderr, "Null arg passed into release.\n");
    return EXIT_FAILURE;
  }

  /* free bottom up, starting with edges */
  if (rst->nets != NULL)
  {
    for (i = 0; i < rst->numNets; i++)
    {
      if (rst->nets[i].nroute.segments != NULL)
      {
        for (j = 0; j < rst->nets[i].nroute.numSegs; j++)
        {
          free(rst->nets[i].nroute.segments[j].edges);
        }

        free(rst->nets[i].nroute.segments);
      }
      // free( rst->nets[i].pins );
    }
  }

  return EXIT_SUCCESS;
}
