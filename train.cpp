#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

using namespace std;

// 算法参数
const int FEATURE_DIM = 24;         // 特征维度
const int POPULATION_SIZE = 210;    // CMA-ES种群大小
const float SELECTION_RATIO = 0.1f; // 选择比例
const float NOISE_INTENSITY = 0.1f; // 噪声强度
const int NUM_EVALS = 20;           // 每次评估游戏次数
const int MAX_PIECES = 1000000;     // 每局最大方块数
const int NUM_THREADS = 14;         // 并行线程数

const int BOARD_HEIGHT = 20;
const int BOARD_WIDTH = 10;

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
    // O型 (保持不变)
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
  bool grid[BOARD_HEIGHT][BOARD_WIDTH];
  int heights[BOARD_WIDTH];
  int score;

public:
  Board() : score(0) {
    memset(grid, 0, sizeof(grid));
    memset(heights, 0, sizeof(heights));
  }

  int get_height(int col) const {
    return (col >= 0 && col < BOARD_WIDTH) ? heights[col] : -1;
  }

  // 模拟落子并返回消除行数和特征
  pair<int, vector<double>> simulate(PieceType type, int x, int rotate,
                                     const vector<double> &weights) const {
    const Piece &p = ROTATIONS[type][rotate];
    bool temp_grid[BOARD_HEIGHT][BOARD_WIDTH];
    memcpy(temp_grid, grid, sizeof(grid));
    int temp_heights[BOARD_WIDTH];
    memcpy(temp_heights, heights, sizeof(heights));

    int max_h = 0;
    for (int dx = 0; dx < p.width; dx++) {
      int col = x + dx;
      if (col >= BOARD_WIDTH)
        return make_pair(-1, vector<double>());
      if (temp_heights[col] > max_h)
        max_h = temp_heights[col];
    }

    vector<pair<int, int>> blocks;
    for (int i = 0; i < p.height; i++) {
      for (int j = 0; j < p.width; j++) {
        if (p.shape[i][j]) {
          int y = max_h + i;
          int col = x + j;
          if (y >= BOARD_HEIGHT || temp_grid[y][col])
            return make_pair(-1, vector<double>());
          blocks.push_back(make_pair(y, col));
        }
      }
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

    vector<double> features(FEATURE_DIM, 0.0);
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

    // 计算RBF高度特征（features[9]~features[13]）
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

    features[14] = (cleared >= 1);
    features[15] = (cleared >= 2);
    features[16] = (cleared >= 3);
    features[17] = (cleared >= 4);

    // 计算最高列高度
    features[18] = max_h;

    // 计算最低非空列高度
    int min_h = BOARD_HEIGHT;
    for (int x = 0; x < BOARD_WIDTH; x++) {
        if (temp_heights[x] > 0 && temp_heights[x] < min_h) {
            min_h = temp_heights[x];
        }
    }
    features[19] = (min_h == BOARD_HEIGHT) ? 0 : min_h;

    // 计算接近满的行数（差1-2个方块）
    int near_full = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        int count = 0;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (temp_grid[y][x]) count++;
        }
        if (count >= BOARD_WIDTH - 2) near_full++;
    }
    features[20] = near_full;

    int overhangs = 0;
    for (int x = 0; x < BOARD_WIDTH; x++) {
        bool has_support = true;
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            if (temp_grid[y][x]) {
                if (!has_support) overhangs++;
                has_support = true;
            } else {
                has_support = false;
            }
        }
    }
    features[21] = overhangs;

    // 计算消除后的平均高度变化
    double original_avg = accumulate(heights, heights + BOARD_WIDTH, 0.0) / BOARD_WIDTH;
    double new_avg = accumulate(temp_heights, temp_heights + BOARD_WIDTH, 0.0) / BOARD_WIDTH;
    features[22] = new_avg - original_avg;

    // 紧急情况指标（最高列接近顶部）
    features[23] = (max_h >= BOARD_HEIGHT - 3);
      
    return make_pair(cleared, features);
  }

  void apply(PieceType type, int x, int rotate) {
    const Piece &p = ROTATIONS[type][rotate];
    int max_h = 0;
    for (int dx = 0; dx < p.width; dx++) {
      int col = x + dx;
      max_h = max(max_h, heights[col]);
    }

    vector<pair<int, int>> blocks;
    for (int i = 0; i < p.height; i++) {
      for (int j = 0; j < p.width; j++) {
        if (p.shape[i][j]) {
          int y = max_h + i;
          int col = x + j;
          blocks.push_back(make_pair(y, col));
        }
      }
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

  void reset() {
    memset(grid, 0, sizeof(grid));
    memset(heights, 0, sizeof(heights));
    score = 0;
  }
};

// CMA-ES优化器
class CMAESOptimizer {
  vector<double> mean;
  vector<double> stddev;
  mt19937 gen;

public:
  CMAESOptimizer() : mean(FEATURE_DIM, 0.0), stddev(FEATURE_DIM, 1.0) {
    random_device rd;
    gen.seed(rd());
  }

  // 生成种群
  vector<vector<double>> generate_population() {
    vector<vector<double>> population;
    normal_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < POPULATION_SIZE; ++i) {
      vector<double> individual(FEATURE_DIM);
      for (int j = 0; j < FEATURE_DIM; ++j) {
        individual[j] = mean[j] + stddev[j] * dist(gen);
      }
      population.push_back(individual);
    }
    return population;
  }

  // 更新分布参数
  void update_distribution(const vector<vector<double>> &population,
                           const vector<double> &scores) {
    vector<size_t> indices(population.size());
    iota(indices.begin(), indices.end(), 0);

    partial_sort(indices.begin(),
                 indices.begin() + POPULATION_SIZE * SELECTION_RATIO,
                 indices.end(),
                 [&](size_t a, size_t b) { return scores[a] > scores[b]; });

    // 更新均值和标准差
    for (int j = 0; j < FEATURE_DIM; ++j) {
      double sum = 0.0;
      for (int i = 0; i < POPULATION_SIZE * SELECTION_RATIO; ++i) {
        sum += population[indices[i]][j];
      }
      mean[j] = sum / (POPULATION_SIZE * SELECTION_RATIO);

      double var = 0.0;
      for (int i = 0; i < POPULATION_SIZE * SELECTION_RATIO; ++i) {
        var += pow(population[indices[i]][j] - mean[j], 2);
      }
      stddev[j] =
          sqrt(var / (POPULATION_SIZE * SELECTION_RATIO)) + NOISE_INTENSITY;
    }
  }

  vector<double> get_best_weights() const { return mean; }
};

