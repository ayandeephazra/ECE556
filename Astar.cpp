#include "Astar.h"

int Node_init(Node_ *N)
{
  N->parent = NULL;
  N->loc.x = -1;
  N->loc.y = -1;
  N->edgeID = 0;
  N->distFromS = -1;
  N->distToT = -1;
  N->edgeUtil = 0;
  N->edgeCap = 0;
  N->F = -1;
  N->G = -1;

  return 1;
}


int solveRoutingAstar(routingInst *rst)
{
  // for fast lookup, we are using unordered_maps for
  // the openset,and closedSet.  In addition to the unordered_map,
  // we have a priority queue for the openset which will efficiently sort
  // through the possible next nodes.

  // set global var used in hash function
  // PointHash::gy = rst->gy;

  // go through all nets and route the pairs of pins
  for (int i = 0; i < rst->numNets; i++)
  {
    // printf( "routing net %d\n" , i );
    // allocate memory for the pins here
    rst->nets[i].nroute.numSegs = rst->nets[i].numPins - 1;

    rst->nets[i].nroute.segments =
        (segment *)malloc((rst->nets[i].numPins - 1) * sizeof(segment));

    for (int j = 0; j < rst->nets[i].numPins - 1; j++)
    {
      // route between current pin and next pin
      int ret = routeNetAstar(rst, i, j, j + 1);
      if (ret == EXIT_FAILURE)
      {
        fprintf(stderr, "failed to route net.\n");
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}

int getEdgeID(routingInst *rst, int x1, int y1, int x2, int y2)
{
  /* determine if horiz or vertical */
  if (y2 > y1)
  {
    /* vertical, (x2,y2) on top */
    return (y2 + (rst->gx - 1) * (rst->gy) + x1 * (rst->gy - 1));
  }
  else if (y2 < y1)
  {
    /* vertical, (x1,y1) on top */
    return (y1 + (rst->gx - 1) * (rst->gy) + x1 * (rst->gy - 1));
  }
  else if (x2 > x1)
  {
    /* horiz, (x2, y2) on right */
    return (x2 + y1 * (rst->gx - 1));
  }
  else
  {
    /* horiz, (x1, y1) on right */
    return (x1 + y1 * (rst->gx - 1));
  }
}

int getEdgeID(routingInst *rst, point p1, point p2)
{
  return getEdgeID(rst, p1.x, p1.y, p2.x, p2.y);
}

// std::priority_queue<Node, std::vector<Node>, decltype(compare)> openSet(compare);

int routeNetAstar(routingInst *rst, int netInd, int SpinInd, int TpinInd)
{
  // route between Start pin and target pin
  // using unordered_maps for fast lookup, and priority quuee for
  // very fast sorting based on cost
  unordered_map<point, Node_*, PointHash> openSet;
  priority_queue<Node_*, vector<Node_*>, CompareNodeCost> openSet_pq;
  unordered_map<point, Node_*, PointHash> closedSet;

  // create target node
  Node_ *nT = new Node_;
  nT->loc = rst->nets[netInd].pins[TpinInd];

  // create the starting node
  Node_ *nS = new Node_;
  nS->loc = rst->nets[netInd].pins[SpinInd];

  nS->parent = NULL;
  nS->distFromS = 0;

  int dx = abs(nS->loc.x - nT->loc.x);
  int dy = abs(nS->loc.y - nT->loc.y);

  // only doing this because we use dx and dy to reserve sizes for
  // the data structures to hopefully avoid reallocating space and copying
  // elements
  nS->distToT = dx + dy;

  int approxElems = dx * dy;
  openSet.reserve(approxElems);
  closedSet.reserve(approxElems);

  // add the starting node to the priority queue and map
  openSet.insert(std::make_pair(nS->loc, nS));
  openSet_pq.push(nS);

  // start the maze routing. while there are still open nodes, we
  // can continue attempting to find a path through the maze
  while (!openSet_pq.empty())
  {
    // first pull the lowest cost node from the priority queue
    auto current = openSet_pq.top();
    openSet_pq.pop();

    // it's possible that the node was pushed onto the priority queue
    // multiple times with different cost, so make sure that the node
    // still exists in the map
    auto it = openSet.find(current->loc);
    if (it == openSet.end())
    {
      // node is no longer on openSet, so it has already been
      // visited, from a lower cost path.  continue to next
      continue;
    }

    // if we are at the target, retrace our path
    if (current->loc.x == nT->loc.x && current->loc.y == nT->loc.y)
    {
      int ret = retrace(rst, nS, current, netInd, SpinInd);

      // release all of our allocated memory
      // since openSet_pq elements are duplicated in
      // openSet map, we don't need to call delete on it
      deleteMap(closedSet);
      deleteMap(openSet);

      return ret;
    }

    // add current to closedSet, remove from openSet
    closedSet.insert(std::make_pair(current->loc, current));
    openSet.erase(it);

    // get neighboring nodes (not including parent)
    vector<Node_> *neighbors;

    getNeighbors(rst, neighbors, current, nT);

    // go through all the neighbors, potentially adding new ones to the openSet
    for (auto neighbor = neighbors->begin(); neighbor != neighbors->end();
         neighbor++)
    {
      // if it's int the closedSet, skip it (this is not optimal, could be
      // improved, but adds much complexity)
      auto elem = closedSet.find(neighbor->loc);
      if (elem != closedSet.end())
      {
        // already in closed, don't add
        continue;
      }

      // check if it's in the openSet.  if the score is better, update the
      // existing node in the openSet, and add it to the priority queue again?
      elem = openSet.find(neighbor->loc);
      if (elem != openSet.end())
      {
        // need to update element in openSet, and add to pq again (shouldn't cause problems?)
        // the parent, distFromS, G, edgeCap, edgeUtil,
        // and F fields in the node have to be updated
        elem->second->parent = current;
        elem->second->distFromS = current->distFromS + 1;
        elem->second->edgeID = getEdgeID(rst, current->loc, neighbor->loc);
        elem->second->edgeCap = rst->edgeCaps[elem->second->edgeID - 1];
        elem->second->edgeUtil = rst->edgeUtils[elem->second->edgeID - 1];
        calcG(elem->second);
        calcF(elem->second);

        openSet_pq.push(elem->second);
      }
      else
      {
        // elem == openSet.end(), so it is not currently in the openSet
        // we need to make a new Node here so that we can push its address
        // into the openSet because the vector one will go away
        // Node_ *tmpN = new Node_(*neighbor);
        Node_ *tmpN = new Node_(*neighbor);
        openSet_pq.push(tmpN);
        openSet.insert(std::make_pair(tmpN->loc, tmpN));
      }
    }
  }

  // if we reach this point, we didn't find the Target node
  printf("couldn't find route for netInd: %d\tSpinInd: %d\tTpinInd: %d\n",
         netInd, SpinInd, TpinInd);
  return EXIT_FAILURE;
}

int retrace(routingInst *rst, Node_ *nS, Node_ *current, int netInd, int segInd)
{
  // Node_ *tmp = current;
  Node_ *tmp = new Node_;
  *tmp = *current;

  /* number of edges is simply the length from the starting node */
  rst->nets[netInd].nroute.segments[segInd].p2 = current->loc;
  rst->nets[netInd].nroute.segments[segInd].p1 = nS->loc;

  // printf("\nstart loc: (%d, %d)\tend loc: (%d, %d)\n",
  //          nS->loc.x, nS->loc.y, current->loc.x, current->loc.y );

  rst->nets[netInd].nroute.segments[segInd].numEdges = current->distFromS;
  rst->nets[netInd].nroute.segments[segInd].edges =
      (int *)malloc(sizeof(int) * current->distFromS);

  if (rst->nets[netInd].nroute.segments[segInd].edges == NULL)
  {
    fprintf(stderr, "Couldn't malloc for segments[segInd].edges pointer.\n");
    return EXIT_FAILURE;
  }

  int i = 0;
  double costSum = 0;
  while (tmp->parent != NULL)
  {
    // printf("loc: (%d,%d)\tparent addr: %x\n",
    //        tmp->loc.x, tmp->loc.y,  tmp->parent);

    int edgeID = getEdgeID(rst, tmp->loc.x, tmp->loc.y,
                           (*tmp->parent).loc.x, (*tmp->parent).loc.y);

    rst->nets[netInd].nroute.segments[segInd].edges[i] = edgeID;

    costSum += tmp->G - 1;

    /* update edge utilization.. assumes array has been
     * initialized with all zeros */
    rst->edgeUtils[edgeID - 1]++;

    /* get previous node */
    tmp = tmp->parent;
    i++;
  }
  // update prevAvgCost
  prevAvgCost = (prevAvgCost + costSum / (double)i) / 2.0;

  return EXIT_SUCCESS;
}

void calcG(Node_ *n)
{
  // using heuristic described in "FastRoute: A Step to Integrate Global Routing
  // into Placement"
  // it basically makes the cost much higher if it is close to going over
  // capacity, but otherwise it doesn't change drastically
  int diff = n->edgeUtil - n->edgeCap;
  double h_cost = MAX_HEUR_COST / (1.0 + exp(-K_OVF_MULT * (double)diff));
  n->G = n->distFromS * unitDist + h_cost;
}

void calcF(Node_ *n)
{
  n->F = n->G + n->distToT * (1.0 + prevAvgCost);
}

int m_dist(const Node_ *n1, const Node_ *n2)
{
  return abs(n1->loc.x - n2->loc.x) + abs(n1->loc.y - n2->loc.y);
}

void getNeighbors(routingInst *rst, vector<Node_> *neighbors,
                  Node_ *current, Node_ *T)
{
  Node_ *tmp = new Node_;
  int leftX, rightX, topY, botY;

  // printf("node passed to getNeighbor: node.id: %d\t(x,y) = (%d,%d)\tF=%d\tG=%d\n",
  //       current->edgeID, current->loc.x, current->loc.y, current->F, current->G);
  leftX = current->loc.x - 1;
  rightX = current->loc.x + 1;
  topY = current->loc.y + 1;
  botY = current->loc.y - 1;

  tmp->distFromS = current->distFromS + 1;
  tmp->parent = current;

  if (leftX >= 0)
  {
    tmp->loc = current->loc;
    tmp->loc.x = leftX;
    tmp->distToT = m_dist(tmp, T);
    tmp->edgeID = getEdgeID(rst, tmp->loc.x, tmp->loc.y, tmp->loc.x + 1, tmp->loc.y);
    /* use edgeID to find specific edgeCap and edgeUtil */
    tmp->edgeCap = rst->edgeCaps[tmp->edgeID - 1];
    tmp->edgeUtil = rst->edgeUtils[tmp->edgeID - 1];
    calcG(tmp);
    calcF(tmp);
    neighbors->push_back(*tmp);
  }

  if (rightX < rst->gx)
  {
    tmp->loc = current->loc;
    tmp->loc.x = rightX;
    tmp->distToT = m_dist(tmp, T);
    tmp->edgeID = getEdgeID(rst, tmp->loc.x, tmp->loc.y, tmp->loc.x - 1, tmp->loc.y);
    tmp->edgeCap = rst->edgeCaps[tmp->edgeID - 1];
    tmp->edgeUtil = rst->edgeUtils[tmp->edgeID - 1];
    calcG(tmp);
    calcF(tmp);
    neighbors->push_back(*tmp);
  }

  if (topY < rst->gy)
  {
    tmp->loc = current->loc;
    tmp->loc.y = topY;
    tmp->distToT = m_dist(tmp, T);
    tmp->edgeID = getEdgeID(rst, tmp->loc.x, tmp->loc.y, tmp->loc.x, tmp->loc.y - 1);
    tmp->edgeCap = rst->edgeCaps[tmp->edgeID - 1];
    tmp->edgeUtil = rst->edgeUtils[tmp->edgeID - 1];
    calcG(tmp);
    calcF(tmp);
    neighbors->push_back(*tmp);
  }

  if (botY >= 0)
  {
    tmp->loc = current->loc;
    tmp->loc.y = botY;
    tmp->distToT = m_dist(tmp, T);
    tmp->edgeID = getEdgeID(rst, tmp->loc.x, tmp->loc.y, tmp->loc.x, tmp->loc.y + 1);
    tmp->edgeCap = rst->edgeCaps[tmp->edgeID - 1];
    tmp->edgeUtil = rst->edgeUtils[tmp->edgeID - 1];
    calcG(tmp);
    calcF(tmp);
    neighbors->push_back(*tmp);
  }
}
// free memory from a map
// void deleteMap(unordered_map<point, Node *, PointHash> &group)
void deleteMap( unordered_map<point, Node_*, PointHash>& group )
{
  for( auto it = group.begin(); it != group.end(); it++ )
  {
    //free element
    delete (it->second);
  }
}