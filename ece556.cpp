// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ece556.h"
#include "quicksort_dec.h"
#include <fstream>
#include <iostream>
#include <sys/time.h>

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

  for (int edge_id = 0; edge_id < rst->numEdges; edge_id++)
  {
    rst->edgeUtils[edge_id] = 0;
  }

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

/*
Updates shortest path to MBB with the new distToSteinerPt
*/
void update_shortestPath(int &shortestPath, int &distToSteinerPt, point &shortestPath_pC, point &pC)
{
  shortestPath = distToSteinerPt;
  shortestPath_pC.x = pC.x;
  shortestPath_pC.y = pC.y;
}

/*
RSMT is a generalization of Rectilinear minimum spanning tree where in addition to
the original nodes in the graph, new nodes called “Steiner Points” may be
added to the graph which may help further reduce the total edge cost

The function takes in the coordinates of the two current points of the current MBB acc. to pin_itr, and finds the RSMT distance to pC in consideration.
*/
void RSMT(int &MBB_x1, int &MBB_x2, int &MBB_y1, int &MBB_y2, int &distToSteinerPt, int &shortestPath, point &pC, point &shortestPath_pC, int &pin_itr)
{
  // 6 scnarios handled where Pc can be located in relation with the Minimum Bouding Box
  bool scenario1 = ((MBB_x1 <= pC.x) && (pC.x <= MBB_x2)) || ((MBB_x2 <= pC.x) && (pC.x <= MBB_x1));
  bool scenario2 = ((MBB_y1 <= pC.y) && (pC.y <= MBB_y2)) || ((MBB_y2 <= pC.y) && (pC.y <= MBB_y1));
  bool scenario3 = (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))) || (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))) || (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))) || (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1)));
  bool scenario4 = (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2))) || (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))) || (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2))) || (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2)));
  bool scenario5 = (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))) || (((pC.x > MBB_x2) && (MBB_x2 >= MBB_x1)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2))) || (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y > MBB_y1) && (MBB_y1 >= MBB_y2))) || (((pC.x < MBB_x2) && (MBB_x2 <= MBB_x1)) && ((pC.y < MBB_y1) && (MBB_y1 <= MBB_y2)));
  bool scenario6 = (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))) || (((pC.x > MBB_x1) && (MBB_x1 >= MBB_x2)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1))) || (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y > MBB_y2) && (MBB_y2 >= MBB_y1))) || (((pC.x < MBB_x1) && (MBB_x1 <= MBB_x2)) && ((pC.y < MBB_y2) && (MBB_y2 <= MBB_y1)));

  // scenario 1
  if (scenario1)
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
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
    else if (distToSteinerPt < shortestPath)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
  }
  // scenario 2
  else if (scenario2)
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
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
    else if (distToSteinerPt < shortestPath)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
  }
  // scenario 3
  else if (scenario3)
  {
    distToSteinerPt = abs(pC.x - MBB_x2) + abs(pC.y - MBB_y2);
    if (pin_itr == 0)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
    else if (distToSteinerPt < shortestPath)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
  }
  // scenario 4
  else if (scenario4)
  {
    distToSteinerPt = abs(pC.x - MBB_x1) + abs(pC.y - MBB_y1);
    if (pin_itr == 0)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
    else if (distToSteinerPt < shortestPath)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
  }
  // scenario 5
  else if (scenario5)
  {
    distToSteinerPt = abs(pC.x - MBB_x2) + abs(pC.y - MBB_y1);
    if (pin_itr == 0)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
    else if (distToSteinerPt < shortestPath)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
  }
  // scenario 6
  else if (scenario6)
  {
    distToSteinerPt = abs(pC.x - MBB_x1) + abs(pC.y - MBB_y2);
    if (pin_itr == 0)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
    else if (distToSteinerPt < shortestPath)
    {
      update_shortestPath(shortestPath, distToSteinerPt, shortestPath_pC, pC);
    }
  }
}

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
int subnetGen(routingInst *rst)
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

            RSMT(MBB_x1, MBB_x2, MBB_y1, MBB_y2, distToSteinerPt, shortestPath, pC, shortestPath_pC, pin_itr);
          }
        }
        pin_itr++;
      }
    }
  }

  return 1;
}

