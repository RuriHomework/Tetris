#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;

const int FEATURES = 14;
const int BOARD_HEIGHT = 20;
const int BOARD_WIDTH = 10;

const double WEIGHTS[] = {
  -785.366,
39758.4,
-5707.22,
2420.21,
-2803.87,
-13750.8,
1959.47,
-22430.7,
2508.77,
-7812.77,
-2682.15,
1803.66,
-806.385,
-16704.9,
};

enum PieceType { I, T, O, J, L, S, Z };

struct Piece {
  int shape[4][4];
  int width, height;
  int leftmost[4];
  int rightmost[4];
};

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

class Board {
private:
  bool grid[BOARD_HEIGHT][BOARD_WIDTH];
  int heights[BOARD_WIDTH];
  int score;

public:
  Board() {
    memset(grid, false, sizeof(grid));
    memset(heights, 0, sizeof(heights));
    score = 0;
  }

  int get_height(int col) const {
    if (col < 0 || col >= BOARD_WIDTH) {
      return -1;
    }
    return heights[col];
  }

  pair<int, vector<double>> simulate(PieceType type, int x, int rotate) const {
    const Piece &p = ROTATIONS[type][rotate];
    bool temp_grid[20][10];
    memcpy(temp_grid, grid, sizeof(grid));
    int temp_heights[10];
    memcpy(temp_heights, heights, sizeof(heights));

    int required_y = 0;
    for (int dx = 0; dx < p.width; dx++) {
        int col = x + dx;
        if (col >= 10) return make_pair(-1, vector<double>());
        int h_col = temp_heights[col];
        int max_i_for_dx = 0;
        bool has_block = false;
        for (int i = 0; i < p.height; i++) {
            if (p.shape[i][dx]) {
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

    vector<pair<int, int>> blocks;
    for (int i = 0; i < p.height; i++) {
        for (int j = 0; j < p.width; j++) {
            if (p.shape[i][j]) {
                int y = required_y + i;
                int col = x + j;
                if (y >= 20 || temp_grid[y][col]) {
                    return make_pair(-1, vector<double>());
                }
                blocks.push_back(make_pair(y, col));
            }
        }
    }

    int max_h = 0;
    for (int dx = 0; dx < p.width; dx++) {
      int col = x + dx;
      if (col >= BOARD_WIDTH)
        return make_pair(-1, vector<double>());
      if (temp_heights[col] > max_h)
        max_h = temp_heights[col];
    }

    for (size_t i = 0; i < blocks.size(); i++) {
      int y = blocks[i].first;
      int col = blocks[i].second;
      temp_grid[y][col] = true;
      temp_heights[col] = max(temp_heights[col], y + 1);
    }

    vector<int> full_rows;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      bool full = true;
      for (int x = 0; x < BOARD_WIDTH; x++) {
        if (!temp_grid[y][x]) {
          full = false;
          break;
        }
      }
      if (full)
        full_rows.push_back(y);
    }
    int cleared = full_rows.size();

    if (cleared > 0) {
      bool new_grid[BOARD_HEIGHT][BOARD_WIDTH] = {{false}};
      int shift = 0;
      for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        if (shift < full_rows.size() &&
            y == full_rows[full_rows.size() - 1 - shift]) {
          shift++;
          continue;
        }
        int new_y = y + shift;
        if (new_y < BOARD_HEIGHT) {
          memcpy(new_grid[new_y], temp_grid[y], BOARD_WIDTH);
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

    vector<double> features(FEATURES, 0.0);
    if (blocks.empty())
      return make_pair(cleared, features);

    // 1. landing_height (最高块的y坐标)
    int landing_height = 0;
    for (size_t i = 0; i < blocks.size(); i++) {
      landing_height = max(landing_height, blocks[i].first);
    }
    features[0] = landing_height;

    // 2. eroded_piece_cells (消除行中的方块数量×消除行数)
    int eroded = 0;
    for (size_t i = 0; i < blocks.size(); i++) {
      int y = blocks[i].first;
      if (find(full_rows.begin(), full_rows.end(), y) != full_rows.end()) {
        eroded++;
      }
    }
    features[1] = eroded * cleared;

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
    features[2] = row_trans;

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
    features[3] = col_trans;

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
    features[4] = holes;

    // 6. board_wells (井深)
    int wells = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      int left = (x > 0) ? temp_heights[x - 1] : temp_heights[x];
      int right = (x < BOARD_WIDTH - 1) ? temp_heights[x + 1] : temp_heights[x];
      int current = temp_heights[x];
      if (current < left && current < right) {
        wells += (min(left, right) - current);
      }
    }
    features[5] = wells;

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
    features[6] = hole_depth;

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
    features[7] = rows_with_holes;

    // 9. diversity (行高度差异)
    double diversity = 0;
    int prev_h = temp_heights[0];
    for (int x = 1; x < BOARD_WIDTH; x++) {
      diversity += abs(temp_heights[x] - prev_h);
      prev_h = temp_heights[x];
    }
    features[8] = diversity;

    // RBF高度特征（features[9]~features[13]）
    const int h = BOARD_HEIGHT;
    double c = 0.0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
        c += temp_heights[x];
    }
    c /= BOARD_WIDTH;
    
    for (int i = 0; i < 5; i++) {
        double term = c - (i * h / 4.0);
        features[9 + i] = exp(-pow(term, 2) / (2 * pow(h / 5.0, 2)));
    }

    return make_pair(cleared, features);
  }

  void apply(PieceType type, int x, int rotate) {
    const Piece &p = ROTATIONS[type][rotate];
    int required_y = 0;
    for (int dx = 0; dx < p.width; dx++) {
        int col = x + dx;
        int h_col = heights[col];
        int max_i_for_dx = 0;
        bool has_block = false;
        for (int i = 0; i < p.height; i++) {
            if (p.shape[i][dx]) {
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

    vector<pair<int, int>> blocks;
    for (int i = 0; i < p.height; i++) {
        for (int j = 0; j < p.width; j++) {
            if (p.shape[i][j]) {
                int y = required_y + i;
                int col = x + j;
                blocks.push_back(make_pair(y, col));
            }
        }
    }
    int max_h = 0;
    for (int dx = 0; dx < p.width; dx++) {
      int col = x + dx;
      max_h = max(max_h, heights[col]);
    }

    for (size_t i = 0; i < blocks.size(); i++) {
      int y = blocks[i].first;
      int col = blocks[i].second;
      grid[y][col] = true;
      heights[col] = max(heights[col], y + 1);
    }

    vector<int> full_rows;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      bool full = true;
      for (int x = 0; x < BOARD_WIDTH; x++) {
        if (!grid[y][x]) {
          full = false;
          break;
        }
      }
      if (full)
        full_rows.push_back(y);
    }

    if (!full_rows.empty()) {
      bool new_grid[BOARD_HEIGHT][BOARD_WIDTH] = {{false}};
      int shift = 0;
      for (int y = 0; y < BOARD_HEIGHT; y++) {
        if (find(full_rows.begin(), full_rows.end(), y) != full_rows.end()) {
          shift++;
          continue;
        }
        int new_y = y - shift;
        if (new_y >= 0 && new_y < BOARD_HEIGHT) {
          memcpy(new_grid[new_y], grid[y], BOARD_WIDTH);
        }
      }
      memcpy(grid, new_grid, sizeof(grid));

      memset(heights, 0, sizeof(heights));
      for (int x = 0; x < BOARD_WIDTH; x++) {
        for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
          if (grid[y][x]) {
            heights[x] = y + 1;
            break;
          }
        }
      }
      int add_score = 0;
      switch (full_rows.size()) {
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
      score += add_score;
    }
  }

  int get_score() const { return score; }

  void draw() const {
    system("clear"); // 清屏（Linux/Unix系统）
    // system("cls"); // Windows系统用此行替代

    cout << "Score: " << score << endl;
    for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
      cout << "|";
      for (int x = 0; x < BOARD_WIDTH; ++x) {
        cout << (grid[y][x] ? "■" : " "); // 已填充则显示■，否则空格
      }
      cout << "|" << endl;
    }
    cout << "--------------------------" << endl;
  }
};

int main() {
  Board board;
  vector<char> next_pieces;
  char s0, s1;
  cin >> s0 >> s1;
  next_pieces.push_back(s0);
  next_pieces.push_back(s1);

  while (true) {
    if (next_pieces.empty())
      break;
    char current_char = next_pieces[0];
    next_pieces.erase(next_pieces.begin());

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

    vector<pair<int, int>> possible_actions;
    for (int rotate = 0; rotate < 4; rotate++) {
      const Piece &p = ROTATIONS[current][rotate];
      for (int x = 0; x <= BOARD_WIDTH - p.width; x++) {
        bool valid = true;
        int max_h = 0;
        for (int dx = 0; dx < p.width; dx++) {
          int col = x + dx;
          if (col >= BOARD_WIDTH) {
            valid = false;
            break;
          }
          max_h = max(max_h, board.get_height(col));
        }
        if (!valid)
          continue;

        for (int i = 0; i < p.height; i++) {
          for (int j = 0; j < p.width; j++) {
            if (p.shape[i][j]) {
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
          possible_actions.push_back(make_pair(rotate, x));
        }
      }
    }

    if (possible_actions.empty()) {
      cout << "0 0" << endl << board.get_score() << endl;
      exit(0);
    }

    double max_score = -1e9;
    pair<int, int> best_action;
    for (size_t i = 0; i < possible_actions.size(); i++) {
      int rotate = possible_actions[i].first;
      int x = possible_actions[i].second;
      pair<int, vector<double>> sim_result = board.simulate(current, x, rotate);
      int cleared = sim_result.first;
      vector<double> features = sim_result.second;
      if (cleared == -1)
        continue;

      double score = 0;
      for (int i = 0; i < FEATURES; i++) {
        score += features[i] * WEIGHTS[i];
      }

      if (score > max_score || (score == max_score && x < best_action.second)) {
        max_score = score;
        best_action.first = rotate;
        best_action.second = x;
      }
    }

    cout << best_action.first * 90 << " " << best_action.second << endl;
    cout << board.get_score() << endl;
    board.draw();
    fflush(stdout);

    board.apply(current, best_action.second, best_action.first);

    char next;
    if (!(cin >> next))
      break;
    if (next == 'X')
      break;
    if (next == 'E')
      exit(0);
    next_pieces.push_back(next);
  }

  return 0;
}