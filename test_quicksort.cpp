/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

void swap(int *a, int *b)
{
  int t = *a;
  *a = *b;
  *b = t;
}

typedef struct
{
  int cost;
  int edge_id;
} sorted_cost_dict;

// function to find the partition position
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

int main()
{
    printf("Hello World");
    sorted_cost_dict *scd_ = (sorted_cost_dict *)malloc(3 * sizeof(sorted_cost_dict));
    
    for (int i = 0; i<3; i++){
        scd_[i].cost = i+90;
        scd_[i].edge_id = i;
    }
    for (int i = 0; i<3; i++){
        printf("cost%d\n", scd_[i].cost);
        printf("id%d\n", scd_[i].edge_id );
     
    }
    quickSort_dec(0, 2, scd_);
    for (int i = 0; i<3; i++){
        printf("cost%d\n", scd_[i].cost);
        printf("id%d\n", scd_[i].edge_id );
     
    }
    return 0;
}
