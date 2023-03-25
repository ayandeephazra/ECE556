// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ece556.h"

int readBenchmark(const char *fileName, routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/

  int num_blockages;

  FILE *file;
  if (!(file = fopen(fileName, "r")))
  {
    perror("Error, file does not exist");
    exit(-1);
  }

  char *buffer = (char *)malloc(200 * sizeof(char *)); // buffer can store 100 characters
  routingInst *routingInst_ = (routingInst *)malloc(sizeof(routingInst));

  // while loop to start parsing file
  // assuming a line is no longer than 200 chars
  while (fgets(buffer, 200 * sizeof(char), file))
  {
    // keyword, 1st number, 2nd number
    char token1[100], token2[100], token3[100], token4[100], token5[100], token6[100], token7[100];
    // data variables
    int phase = 1;
    int blockages;

    if (sscanf(buffer, "%s", token1) > 0 & phase == 1)
    {

      ///////////////////////////////////////////////////
      //   grid parsing                               //
      /////////////////////////////////////////////////
      if (strcmp(token1, "grid") == 0)
      {
        char *x = (char *)malloc(sizeof(char));
        if (sscanf(buffer, "%s", x) > 0)
        {
          routingInst_->gx = atoi(x);
        }
        free(x);
        char *y = (char *)malloc(sizeof(char));
        if (sscanf(buffer, "%s", y) > 0)
        {
          routingInst_->gy = atoi(y);
        }
        free(y);

        phase = 2;
      }

      ///////////////////////////////////////////////////
      //   capacity parsing                           //
      /////////////////////////////////////////////////
      else if (strcmp(token1, "capacity") == 0 & phase == 2)
      {
        char *capacity = (char *)malloc(sizeof(char));
        if (sscanf(buffer, "%s", capacity) > 0)
        {
          routingInst_->cap = atoi(capacity);
        }
        free(capacity);
        phase = 3;
      }

      ///////////////////////////////////////////////////
      //   nets parsing                               //
      /////////////////////////////////////////////////
      else if (strcmp(token1, "num") == 0 & phase == 3)
      {
        if (sscanf(buffer, "%s", token2) > 0 & strcmp(token2, "nets") == 0)
        {
          char *numnets = (char *)malloc(sizeof(char));
          if (sscanf(buffer, "%s", numnets) > 0)
          {
            routingInst_->numNets = atoi(numnets);
          }
          free(numnets);
        }

        // instantiate nets filed in rst
        routingInst_->nets = (net *)malloc(routingInst_->numNets * sizeof(net));
        phase = 4;
      }

      ///////////////////////////////////////////////////
      //   initialization of nets struct within rst   //
      /////////////////////////////////////////////////
      else if (phase == 4)
      {
        for (int i = 0; i < routingInst_->numNets; i++)
        {
          routingInst_->nets[i].id = i;
          if (sscanf(buffer, "%s", token3) > 0 & sscanf(buffer, "%s", token4) > 0)
          {
            routingInst_->nets[i].numPins = atoi(token4);
          }
          // declaring the points/pins in each net
          routingInst_->nets[i].pins = (point *)malloc(routingInst_->nets[i].numPins * sizeof(point));

          for (int j = 0; j < routingInst_->nets[i].numPins; j++)
          {
            fgets(buffer, 200 * sizeof(char), file);

            if (sscanf(buffer, "%s", token5) > 0 & sscanf(buffer, "%s", token6) > 0)
            {
              routingInst_->nets[i].pins[j].x = atoi(token5);
              routingInst_->nets[i].pins[j].y = atoi(token6);
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
          blockages = atoi(token7);
        }
        for (int k = 0; k < blockages; k++)
        {
          fgets(buffer, 200 * sizeof(char), file);
          char tokena[100], tokenb[100], tokenc[100], tokend[100], tokene[100];
          int one, two, three, four, five;

          if (sscanf(buffer, "%s", tokena))
          {
            one = atoi(tokena);
          }
          if (sscanf(buffer, "%s", tokenb))
          {
            two = atoi(tokenb);
          }
          if (sscanf(buffer, "%s", tokenc))
          {
            three = atoi(tokenc);
          }
          if (sscanf(buffer, "%s", tokend))
          {
            four = atoi(tokend);
          }
          if (sscanf(buffer, "%s", tokene))
          {
            five = atoi(tokene);
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

  return 1;
}

int release(routingInst *rst)
{
  /*********** TO BE FILLED BY YOU **********/
  free(rst->edgeCaps);
  free(rst->edgeUtils);

  //free(rst->nets->nroute.segments)

  return 1;
}
