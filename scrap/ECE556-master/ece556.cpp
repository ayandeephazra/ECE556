// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <climits>
using namespace std;

void subnetGen(routingInst *rst);
int getEdgeID(routingInst *rst, int x1, int y1, int x2, int y2);


int getEdgeID(routingInst *rst, point p1, point p2)
{
  return getEdgeID(rst, p1.x, p1.y, p2.x, p2.y );
}

int readBenchmark(const char *fileName, routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/

  int num_blockages;

  char token1[100], token2[100]; // token3[100], token4[100], token5[100], token6[100], token7[100];
                                 // int one, two, three, four, five;

  FILE *file;
  if (!(file = fopen(fileName, "r")))
  {
    perror("Error, file does not exist");
    return 0;
  }

  char buffer[200]; //= (char*)malloc(200 * sizeof(char *)); // buffer can store 200 characters
  // rst = (routingInst *)malloc(sizeof(routingInst));

  // while loop to start parsing file
  // assuming a line is no longer than 200 chars
  int phase = 1;

  while (fgets(buffer, 200 * sizeof(char), file))
  {
    // keyword, 1st number, 2nd number
    // data variables

    if ((sscanf(buffer, "%s", token1) > 0))
    {
      ///////////////////////////////////////////////////
      //   grid parsing                               //
      /////////////////////////////////////////////////
      if ((phase == 1) & (strcmp(token1, "grid") == 0))
      {

        char x_[100], y_[100];
        if (sscanf(buffer, "%s %s %s\n", token1, x_, y_) > 0)
        {
          rst->gx = atoi(x_);
          rst->gy = atoi(y_);
          // num of edges = x*(y-1) + y*(x-1)
          rst->numEdges = rst->gx * (rst->gy - 1) + rst->gy * (rst->gx - 1);
        }
        phase = 2;
        continue;
      }
      ///////////////////////////////////////////////////
      //   capacity parsing                           //
      /////////////////////////////////////////////////
      else if ((phase == 2) & (strcmp(token1, "capacity") == 0))
      {
        char capacity[100];
        if ((sscanf(buffer, "%s %s\n", token1, capacity)) > 0)
        {
          rst->cap = atoi(capacity);
        }
        phase = 3;
        continue;
      }

      ///////////////////////////////////////////////////
      //   nets parsing                               //
      /////////////////////////////////////////////////
      else if ((phase == 3) & (strcmp(token1, "num") == 0))
      {
        char nnets[100];
        if ((sscanf(buffer, "%s %s %s\n", token1, token2, nnets) > 0) & (strcmp(token2, "net") == 0))
        {
          rst->numNets = atoi(nnets);
        }

        phase = 4;
        break;
      }
    }
  }

  // instantiate nets filed in rst
  rst->nets = (net *)malloc(rst->numNets * sizeof(net));

  ///////////////////////////////////////////////////
  //   initialization of nets struct within rst   //
  /////////////////////////////////////////////////
  for (int net_indx = 0; net_indx < rst->numNets; net_indx++)
  {
    fgets(buffer, 200 * sizeof(char), file);
    char temp1[100], temp2[100];

    rst->nets[net_indx].id = net_indx;
    if ((sscanf(buffer, "%s %s\n", temp1, temp2) > 0))
    {
      rst->nets[net_indx].numPins = atoi(temp2);
    }
    // allocate mem for pins
    rst->nets[net_indx].pins = (point *)malloc(rst->nets[net_indx].numPins * sizeof(point));
    for (int pin_indx = 0; pin_indx < rst->nets[net_indx].numPins; pin_indx++)
    {
      fgets(buffer, 200 * sizeof(char), file);
      char temp3[100], temp4[100];

      if (sscanf(buffer, "%s %s\n", temp3, temp4) > 0)
      {
        rst->nets[net_indx].pins[pin_indx].x = atoi(temp3);
        rst->nets[net_indx].pins[pin_indx].y = atoi(temp4);
      }
    }
  }
  ////////////////////////////////////////////////////////////////
  //   Performs blockage parsing. Assuming that bad output      //
  //   is not a real scenario and so in that case, blockage     //
  //   statements parse the single integer in the following     //
  //   line after the last nets' pins. Uses that in a for loop  //
  //   to cycle through the five integer line input of the      //
  //   blockages and add to rst instance aptly.                 //
  ////////////////////////////////////////////////////////////////
  char temp5[100];
  fgets(buffer, 200 * sizeof(char), file);
  if (sscanf(buffer, "%s\n", temp5) > 0)
  {
    num_blockages = atoi(temp5);
  }
  typedef struct
  {
    int MBB_x1;
    int MBB_y1;
    int MBB_x2;
    int MBB_y2;
    int newcap;
  } blockage;

  blockage *blockage_ = (blockage *)malloc(num_blockages * sizeof(blockage));

  rst->edgeCaps = (int *)malloc(rst->numEdges * sizeof(int));
  rst->edgeUtils = (int *)malloc(rst->numEdges * sizeof(int));
  // rst->cap = 1; // change as instructed

  for (int edge_id = 0; edge_id < rst->numEdges; edge_id++)
  {
    rst->edgeUtils[edge_id] = 0;
  }

  for (int blockage_indx = 0; blockage_indx < num_blockages; blockage_indx++)
  {
    fgets(buffer, 200 * sizeof(char), file);

    char MBB_x1[100], MBB_x2[100], MBB_y1[100], MBB_y2[100], newcap[100];

    if (sscanf(buffer, "%s %s %s %s %s\n", MBB_x1, MBB_y1, MBB_x2, MBB_y2, newcap) > 0)
    {
      blockage_[blockage_indx].MBB_x1 = atoi(MBB_x1);
      blockage_[blockage_indx].MBB_y1 = atoi(MBB_y1);
      blockage_[blockage_indx].MBB_x2 = atoi(MBB_x2);
      blockage_[blockage_indx].MBB_y2 = atoi(MBB_y2);
      blockage_[blockage_indx].newcap = atoi(newcap);
    }
  }
  // default capacity initialization
  for (int edge_indx = 0; edge_indx < rst->numEdges; edge_indx++)
  {
    rst->edgeCaps[edge_indx] = rst->cap;
  }

  for (int recalc_indx = 0; recalc_indx < num_blockages; recalc_indx++)
  {
    // same row blockage calculation
    if (blockage_[recalc_indx].MBB_x1 == blockage_[recalc_indx].MBB_x2)
    {
      if (blockage_[recalc_indx].MBB_y1 != blockage_[recalc_indx].MBB_y2)
      {
        if (blockage_[recalc_indx].MBB_y1 < blockage_[recalc_indx].MBB_y2)
        {
          rst->edgeCaps[(rst->gy * (rst->gx - 1)) + blockage_[recalc_indx].MBB_x1 +
                        (rst->gx * blockage_[recalc_indx].MBB_y1)] = blockage_[recalc_indx].newcap;
        }
        else
        {
          rst->edgeCaps[(rst->gy * (rst->gx - 1)) + blockage_[recalc_indx].MBB_x2 +
                        (rst->gx * blockage_[recalc_indx].MBB_y2)] = blockage_[recalc_indx].newcap;
        }
      }
    }
    // same column blockage calculation
    else
    {
      if (blockage_[recalc_indx].MBB_x1 < blockage_[recalc_indx].MBB_x2)
      {
        rst->edgeCaps[(blockage_[recalc_indx].MBB_y1 * (rst->gx - 1)) +
                      blockage_[recalc_indx].MBB_x1] = blockage_[recalc_indx].newcap;
      }
      else
      {
        rst->edgeCaps[(blockage_[recalc_indx].MBB_y2 * (rst->gx - 1)) +
                      blockage_[recalc_indx].MBB_x2] = blockage_[recalc_indx].newcap;
      }
    }
  }

  fclose(file);
  return 1;
}

