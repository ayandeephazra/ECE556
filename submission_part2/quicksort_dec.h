#ifndef QUICKSORT_DEC_H
#define QUICKSORT_DEC_H
#include "ece556.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct
{
  int cost;
  int net_id;
} sorted_cost_dict;

/* simple function that swaps two integers*/
void swap(int *a, int *b);

/* function to find the partition's position */ //sorted_cost_dict* scd_, 
int partition(int low, int high, routingInst* rst);

/* function that implements the actual quicksort functionality */ //sorted_cost_dict* scd_,
void quickSort_dec(int low, int high,  routingInst* rst);

#endif