int getEdgeID(routingInst *rst, int x1, int y1, int x2, int y2)
{
  int id;
  if (y2 < y1)
  {
    id = (y1 + (rst->gx - 1) * (rst->gy) + x1 * (rst->gy - 1));
  }
  else if (y2 > y1)
  {
    id = (y2 + (rst->gx - 1) * (rst->gy) + x1 * (rst->gy - 1));
  }
  else if (x2 > x1)
  {
    id = (x2 + y1 * (rst->gx - 1));
  }
  else
  {
    id = (x1 + y1 * (rst->gx - 1));
  }

  return id;
}



void horizontal(routingInst *rst, int net_itr, int seg_itr, int dx, int dy)
{
  int ptr, id;

  if (dx < 0)
  {
    // Route horizontally towards left
    for (ptr = 0; ptr < -dx; ptr++)
    {
      id = getEdgeID(rst, rst->nets[net_itr].pins[seg_itr].x - ptr, rst->nets[net_itr].pins[seg_itr].y + dy, rst->nets[net_itr].pins[seg_itr].x - ptr - 1, rst->nets[net_itr].pins[seg_itr].y + dy);
      rst->nets[net_itr].nroute.segments[seg_itr].edges[ptr + abs(dy)] = id;
    }
  }
  else
  {
    // Route horizontally towards right
    for (ptr = 0; ptr < dx; ptr++)
    {
      id = getEdgeID(rst, rst->nets[net_itr].pins[seg_itr].x + ptr, rst->nets[net_itr].pins[seg_itr].y + dy, rst->nets[net_itr].pins[seg_itr].x + ptr + 1, rst->nets[net_itr].pins[seg_itr].y + dy);
      rst->nets[net_itr].nroute.segments[seg_itr].edges[ptr + abs(dy)] = id;
    }
  }
}

/* this function routes the vertical edges between
 * pin[seg_itr] and pin[seg_itr+1] (using dy)
 */
void vertical(routingInst *rst, int net_itr, int seg_itr, int dx, int dy)
{

  int ptr, id;

  if (dy > 0)
  {
    // Route vertically upwards
    for (ptr = 0; ptr < dy; ptr++)
    {
      id = getEdgeID(rst, rst->nets[net_itr].pins[seg_itr].x + dx, rst->nets[net_itr].pins[seg_itr].y + ptr, rst->nets[net_itr].pins[seg_itr].x + dx, rst->nets[net_itr].pins[seg_itr].y + ptr + 1);
      rst->nets[net_itr].nroute.segments[seg_itr].edges[ptr + abs(dx)] = id;
    }
   
  }
  else
  {
    // Route vertically downwards
    for (ptr = 0; ptr < -dy; ptr++)
    {
      id = getEdgeID(rst, rst->nets[net_itr].pins[seg_itr].x + dx, rst->nets[net_itr].pins[seg_itr].y - ptr, rst->nets[net_itr].pins[seg_itr].x + dx, rst->nets[net_itr].pins[seg_itr].y - ptr - 1);
      rst->nets[net_itr].nroute.segments[seg_itr].edges[ptr + abs(dx)] = id;
    }
  }
}