/* void subnetGen(routingInst *rst)
   This function creates an optimized subnet before the initial
   basic solverouting is run.
   input: pointer to the routing instance
   output: 1 if successful, 0 otherwise (e.g. the data structures are not populated)
*/

void subnetGen(routingInst *rst)
{

  int shortestPath;
  int MBB_x1, MBB_y1, MBB_x2, MBB_y2;
  point *tempArray = NULL;
  int distToSteinerPt;
  int flag;
  point pMBB_init0, pMBB_init1;
  point pC, shortestPath_pC;
  point temp0, temp1;

  for (auto net_itr = 0; net_itr != rst->numNets; net_itr++)
  {

    if (net_itr == rst->numNets)
    {
      free(tempArray);
    }
    // shortestPath between 2 points to draw the first MBB
    shortestPath = abs(rst->nets[net_itr].pins[0].x - rst->nets[net_itr].pins[1].x) +
                   abs(rst->nets[net_itr].pins[0].y - rst->nets[net_itr].pins[1].y);
    // First 2 points of the first MBB
    pMBB_init0 = rst->nets[net_itr].pins[0];
    pMBB_init1 = rst->nets[net_itr].pins[1];

    for (auto loop1 = 0; loop1 != rst->nets[net_itr].numPins;)
    {
      for (auto loop2 = 0; loop2 != rst->nets[net_itr].numPins;)
      {
        loop1++;
        loop2++;
        if (loop1 == rst->nets[net_itr].numPins && loop2 == rst->nets[net_itr].numPins)
        {
          // Place the closest 2 start points of MBB into a temp Array.
          tempArray = (point *)realloc(tempArray, rst->nets[net_itr].numPins * sizeof(point));

          tempArray[0] = pMBB_init0;
          tempArray[1] = pMBB_init1;
        }

        distToSteinerPt = abs(rst->nets[net_itr].pins[loop1].x - rst->nets[net_itr].pins[loop2].x) + abs(rst->nets[net_itr].pins[loop1].y - rst->nets[net_itr].pins[loop2].y);

        // do nothing as it's the same point
        if (distToSteinerPt == 0)
        {
        }
        // is distance to Steiner Pt < current shortest path calculated
        else if (distToSteinerPt < shortestPath)
        {
          pMBB_init0 = rst->nets[net_itr].pins[loop1];
          pMBB_init1 = rst->nets[net_itr].pins[loop2];

          shortestPath = distToSteinerPt;
        }
      }
    }

    for (auto arr_itr = 0; arr_itr != (rst->nets[net_itr].numPins - 1); arr_itr++)
    {
      if (arr_itr == (rst->nets[net_itr].numPins - 2))
      {
        // Update the routing instance with the recently added points in the tempArray
        for (auto pin_itr = 0; pin_itr != rst->nets[net_itr].numPins; pin_itr++)
        {
          rst->nets[net_itr].pins[pin_itr].x = tempArray[pin_itr].x;
          rst->nets[net_itr].pins[pin_itr].y = tempArray[pin_itr].y;
        }
        break;
      }
      shortestPath = 0x7fffffff;

      for (int pin_itr = 0; pin_itr < rst->nets[net_itr].numPins + 1;)
      {
        if (pin_itr == rst->nets[net_itr].numPins)
        {
          // Update the temp Array with the points closest to MBB
          tempArray[arr_itr + 2] = shortestPath_pC;
          break;
        }
        flag = 0;
        pC = rst->nets[net_itr].pins[pin_itr];

        int itr = 0;

        // Comparing the 2 points in the current MBB with pC.x and pC.y
        temp0 = tempArray[arr_itr];
        temp1 = tempArray[arr_itr + 1];

        while ((itr - 2) != arr_itr)
        {
          if ((pC.x == tempArray[itr].x) && (pC.y == tempArray[itr].y))
          {
            flag = 1;
            break;
          }
          itr++;
          if ((itr - 2 == arr_itr) && !(flag))
          {

            MBB_x1 = temp0.x;
            MBB_x2 = temp1.x;
            MBB_y1 = temp0.y;
            MBB_y2 = temp1.y;

            if (((MBB_x1 <= pC.x) && (pC.x <= MBB_x2)) || ((MBB_x2 <= pC.x) && (pC.x <= MBB_x1)))
            {
              if (abs(pC.y - MBB_y1) < abs(pC.y - MBB_y2))
              {
                distToSteinerPt = abs(pC.y - MBB_y1);
              }
              else
              {
                distToSteinerPt = abs(pC.y - MBB_y2);
              }

              if (pin_itr == 0)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
              else if (distToSteinerPt < shortestPath)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
            }
            else if (((MBB_y1 <= pC.y) && (pC.y <= MBB_y2)) || ((MBB_y2 <= pC.y) && (pC.y <= MBB_y1)))
            {
              if (abs(pC.x - MBB_x1) < abs(pC.x - MBB_x2))
              {
                distToSteinerPt = abs(pC.x - MBB_x1);
              }
              else
              {
                distToSteinerPt = abs(pC.x - MBB_x2);
              }
              if (pin_itr == 0)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
              else if (distToSteinerPt < shortestPath)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
            }
            else if (
                (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))) ||
                (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))) ||
                (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))) ||
                (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))))
            {
              distToSteinerPt = abs(pC.x - MBB_x2) + abs(pC.y - MBB_y2);
              if (pin_itr == 0)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
              else if (distToSteinerPt < shortestPath)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
            }
            else if (
                (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2))) ||
                (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))) ||
                (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2))) ||
                (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))))
            {
              distToSteinerPt = abs(pC.x - MBB_x1) + abs(pC.y - MBB_y1);
              if (pin_itr == 0)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
              else if (distToSteinerPt < shortestPath)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
            }

            else if ((((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))) ||
                     (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2))) ||
                     (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))) ||
                     (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2)))

            )
            {
              distToSteinerPt = abs(pC.x - MBB_x2) + abs(pC.y - MBB_y1);
              if (pin_itr == 0)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
              else if (distToSteinerPt < shortestPath)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
            }
            else if ((((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))) ||
                     (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))) || (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))) ||
                     (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))))
            {
              distToSteinerPt = abs(pC.x - MBB_x1) + abs(pC.y - MBB_y2);
              if (pin_itr == 0)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
              else if (distToSteinerPt < shortestPath)
              {
                shortestPath = distToSteinerPt;
                shortestPath_pC.x = pC.x;
                shortestPath_pC.y = pC.y;
              }
            }
          }
        }
        pin_itr++;
      }
    }
  }
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



