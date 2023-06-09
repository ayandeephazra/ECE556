// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ece556.h"
#include <fstream>
#include <iostream>

using namespace std;

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
    int x1;
    int y1;
    int x2;
    int y2;
    int newcap;
  } blockage;

  blockage *blockage_ = (blockage *)malloc(num_blockages * sizeof(blockage));

  rst->edgeCaps = (int *)malloc(rst->numEdges * sizeof(int *));
  rst->edgeUtils = (int *)malloc(rst->numEdges * sizeof(int *));
  // rst->cap = 1; // change as instructed

  for (int blockage_indx = 0; blockage_indx < num_blockages; blockage_indx++)
  {
    fgets(buffer, 200 * sizeof(char), file);

    char x1[100], x2[100], y1[100], y2[100], newcap[100];

    if (sscanf(buffer, "%s %s %s %s %s\n", x1, y1, x2, y2, newcap) > 0)
    {
      blockage_[blockage_indx].x1 = atoi(x1);
      blockage_[blockage_indx].y1 = atoi(y1);
      blockage_[blockage_indx].x2 = atoi(x2);
      blockage_[blockage_indx].y2 = atoi(y2);
      blockage_[blockage_indx].newcap = atoi(newcap);
    }
  }
  // default capacity initialization
  for (int edge_indx = 0; edge_indx < rst->numEdges; edge_indx++)
  {
    rst->edgeCaps[edge_indx] = rst->cap;
  }

  // calculated blockage edgeCap and edgeUtils
  for (int recalc_indx = 0; recalc_indx < num_blockages; recalc_indx++)
  {
    // same row blockage calculation
    if (blockage_[recalc_indx].x1 == blockage_[recalc_indx].x2)
    {
      if (blockage_[recalc_indx].y1 != blockage_[recalc_indx].y2)
      {
        if (blockage_[recalc_indx].y1 < blockage_[recalc_indx].y2)
        {
          rst->edgeCaps[(rst->gy * (rst->gx - 1)) + blockage_[recalc_indx].x1 +
                        (rst->gx * blockage_[recalc_indx].y1)] = blockage_[recalc_indx].newcap;
        }
        else
        {
          rst->edgeCaps[(rst->gy * (rst->gx - 1)) + blockage_[recalc_indx].x2 +
                        (rst->gx * blockage_[recalc_indx].y2)] = blockage_[recalc_indx].newcap;
        }
      }
    }
    // same column blockage calculation
    else
    {
      if (blockage_[recalc_indx].x1 < blockage_[recalc_indx].x2)
      {
        rst->edgeCaps[(blockage_[recalc_indx].y1 * (rst->gx - 1)) +
                      blockage_[recalc_indx].x1] = blockage_[recalc_indx].newcap;
      }
      else
      {
        rst->edgeCaps[(blockage_[recalc_indx].y2 * (rst->gx - 1)) +
                      blockage_[recalc_indx].x2] = blockage_[recalc_indx].newcap;
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
      int x1 = rst->nets[i].nroute.segments[j].p1.x;
      int y1 = rst->nets[i].nroute.segments[j].p1.y;
      int x2 = rst->nets[i].nroute.segments[j].p2.x;
      int y2 = rst->nets[i].nroute.segments[j].p2.y;

      // if the points have some sort of a diagonal relation
      // meaning they aren't on the same x or y axis
      if ((rst->nets[i].nroute.segments[j].p1.x != rst->nets[i].nroute.segments[j].p2.x) &
          (rst->nets[i].nroute.segments[j].p1.y != rst->nets[i].nroute.segments[j].p2.y))
      {
        ///////////////////////////////////////////////////////////
        //  We opted to connect x1,y1 and x2,y2 through x2,y1   //
        /////////////////////////////////////////////////////////
        fprintf(file, "(%d,%d)-(%d,%d)\n", x1, y1, x2, y1);
        fprintf(file, "(%d,%d)-(%d,%d)\n", x2, y1, x2, y2);
      }
      // if there is a horizontal or vertical relation between the points
      else
      {
        fprintf(file, "(%d,%d)-(%d,%d)\n", x1, y1, x2, y2);
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