int solveRouting(routingInst *rst)
{

  int dx, dy;

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

    for (auto seg_itr = 0; seg_itr != rst->nets[net_itr].nroute.numSegs; seg_itr++)
    {
      // start and end pins */
      rst->nets[net_itr].nroute.segments[seg_itr].p1 = rst->nets[net_itr].pins[seg_itr];
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = rst->nets[net_itr].pins[seg_itr + 1];

      dx = rst->nets[net_itr].pins[seg_itr + 1].x - rst->nets[net_itr].pins[seg_itr].x;
      dy = rst->nets[net_itr].pins[seg_itr + 1].y - rst->nets[net_itr].pins[seg_itr].y;

      /* number of edges in the segment*/
      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = abs(dx) + abs(dy);

      /* array of edges representing the segment*/
      rst->nets[net_itr].nroute.segments[seg_itr].edges = (int *)malloc(rst->nets[net_itr].nroute.segments[seg_itr].numEdges * sizeof(int));

      /*
      For numbering the edge ids, we first number the horizontal edges from left to right starting from the bottom most one. We then number the vertical edges starting from the left most one then moving up.
        ____e5_______e6__
      e8|     e10|       |e12
        |___e3___|___e4__|
      e7|      e9|       |e11
        |___e1___|___e2__|
      */
      if (dx < 0)
      {
        horizontal(rst, net_itr, seg_itr, dx, 0);
        vertical(rst, net_itr, seg_itr, dx, dy);
      }
      else
      {
        vertical(rst, net_itr, seg_itr, 0, dy);
        horizontal(rst, net_itr, seg_itr, dx, dy);
      }
    }
  }

  return 1;
}


int solveRouting2(routingInst *rst)
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

/* int computingEdgeUtilizations(routingInst *rst)
   This function calculates edgeUtils param in rst after
   all other datavariables have been filled out (called at end of solveRouting)
   input: pointer to the routing instance
   output: 1 if successful, 0 otherwise (e.g. the data structures are not populated)
*/

int computeEdgeUtilizations(routingInst *rst)
{

  for (int net_id = 0; net_id < rst->numNets - 1; net_id++)
  {

    for (int seg_id = 0; seg_id < rst->nets[net_id].nroute.numSegs - 1; seg_id++)
    {

      for (int edge_id = 0; edge_id < rst->nets[net_id].nroute.segments[seg_id].numEdges - 1; edge_id++)
      {

        int edge_in_question = rst->nets[net_id].nroute.segments[seg_id].edges[edge_id];
        // defaulted to zero, so just incremenent whenever there's a match
        // printf("\n\n\nrvewvefbv");
        rst->edgeUtils[edge_in_question - 1]++;
      }
    }
  }
  return 1;
}

/*
The weight of an edge is based on negotiation-based routing and given by
this formula:
– w ke = o k-1e x hke
*/
int computeEdgeWeights(routingInst *rst, edge_params *edge_params_)
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
  }

  return 1;
}