/* this function takes in an edge id, and fills in the passed
 * in points with the edge points.  It follows the same numbering
 * described above for getEdgeID
 */

int NumEdges(int p1_x, int p1_y, int p2_x, int p2_y)
{
  int num_edges = abs(p1_x - p2_x) + abs(p1_y - p2_y);

  return num_edges;
}

int solveRouting(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/
  int x, y;
  point p1, p2;
  int x_diff, y_diff;

  for (auto net_itr = 0; net_itr != rst->numNets; net_itr++)
  {
    /* Number of segments = Number of pins - 1 */
    rst->nets[net_itr].nroute.numSegs = rst->nets[net_itr].numPins - 1;

    rst->nets[net_itr].nroute.segments = (segment *)malloc(rst->nets[net_itr].nroute.numSegs * sizeof(segment));

    /* Marking start-end pins and it's segments for different types of route
    |                     --------                                   |
    |                     |                                          |
    |                     |                                          |
    |________      or     |            OR     _________    OR        |
    */

    int pin_num = 0;
    for (auto seg_itr = 0; seg_itr != rst->nets[net_itr].nroute.numSegs; seg_itr++)
    {

      rst->nets[net_itr].nroute.segments[seg_itr].p1 = rst->nets[net_itr].pins[seg_itr];
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = rst->nets[net_itr].pins[seg_itr + 1];

      x_diff = rst->nets[net_itr].pins[seg_itr + 1].x - rst->nets[net_itr].pins[seg_itr].x;
      y_diff = rst->nets[net_itr].pins[seg_itr + 1].y - rst->nets[net_itr].pins[seg_itr].y;

      rst->nets[net_itr].nroute.segments[seg_itr].p1 = p1; /* start point of current segment */
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = p2; /* end point of current segment */

      /* number of edges in the segment*/
      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = abs(x_diff) + abs(y_diff);

      /* array of edges representing the segment*/
      rst->nets[net_itr].nroute.segments[seg_itr].edges =
          (int *)malloc(rst->nets[net_itr].nroute.segments[seg_itr].numEdges * sizeof(int));
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
        /*
         Routing Horizontally towards left side
        */
        y = rst->nets[net_itr].pins[seg_itr].y;
        for (auto itr = 0; itr != abs(x_diff); itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x - itr) + y * (rst->gx - 1);
        }
        /*
        Routing Vertically downwards
        */
        x = rst->nets[net_itr].pins[seg_itr].x + x_diff;
        for (auto itr = 0; itr != abs(y_diff); itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(x_diff)] = (rst->nets[net_itr].pins[seg_itr].y - itr) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
        }
      }
      else
      {
        /*
        Routing Vertically upwards
        */
        x = rst->nets[net_itr].pins[seg_itr].x;
        for (auto itr = 0; itr != abs(y_diff); itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(0)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
        }
        /*
        Routing Horizontally towards right side
        */
        y = rst->nets[net_itr].pins[seg_itr].y + y_diff;
        for (auto itr = 0; itr != abs(x_diff); itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
        }
      }
    }
  }

  return 1;
}


