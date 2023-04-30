// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Astar.h"
#include "quicksort_dec.h"
#include <fstream>
#include <iostream>

//using namespace std;

typedef struct
{
  int edgeOverflows;
  int edgeHistories;
  int edgeWeights;
} edge_params;

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

  rst->edgeCaps = (int *)malloc(rst->numEdges * sizeof(int *));
  rst->edgeUtils = (int *)malloc(rst->numEdges * sizeof(int *));
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

int NumEdges(int p1_x, int p1_y, int p2_x, int p2_y)
{
  int num_edges = abs(p1_x - p2_x) + abs(p1_y - p2_y);

  return num_edges;
}

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

int solveRouting(routingInst *rst)
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

      p1.x = rst->nets[net_itr].pins[pin_num].x;   /* x coordinate of start point p1( >=0 in the routing grid)*/
      p1.y = rst->nets[net_itr].pins[pin_num++].y; /* y coordinate of end point p1  ( >=0 in the routing grid)*/
      p2.x = rst->nets[net_itr].pins[pin_num].x;   /* x coordinate of start point p2( >=0 in the routing grid)*/
      p2.y = rst->nets[net_itr].pins[pin_num].y;   /* y coordinate of end point p2  ( >=0 in the routing grid)*/

      rst->nets[net_itr].nroute.segments[seg_itr].p1 = p1; /* start point of current segment */
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = p2; /* end point of current segment */

      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = NumEdges(p1.x, p2.x, p1.y, p2.y); /* number of edges in the segment*/

      rst->nets[net_itr].nroute.segments[seg_itr].edges = new int[abs(p1.x - p2.x) + abs(p1.y - p2.y)]; /* array of edges representing the segment*/

      /*
      P2 is above P1 and P2 is to the right of P1
      */
      int x_diff = p2.x - p1.x;
      int y_diff = p2.y - p1.y;

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
        /*
         Routing Horizontally towards left side
        */
        y = rst->nets[net_itr].pins[seg_itr].y;
        for (auto itr = 0; itr != x_diff_abs; itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x - itr) + y * (rst->gx - 1);
        }
        /*
        Routing Vertically downwards
        */
        x = rst->nets[net_itr].pins[seg_itr].x + x_diff;
        for (auto itr = 0; itr != y_diff_abs; itr++)
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
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(x_diff)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
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

/* int computingEdgeUtilizations(routingInst *rst)
   This function calculates edgeUtils param in rst after
   all other datavariables have been filled out (called at end of solveRouting)
   input: pointer to the routing instance
   output: 1 if successful, 0 otherwise (e.g. the data structures are not populated)
*/

int computeEdgeUtilizations(routingInst *rst)
{

  for (int net_id = 0; net_id < rst->numNets; net_id++)
  {

    for (int seg_id = 0; seg_id < rst->nets[net_id].nroute.numSegs; seg_id++)
    {

      for (int edge_id = 0; edge_id < rst->nets[net_id].nroute.segments[seg_id].numEdges; edge_id++)
      {

        int edge_in_question = rst->nets[net_id].nroute.segments[seg_id].edges[edge_id];
        // defaulted to zero, so just incremenent whenever there's a match
        rst->edgeUtils[edge_in_question - 1]++;
      }
    }
  }
  return 1;
}

int computeEdgeWeights(routingInst *rst, edge_params *edge_params_)
{
  // edge_params *edge_params_ = (edge_params *)malloc(rst->numEdges * sizeof(edge_params));

  for (int edge_id = 0; edge_id < rst->numEdges; edge_id++)
  {
    if (rst->edgeUtils[edge_id] - rst->edgeCaps[edge_id] > 0)
    {
      edge_params_[edge_id].edgeOverflows = rst->edgeUtils[edge_id] - rst->edgeCaps[edge_id];
    }
    else
    {
      edge_params_[edge_id].edgeOverflows = 0;
    }
  }
  return 1;
}


