#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEATURES 13
#define BOARD_HEIGHT 15
#define BOARD_WIDTH 10

const double WEIGHTS[] = {-1464772.166456, 2535297.130013,  -2638462.645342,
                          -372351.515440,  1782742.689903,  -1883234.918781,
                          4420.968667,     -9988776.620538, 948594.666888,
                          3610431.536749,  -3355542.370633, -1120426.582938,
                          -3233372.471683};

typedef enum { I, T, O, J, L, S, Z } PieceType;

typedef struct {
  int shape[4][4];
  int width, height;
  int leftmost[4];
  int rightmost[4];
} Piece;

const Piece ROTATIONS[7][4] = {
    // I型
    {{{{1, 1, 1, 1}, {0}, {0}, {0}}, 4, 1, {0, 0, 0, 0}, {3, 0, 0, 0}},
     {{{1}, {1}, {1}, {1}}, 1, 4, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{{1, 1, 1, 1}, {0}, {0}, {0}}, 4, 1, {0, 0, 0, 0}, {3, 0, 0, 0}},
     {{{1}, {1}, {1}, {1}}, 1, 4, {0, 0, 0, 0}, {0, 0, 0, 0}}},
    // T型
    {{{{0, 1, 0, 0}, {1, 1, 1, 0}, {0}, {0}}, 3, 2, {1, 0, 0, 0}, {1, 2, 0, 0}},
     {{{1, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 0, 0, 0}},
     {{{1, 1, 1, 0}, {0, 1, 0, 0}, {0}, {0}}, 3, 2, {0, 1, 0, 0}, {2, 1, 0, 0}},
     {{{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}}},
    // O型
    {{{{1, 1, 0, 0}, {1, 1, 0, 0}, {0}, {0}}, 2, 2, {0, 0, 0, 0}, {1, 1, 0, 0}},
     {{{1, 1, 0, 0}, {1, 1, 0, 0}, {0}, {0}}, 2, 2, {0, 0, 0, 0}, {1, 1, 0, 0}},
     {{{1, 1, 0, 0}, {1, 1, 0, 0}, {0}, {0}}, 2, 2, {0, 0, 0, 0}, {1, 1, 0, 0}},
     {{{1, 1, 0, 0}, {1, 1, 0, 0}, {0}, {0}},
      2,
      2,
      {0, 0, 0, 0},
      {1, 1, 0, 0}}},
    // J型
    {{{{1, 0, 0, 0}, {1, 1, 1, 0}, {0}, {0}}, 3, 2, {0, 0, 0, 0}, {2, 0, 0, 0}},
     {{{1, 1, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 0, 0, 0}},
     {{{1, 1, 1, 0}, {0, 0, 1, 0}, {0}, {0}}, 3, 2, {0, 0, 0, 0}, {2, 0, 0, 0}},
     {{{0, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}}},
    // L型
    {{{{0, 0, 1, 0}, {1, 1, 1, 0}, {0}, {0}}, 3, 2, {2, 0, 0, 0}, {2, 2, 0, 0}},
     {{{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}},
     {{{1, 1, 1, 0}, {1, 0, 0, 0}, {0}, {0}}, 3, 2, {0, 0, 0, 0}, {2, 0, 0, 0}},
     {{{1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}}},
    // S型
    {{{{0, 1, 1, 0}, {1, 1, 0, 0}, {0}, {0}}, 3, 2, {1, 0, 0, 0}, {2, 1, 0, 0}},
     {{{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}},
     {{{0, 1, 1, 0}, {1, 1, 0, 0}, {0}, {0}}, 3, 2, {1, 0, 0, 0}, {2, 1, 0, 0}},
     {{{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}}},
    // Z型
    {{{{1, 1, 0, 0}, {0, 1, 1, 0}, {0}, {0}}, 3, 2, {0, 0, 0, 0}, {1, 2, 0, 0}},
     {{{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}},
     {{{1, 1, 0, 0}, {0, 1, 1, 0}, {0}, {0}}, 3, 2, {0, 0, 0, 0}, {1, 2, 0, 0}},
     {{{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0}},
      2,
      3,
      {0, 0, 0, 0},
      {1, 1, 0, 0}}}};

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
} SimulateResult;

SimulateResult board_simulate(const Board *board, PieceType type, int x,
                              int rotate) {
  const Piece *p = &ROTATIONS[type][rotate];
  bool temp_grid[BOARD_HEIGHT][BOARD_WIDTH];
  memcpy(temp_grid, board->grid, sizeof(board->grid));
  int temp_heights[BOARD_WIDTH];
  memcpy(temp_heights, board->heights, sizeof(board->heights));

  SimulateResult result;
  result.cleared = -1;
  memset(result.features, 0, sizeof(result.features));

  int required_y = 0;
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
    if (col >= BOARD_WIDTH)
      return result;
    int h_col = temp_heights[col];
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

  int blocks_count = 0;
  int blocks_y[4 * 4], blocks_col[4 * 4]; // 最多4*4个方块

  for (int i = 0; i < p->height; i++) {
    for (int j = 0; j < p->width; j++) {
      if (p->shape[i][j]) {
        int y = required_y + i;
        int col = x + j;
        if (y >= BOARD_HEIGHT || temp_grid[y][col]) {
          return result;
        }
        blocks_y[blocks_count] = y;
        blocks_col[blocks_count] = col;
        blocks_count++;
      }
    }
  }

  int max_h = 0;
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
    if (col >= BOARD_WIDTH)
      return result;
    if (temp_heights[col] > max_h)
      max_h = temp_heights[col];
  }

  for (int i = 0; i < blocks_count; i++) {
    int y = blocks_y[i];
    int col = blocks_col[i];
    temp_grid[y][col] = true;
    temp_heights[col] = (temp_heights[col] > y + 1) ? temp_heights[col] : y + 1;
  }

  int full_rows[BOARD_HEIGHT];
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
  int cleared = full_rows_count;

  if (cleared > 0) {
    bool new_grid[BOARD_HEIGHT][BOARD_WIDTH] = {{false}};
    int shift = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
      if (shift < full_rows_count &&
          y == full_rows[full_rows_count - 1 - shift]) {
        shift++;
        continue;
      }
      int new_y = y + shift;
      if (new_y < BOARD_HEIGHT) {
        memcpy(new_grid[new_y], temp_grid[y], sizeof(temp_grid[y]));
      }
    }
    memcpy(temp_grid, new_grid, sizeof(temp_grid));

    memset(temp_heights, 0, sizeof(temp_heights));
    for (int x = 0; x < BOARD_WIDTH; x++) {
      for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        if (temp_grid[y][x]) {
          temp_heights[x] = y + 1;
          break;
        }
      }
    }
  }

  if (blocks_count == 0) {
    result.cleared = cleared;
    return result;
  }

  // 1. landing_height (最高块的y坐标)
  int landing_height = 0;
  for (int i = 0; i < blocks_count; i++) {
    landing_height =
        (blocks_y[i] > landing_height) ? blocks_y[i] : landing_height;
  }
  result.features[0] = landing_height;

  // 2. eroded_piece_cells (消除行中的方块数量×消除行数)
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
  result.features[1] = eroded * cleared;

  // 3. row_transitions (行变换次数)
  int row_trans = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    int prev = 1, cnt = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      int curr = temp_grid[y][x] ? 1 : 0;
      if (curr != prev)
        cnt++;
      prev = curr;
    }
    if (prev == 0)
      cnt++;
    row_trans += cnt;
  }
  result.features[2] = row_trans;

  // 4. column_transitions (列变换次数)
  int col_trans = 0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int prev = 1, cnt = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      int curr = temp_grid[y][x] ? 1 : 0;
      if (curr != prev)
        cnt++;
      prev = curr;
    }
    if (prev == 0)
      cnt++;
    col_trans += cnt;
  }
  result.features[3] = col_trans;

  // 5. holes (空洞数量)
  int holes = 0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int top = -1;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
      if (temp_grid[y][x]) {
        top = y;
        break;
      }
    }
    if (top == -1)
      continue;
    for (int y = 0; y < top; y++) {
      if (!temp_grid[y][x])
        holes++;
    }
  }
  result.features[4] = holes;

  // 6. board_wells (井深)
  int wells = 0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int left = (x > 0) ? temp_heights[x - 1] : temp_heights[x];
    int right = (x < BOARD_WIDTH - 1) ? temp_heights[x + 1] : temp_heights[x];
    int current = temp_heights[x];
    if (current < left && current < right) {
      wells += ((left < right) ? left : right) - current;
    }
  }
  result.features[5] = wells;

  // 7. hole_depth (孔深度)
  int hole_depth = 0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    int current_h = temp_heights[x];
    for (int y = 0; y < current_h; y++) {
      if (!temp_grid[y][x]) {
        hole_depth += (current_h - y);
      }
    }
  }
  result.features[6] = hole_depth;

  // 8. rows_with_holes (包含空洞的行数)
  int rows_with_holes = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    bool has_hole = false;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (!temp_grid[y][x] && temp_heights[x] > y) {
        has_hole = true;
        break;
      }
    }
    if (has_hole)
      rows_with_holes++;
  }
  result.features[7] = rows_with_holes;

  // 9. diversity (行高度差异)
  double diversity = 0;
  int prev_h = temp_heights[0];
  for (int x = 1; x < BOARD_WIDTH; x++) {
    diversity += abs(temp_heights[x] - prev_h);
    prev_h = temp_heights[x];
  }
  result.features[8] = diversity;

  // RBF高度特征（features[9]~features[12]）
  const int h = BOARD_HEIGHT;
  double c = 0.0;
  for (int x = 0; x < BOARD_WIDTH; x++) {
    c += temp_heights[x];
  }
  c /= BOARD_WIDTH;

  for (int i = 0; i < 4; i++) {
    double term = c - (i * h / 3.0);
    result.features[9 + i] = exp(-pow(term, 2) / (2 * pow(h / 5.0, 2)));
  }

  result.cleared = cleared;
  return result;
}

