#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <omp.h>
using namespace std;
#define MATCH 1
#define MISMATCH 0
#define INDEL -1
#define MAXSIZE 70000
#define CORES 4
#ifndef TILESIZE
#define TILESIZE 1024
#endif

void nw_tile(int x_vals[], int y_vals[], const char x_chars[], const char y_chars[], int x_size, int y_size, int corner) {

  int grid[TILESIZE][TILESIZE];
  if (x_chars[0] == y_chars[0]) {
    grid[0][0] = corner + MATCH;
  } else {
    grid[0][0] = corner + MISMATCH;
  }
  grid[0][0] = max(grid[0][0],
               max(x_vals[0] + INDEL,
                   y_vals[0] + INDEL));
  for (int i = 1; i < x_size; i++) {
    if (x_chars[i] == y_chars[0]) {
      grid[i][0] = x_vals[i-1] + MATCH;
    } else {
      grid[i][0] = x_vals[i-1] + MISMATCH;
    }
    grid[i][0] = max(grid[i][0],
                 max(grid[i-1][0] + INDEL,
                     x_vals[i] + INDEL));
  }
  for (int i = 1; i < y_size; i++) {
    if (y_chars[i] == x_chars[0]) {
      grid[0][i] = y_vals[i-1] + MATCH;
    } else {
      grid[0][i] = y_vals[i-1] + MISMATCH;
    }
    grid[0][i] = max(grid[0][i],
                 max(grid[0][i-1] + INDEL,
                     y_vals[i] + INDEL));
  }
  for (int i = 1; i < x_size; i++) {
    for (int j = 1; j < y_size; j++) {
      if (x_chars[i] == y_chars[j]) {
        grid[i][j] = grid[i-1][j-1] + MATCH;
      } else {
        grid[i][j] = grid[i-1][j-1] + MISMATCH;
      }
      grid[i][j] = max(grid[i][j],
                   max(grid[i-1][j] + INDEL,
                       grid[i][j-1] + INDEL));
    }
  }
  for (int i = 0; i < x_size; i++) {
    x_vals[i] = grid[i][y_size-1];
  }
  for (int i = 0; i < y_size; i++) {
    y_vals[i] = grid[x_size-1][i];
  }

}
size_t index( int x, int y, int m_width ){
  return x + m_width * y;
}

int main(int argc, char *argv[]) {
  string xstr, ystr;
  cin >> xstr >> ystr;
  int x_size = xstr.size();
  int y_size = ystr.size();
  const char* x_chars = xstr.c_str();
  const char* y_chars = ystr.c_str();
  int x_vals[MAXSIZE];
  int y_vals[MAXSIZE];
  int T_x = MAXSIZE/TILESIZE;
  int *corners = new int[T_x * T_x];
  int tile_y, tile_x;
  for (int i = 0; i < MAXSIZE; i++) {
    x_vals[i] = (i + 1) * INDEL;
    y_vals[i] = x_vals[i];
  }
  tile_x = TILESIZE;
  tile_y = TILESIZE;
  if (x_size < TILESIZE) tile_x = x_size;
  if (y_size < TILESIZE) tile_y = y_size;
  nw_tile(x_vals, y_vals, x_chars, y_chars, tile_x, tile_y, 0);
  corners[index(1,1,T_x)] = x_vals[TILESIZE-1];
  int x_tiles = ((TILESIZE + x_size - 1) / TILESIZE) - 1;
  int y_tiles = ((TILESIZE + y_size - 1) / TILESIZE) - 1;
  for (int i = 1; i <= x_tiles; i++){
    corners[index(i,0,T_x)] = i * TILESIZE * INDEL;
  }
  for (int i = 1; i <= y_tiles; i++){
    corners[index(0,i,T_x)] = i * TILESIZE * INDEL;
  }
  int x_offset, y_offset;
  int max_i;
  int offset = 1;
  int j;
  int edge_x = x_size % TILESIZE;
  int edge_y = y_size % TILESIZE;
  if (!edge_x) edge_x = TILESIZE;
  if (!edge_y) edge_y = TILESIZE;
  if (x_tiles + y_tiles) {
    for (int i = 1; 1; i++) {
      j = 0;
      if (i > x_tiles){
        j = offset++;
      }
      max_i = min(i, y_tiles);
      if (j == max_i) {
        x_offset = TILESIZE*j;
        y_offset = TILESIZE*(i-j);
        //printf("%d, %d\n", x_offset, y_offset);
        nw_tile(&x_vals[x_offset], &y_vals[y_offset], &x_chars[x_offset], &y_chars[y_offset], edge_x, edge_y, corners[index(j,i-j,T_x)]);
        break;
      }
#pragma omp parallel for shared(x_vals, y_vals, x_chars, y_chars, corners, T_x)  private(x_offset, y_offset, tile_x, tile_y)
      for (int k = j; k <= max_i; k++) {
        x_offset = TILESIZE*k;
        y_offset = TILESIZE*(i-k);
        tile_x = TILESIZE;
        tile_y = TILESIZE;
        if (k == x_tiles) tile_x = edge_x;
        if (i-k == y_tiles) tile_y = edge_y;
        nw_tile(&x_vals[x_offset], &y_vals[y_offset], &x_chars[x_offset], &y_chars[y_offset], tile_x, tile_y, corners[index(k,i-k,T_x)]);
        corners[index(k+1,i-k+1, T_x)] = x_vals[x_offset + TILESIZE - 1];
      }
    }
  }
  delete[] corners;
  /*for (int i = 0; i < x_size; i++) {
    printf("%d ", x_vals[i]);
  }
  printf("\n");
  for (int i = 0; i < y_size; i++) {
    printf("%d ", y_vals[i]);
  }*/
  printf("Final score: %d\n", x_vals[x_size - 1]);
  return 0;
}