class ParallelEvaluator {
  vector<Board> boards;
  vector<vector<double>> weights_batch;
  vector<int> results;
  vector<thread> workers;

public:
  void evaluate_batch(const vector<vector<double>> &population) {
    const int batch_size = population.size();
    results.resize(batch_size);
    weights_batch = population;

    const int chunk_size = (batch_size + NUM_THREADS - 1) / NUM_THREADS;
    workers.clear();

    for (int t = 0; t < NUM_THREADS; ++t) {
      workers.emplace_back([this, t, chunk_size, batch_size]() {
        const int start = t * chunk_size;
        const int end = min(start + chunk_size, batch_size);

        for (int i = start; i < end; ++i) {
          if (i >= batch_size)
            break;

          Board board;
          int total = 0;
          for (int g = 0; g < NUM_EVALS; ++g) {
            total += play_single_game(weights_batch[i], board);
          }
          results[i] = total / NUM_EVALS;
        }
      });
    }

    for (auto &t : workers) {
      if (t.joinable())
        t.join();
    }
  }

  vector<int> get_results() const { return results; }

private:
  int play_single_game(const vector<double> &weights, Board &board) {
    board.reset();
    mt19937 gen(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> piece_dist(0, 6);

    for (int p = 0; p < MAX_PIECES; ++p) {
      PieceType type = static_cast<PieceType>(piece_dist(gen));
      auto actions = generate_actions(type, board);
      if (actions.empty())
        break;

      auto best_action = select_best_action(type, board, actions, weights);
      board.apply(type, best_action.second, best_action.first);
    }
    return board.get_score();
  }

  vector<pair<int, int>> generate_actions(PieceType type, const Board &board) {
    vector<pair<int, int>> actions;
    for (int rotate = 0; rotate < 4; ++rotate) {
      const Piece &p = ROTATIONS[type][rotate];
      for (int x = 0; x <= BOARD_WIDTH - p.width; ++x) {
        bool valid = true;
        int max_h = 0;
        for (int dx = 0; dx < p.width; ++dx) {
          int col = x + dx;
          if (col >= BOARD_WIDTH) {
            valid = false;
            break;
          }
          max_h = max(max_h, board.get_height(col));
        }

        if (valid) {
          for (int i = 0; i < p.height; ++i) {
            for (int j = 0; j < p.width; ++j) {
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
        }

        if (valid) {
          actions.emplace_back(rotate, x);
        }
      }
    }
    return actions;
  }

  pair<int, int> select_best_action(PieceType type, Board &board,
                                    const vector<pair<int, int>> &actions,
                                    const vector<double> &weights) {
    double max_score = -INFINITY;
    pair<int, int> best_action = {0, 0};

    for (const auto &action : actions) {
      auto [cleared, features] =
          board.simulate(type, action.second, action.first, weights);
      if (cleared == -1)
        continue;

      double score =
          inner_product(features.begin(), features.end(), weights.begin(), 0.0);
      if (score > max_score) {
        max_score = score;
        best_action = action;
      }
    }
    return best_action;
  }
};

// 训练系统
class TrainingSystem {
  CMAESOptimizer optimizer;
  ParallelEvaluator evaluator;
  ofstream logfile;

public:
  TrainingSystem() {
    logfile.open("training_log.csv");
    logfile << "generation,time_elapsed,best_score,average_score\n";
  }

  void run(int total_generations) {
    auto start_time = chrono::high_resolution_clock::now();

    for (int gen = 0; gen < total_generations; ++gen) {
      auto population = optimizer.generate_population();
      evaluator.evaluate_batch(population);
      auto scores_int = evaluator.get_results();
      vector<double> scores(scores_int.begin(), scores_int.end());

      optimizer.update_distribution(population, scores);

      auto current_time = chrono::high_resolution_clock::now();
      auto elapsed =
          chrono::duration_cast<chrono::seconds>(current_time - start_time)
              .count();

      double avg_score =
          accumulate(scores.begin(), scores.end(), 0.0) / scores.size();
      int best_score = *max_element(scores.begin(), scores.end());

      logfile << gen << "," << elapsed << "," << best_score << "," << avg_score
              << "\n";
      logfile.flush();

      if (gen % 10 == 9) {
        save_weights(gen);
      }

      print_progress(gen, best_score, avg_score);
    }
  }

private:
  void print_progress(int gen, int best, int avg) {
    cout << "\n=== Generation " << gen << " ==="
         << "\nBest Score: " << best << "\nAverage Score: " << avg
         << "\nCurrent Weights: ";

    auto weights = optimizer.get_best_weights();
    for (double w : weights) {
      cout << w << " ";
    }
    cout << "\n\n";
  }

  void save_weights(int gen) {
    system("mkdir -p weights");
    ofstream outfile("weights/weights_gen" + to_string(gen) + ".txt");
    auto weights = optimizer.get_best_weights();
    for (double w : weights) {
      outfile << w << "\n";
    }
  }
};

int main() {
  TrainingSystem system;
  system.run(1000);
  return 0;
}