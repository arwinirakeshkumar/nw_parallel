//
// Created by matthieu on 07/10/16.
//

#include "core_algo.h"
#include "algorithm"
#include <stdio.h>
#include <math.h>
#define MATCH 1
#define MISMATCH -1
#define INDEL -1
#ifndef TILESIZE
#define TILESIZE 20
#endif

void nw_tile(int x_vals[], int y_vals[], const char x_chars[], const char y_chars[], int x_size, int y_size, int *corner) {
  /*
  printf("corner : %d\n", *corner);

  printf("x_vals :\n");
  for (int k = 0; k < x_size; ++k) {
   printf(" %d",x_vals[k]);
  }
  printf("\n");
  printf("x_chars :\n");
  for (int k = 0; k < x_size; ++k) {
    printf(" %c",x_chars[k]);
  }
  printf("\n");
  printf("y_vals :\n");
  for (int k = 0; k < y_size; ++k) {
    printf(" %d",y_vals[k]);
  }
  printf("\n");
  printf("y_chars :\n");
  for (int k = 0; k < y_size; ++k) {
    printf(" %c",y_chars[k]);
  }
  printf("\n");
   */
  int grid[x_size][y_size];
  if (x_chars[0] == y_chars[0]) {
    grid[0][0] = *corner + MATCH;
  } else {
    grid[0][0] = *corner + MISMATCH;
  }
  grid[0][0] = std::max(grid[0][0],
               std::max(x_vals[0] + INDEL,
                   y_vals[0] + INDEL));
  for (int i = 1; i < x_size; i++) {
    if (x_chars[i] == y_chars[0]) {
      grid[i][0] = x_vals[i-1] + MATCH;
    } else {
      grid[i][0] = x_vals[i-1] + MISMATCH;
    }
    grid[i][0] = std::max(grid[i][0],
                 std::max(grid[i-1][0] + INDEL,
                     x_vals[i] + INDEL));
  }
  for (int i = 1; i < y_size; i++) {
    if (y_chars[i] == x_chars[0]) {
      grid[0][i] = y_vals[i-1] + MATCH;
    } else {
      grid[0][i] = y_vals[i-1] + MISMATCH;
    }
    grid[0][i] = std::max(grid[0][i],
                 std::max(grid[0][i-1] + INDEL,
                     y_vals[i] + INDEL));
  }
  for (int i = 1; i < x_size; i++) {
    for (int j = 1; j < y_size; j++) {
      if (x_chars[i] == y_chars[j]) {
        grid[i][j] = grid[i-1][j-1] + MATCH;
      } else {
        grid[i][j] = grid[i-1][j-1] + MISMATCH;
      }
      grid[i][j] = std::max(grid[i][j],
                   std::max(grid[i-1][j] + INDEL,
                       grid[i][j-1] + INDEL));
    }
  }
  for (int i = 0; i < x_size; i++) {
    x_vals[i] = grid[i][y_size-1];
  }
  for (int i = 0; i < y_size; i++) {
    y_vals[i] = grid[x_size-1][i];
  }
/*  for (int j = 0; j < y_size; j++) {
    for (int i = 0; i < x_size; i++) {
      if (grid[i][j] < -9) {
        printf(" %d", grid[i][j]);
      } else if (grid[i][j] < 0) {
        printf("  %d", grid[i][j]);
      } else if (grid[i][j] < 10) {
        printf("   %d", grid[i][j]);
      } else {
        printf("  %d", grid[i][j]);
      }
    }
    printf("\n");
  }
  printf("\n");*/
    *corner = x_vals[x_size-1];

  /*
  printf("Final grid :\n");
  for(int i = 0; i < x_size; i ++) {
    for (int j = 0; j < y_size; j++) {
      printf(" %d", grid[i][j]);
    }
    printf("\n");
  }
   */
}
