// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include "quicksort_dec.h"
#include <sys/time.h>

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

	bool enable_subnet_gen = false;
	bool enable_net_order_and_rrr = false;

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
	int d = arg1[3] - '0';
	int n = arg2[3] - '0';

	if (d == 1)
	{
		enable_subnet_gen = true;
	}
	if (n == 1)
	{
		enable_net_order_and_rrr = true;
	}

	// use these two to enable/disable these components

	char *inputFileName = argv[3];
	char *outputFileName = argv[4];

	/// create a new routing instance
	routingInst *rst = new routingInst;

	/// read benchmark
	status = readBenchmark(inputFileName, rst);
	if (status == 0)
	{
		printf("ERROR: reading input file \n");
		return 1;
	}

	/// run SUBNET gen
	if (enable_subnet_gen)
	{
		subnetGen(rst);
	}
	/*if(status==0){
		printf("ERROR: running routing \n");
		release(rst);
		return 1;
	}
	*/
	/// run actual routing
	status = solveRouting(rst);
	if (status == 0)
	{
		printf("ERROR: running routing \n");
		release(rst);
		return 1;
	}
	else{
		printf("Successfully run routing! \n");
	}

	/// RRR
	if (enable_net_order_and_rrr)
	{
		rrr(rst, startTime);
	}

	/// write the result
	status = writeOutput(outputFileName, rst);
	if (status == 0)
	{
		printf("ERROR: writing the result \n");
		release(rst);
		return 1;
	}

	release(rst);
	printf("\nDONE!\n");
	return 0;
}
