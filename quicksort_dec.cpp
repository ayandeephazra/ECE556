//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      Q      U      I      C      K      S       O      R      T                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "quicksort_dec.h"

void swap(int *a, int *b)
{
  int t = *a;
  *a = *b;
  *b = t;
}


// function to find the partition's position
int partition(int low, int high, sorted_cost_dict* scd_)
{

  // select the rightmost element as pivot                  int array[], 
  int pivot = scd_[high].cost;//array[high];

  // pointer for greater element
  int i = (low - 1);

  // traverse each element of the array
  // compare them with the pivot
  for (int j = low; j < high; j++)
  {
    if (scd_[j].cost >= pivot)//(array[j] <= pivot)
    {

      // if element smaller than pivot is found
      // swap it with the greater element pointed by i
      i++;

      // swap element at i with element at j
      swap(&scd_[i].cost, &scd_[j].cost);
      swap(&scd_[i].edge_id, &scd_[j].edge_id);
    }
  }

  // swap the pivot element with the greater element at i
  swap(&scd_[i + 1].cost, &scd_[high].cost);
  swap(&scd_[i + 1].edge_id, &scd_[high].edge_id);

  // return the partition point
  return (i + 1);
}

void quickSort_dec(int low, int high, sorted_cost_dict* scd_) {
  // int array[], 
  if (low < high) {
    
    // find the pivot element such that
    // elements smaller than pivot are on left of pivot
    // elements greater than pivot are on right of pivot
    int pi = partition(low, high, scd_);
    
    // recursive call on the left of pivot
    quickSort_dec(low, pi - 1, scd_);
    
    // recursive call on the right of pivot
    quickSort_dec(pi + 1, high, scd_);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      E      N      D                                                             //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////