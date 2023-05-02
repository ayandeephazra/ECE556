// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include "aStar.h"
#include "netOrdering.h"
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

using std::vector;

int shouldContinue(timeval startTime)
{
  timeval currTime;
  gettimeofday(&currTime, NULL);

  if (currTime.tv_sec - startTime.tv_sec > MAX_TIME)
  {
    return false;
  }
  else
  {
	return true;
  }

}

int main(int argc, char **argv)
{

  timeval startTime;
  gettimeofday(&startTime, NULL);

  if (argc != 5)
	{
		printf("Usage : ./ROUTE.exe -d=<either 0 or 1> -n=<either 0 or 1> <input_benchmark_name> <output_file_name> \n");
		return 1;
	}

  int status;
  int netDecompEnabled;
  int netOrderingEnabled;

  char *inputFileName = argv[3];
  char *outputFileName = argv[4];
  vector<int> edgeOverflow;
  vector<int> edgeWeight;
  vector<int> edgeHistory;
 	
 	/// create a new routing instance
 	routingInst *rst = new routingInst;

  	char *arg1 = argv[1];
	char *arg2 = argv[2];

  if (((arg1[0] != '-') & (arg1[1] != 'd') & (arg1[2] != '=') & ((arg1[3] != '0') | (arg1[3] != '1'))))
	{
		printf("ERROR: reading input file \n");
		return 1;
	}
	if (((arg2[0] != '-') & (arg2[1] != 'd') & (arg2[2] != '=') & ((arg2[3] != '0') | (arg2[3] != '1'))))
	{
		printf("ERROR: reading input file \n");
		return 1;
	}

  	// character to integer
	netDecompEnabled = arg1[3] - '0';
	netOrderingEnabled = arg2[3] - '0';

  /* initializes routingInst pointers to null */
  rst->edgeCaps = NULL;
  rst->edgeUtils = NULL;
  rst->nets = NULL;

    
 	/// read benchmark
 	status = readBenchmark(inputFileName, rst);
 	if(status==0){
 		printf("ERROR: cannot read file \n");
 		return 1;
 	}

    printf("RUNNING initial routing \n");
		status = solveRouting(rst);
    printf("Completed initial routing \n");
 
   	if(status==EXIT_FAILURE){
   		printf("ERROR: initial solution fails \n");
   		release(rst);
   		return 1;
   	}

  if (netDecompEnabled == 1)
  {
    subnetGen(rst);
  }

  int vecSize = rst->numEdges;

  edgeOverflow.resize(vecSize);
  edgeWeight.resize(vecSize);
  edgeHistory.resize(vecSize);;

  for (int i = 0; i < rst->numEdges; i++)
  {
    edgeOverflow.at(i) = 0;
    edgeWeight.at(i) = 0;
    edgeHistory.at(i) = 0;
  }
 
  int temp = 0;
  // Calculate edge utilization after inital solution
  for(int i=0; i < ((rst->numNets)-1); i++){ //for each net in routing instance
    for(int j=0; j < ((rst->nets[i].nroute.numSegs)-1); j++){ //for each segment in the net
      for(int k=0; k < ((rst->nets[i].nroute.segments[j].numEdges)-1); k++){ //for each edge in the segment
        temp = rst->nets[i].nroute.segments[j].edges[k]; //get edge number
        rst->edgeUtils[temp-1] += 1; //update edge utilization
      }
    }
  }
  int loop_var= 1;
  int bestCost = 0;
  int firstRun = 1;
  int noChange = 0;

  // RRR
  if((netOrderingEnabled == 1)){ 
	  while (shouldContinue(startTime) || (noChange < 1))
	  {
		printf("RRR iteration: %d\n", loop_var);
		int totalCost = 0;
	    // compute total edge cost for each net

	int temp = 0;

	for(int i = 0; i <= ((rst->numNets)-1); i++){
		for(int j=0; j <= ((rst->nets[i].nroute.numSegs)-1); j++){ //for each segment in the net
			for(int k=0; k <= ((rst->nets[i].nroute.segments[j].numEdges)-1); k++){ //for each edge in the segment
				//parst3: compute overflow for each edge in route
				temp = rst->nets[i].nroute.segments[j].edges[k]; //get edge number
				//edgeOverflow[temp-1] = rst->edgeUtils[temp-1] - rst->edgeCaps[temp-1];
				edgeOverflow.at(temp-1) = rst->edgeUtils[temp-1] - rst->edgeCaps[temp-1];
				if(edgeOverflow.at(temp-1) < 0){
					edgeOverflow.at(temp-1) = 0;
				}
				//part4: compute weight for each edge in route
				if(edgeOverflow.at(temp-1) > 0){ //compute edge history
					edgeHistory.at(temp-1) +=1;
				}
				edgeWeight.at(temp-1) = edgeOverflow.at(temp-1) * edgeHistory.at(temp-1);
				//part 5: total edge weights in route
				rst->nets[i].nroute.cost += edgeWeight.at(temp-1);
			}
		}
	}


      //////////////////////////////////////////////////////////////////////////
	    
      // to allow A star to work correctly we must free segs and edges
	    releaseSegsAndEdges(rst);

	    if (netOrderingEnabled)
	    {
	      orderNets(rst);
	  	}

	    /*printRoutingInst(*rst); */
	   	/// run actual routing

			status = solveRoutingAstar(rst);
	   	if(status==EXIT_FAILURE){
	   		printf("ERROR: running routing \n");
	   		release(rst);
	   		return 1;
	   	}

		//calculate total cost of the routing instance
		for(int i = 0; i < rst->numNets; i++){
			totalCost += rst->nets[i].nroute.cost;
		}

		if(firstRun == 1){
			bestCost = totalCost;
			firstRun = 0;
		}


printf("totalCost = %d\nbestCost = %d\n", totalCost, bestCost);

		if(bestCost <= totalCost & totalCost>0){
			noChange += 1;
		}

		// IF OUR CURRENT COST IS LESS THAN BEST COST REWRITE OUTPUT
    // OR IF FIRST RUN
		if((firstRun == 1) || (totalCost < bestCost) ){
			/// write the result
			bestCost = totalCost;
			noChange = 0;
			status = writeOutput(outputFileName, rst);
			if(status==0){
				printf("ERROR: writing the result \n");
				release(rst);
				return 1;
			}
		}
    loop_var += 1;
	  }
  }	

	release(rst);
	delete rst;

 	printf("\nDONE!\n");	

 	return 1;
}
