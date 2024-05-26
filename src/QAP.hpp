#pragma once

#include <iostream>
#include <vector>
#include <istream>
#include <algorithm>
#include <climits>
#include <numeric>
#include <random>

class Matrix{
public:
    Matrix() = default;

    Matrix(size_t rows, size_t cols) : matrix(rows, std::vector<int>(cols)) {}

    auto operator[](size_t index) -> std::vector<int>& {
        return matrix[index];
    }

    auto operator[](size_t index) const -> const std::vector<int>& {
        return matrix[index];
    }

    friend auto operator>>(std::istream& is, Matrix& m) -> std::istream&;
    
private:
    std::vector<std::vector<int>> matrix;
};


auto operator>>(std::istream& is, Matrix& m) -> std::istream& {
    int value;
    for (auto& row : m.matrix) {
        for (auto& elem : row) {
            if (!(is >> value)) {
                is.setstate(std::ios_base::failbit);
                break;
            }
            elem = value;
        }
    }
    return is;
}

auto operator<<(std::ostream& os, const std::vector<int>& vec) -> std::ostream& {
    bool first = true;
    for (const auto& element : vec) {
        if (!first) {
            os << " ";
        } else {
            first = false;
        }
        os << element;
    }
    return os;
}


class IteratedLocalSearch;


class QAP {
    friend class IteratedLocalSearch;
public:
    QAP(int n, Matrix& distance_matrix, Matrix& flow_matrix) :
        m_size(n), D(distance_matrix), F(flow_matrix) {
        best_solution.resize(n);
        dont_look_bits.resize(n, false);
        initialize_first_solution();
    }

    void local_search_first_improvement() {
        bool improvement = true;

        while (improvement) {
            improvement = false;
            int best_delta = 0;
            int best_r = -1;
            int best_s = -1;

            for (int r = 0; r < m_size; ++r) {
                if (dont_look_bits[r]) {
                    continue;
                }
                for (int s = r + 1; s < m_size; ++s) {
                    if (dont_look_bits[s]) {
                        continue;
                    }
                    int delta = calculate_delta(best_solution, r, s);

                    if (delta < 0) {
                        std::swap(best_solution[r], best_solution[s]);
                        score_of_best_solution += delta;
                        improvement = true;
                        break;
                    }

                    dont_look_bits[r] = dont_look_bits[s] = true;
                }

                if (improvement) { break; }
            }
        }
    }

    auto get_best_solution() -> std::vector<int> {
        return best_solution;
    }

    auto get_best_score() -> int {
        return score_of_best_solution;
    }

private:
    // random
    void initialize_first_solution() {
        std::random_device rd;
        std::mt19937 gen(rd());
        for (int i = 0; i < m_size; ++i) {
            best_solution[i] = i;
        }
        std::shuffle(best_solution.begin(), best_solution.end(), gen);
        score_of_best_solution = calculate_score(best_solution);
    }

    // auto calculate_delta_streak(const std::vector<int>& phi_streak, int delta_r_s, int r, int s, int u, int v) -> int {
    //     int temp = (F[r][u] - F [r][v] + F[s][v] - F[s][u]) *
    //         (D[phi_streak[s]][phi_streak[u]] - D[phi_streak[s]][phi_streak[v]] +
    //         D[phi_streak[r]][phi_streak[v]] - D[phi_streak[r]][phi_streak[u]]) +
    //         (F[u][r] - F [r][v] + F[v][s] - F[u][s]) *
    //         (D[phi_streak[u]][phi_streak[s]] - D[phi_streak[v]][phi_streak[s]] +
    //         D[phi_streak[v]][phi_streak[r]] - D[phi_streak[u]][phi_streak[r]]);
    //     return delta_r_s + temp;
    // }

    auto calculate_delta(const std::vector<int>& solution, int r, int s) -> int {
        int delta = 0;
        for (int k = 0; k < m_size; ++k) {
            if (k != r && k != s) {
                delta += (F[s][k] - F[r][k]) * (D[solution[s]][solution[k]] - D[solution[r]][solution[k]]);
            }
        }
        return 2 * delta;
    }

    auto calculate_score(const std::vector<int>& solution) -> int {
        int score = 0;
        for (int i = 0; i < m_size; ++i) {
            for (int j = 0; j < m_size; ++j) {
                score += F[i][j] * D[solution[i]][solution[j]];
            }
        }
        return score;
    }

    int m_size;
    Matrix D;
    Matrix F;

    int score_of_best_solution;
    std::vector<int> best_solution;

    std::vector<bool> dont_look_bits;
};


class IteratedLocalSearch {
public:
    IteratedLocalSearch(int n, Matrix& distance_matrix, Matrix& flow_matrix, int max_iterations, int k) :
        qap(n, distance_matrix, flow_matrix), max_iterations(max_iterations), k(k), gen(std::random_device{}()) {
        qap.local_search_first_improvement();
        s0 = qap.get_best_solution();
        s = s0;
        best_score = qap.get_best_score();
    }

    void run() {
        int iteration = 0;

        while (iteration < max_iterations) {
            auto s_prime = perturbation(s);
            auto s_double_prime = local_search(s_prime);
            s = acceptance_criterion(s, s_double_prime);
            ++iteration;
        }
    }

    auto get_best_solution() -> std::vector<int> {
        return s;
    }

    auto get_best_score() -> int {
        return best_score;
    }

private:
    auto local_search(const std::vector<int>& initial_solution) -> std::vector<int> {
        qap.local_search_first_improvement();
        return qap.get_best_solution();
    }

    auto perturbation(std::vector<int>& solution) -> std::vector<int> {
        std::uniform_int_distribution<> dis(0, qap.m_size);

        std::vector<int> perturbed_solution = solution;

        for (int i = 0; i < k; ++i) {
            int idx1 = dis(gen);
            int idx2 = dis(gen);
            while (idx1 == idx2) {
                idx2 = dis(gen);
            }

            if (idx1 > idx2)  {
                std::swap(idx1, idx2);
            }

            std::reverse(perturbed_solution.begin() + idx1, perturbed_solution.begin() + idx2);
        }

        return perturbed_solution;
    }

    auto acceptance_criterion(const std::vector<int>& s, const std::vector<int>& s_double_prime) -> std::vector<int> {
        int score_s = qap.calculate_score(s);
        int score_s_double_prime = qap.calculate_score(s_double_prime);

        if (score_s_double_prime < score_s) {
            best_score = score_s_double_prime;
            return s_double_prime;
        }
        return s;
    }

    int max_iterations;
    int k;
    std::mt19937 gen;
    QAP qap;
    std::vector<int> s0;
    std::vector<int> s;
    int best_score;
};