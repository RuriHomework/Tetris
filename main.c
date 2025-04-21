#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- 常量部分

#define FEATURES 13
#define BOARD_HEIGHT 15
#define BOARD_WIDTH 10

const double WEIGHTS[] = {-1464772.166456, 2535297.130013,  -2638462.645342,
                          -372351.515440,  1782742.689903,  -1883234.918781,
                          4420.968667,     -9988776.620538, 948594.666888,
                          3610431.536749,  -3355542.370633, -1120426.582938,
                          -3233372.471683};

typedef enum { I, T, O, J, L, S, Z } PieceType;

// --- 棋盘部分

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

  int required_y = get_required_y(board, p, x);
  if (required_y == -1)
    return result;

  int blocks_count = 0;
  int blocks_y[4 * 4], blocks_col[4 * 4];

  update_blocks(&blocks_count, blocks_y, blocks_col, p, x, required_y);
  if (check_blocks_vaild(&blocks_count, blocks_y, blocks_col, p, x, required_y,
                         temp_grid) == -1)
    return result;

  if (update_board(blocks_count, blocks_y, blocks_col, temp_grid,
                   temp_heights) == -1)
    return result;

  int max_h = get_max_h(temp_heights, x, p->width);
  if (max_h == -1)
    return result;

  int full_rows[BOARD_HEIGHT];
  int full_rows_count = get_full_rows(full_rows, temp_grid);
  if (full_rows_count > 0) {
    clear_full_rows(full_rows_count, full_rows, temp_grid, sizeof(temp_grid),
                    temp_heights, sizeof(temp_heights));
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
  result.features[1] = eroded * full_rows_count;

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

  result.cleared = full_rows_count;
  return result;
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

// --- 主函数部分

typedef struct {
  int rotate;
  int x;
} Action;

void read_initial_pieces(char next_pieces[], int *next_pieces_count) {
  char s0, s1;
  scanf(" %c %c", &s0, &s1);
  next_pieces[(*next_pieces_count)++] = s0;
  next_pieces[(*next_pieces_count)++] = s1;
}

PieceType char_to_piece_type(char c) {
  switch (c) {
      case 'I': return I;
      case 'T': return T;
      case 'O': return O;
      case 'J': return J;
      case 'L': return L;
      case 'S': return S;
      case 'Z': return Z;
      default: exit(0);
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

Action find_best_action(const Board *board, PieceType current, 
                      const Action actions[], int actions_count) {
  double max_score = -1e9;
  Action best_action = {0, 0};
  
  for (int i = 0; i < actions_count; i++) {
      int rotate = actions[i].rotate;
      int x = actions[i].x;
      SimulateResult sim_result = board_simulate(board, current, x, rotate);
      
      if (sim_result.cleared == -1) continue;
      
      double score = 0;
      for (int j = 0; j < FEATURES; j++) {
          score += sim_result.features[j] * WEIGHTS[j];
      }

      if (score > max_score || (score == max_score && x < best_action.x)) {
          max_score = score;
          best_action.rotate = rotate;
          best_action.x = x;
      }
  }
  
  return best_action;
}

void process_next_piece(Board *board, char next_pieces[], int *next_pieces_count) {
  if (*next_pieces_count == 0) return;
  
  char current_char = next_pieces[0];
  for (int i = 0; i < *next_pieces_count - 1; i++) {
      next_pieces[i] = next_pieces[i + 1];
  }
  (*next_pieces_count)--;
  
  PieceType current = char_to_piece_type(current_char);
  
  Action possible_actions[100];
  int possible_actions_count;
  get_possible_actions(board, current, possible_actions, &possible_actions_count);
  
  if (possible_actions_count == 0) {
      exit(0);
  }
  
  Action best_action = find_best_action(board, current, possible_actions, possible_actions_count);
  board_apply(board, current, best_action.x, best_action.rotate);
  
  printf("%d %d\n%d\n", best_action.rotate * 90, best_action.x, board_get_score(board));
  fflush(stdout);
}

void read_next_piece(char next_pieces[], int *next_pieces_count) {
  char next;
  if (scanf(" %c", &next) != 1) return;
  
  if (next == 'X' || next == 'E') {
      exit(0);
  }
  
  next_pieces[(*next_pieces_count)++] = next;
}

int main() {
  Board board;
  board_init(&board);
  
  char next_pieces[100];
  int next_pieces_count = 0;
  
  read_initial_pieces(next_pieces, &next_pieces_count);
  
  while (1) {
      process_next_piece(&board, next_pieces, &next_pieces_count);
      read_next_piece(next_pieces, &next_pieces_count);
  }
  
  return 0;
}