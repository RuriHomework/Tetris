#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- 常量部分

#define FEATURES 13
#define BOARD_HEIGHT 15
#define BOARD_WIDTH 10
#define TOP_N 5

const double WEIGHTS[] = {-1464772.166456, 2535297.130013,  -2638462.645342,
                          -372351.515440,  1782742.689903,  -1883234.918781,
                          4420.968667,     -9988776.620538, 948594.666888,
                          3610431.536749,  -3355542.370633, -1120426.582938,
                          -3233372.471683};

typedef enum { I, T, O, J, L, S, Z, END } PieceType;

// --- 棋盘部分

typedef struct {
  int shape[4][4];
  int width, height;
  int leftmost[4];
  int rightmost[4];
} Piece;

// clang-format off
const Piece ROTATIONS[7][4] = {
  // I
  {
      {
          .shape = {{1, 1, 1, 1}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 4,
          .height = 1,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {3, 0, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {1, 0, 0, 0}},
          .width = 1,
          .height = 4,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {0, 0, 0, 0},
      },
      {
          .shape = {{1, 1, 1, 1}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 4,
          .height = 1,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {3, 0, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {1, 0, 0, 0}},
          .width = 1,
          .height = 4,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {0, 0, 0, 0},
      },
  },
  // T
  {
      {
          .shape = {{1, 1, 1, 0}, 
                  {0, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {0, 1, 0, 0},
          .rightmost = {2, 1, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 0, 0, 0},
      },
      {
          .shape = {{0, 1, 0, 0}, 
                  {1, 1, 1, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {1, 0, 0, 0},
          .rightmost = {1, 2, 0, 0},
      },
      {
          .shape = {{0, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 1, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
  },
  // O
  {
      {
          .shape = {{1, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
      {
          .shape = {{1, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
      {
          .shape = {{1, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
      {
          .shape = {{1, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
  },
  // J
  {
      {
          .shape = {{1, 1, 1, 0}, 
                  {1, 0, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {2, 0, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
      {
          .shape = {{0, 0, 1, 0}, 
                  {1, 1, 1, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {2, 0, 0, 0},
          .rightmost = {2, 2, 0, 0},
      },
      {
          .shape = {{1, 1, 0, 0}, 
                  {0, 1, 0, 0}, 
                  {0, 1, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
  },
  // L
  {
      {
          .shape = {{1, 1, 1, 0}, 
                  {0, 0, 1, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {2, 0, 0, 0},
      },
      {
          .shape = {{1, 1, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 0, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 1, 1, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {2, 0, 0, 0},
      },
      {
          .shape = {{0, 1, 0, 0}, 
                  {0, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
  },
  // S
  {
      {
          .shape = {{1, 1, 0, 0}, 
                  {0, 1, 1, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 2, 0, 0},
      },
      {
          .shape = {{0, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
      {
          .shape = {{1, 1, 0, 0}, 
                  {0, 1, 1, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 2, 0, 0},
      },
      {
          .shape = {{0, 1, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {1, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {1, 1, 0, 0},
      },
  },
  // Z
  {
      {
          .shape = {{0, 1, 1, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {1, 0, 0, 0},
          .rightmost = {2, 1, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 1, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {0, 1, 0, 0},
      },
      {
          .shape = {{0, 1, 1, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 0, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 3,
          .height = 2,
          .leftmost = {1, 0, 0, 0},
          .rightmost = {2, 1, 0, 0},
      },
      {
          .shape = {{1, 0, 0, 0}, 
                  {1, 1, 0, 0}, 
                  {0, 1, 0, 0}, 
                  {0, 0, 0, 0}},
          .width = 2,
          .height = 3,
          .leftmost = {0, 0, 0, 0},
          .rightmost = {0, 1, 0, 0},
      },
  },
};
// clang-format on

typedef struct {
  bool grid[BOARD_HEIGHT][BOARD_WIDTH];
  int heights[BOARD_WIDTH];
  int score;
} Board;

void board_init(Board *board) {
  memset(board->grid, false, sizeof(board->grid));
  memset(board->heights, 0, sizeof(board->heights));
  board->score = 0;
}

int board_get_height(const Board *board, int col) {
  if (col < 0 || col >= BOARD_WIDTH) {
    return -1;
  }
  return board->heights[col];
}

typedef struct {
  int cleared;
  double features[FEATURES];
  Board new_board;
} SimulateResult;

int get_required_y(const Board *board, const Piece *p, int x) {
  int required_y = 0;
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
    if (col >= BOARD_WIDTH)
      return -1;
    int h_col = board->heights[col];
    int max_i_for_dx = 0;
    bool has_block = false;
    for (int i = 0; i < p->height; i++) {
      if (p->shape[i][dx]) {
        has_block = true;
        int current_required_y = h_col - i;
        if (current_required_y > max_i_for_dx) {
          max_i_for_dx = current_required_y;
        }
      }
    }
    if (has_block && max_i_for_dx > required_y) {
      required_y = max_i_for_dx;
    }
  }
  return required_y;
}

int get_max_h(const int *heights, int start_x, int piece_weight) {
  int max_h = 0;
  for (int dx = 0; dx < piece_weight; dx++) {
    int col = start_x + dx;
    if (col >= BOARD_WIDTH)
      return -1;
    if (heights[col] > max_h)
      max_h = heights[col];
  }
  return max_h;
}

void update_blocks(int *blocks_count, int blocks_y[], int blocks_col[],
                   const Piece *p, int start_x, int required_y) {
  for (int i = 0; i < p->height; i++) {
    for (int j = 0; j < p->width; j++) {
      if (p->shape[i][j]) {
        int y = required_y + i;
        int col = start_x + j;
        blocks_y[*blocks_count] = y;
        blocks_col[*blocks_count] = col;
        (*blocks_count)++;
      }
    }
  }
}

int check_blocks_vaild(int *blocks_count, int blocks_y[], int blocks_col[],
                       const Piece *p, int start_x, int required_y,
                       bool grid[BOARD_HEIGHT][BOARD_WIDTH]) {
  for (int i = 0; i < p->height; i++) {
    for (int j = 0; j < p->width; j++) {
      if (p->shape[i][j]) {
        int y = required_y + i;
        int col = start_x + j;
        if (y >= BOARD_HEIGHT || grid[y][col]) {
          return -1;
        }
      }
    }
  }
  return 0;
}

int update_board(int blocks_count, int blocks_y[], int blocks_col[],
                 bool grid[BOARD_HEIGHT][BOARD_WIDTH], int heights[]) {
  for (int i = 0; i < blocks_count; i++) {
    int y = blocks_y[i];
    int col = blocks_col[i];
    grid[y][col] = true;
    heights[col] = (heights[col] > y + 1) ? heights[col] : y + 1;
  }
  return 0;
}

int get_full_rows(int full_rows[BOARD_HEIGHT],
                  const bool temp_grid[BOARD_HEIGHT][BOARD_WIDTH]) {
  int full_rows_count = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    bool full = true;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (!temp_grid[y][x]) {
        full = false;
        break;
      }
    }
    if (full)
      full_rows[full_rows_count++] = y;
  }
  return full_rows_count;
}

void clear_full_rows(int full_rows_count, const int *full_rows,
                     bool grid[BOARD_HEIGHT][BOARD_WIDTH], int grid_size,
                     int heights[BOARD_WIDTH], int heights_size) {
  bool new_grid[BOARD_HEIGHT][BOARD_WIDTH] = {{false}};
  int shift = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    bool is_full = false;
    for (int j = 0; j < full_rows_count; j++) {
      if (y == full_rows[j]) {
        is_full = true;
        break;
      }
    }
    if (is_full) {
      shift++;
      continue;
    }
    int new_y = y - shift;
    if (new_y >= 0 && new_y < BOARD_HEIGHT) {
      memcpy(new_grid[new_y], grid[y], sizeof(grid[y]));
    }
  }
  memcpy(grid, new_grid, grid_size);

  memset(heights, 0, heights_size);
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
      if (grid[y][x]) {
        heights[x] = y + 1;
        break;
      }
    }
  }
}

// -- 特征部分

// landing_height (最高块的y坐标)
int flanding_height(int blocks_count, int *blocks_y) {
  int landing_height = 0;
  for (int i = 0; i < blocks_count; i++) {
    landing_height =
        (blocks_y[i] > landing_height) ? blocks_y[i] : landing_height;
  }
  return landing_height;
}

// eroded_piece_cells (消除行中的方块数量×消除行数)
int feroded_piece_cells(int blocks_count, int full_rows_count, int *blocks_y,
                        int *full_rows) {
  int eroded = 0;
  for (int i = 0; i < blocks_count; i++) {
    int y = blocks_y[i];
    for (int j = 0; j < full_rows_count; j++) {
      if (y == full_rows[j]) {
        eroded++;
        break;
      }
    }
  }
  return eroded * full_rows_count;
}

// row_transitions (行变换次数)
int row_trans_table[1024]; // 2^10

void init_row_trans_table() {
  for (int bits = 0; bits < 1024; bits++) {
    int prev = 1, cnt = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      int curr = (bits >> x) & 1;
      if (curr != prev)
        cnt++;
      prev = curr;
    }
    if (prev == 0)
      cnt++;
    row_trans_table[bits] = cnt;
  }
}

int frow_transitions(bool temp_grid[BOARD_HEIGHT][BOARD_WIDTH]) {
  static bool initialized = false;
  if (!initialized) {
    init_row_trans_table();
    initialized = true;
  }
  int row_trans = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    int row_bits = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      row_bits |= (temp_grid[y][x] ? 1 : 0) << x;
    }
    row_trans += row_trans_table[row_bits];
  }
  return row_trans;
}

// column_transitions (列变换次数)
int col_trans_table[32768]; // 2^15

void init_col_trans_table() {
  for (int bits = 0; bits < 32768; bits++) {
    int prev = 1, cnt = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      int curr = (bits >> y) & 1;
      if (curr != prev)
        cnt++;
      prev = curr;
    }
    if (prev == 0)
      cnt++;
    col_trans_table[bits] = cnt;
  }
}

int fcolumn_transitions(bool temp_grid[BOARD_HEIGHT][BOARD_WIDTH]) {
  static bool initialized = false;
  if (!initialized) {
    init_col_trans_table();
    initialized = true;
  }
  int col_trans = 0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int col_bits = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      col_bits |= (temp_grid[y][x] ? 1 : 0) << y;
    }
    col_trans += col_trans_table[col_bits];
  }
  return col_trans;
}

// board_wells (井深)
int fboard_wells(int *temp_heights) {
  int wells = 0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int left = (x > 0) ? temp_heights[x - 1] : temp_heights[x];
    int right = (x < BOARD_WIDTH - 1) ? temp_heights[x + 1] : temp_heights[x];
    int current = temp_heights[x];
    if (current < left && current < right) {
      wells += ((left < right) ? left : right) - current;
    }
  }
  return wells;
}

void compute_holes_features(const bool grid[BOARD_HEIGHT][BOARD_WIDTH],
                            const int heights[BOARD_WIDTH], int *holes,
                            int *hole_depth, int *rows_with_holes) {
  *holes = 0;
  *hole_depth = 0;
  bool row_has_hole[BOARD_HEIGHT] = {false};
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int current_h = heights[x];
    for (int y = 0; y < current_h; y++) {
      if (!grid[y][x]) {
        (*holes)++;
        (*hole_depth) += (current_h - y);
        row_has_hole[y] = true;
      }
    }
  }
  *rows_with_holes = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    if (row_has_hole[y]) {
      (*rows_with_holes)++;
    }
  }
}

// diversity (行高度差异)
double fdiversity(int *temp_heights) {
  double diversity = 0;
  int prev_h = temp_heights[0];
  for (int x = 1; x < BOARD_WIDTH; x++) {
    diversity += abs(temp_heights[x] - prev_h);
    prev_h = temp_heights[x];
  }
  return diversity;
}

// RBF高度特征（features[9]~features[12]）
void fRBF(int *temp_heights, SimulateResult *result) {
  const int h = BOARD_HEIGHT;
  double c = 0.0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    c += temp_heights[x];
  }
  c /= BOARD_WIDTH;

  for (int i = 0; i < 4; i++) {
    double term = c - (i * h / 3.0);
    result->features[9 + i] = exp(-pow(term, 2) / (2 * pow(h / 5.0, 2)));
  }
}

void board_simulate(const Board *board, PieceType type, int x, int rotate,
                    SimulateResult *result) {
  const Piece *p = &ROTATIONS[type][rotate];
  bool temp_grid[BOARD_HEIGHT][BOARD_WIDTH];
  memcpy(temp_grid, board->grid, sizeof(board->grid));
  int temp_heights[BOARD_WIDTH];
  memcpy(temp_heights, board->heights, sizeof(board->heights));

  result->cleared = -1;
  memset(result->features, 0, sizeof(result->features));

  int required_y = get_required_y(board, p, x);
  if (required_y == -1)
    return;

  int blocks_count = 0;
  int blocks_y[4 * 4], blocks_col[4 * 4];

  update_blocks(&blocks_count, blocks_y, blocks_col, p, x, required_y);
  if (check_blocks_vaild(&blocks_count, blocks_y, blocks_col, p, x, required_y,
                         temp_grid) == -1)
    return;

  if (update_board(blocks_count, blocks_y, blocks_col, temp_grid,
                   temp_heights) == -1)
    return;

  int max_h = get_max_h(temp_heights, x, p->width);
  if (max_h == -1)
    return;

  int full_rows[BOARD_HEIGHT];
  int full_rows_count = get_full_rows(full_rows, temp_grid);
  if (full_rows_count > 0) {
    clear_full_rows(full_rows_count, full_rows, temp_grid, sizeof(temp_grid),
                    temp_heights, sizeof(temp_heights));
  }

  int holes, hole_depth_val, rows_with_holes_val;
  compute_holes_features(temp_grid, temp_heights, &holes, &hole_depth_val,
                         &rows_with_holes_val);

  result->features[0] = flanding_height(blocks_count, blocks_y);
  result->features[1] =
      feroded_piece_cells(blocks_count, full_rows_count, blocks_y, full_rows);
  result->features[2] = frow_transitions(temp_grid);
  result->features[3] = fcolumn_transitions(temp_grid);
  result->features[4] = holes;
  result->features[5] = fboard_wells(temp_heights);
  result->features[6] = hole_depth_val;
  result->features[8] = fdiversity(temp_heights);
  result->features[7] = rows_with_holes_val;
  result->cleared = full_rows_count;

  int add_score = 0;
  switch (full_rows_count) {
  case 1:
    add_score = 100;
    break;
  case 2:
    add_score = 300;
    break;
  case 3:
    add_score = 500;
    break;
  case 4:
    add_score = 800;
    break;
  }

  memcpy(result->new_board.grid, temp_grid, sizeof(temp_grid));
  memcpy(result->new_board.heights, temp_heights, sizeof(temp_heights));
  result->new_board.score = board->score + add_score;
}

void board_apply(Board *board, PieceType type, int x, int rotate) {
  const Piece *p = &ROTATIONS[type][rotate];
  int required_y = get_required_y(board, p, x);

  int blocks_count = 0;
  int blocks_y[4 * 4], blocks_col[4 * 4];
  update_blocks(&blocks_count, blocks_y, blocks_col, p, x, required_y);
  update_board(blocks_count, blocks_y, blocks_col, board->grid, board->heights);

  int full_rows[BOARD_HEIGHT];
  int full_rows_count = get_full_rows(full_rows, board->grid);
  if (full_rows_count > 0) {
    clear_full_rows(full_rows_count, full_rows, board->grid,
                    sizeof(board->grid), board->heights,
                    sizeof(board->heights));
    int add_score = 0;
    switch (full_rows_count) {
    case 1:
      add_score = 100;
      break;
    case 2:
      add_score = 300;
      break;
    case 3:
      add_score = 500;
      break;
    case 4:
      add_score = 800;
      break;
    }
    board->score += add_score;
  }
}

int board_get_score(const Board *board) { return board->score; }

// void board_draw(const Board *board) {
//   system("clear"); // 清屏（Linux/Unix系统）
//   // system("cls"); // Windows系统用此行替代

//   printf("Score: %d\n", board->score);
//   for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
//     printf("|");
//     for (int x = 0; x < BOARD_WIDTH; ++x) {
//       printf("%s", board->grid[y][x] ? "■" : " ");
//     }
//     printf("|\n");
//   }
//   printf("------------\n");
// }

// --- 主函数部分

typedef struct {
  int rotate;
  int x;
} Action;

// void read_initial_pieces(char next_pieces[], int *next_pieces_count) {
//   char s0, s1;
//   scanf("%c%c", &s0, &s1);
//   next_pieces[(*next_pieces_count)++] = s0;
//   next_pieces[(*next_pieces_count)++] = s1;
// }

PieceType char_to_piece_type(char c) {
  switch (c) {
  case 'I':
    return I;
  case 'T':
    return T;
  case 'O':
    return O;
  case 'J':
    return J;
  case 'L':
    return L;
  case 'S':
    return S;
  case 'Z':
    return Z;
  case 'X':
    return END;
  default:
    return END;
  }
}

bool is_position_valid(const Board *board, const Piece *p, int x) {
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
    if (col >= BOARD_WIDTH) {
      return false;
    }
  }

  int max_h = 0;
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
    int h = board_get_height(board, col);
    max_h = (h > max_h) ? h : max_h;
  }

  for (int i = 0; i < p->height; i++) {
    for (int j = 0; j < p->width; j++) {
      if (p->shape[i][j]) {
        int y = max_h + i;
        if (y >= BOARD_HEIGHT) {
          return false;
        }
      }
    }
  }
  return true;
}

void get_possible_actions(const Board *board, PieceType current,
                          Action actions[], int *actions_count) {
  *actions_count = 0;

  for (int rotate = 0; rotate < 4; rotate++) {
    const Piece *p = &ROTATIONS[current][rotate];
    for (int x = 0; x <= BOARD_WIDTH - p->width; x++) {
      if (is_position_valid(board, p, x)) {
        actions[*actions_count].rotate = rotate;
        actions[*actions_count].x = x;
        (*actions_count)++;
      }
    }
  }
}

double evaluate_next_piece(const Board *board, PieceType next_type) {
  Action next_actions[100];
  int next_actions_count;
  get_possible_actions(board, next_type, next_actions, &next_actions_count);

  double max_score = -1e9;
  for (int i = 0; i < next_actions_count; ++i) {
    SimulateResult sim;

    board_simulate(board, next_type, next_actions[i].x, next_actions[i].rotate,
                   &sim);
    if (sim.cleared == -1)
      continue;

    double score = 0;
    for (int j = 0; j < FEATURES; ++j) {
      score += sim.features[j] * WEIGHTS[j];
    }
    if (score > max_score) {
      max_score = score;
    }
  }
  return (max_score == -1e9) ? 0 : max_score;
}

Action find_best_action(const Board *board, PieceType current,
                        PieceType next_type, const Action actions[],
                        int actions_count) {

  typedef struct {
    Action action;
    double score;
    SimulateResult sim;
  } ScoredAction;

  ScoredAction scored_actions[actions_count];
  int valid_actions = 0;

  for (int i = 0; i < actions_count; ++i) {
    SimulateResult sim;

    board_simulate(board, current, actions[i].x, actions[i].rotate, &sim);
    if (sim.cleared == -1)
      continue;

    double current_score = 0;
    for (int j = 0; j < FEATURES; ++j) {
      current_score += sim.features[j] * WEIGHTS[j];
    }

    scored_actions[valid_actions].action = actions[i];
    scored_actions[valid_actions].score = current_score;
    scored_actions[valid_actions].sim = sim;
    valid_actions++;
  }

  if (valid_actions == 0) {
    return actions[0];
  }

  for (int i = 0; i < valid_actions - 1; ++i) {
    for (int j = i + 1; j < valid_actions; ++j) {
      if (scored_actions[i].score < scored_actions[j].score) {
        ScoredAction temp = scored_actions[i];
        scored_actions[i] = scored_actions[j];
        scored_actions[j] = temp;
      }
    }
  }

  double max_total = -1e9;
  Action best = scored_actions[0].action;

  int n = (valid_actions < TOP_N) ? valid_actions : TOP_N;
  if (next_type == END) {
    return best;
  }
  for (int i = 0; i < n; ++i) {
    double next_score =
        evaluate_next_piece(&scored_actions[i].sim.new_board, next_type);
    double total = scored_actions[i].score + next_score;

    if (total > max_total ||
        (total == max_total && scored_actions[i].action.x < best.x)) {
      max_total = total;
      best = scored_actions[i].action;
    }
  }

  return best;
}

void process_next_piece(Board *board, char next_pieces[], int *count,
                        bool is_timeout) {
  if (*count < 2)
    return;

  PieceType current = char_to_piece_type(next_pieces[0]);
  PieceType next = char_to_piece_type(next_pieces[1]);

  Action actions[100];
  int actions_count;
  get_possible_actions(board, current, actions, &actions_count);

  if (actions_count == 0) {
    printf("0 0\n%d\n", board->score);
    fflush(stdout);
    for (int i = 0; i < *count - 1; ++i) {
      next_pieces[i] = next_pieces[i + 1];
    }
    (*count)--;
    if (current == END || next == END) {
      exit(0);
    }
    return;
  }

  Action best;
  if (is_timeout) {
    best.rotate = 0;
    best.x = 0;
    board_apply(board, current, best.x, best.rotate);
  } else {
    best = find_best_action(board, current, next, actions, actions_count);
    board_apply(board, current, best.x, best.rotate);
  }

  for (int i = 0; i < *count - 1; ++i) {
    next_pieces[i] = next_pieces[i + 1];
  }
  (*count)--;

  printf("%d %d\n%d\n", best.rotate, best.x, board->score);
  fflush(stdout);
  if (current == END || next == END) {
    exit(0);
  }
  return;
}

void read_next_piece(char next_pieces[], int *next_pieces_count) {
  char next;
  while (1) {
    fflush(stdout);
    if (scanf("%c", &next) != 1) {
      exit(0);
    }
    if (next == 'E') {
      exit(0);
    }
    if (next != '\n' && next != ' ' && next != '\t') {
      next_pieces[(*next_pieces_count)++] = next;
      return;
    }
  }
}

int main() {
  clock_t start, finish;
  double duration;
  start = clock();
  Board board;
  board_init(&board);

  char next_pieces[100];
  int next_pieces_count = 0;
  fflush(stdout);
  read_next_piece(next_pieces, &next_pieces_count);
  read_next_piece(next_pieces, &next_pieces_count);
  while (1) {
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    bool is_timeout = duration >= 9;
    process_next_piece(&board, next_pieces, &next_pieces_count, is_timeout);
    read_next_piece(next_pieces, &next_pieces_count);
  }

  return 0;
}