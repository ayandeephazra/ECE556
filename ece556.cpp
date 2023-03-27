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

  char token1[100], token2[100], token3[100], token4[100], token5[100], token6[100], token7[100];
  int one, two, three, four, five;

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
      if ((phase == 1) & strcmp(token1, "grid") == 0)
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
  rst->cap = 1; // change as instructed

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
      // printf("%d %d %d %d %d ", blockage_[blockage_indx].x1, blockage_[blockage_indx].y1, blockage_[blockage_indx].x2, blockage_[blockage_indx].y2, blockage_[blockage_indx].newcap);
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
  /*
    for(int i = 0; i<rst->numEdges; i++){
      printf(" %d,%d ", rst->edgeCaps[i], rst->edgeUtils[i]);
    }*/
  printf("%ld\n",sizeof(buffer));
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
  point p1, p2;

  for (net_itr = 0; net_itr < rst->numNets; net_itr++)
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
    for (seg_itr = 0; seg_itr < rst->nets[net_itr].nroute.numSegs; seg_itr++)
    {

      p1.x = rst->nets[net_itr].pins[pin_num].x;   /* x coordinate of start point p1( >=0 in the routing grid)*/
      p1.y = rst->nets[net_itr].pins[pin_num].y; /* y coordinate of end point p1  ( >=0 in the routing grid)*/
      pin_num++;
      p2.x = rst->nets[net_itr].pins[pin_num].x; /* x coordinate of start point p2( >=0 in the routing grid)*/
      p2.y = rst->nets[net_itr].pins[pin_num].y; /* y coordinate of end point p2  ( >=0 in the routing grid)*/

      rst->nets[net_itr].nroute.segments[seg_itr].p1 = p1; /* start point of current segment */
      rst->nets[net_itr].nroute.segments[seg_itr].p2 = p2; /* end point of current segment */

      rst->nets[net_itr].nroute.segments[seg_itr].numEdges = NumEdges(p1.x, p2.x, p1.y, p2.y);

      rst->nets[net_itr].nroute.segments[seg_itr].edges=new int [abs(p1.x - p2.x) + abs(p1.y - p2.y)];

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

 /* FILE *file;
  if (!(file = fopen(outRouteFile, "w")))
  {
    perror("Error opening file");
    return 0;
  }
  //fprintf(file, "%d", rst->numNets + 1);
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
  */
  int i,j;  
ofstream out_stream(outRouteFile);
	if (!out_stream){
		cout << "Unable to open the file" << endl;
		out_stream.close();
		return 0;
	}
	for ( i = 0; i < rst->numNets; i++){
		out_stream << "n" << rst->nets[i].id << endl;
                 
		for ( j = 0; j < rst->nets[i].nroute.numSegs; j++){
			//if (rst->nets[])
			segment seg = rst->nets[i].nroute.segments[j];
			
			if (seg.p1.x == seg.p2.x || seg.p1.y == seg.p2.y) {
			out_stream << "(" << seg.p1.x << "," << seg.p1.y << ")-";
			out_stream << "(" << seg.p2.x << "," << seg.p2.y << ")" << endl;
			}
			
			else {
			out_stream << "(" << seg.p1.x << "," << seg.p1.y << ")-";
			out_stream << "(" << seg.p2.x << "," << seg.p1.y << ")" << endl;
			out_stream << "(" << seg.p2.x << "," << seg.p1.y << ")-" ;
			out_stream << "(" << seg.p2.x << "," << seg.p2.y << ")" << endl;
			}
/*
			out_stream << "(" << seg.p1.x << "," << seg.p1.y << ")-";
			out_stream << "(" << seg.p2.x << "," << seg.p2.y << ")" << endl;
*/
		}
		out_stream << "!" << endl;
	}

	out_stream.close();

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
