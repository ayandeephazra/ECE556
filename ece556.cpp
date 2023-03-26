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
  int one, two, three, four, five;

  FILE *file;
  if (!(file = fopen(fileName, "r")))
  {
    perror("Error, file does not exist");
    return 0;
  }

  char *buffer = (char *)malloc(200 * sizeof(char *)); // buffer can store 200 characters
  rst = (routingInst *)malloc(sizeof(routingInst));

  // while loop to start parsing file
  // assuming a line is no longer than 200 chars
  while (fgets(buffer, 200 * sizeof(char), file))
  {
    // keyword, 1st number, 2nd number
    char token1[100], token2[100], token3[100], token4[100], token5[100], token6[100], token7[100];
    // data variables
    int phase = 1;

    if ((sscanf(buffer, "%s", token1) > 0) & (phase == 1))
    {

      ///////////////////////////////////////////////////
      //   grid parsing                               //
      /////////////////////////////////////////////////
      if (strcmp(token1, "grid") == 0)
      {
        char *x = (char *)malloc(sizeof(char));
        if (sscanf(buffer, "%s", x) > 0)
        {
          rst->gx = atoi(x);
        }
        free(x);
        char *y = (char *)malloc(sizeof(char));
        if (sscanf(buffer, "%s", y) > 0)
        {
          rst->gy = atoi(y);
        }
        free(y);

        phase = 2;
      }

      ///////////////////////////////////////////////////
      //   capacity parsing                           //
      /////////////////////////////////////////////////
      else if ((strcmp(token1, "capacity") == 0) & (phase == 2))
      {
        char *capacity = (char *)malloc(sizeof(char));
        if ((sscanf(buffer, "%s", capacity)) > 0)
        {
          rst->cap = atoi(capacity);
        }
        free(capacity);
        phase = 3;
      }

      ///////////////////////////////////////////////////
      //   nets parsing                               //
      /////////////////////////////////////////////////
      else if ((strcmp(token1, "num") == 0) & (phase == 3))
      {
        if ((sscanf(buffer, "%s", token2) > 0) & (strcmp(token2, "nets") == 0))
        {
          char *numnets = (char *)malloc(sizeof(char));
          if (sscanf(buffer, "%s", numnets) > 0)
          {
            rst->numNets = atoi(numnets);
          }
          free(numnets);
        }

        // instantiate nets filed in rst
        rst->nets = (net *)malloc(rst->numNets * sizeof(net));
        phase = 4;
      }

      ///////////////////////////////////////////////////
      //   initialization of nets struct within rst   //
      /////////////////////////////////////////////////
      else if (phase == 4)
      {
        for (int i = 0; i < rst->numNets; i++)
        {
          rst->nets[i].id = i;
          if ((sscanf(buffer, "%s", token3) > 0) & (sscanf(buffer, "%s", token4) > 0))
          {
            rst->nets[i].numPins = atoi(token4);
          }
          // declaring the points/pins in each net
          rst->nets[i].pins = (point *)malloc(rst->nets[i].numPins * sizeof(point));

          for (int j = 0; j < rst->nets[i].numPins; j++)
          {
            fgets(buffer, 200 * sizeof(char), file);

            if ((sscanf(buffer, "%s", token5) > 0) & (sscanf(buffer, "%s", token6) > 0))
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
    }
  }

  fclose(file);
  return 1;
}

int solveRouting(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/

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
  for (int i = 0; i < rst->numNets; i++)
  {
    fprintf(file, "%s%s\n", "n", i);
    for (int j = 0; j<rst->nets[i].nroute.numSegs; j++){
      int x1 = rst->nets[i].nroute.segments->p1.x;
      int y1 = rst->nets[i].nroute.segments->p1.y;
      int x2 = rst->nets[i].nroute.segments->p2.x;
      int y2 = rst->nets[i].nroute.segments->p2.y;
      fprintf(file, "(%s,%s)-(%s,%s)\n", x1, y1, x2, y2);
    }
  }

  fclose(file);
  return 1;
}

int release(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/
  free(rst->edgeCaps);
  free(rst->edgeUtils);

  // free(rst->nets->nroute.segments)

  return 1;
}