int writeOutput(const char *outRouteFile, routingInst *rst)
{

  // deltas
  int dxprev, dyprev, x_diff, y_diff;

  // how many flats in a row, either vertical or horiz
  int numflats = 0;

  // bends in seg
  int numBends = 0;
  FILE *fp;

  // CURRENT POINTS
  point p1, p2;
  // PREV POINTS
  point p1prev, p2prev;

  if (!(fp = fopen(outRouteFile, "w")))
  {
    perror("Error opening file");
    return 0;
  }

  /* check input parameters */
  if (outRouteFile == NULL || rst == NULL)
  {
    printf("File name is NULL, ending.\n");
    return 0;
  }

  if (fp == NULL)
  {
    printf("Output file could not be created.\n");
    return 0;
  }

  // we store previous points to help us navigate the segment
  p1prev.x = 0;
  p1prev.y = 0;
  p2prev.x = 0;
  p2prev.y = 0;

  for (auto net_iter = 0; net_iter != rst->numNets; net_iter++)
  {
    fprintf(fp, "n%d\n", rst->nets[net_iter].id);

    for (auto seg_iter = 0; seg_iter != rst->nets[net_iter].nroute.numSegs; seg_iter++)
    {

      for (auto edge_iter = 0; edge_iter != rst->nets[net_iter].nroute.segments[seg_iter].numEdges; edge_iter++)
      {

        int vEdgeID = rst->nets[net_iter].nroute.segments[seg_iter].edges[edge_iter] - (rst->gx - 1) * (rst->gy);
        /* first determine the direction of the edge by its magnitude */
        if (vEdgeID > 0)
        {
          /* Vertical segment */
          /* vEdgeId  = larger Y +  X * (rst->gy - 1) */
          /* X = (int) vEdgeId / (rst->gy) */
          p1.x = (vEdgeID - 1) / (rst->gy - 1);
          p2.x = p1.x;

          p2.y = vEdgeID - p2.x * (rst->gy - 1);
          p1.y = p2.y - 1;
        }
        else
        {
          /* horizontal segment */
          /* edgeID = larger X + Y * (rst->gx - 1)  */
          p1.y = (rst->nets[net_iter].nroute.segments[seg_iter].edges[edge_iter] - 1) / (rst->gx - 1);
          p2.y = p1.y;

          p2.x = rst->nets[net_iter].nroute.segments[seg_iter].edges[edge_iter] - p2.y * (rst->gx - 1);
          p1.x = p2.x - 1;
        }

        // special case where only one edge exists int he seg
        // just print that edge and exit
        if (rst->nets[net_iter].nroute.segments[seg_iter].numEdges == 1)
        {
          fprintf(fp, "(%d,%d)-(%d,%d)\n", p1.x, p1.y, p2.x, p2.y);
        }

        // if first edge in segment, set this edge as the longest flat path
        if (edge_iter == 0)
        {
          p1prev = p1;
          p2prev = p2;
          // subbing p1 from p2
          dxprev = abs(p2.x - p1.x);
          dyprev = abs(p2.y - p1.y);
          numflats = numflats + 1;
        }

        else
        {
          x_diff = abs(p2.x - p1.x);
          y_diff = abs(p2.y - p1.y);

          // Horizontal portion old and new, as delta in x's
          if ((dxprev > 0) && (x_diff > 0))
          {
            numflats = numflats + 1;
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
            // last means print
            if (edge_iter == (rst->nets[net_iter].nroute.segments[seg_iter].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
          // old was horiz and new is vertical
          else if ((dxprev > 0) && (y_diff > 0))
          {
            numBends = numBends + 1;
            fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            p1prev = p1;
            p2prev = p2;
            dxprev = abs(p2.x - p1.x);
            dyprev = abs(p2.y - p1.y);

            // last means print
            if (edge_iter == (rst->nets[net_iter].nroute.segments[seg_iter].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
          // Vertical portion old and new, as delta in y's
          else if ((dyprev > 0) && (y_diff > 0))
          {
            numflats = numflats + 1;
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
            // last means print
            if (edge_iter == (rst->nets[net_iter].nroute.segments[seg_iter].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
          // old was vert and new is horiz
          else if ((dyprev > 0) && (x_diff > 0))
          {
            numBends = numBends + 1;
            fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            p1prev = p1;
            p2prev = p2;
            dxprev = abs(p2.x - p1.x);
            dyprev = abs(p2.y - p1.y);

            // last means print
            if (edge_iter == (rst->nets[net_iter].nroute.segments[seg_iter].numEdges - 1))
            {
              fprintf(fp, "(%d,%d)-(%d,%d)\n", p1prev.x, p1prev.y, p2prev.x, p2prev.y);
            }
          }
        }
      }
    }

    fprintf(fp, "!\n");
  }

  return 1;
}

int release(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/
  //
  //

  rst->cap = 0;
  rst->numEdges = 0;
  rst->gx = 0;
  rst->gy = 0;

  free(rst->edgeCaps);
  rst->edgeCaps = NULL;

  free(rst->edgeUtils);
  rst->edgeUtils = NULL;

  if (rst->nets != NULL)
  {
    for (int i = 0; i < rst->numNets; i++)
    {
      if (rst->nets[i].nroute.segments != NULL)
      {
        for (int j = 0; j < rst->nets[i].nroute.numSegs; j++)
        {
          free(rst->nets[i].nroute.segments[j].edges);
        }

        free(rst->nets[i].nroute.segments);
      }
      free(rst->nets[i].pins);
    }
  }

  rst->numNets = 0;
  free(rst->nets);
  rst->nets = NULL;
  // free(rst->nets->nroute.segments)

  return 1;
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