int patternRouting(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/
  int x, y;
  // point p1, p2;
  int dx, dy;
  int x_diff_abs, y_diff_abs;

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

      // * so set starting and ending points from pins */
      rst->nets[net_itr].nroute.segments[seg_itr].p1 = rst->nets[net_itr].pins[seg_itr];
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = rst->nets[net_itr].pins[seg_itr + 1];

      /*route between pin[j] and pin [j+1] */
      /* find delta x and delta y */
      dx = rst->nets[net_itr].pins[seg_itr + 1].x - rst->nets[net_itr].pins[seg_itr].x;
      dy = rst->nets[net_itr].pins[seg_itr + 1].y - rst->nets[net_itr].pins[seg_itr].y;

      // rst->nets[net_itr].nroute.segments[seg_itr].p1 = p1; /* start point of current segment */
      // rst->nets[net_itr].nroute.segments[seg_itr].p2 = p2; /* end point of current segment */

      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = abs(dx) + abs(dy); /* number of edges in the segment*/

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

      point p1 = rst->nets[net_itr].pins[seg_itr];
      point p2 = rst->nets[net_itr].pins[seg_itr + 1];

      if (abs(dx) == 0 & abs(dy) == 0)
      {
        printf("\nloop42");
        continue;
      }
      if (abs(dx) == 0)
      {
        printf("\nloop32");
        if (dy > 0)
        {
          vertical(rst, net_itr, seg_itr, 0, dy);
        }
        else
        {
          vertical(rst, net_itr, seg_itr, 0, dy);
        }
        // vertical route
      }
      if (abs(dy) == 0)
      {
        printf("\nloop22");
        if (dx > 0)
        {
          horizontal(rst, net_itr, seg_itr, dx, 0);
        }
        else
        {
          horizontal(rst, net_itr, seg_itr, dx, 0);
        }

        // horizontal route
      }
      else
      {
        int nedges = abs(dx) + abs(dy);
        int midpoint = nedges / 3;
        printf("\nloop12 %d,%d %d,%d", p1.x, p1.y, p2.x, p2.y);
        //          *p1
        //
        //
        //   *p2
        if (p1.x > p2.x && p1.y > p2.y)
        {
          printf("\nloop6");
          for (int itr = 0; itr < abs(dx); itr++)
          {
            if (itr < midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            else if (itr == midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(0)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
            }
            else
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            // rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
          }
        }

        // *p2
        //
        //
        //          *p1

        if (p1.x > p2.x && p1.y < p2.y)
        {
          printf("\nloop66");
          for (int itr = 0; itr < abs(dx); itr++)
          {
            if (itr < midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            else if (itr == midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(0)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
            }
            else
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            // rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
          }
        }
        // *p1
        //
        //
        //          *p2

        if (p1.x < p2.x && p1.y > p2.y)
        {
          printf("\nloop666");
          for (int itr = 0; itr < abs(dx); itr++)
          {
            if (itr < midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            else if (itr == midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(0)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
            }
            else
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            // rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
          }
        }

        if (p1.x < p2.x && p1.y < p2.y)
        {
          printf("\nloop6666");
          for (int itr = 0; itr < abs(dx); itr++)
          {
            if (itr < midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            else if (itr == midpoint)
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(0)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
            }
            else
            {
              rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(dy)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
            }
            // rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x + itr + 1) + y * (rst->gx - 1);
          }
        }
      }
    }
  }
  return 1;
}

/*
1>We first update edge weights
2>We then calculate net ordering
3>Given the updated edge weights at the beginning of each RRR
iteration, for each net n, we calculate a cost
4>For each net in the new order, we remove the existing route stored for the net by
decreasing the edge utilizations corresponding to the route and then reroute to generate a better route (of lower cost) for the net

   input: pointer to the routing instance, startTime
   output: 1 if successful, 0 otherwise
*/
int rrr(routingInst *rst, timeval startTime)
{

  // calculates edgeUtils param in rst
  int status = computeEdgeUtilizations(rst);
  if (status == 0)
  {
    printf("ERROR: calculating Edge Utilizations\n");
    release(rst);
    return 1;
  }
  else
  {
    printf("Successfully run Edge Utilizations calculations! \n");
  }

  bool terminate = false;
  int loop_var = 1;
  edge_params *edge_params_ = (edge_params *)malloc(rst->numEdges * sizeof(edge_params));

  for (int i = 0; i < rst->numNets; i++)
  {
    rst->nets[i].nroute.cost = 0;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                       R I P - U P - A N D - R E R O U T E   W H I L E   L O O P                                  //
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  printf("\n");
  while (!terminate)
  {
    printf("RRR ITERATION %d\n", loop_var);
    /* COMPUTING EDGE WEIGHTS*/
    if (loop_var == 1)
    {

      for (int edge_id = 0; edge_id < rst->numEdges - 1; edge_id++)
      {
        if (rst->edgeUtils[edge_id - 1] - rst->edgeCaps[edge_id - 1] > 0)
        {
          edge_params_[edge_id - 1].edgeOverflows = rst->edgeUtils[edge_id - 1] - rst->edgeCaps[edge_id - 1];
        }
        else
        {
          edge_params_[edge_id - 1].edgeOverflows = 0;
        }
        // for loop 1 default histories to 1
        edge_params_[edge_id - 1].edgeHistories = 1;
        // for every kth iteration of rip up and reroute update history using formula from slides and then using
        // history value and overflow, find weight (w=h*o)
        edge_params_[edge_id - 1].edgeWeights = edge_params_[edge_id - 1].edgeHistories * edge_params_[edge_id - 1].edgeOverflows;
      }
    }
    /* NOT loop_var = 1*/
    else
    {

      for (int edge_id = 0; edge_id < rst->numEdges; edge_id++)
      {
        if (rst->edgeUtils[edge_id - 1] - rst->edgeCaps[edge_id - 1] > 0)
        {
          edge_params_[edge_id - 1].edgeOverflows = rst->edgeUtils[edge_id - 1] - rst->edgeCaps[edge_id - 1];
        }
        else
        {
          edge_params_[edge_id - 1].edgeOverflows = 0;
        }
        // for loop != 1 histories increment if overflows > 0
        if (edge_params_[edge_id - 1].edgeOverflows > 0)
        {
          edge_params_[edge_id - 1].edgeHistories++;
        }
        // for every kth iteration of rip up and reroute update history using formula from slides and then using
        // history value and overflow, find weight (w=h*o)
        edge_params_[edge_id - 1].edgeWeights = edge_params_[edge_id - 1].edgeHistories * edge_params_[edge_id - 1].edgeOverflows;
      }
    }

    printf("completed edge weights calculation in iter: %d\n", loop_var);

    /* CALCULATE NET ORDERING */

    // defaulting edge net route costs to 0
    for (int i = 0; i < rst->numNets; i++)
    {
      rst->nets[i].nroute.cost = 0;
    }

    for (int net_id = 0; net_id < rst->numNets - 1; net_id++)
    {
      for (int seg_id = 0; seg_id < rst->nets[net_id].nroute.numSegs - 1; seg_id++)
      {
        for (int edge_id = 0; edge_id < rst->nets[net_id].nroute.segments[seg_id].numEdges - 1; edge_id++)
        {
          rst->nets[net_id].nroute.cost += edge_params_[edge_id].edgeWeights;
          // cost_array[net_id] = cost_array[net_id] + edge_params_[edge_id].edgeWeights;
        }
      }
    }
    printf("completed adding cost per net in iter: %d\n", loop_var);

    /* sort scd_ in nlogn */
    /* changes nets in rst too now*/
    // quickSort_dec(0, rst->numNets - 2, rst);

    for (int i = 0; i < rst->numNets - 1; i++)
    {
      for (int j = 0; j < rst->numNets - i - 1; j++)
      {

        if (rst->nets[j].nroute.cost < rst->nets[i].nroute.cost)
        {
          net temp = rst->nets[i];
          rst->nets[i] = rst->nets[j];
          rst->nets[j] = temp;
        }
      }
    }

    printf("completed quicksorting the nets by their costs: %d\n", loop_var);

    /* Pattern Routing / A* */
    int status = patternRouting(rst);
    if (status == 0)
    {
      printf("ERROR: failed pattern Routing\n");
      release(rst);
      return 1;
    }
    else
    {
      printf("Successfully run pattern Routing! \n");
    }

    /* TERMINATION CONDITION */
    // change value of terminate
    // increment loop_var
    // int runstatus = is_time_up(startTime);
    timeval curr;
    gettimeofday(&curr, NULL);
    if (curr.tv_sec - startTime.tv_sec > MAX_ALLOWED_RUNTIME)
    {
      terminate = true;
    }
    else
    {
      terminate = false;
    }

    if (loop_var == 1000)
    {
      terminate = true;
    }
    loop_var++;
  }

  return 1;
}

int writeOutput_sub(const char *outRouteFile, routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/

  FILE *file;
  if (!(file = fopen(outRouteFile, "w")))
  {
    perror("Error opening file");
    return 0;
  }

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
        //  We opCed to connect x1,y1 and x2,y2 through x2,y1   //
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
