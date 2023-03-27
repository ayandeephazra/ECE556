// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ece556.h"

int readBenchmark(const char *fileName, routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/

  int num_blockages;
  char tokena[100], tokenb[100], tokenc[100], tokend[100], tokene[100];
  char token1[100], token2[100], token3[100], token4[100], token5[100], token6[100], token7[100];
  int one, two, three, four, five;

  FILE *file;
  if (!(file = fopen(fileName, "r")))
  {
    perror("Error, file does not exist");
    return 0;
  }

  char *buffer = (char *)malloc(200 * sizeof(char *)); // buffer can store 200 characters
  // rst = (routingInst *)malloc(sizeof(routingInst));

  // while loop to start parsing file
  // assuming a line is no longer than 200 chars
  int t = 0;
  int phase = 1;

  while (fgets(buffer, 200 * sizeof(char), file))
  {
    // keyword, 1st number, 2nd number

    // data variables

    if ((sscanf(buffer, "%s", token1) > 0))
    {

      if (t == 0 | t == 1)
      {
        // printf("%s", token1);
        // t++;
      }

      ///////////////////////////////////////////////////
      //   grid parsing                               //
      /////////////////////////////////////////////////
      if ((phase == 1) & strcmp(token1, "grid") == 0)
      {
        // printf("%s", token1);
        char x_[100], y_[100];
        if (sscanf(buffer, "%s %s %s\n", token1, x_, y_) > 0)
        {
          rst->gx = atoi(x_);
          rst->gy = atoi(y_);
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

        // instantiate nets filed in rst
        rst->nets = (net *)malloc(rst->numNets * sizeof(net));
        phase = 4;
        continue;
      }
    }
  }

  ///////////////////////////////////////////////////
  //   initialization of nets struct within rst   //
  /////////////////////////////////////////////////
  if (phase == 4)
  {
    for (int i = 0; i < rst->numNets; i++)
    {
      rst->nets[i].id = i;
      if ((sscanf(buffer, "%s %s\n", token3, token4) > 0))
      {
        // printf("%s", token4);
        rst->nets[i].numPins = atoi(token4);
      }
      // declaring the points/pins in each net
      rst->nets[i].pins = (point *)malloc(rst->nets[i].numPins * sizeof(point));

      for (int j = 0; j < rst->nets[i].numPins; j++)
      {
        // printf("this%d", j);
        fgets(buffer, 200 * sizeof(char), file);
        printf(buffer);

        if (sscanf(buffer, "%s %s\n", token5, token6) > 0)
        {
          rst->nets[i].pins[j].x = atoi(token5);
          rst->nets[i].pins[j].y = atoi(token6);
        }
      }
      // route malloc
      // will be assigned in solveRouting

      ////////////////////////////////////////////////////////////////
      //   Performs blockage parsing. Assuming that bad output      //
      //   is not a real scenario and so in that case, blockage     //
      //   statements parse the single integer in the following     //
      //   line after the last nets' pins. Uses that in a for loop  //
      //   to cycle through the five integer line input of the      //
      //   blockages and add to rst instance aptly.                 //
      ////////////////////////////////////////////////////////////////
    }
    phase = 5;
  }
  else
  {
    if (sscanf(buffer, "%s", token7) > 0)
    {
      num_blockages = atoi(token7);

      for (int k = 0; k < num_blockages; k++)
      {
        fgets(buffer, 200 * sizeof(char), file);

        if (sscanf(buffer, "%s", tokena) > 0)
        {
          one = atoi(tokena);
        }
        if (sscanf(buffer, "%s", tokenb) > 0)
        {
          two = atoi(tokenb);
        }
        if (sscanf(buffer, "%s", tokenc) > 0)
        {
          three = atoi(tokenc);
        }
        if (sscanf(buffer, "%s", tokend) > 0)
        {
          four = atoi(tokend);
        }
        if (sscanf(buffer, "%s", tokene) > 0)
        {
          five = atoi(tokene);
        }
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

  int x;
  int y;
  int net_itr = 0;
  int seg_itr = 0;
  int pin_num = 0;
  point p1, p2;

  for (net_itr = 0; net_itr < rst->numNets; net_itr++)
  {
    /* Number of segments = Number of pins - 1 */
    rst->nets[net_itr].nroute.numSegs = rst->nets[net_itr].numPins - 1;
    printf("For net%d, number of pins %d, number of segments %d", net_itr, rst->nets[net_itr].numPins, rst->nets[net_itr].nroute.numSegs);

    rst->nets[net_itr].nroute.segments = (segment *)malloc(rst->nets[net_itr].nroute.numSegs * sizeof(segment));
    if (rst->nets[net_itr].nroute.segments == NULL)
    {
      fprintf(stderr, "Error while allocating memory space for segments\n");
      return 0;
    }

    /* Marking start-end pins and it's segments for
    |                     --------                                  |
    |                     |                                         |
    |                     |                                         |
    |________      or     |              OR   ________________  OR  |         */

    for (seg_itr = 0; seg_itr < rst->nets[net_itr].nroute.numSegs; seg_itr++)
    {

      p1.x = rst->nets[net_itr].pins[pin_num].x;   /* x coordinate of start point p1( >=0 in the routing grid)*/
      p1.y = rst->nets[net_itr].pins[pin_num++].y; /* y coordinate of end point p1  ( >=0 in the routing grid)*/

      p2.x = rst->nets[net_itr].pins[pin_num].x; /* x coordinate of start point p2( >=0 in the routing grid)*/
      p2.y = rst->nets[net_itr].pins[pin_num].y; /* y coordinate of end point p2  ( >=0 in the routing grid)*/

      printf("In Net #%d, P1 and P2 are (%d,%d) & (%d,%d)\n", net_itr, p1.x, p1.y, p2.x, p2.y);
      rst->nets[net_itr]
          .nroute.segments[seg_itr]
          .p1 = p1;                                        /* start point of current segment */
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = p2; /* end point of current segment */

      printf("This is Segment #%d\n", seg_itr);
      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = NumEdges(p1.x, p2.x, p1.y, p2.y);

      rst->nets[net_itr].nroute.segments[seg_itr].edges = (int *)malloc(rst->nets[net_itr].nroute.segments[seg_itr].numEdges * sizeof(int));
      if (rst->nets[net_itr].nroute.segments[seg_itr].edges == NULL)
      {
        fprintf(stderr, "Error while alocating memory space for edges array\n");
        return 0;
      }

      int x_diff, y_diff;
      int itr;

      x_diff = p2.x - p1.x;
      y_diff = p2.y - p1.y;

      if (p1.x > p2.x)
      {
        /*Routing Horizontally*/
        y = rst->nets[net_itr].pins[seg_itr].y;
        for (itr = 0; itr < -x_diff; itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(y_diff)] = (rst->nets[net_itr].pins[seg_itr].x - itr) + y * (rst->gx - 1);
        }
        /*Routing Vertically*/
        x = rst->nets[net_itr].pins[seg_itr].x + x_diff;
        for (itr = 0; itr < -y_diff; itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(x_diff)] = (rst->nets[net_itr].pins[seg_itr].y - itr) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
        }
      }
      else
      {
        /*Routing Vertically*/
        x = rst->nets[net_itr].pins[seg_itr].x;
        for (itr = 0; itr < y_diff; itr++)
        {
          rst->nets[net_itr].nroute.segments[seg_itr].edges[itr + abs(x_diff)] = (rst->nets[net_itr].pins[seg_itr].y + itr + 1) + (rst->gx - 1) * (rst->gy) + x * (rst->gy - 1);
        }
        /*Routing Horizontally*/
        y = rst->nets[net_itr].pins[seg_itr].y + y_diff;
        for (itr = 0; itr < x_diff; itr++)
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
  fprintf(file, "%d", rst->numNets);
  for (int i = 0; i < rst->numNets; i++)
  {
    fprintf(file, "%s%d\n", "n", i);
    for (int j = 0; j < rst->nets[i].nroute.numSegs; j++)
    {
      int x1 = rst->nets[i].nroute.segments->p1.x;
      int y1 = rst->nets[i].nroute.segments->p1.y;
      int x2 = rst->nets[i].nroute.segments->p2.x;
      int y2 = rst->nets[i].nroute.segments->p2.y;

      // if the points have some sort of a diagonal relation
      // meaning they aren't on the same x or y axis
      if ((rst->nets[i].nroute.segments->p1.x != rst->nets[i].nroute.segments->p2.x) &
          (rst->nets[i].nroute.segments->p1.y != rst->nets[i].nroute.segments->p2.y))
      {
        ///////////////////////////////////////////////////////////
        //  We opted to connect x1,y1 and x2,y2 through x1,y2   //
        /////////////////////////////////////////////////////////
        fprintf(file, "(%d,%d)-(%d,%d)\n", x1, y1, x1, y2);
        fprintf(file, "(%d,%d)-(%d,%d)\n", x1, y2, x2, y2);
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
  // free(rst->edgeCaps);
  // free(rst->edgeUtils);

  rst->cap = 0;
  rst->numEdges = 0;
  rst->gx = 0;
  rst->gy = 0;

  delete[] rst->edgeCaps;
  rst->edgeCaps = NULL;

  delete[] rst->edgeUtils;
  rst->edgeUtils = NULL;

  for (int i = 0; i < rst->numNets; i++)
  {
    //		delete [] rst->nets[i].nroute->segments;
    delete[] rst->nets[i].pins;
  }

  rst->numNets = 0;
  rst->nets = NULL;

  // free(rst->nets->nroute.segments)

  return 1;
}