void board_apply(Board *board, PieceType type, int x, int rotate) {
  const Piece *p = &ROTATIONS[type][rotate];
  int required_y = 0;
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
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

  int blocks_count = 0;
  int blocks_y[4 * 4], blocks_col[4 * 4]; // 最多4*4个方块

  for (int i = 0; i < p->height; i++) {
    for (int j = 0; j < p->width; j++) {
      if (p->shape[i][j]) {
        int y = required_y + i;
        int col = x + j;
        blocks_y[blocks_count] = y;
        blocks_col[blocks_count] = col;
        blocks_count++;
      }
    }
  }

  int max_h = 0;
  for (int dx = 0; dx < p->width; dx++) {
    int col = x + dx;
    max_h = (board->heights[col] > max_h) ? board->heights[col] : max_h;
  }

  for (int i = 0; i < blocks_count; i++) {
    int y = blocks_y[i];
    int col = blocks_col[i];
    board->grid[y][col] = true;
    board->heights[col] =
        (board->heights[col] > y + 1) ? board->heights[col] : y + 1;
  }

  int full_rows[BOARD_HEIGHT];
  int full_rows_count = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    bool full = true;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (!board->grid[y][x]) {
        full = false;
        break;
      }
    }
    if (full)
      full_rows[full_rows_count++] = y;
  }

  if (full_rows_count > 0) {
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
        memcpy(new_grid[new_y], board->grid[y], sizeof(board->grid[y]));
      }
    }
    memcpy(board->grid, new_grid, sizeof(board->grid));

    memset(board->heights, 0, sizeof(board->heights));
    for (int x = 0; x < BOARD_WIDTH; x++) {
      for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        if (board->grid[y][x]) {
          board->heights[x] = y + 1;
          break;
        }
      }
    }
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

