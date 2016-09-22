#include <cstdio>
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
using namespace std;
#define MATCH 1
#define MISMATCH 0
#define INDEL -1
#define BLOCKSIZE 16
#define MAXSIZE 1024

void nw_tile(int x_vals[], int y_vals[], char x_chars[], char y_chars[], int x_size, int y_size) {
  int grid[x_size][y_size];
  grid[0][0] = y_vals[0];
  for (int i = 1; i < x_size; i++) {
    grid[i][0] = x_vals[i];
  }
  for (int i = 1; i < y_size; i++) {
    grid[0][i] = y_vals[i];
  }
  for (int i = 1; i < x_size; i++) {
    for (int j = 1; j < y_size; j++) {
      if (x_chars[i-1] == y_chars[j-1]) {
        grid[i][j] = grid[i-1][j-1] + MATCH;
      } else {
        grid[i][j] = max(grid[i-1][j-1] + MISMATCH,
                     max(grid[i][j-1] + INDEL,
                         grid[i-1][j] + INDEL));
      }
    }
  }
  for (int i = 0; i < x_size; i++) {
    x_vals[i] = grid[i][y_size-1];
  }
  for (int i = 0; i < y_size; i++) {
    y_vals[i] = grid[x_size-1][i];
  }
}

int main(int argc, char *argv[]) {
  string xstr, ystr;
  cin >> xstr >> ystr;
  int x_size = xstr.size();
  int y_size = ystr.size();
  char* x_chars = xstr.c_str();
  char* y_chars = ystr.c_str();
  int x_vals[MAXSIZE];
  int y_vals[MAXSIZE];
  for (int i = 0; i < MAXSIZE; i++) {
    x_vals[i] = i * INDEL;
    y_vals[i] = x_vals[i];
  }
  int x_val_tmp[BLOCKSIZE], y_val_tmp[BLOCKSIZE];
  char x_char_tmp[BLOCKSIZE], y_char_tmp[BLOCKSIZE];
  mem_cpy(x_val_tmp, x_vals, sizeof(int)*BLOCKSIZE);
  mem_cpy(y_val_tmp, y_vals, sizeof(int)*BLOCKSIZE);
  mem_cpy(x_char_tmp, x_chars, sizeof(char)*BLOCKSIZE);
  mem_cpy(y_char_tmp, y_chars, sizeof(char)*BLOCKSIZE);
  nw_tile(x_tmp, y_tmp, x_char_tmp, y_char_tmp, BLOCKSIZE, BLOCKSIZE);
  return 0;
}
