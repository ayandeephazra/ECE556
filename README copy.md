Team - AMD Ryzen 10

Members -

    1) Surya Santhan Thenarasu

    2) Ayan Deep Hazra

Share of Workload for PART 1- 

    1) int readBenchmark(const char *fileName, routingInst *rst) -> Ayan

    2) int solveRouting(routingInst *rst) -> Surya

    3) int writeOutput(const char *outRouteFile, routingInst *rst) -> Ayan

    4) int release(routingInst *rst) -> Surya/Ayan

Share of Workload for PART 2- 

    1) int subnetGen(routingInst *rst); -> Surya

    2) int rrr(routingInst *rst, timeval startTime); -> Ayan

    3) int computeEdgeUtilizations(routingInst *rst); -> Ayan

    4) int release(routingInst *rst) -> Surya/Ayan

    5) void RSMT(int &MBB_x1, int &MBB_x2, int &MBB_y1, int &MBB_y2, int &distToSteinerPt, int &shortestPath, point &pC, point &shortestPath_pC, int &pin_itr); -> Surya

    6) void update_shortestPath(int &shortestPath, int &distToSteinerPt, point &shortestPath_pC, point &pC); -> Surya

    7) int writeOutput_sub(const char *outRouteFile, routingInst *rst) -> Ayan


Benchmarks	      "-d=0 -n=0"				"-d=0 -n=1"				          "-d=1 -n=0"	                    "-d=1 -n=1"
adaptec1.gr	|TWL = 5465212, TOF = 1669008	|TWL = 5465212, TOF = 1669008	  |TWL = 4376931, TOF = 997169	    |TWL = 4376931, TOF = 997169

adaptec2.gr |TWL = 5175684, TOF = 513012	|TWL = 5175684, TOF = 513012	  |TWL = 4076248, TOF = 273124	    |TWL = 4076248, TOF = 273124

adaptec3.gr	|TWL = 14953073, TOF = 3287533  |TWL = 14953073, TOF = 3287533    |TWL = 11997314, TOF = 1942862	|TWL = 11997314, TOF = 1942862

    