void board_draw(const Board *board) {
  system("clear"); // 清屏（Linux/Unix系统）
  // system("cls"); // Windows系统用此行替代

  printf("Score: %d\n", board->score);
  for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
    printf("|");
    for (int x = 0; x < BOARD_WIDTH; ++x) {
      printf("%s", board->grid[y][x] ? "■" : " ");
    }
    printf("|\n");
  }
  printf("------------\n");
}

int main() {
  Board board;
  board_init(&board);

  char next_pieces[100];
  int next_pieces_count = 0;
  char s0, s1;
  scanf(" %c %c", &s0, &s1);
  next_pieces[next_pieces_count++] = s0;
  next_pieces[next_pieces_count++] = s1;

  while (1) {
    if (next_pieces_count == 0)
      break;
    char current_char = next_pieces[0];
    for (int i = 0; i < next_pieces_count - 1; i++) {
      next_pieces[i] = next_pieces[i + 1];
    }
    next_pieces_count--;

    PieceType current;
    switch (current_char) {
    case 'I':
      current = I;
      break;
    case 'T':
      current = T;
      break;
    case 'O':
      current = O;
      break;
    case 'J':
      current = J;
      break;
    case 'L':
      current = L;
      break;
    case 'S':
      current = S;
      break;
    case 'Z':
      current = Z;
      break;
    default:
      exit(0);
    }

    int possible_actions_count = 0;
    int possible_rotates[100], possible_xs[100];

    for (int rotate = 0; rotate < 4; rotate++) {
      const Piece *p = &ROTATIONS[current][rotate];
      for (int x = 0; x <= BOARD_WIDTH - p->width; x++) {
        bool valid = true;
        int max_h = 0;
        for (int dx = 0; dx < p->width; dx++) {
          int col = x + dx;
          if (col >= BOARD_WIDTH) {
            valid = false;
            break;
          }
          max_h = (board_get_height(&board, col) > max_h)
                      ? board_get_height(&board, col)
                      : max_h;
        }
        if (!valid)
          continue;

        for (int i = 0; i < p->height; i++) {
          for (int j = 0; j < p->width; j++) {
            if (p->shape[i][j]) {
              int y = max_h + i;
              if (y >= BOARD_HEIGHT) {
                valid = false;
                break;
              }
            }
          }
          if (!valid)
            break;
        }
        if (valid) {
          possible_rotates[possible_actions_count] = rotate;
          possible_xs[possible_actions_count] = x;
          possible_actions_count++;
        }
      }
    }

    if (possible_actions_count == 0) {
      // printf("0 0\n%d\n", board_get_score(&board));
      exit(0);
    }

    double max_score = -1e9;
    int best_rotate = 0, best_x = 0;
    for (int i = 0; i < possible_actions_count; i++) {
      int rotate = possible_rotates[i];
      int x = possible_xs[i];
      SimulateResult sim_result = board_simulate(&board, current, x, rotate);
      int cleared = sim_result.cleared;
      if (cleared == -1)
        continue;

      double score = 0;
      for (int j = 0; j < FEATURES; j++) {
        score += sim_result.features[j] * WEIGHTS[j];
      }

      if (score > max_score || (score == max_score && x < best_x)) {
        max_score = score;
        best_rotate = rotate;
        best_x = x;
      }
    }

    board_apply(&board, current, best_x, best_rotate);
    printf("%d %d\n%d\n", best_rotate * 90, best_x, board_get_score(&board));
    // board_draw(&board);
    fflush(stdout);


    char next;
    if (scanf(" %c", &next) != 1)
      break;
    if (next == 'X')
      break;
    if (next == 'E')
      exit(0);
    next_pieces[next_pieces_count++] = next;
  }

  return 0;
}