int rrr(routingInst *rst)
{

  // edge Utilization calculation, edgeUtils
  computeEdgeUtilizations(rst);

  bool terminate = false;
  int loop_var = 1;
  edge_params *edge_params_ = (edge_params *)malloc(rst->numEdges * sizeof(edge_params));

  int *cost_array;
  cost_array = (int *)malloc(rst->numNets * sizeof(int));
  // default cost array to 0
  for (int i = 0; i < rst->numNets; i++)
  {
    cost_array[i] = 0;
  }

  sorted_cost_dict *scd_ = (sorted_cost_dict *)malloc(rst->numNets * sizeof(sorted_cost_dict));

  // sorted_cost_array = (int*)malloc(rst->numNets*sizeof(int));

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                       R I P - U P - A N D - R E R O U T E   W H I L E   L O O P                                  //
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  while (terminate & loop_var<200) //!terminate & 
  {
    /* COMPUTING EDGE WEIGHTS*/
    if (loop_var == 1)
    {
      // history = 1 ffor all edges

      for (int edge_id = 0; edge_id < rst->numEdges; edge_id++)
      {
        if (rst->edgeUtils[edge_id] - rst->edgeCaps[edge_id] > 0)
        {
          edge_params_[edge_id].edgeOverflows = rst->edgeUtils[edge_id] - rst->edgeCaps[edge_id];
        }
        else
        {
          edge_params_[edge_id].edgeOverflows = 0;
        }
        // for loop 1 default histories to 1
        edge_params_[edge_id].edgeHistories = 1;
        // for every kth iteration of rip up and reroute update history using formula from slides and then using
        // history value and overflow, find weight (w=h*o)
        edge_params_[edge_id].edgeWeights = edge_params_[edge_id].edgeHistories * edge_params_[edge_id].edgeOverflows;
      }
    }
    /* NOT loop_var = 1*/
    else
    {

      for (int edge_id = 0; edge_id < rst->numEdges; edge_id++)
      {
        if (rst->edgeUtils[edge_id] - rst->edgeCaps[edge_id] > 0)
        {
          edge_params_[edge_id].edgeOverflows = rst->edgeUtils[edge_id] - rst->edgeCaps[edge_id];
        }
        else
        {
          edge_params_[edge_id].edgeOverflows = 0;
        }
        // for loop != 1 histories increment if overflows > 0
        if (edge_params_[edge_id].edgeOverflows > 0)
        {
          edge_params_[edge_id].edgeHistories++;
        }
        // for every kth iteration of rip up and reroute update history using formula from slides and then using
        // history value and overflow, find weight (w=h*o)
        edge_params_[edge_id].edgeWeights = edge_params_[edge_id].edgeHistories * edge_params_[edge_id].edgeOverflows;
      }
    }

    /* CALCULATE NET ORDERING */
    for (int net_id = 0; net_id < rst->numNets; net_id++)
    {
      for (int seg_id = 0; seg_id < rst->nets[net_id].nroute.numSegs; seg_id++)
      {
        for (int edge_id = 0; edge_id < rst->nets[net_id].nroute.segments[seg_id].numEdges; edge_id++)
        {
          cost_array[net_id] = cost_array[net_id] + edge_params_[edge_id].edgeWeights;
        }
      }
    }

    /* default cost array to be exactly what cost_array is */
    for (int ele = 0; ele < rst->numNets; ele++)
    {
      scd_[ele].cost = cost_array[ele];
      scd_[ele].net_id = ele;
    }

    /* sort scd_ in nlogn */
    /* changes nets in rst too now*/
    quickSort_dec(0, rst->numNets - 1, scd_, rst);
    
    solveRouting(rst);
    /* A* */
   // int status = solveRoutingAstar(rst);
    /* TERMINATION CONDITION */
    // change value of terminate
    if(loop_var == 500)
    {
      terminate = true;
      
    }
    else
    {
      loop_var++;
    }
    // increment loop_var
    //loop_var++;
  }
  // free memory formerly declared
  free(scd_);
  free(cost_array);
  free(edge_params_);
  return 1;
}

int writeOutput(const char *outRouteFile, routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/

  FILE *file;
  if (!(file = fopen(outRouteFile, "w")))
  {
    perror("Error opening file");
    return 0;
  }
  // fprintf(file, "%d", rst->numNets + 1);
  for (int i = 0; i < rst->numNets; i++)
  {
    fprintf(file, "%s%d\n", "n", i);
    for (int j = 0; j < rst->nets[i].nroute.numSegs; j++)
    {
      int MBB_x1 = rst->nets[i].nroute.segments[j].p1.x;
      int MBB_y1 = rst->nets[i].nroute.segments[j].p1.y;
      int MBB_x2 = rst->nets[i].nroute.segments[j].p2.x;
      int MBB_y2 = rst->nets[i].nroute.segments[j].p2.y;

      // if the points have some sort of a diagonal relation
      // meaning they aren't on the same x or y axis
      if ((rst->nets[i].nroute.segments[j].p1.x != rst->nets[i].nroute.segments[j].p2.x) &
          (rst->nets[i].nroute.segments[j].p1.y != rst->nets[i].nroute.segments[j].p2.y))
      {
        ///////////////////////////////////////////////////////////
        //  We opCed to connect MBB_x1,MBB_y1 and MBB_x2,MBB_y2 through MBB_x2,MBB_y1   //
        /////////////////////////////////////////////////////////
        fprintf(file, "(%d,%d)-(%d,%d)\n", MBB_x1, MBB_y1, MBB_x2, MBB_y1);
        fprintf(file, "(%d,%d)-(%d,%d)\n", MBB_x2, MBB_y1, MBB_x2, MBB_y2);
      }
      // if there is a horizontal or vertical relation between the points
      else
      {
        fprintf(file, "(%d,%d)-(%d,%d)\n", MBB_x1, MBB_y1, MBB_x2, MBB_y2);
      }
    }
    fprintf(file, "%s\n", "!");
  }

  fclose(file);
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
