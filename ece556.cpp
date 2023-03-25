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

  char *buffer = (char *)malloc(100 * sizeof(char *)); // buffer can store 100 characters
  routingInst *routingInst_ = (routingInst *)malloc(sizeof(routingInst));

  // while loop to start parsing file
  while (fgets(buffer, sizeof(routingInst_), file))
  {

    // keyword, 1st number, 2nd number
    char token1[100], token2[100], token3[100], token4[100], token5[100], token6[100], token7[100];

    if (scanf(buffer, "%s", token1) > 0)
    {

      ///////////////////////////////////////////////////
      //   grid parsing                               //
      /////////////////////////////////////////////////
      if (strcmp(token1, "grid") == 0)
      {
        char *x = (char *)malloc(sizeof(char));
        if (scanf(buffer, "%s", x) > 0)
        {
          routingInst_->gx = atoi(x);
        }
        free(x);
        char *y = (char *)malloc(sizeof(char));
        if (scanf(buffer, "%s", y) > 0)
        {
          routingInst_->gy = atoi(y);
        }
        free(y);
      }

      ///////////////////////////////////////////////////
      //   capacity parsing                           //
      /////////////////////////////////////////////////
      else if (strcmp(token1, "capacity") == 0)
      {
        char *capacity = (char *)malloc(sizeof(char));
        if (scanf(buffer, "%s", capacity) > 0)
        {
          routingInst_->cap = atoi(capacity);
        }
        free(capacity);
      }

      ///////////////////////////////////////////////////
      //   nets parsing                               //
      /////////////////////////////////////////////////
      else if (strcmp(token1, "num") == 0)
      {
        if (scanf(buffer, "%s", token2) > 0 & strcmp(token2, "nets") == 0)
        {
          char *numnets = (char *)malloc(sizeof(char));
          if (scanf(buffer, "%s", numnets) > 0)
          {
            routingInst_->numNets = atoi(numnets);
          }
          free(numnets);
        }
      }
      ///////////////////////////////////////////////////
      //   initialization of nets struct within rst   //
      //   or bad input handling because its nums    //
      ////////////////////////////////////////////////
      else
      {

        routingInst_->nets = (net *)malloc(routingInst_->numNets * sizeof(net));

        for (int i = 0; i < routingInst_->numNets; i++)
        {
          routingInst_->nets[i].id = i;
          if (scanf(buffer, "%s", token3) > 0 & scanf(buffer, "%s", token4) > 0)
          {
            routingInst_->nets[i].numPins = atoi(token4);
          }
          // declaring the points/pins in each net
          routingInst_->nets[i].pins = (point *)malloc(routingInst_->nets[i].numPins * sizeof(point));

          for (int j = 0; j < routingInst_->nets[i].numPins; j++)
          {
            if (scanf(buffer, "%s", token5) > 0 & scanf(buffer, "%s", token6) > 0)
            {
              routingInst_->nets[i].pins[j].x = atoi(token5);
              routingInst_->nets[i].pins[j].y = atoi(token6);
            }
          }
          // route malloc
          // not sure, idk when the route is ready to be loaded

        ///////////////////////////////////////////////////
        //   do the blockage parsing, jsut continue here
        //   I assume that bad output is not a real scenario
        //   so in that case, blockage statements follow nets' pins   
        //   parse the single integer line after the last nets' pins
        //   use that in a for loop to cycle through the five integer per line
        //   input of the blockages and add to rst aptly
        /////////////////////////////////////////////////
        
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

  return 1;
}
