#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <string>
#include <chrono>
#include "QAP.hpp"


std::vector<std::string> files = {"./benchmarks/tai20a",
                                    "./benchmarks/tai40a",
                                    "./benchmarks/tai60a",
                                    "./benchmarks/tai80a",
                                    "./benchmarks/tai100a"
};


std::vector<std::string> res_files = { "Tai20a.sol",
                                    "Tai40a.sol",
                                    "Tai60a.sol",
                                    "Tai80a.sol",
                                    "Tai100a.sol"
};


void read_info(const std::string& file_name, int& n, Matrix& distance_matrix, Matrix& flow_matrix) {
    std::ifstream inputFile(file_name);

    if (!inputFile.is_open()) {
        std::cerr << "Can't open file" << std::endl;
        return;
    }

    inputFile >> n;

    distance_matrix = Matrix(n, n);
    flow_matrix = Matrix(n,n);

    inputFile >> distance_matrix;

    std::string dummy_line;
    std::getline(inputFile, dummy_line);

    inputFile >> flow_matrix;

    inputFile.close();
}

void write_res_file(const std::string& res_file, const std::vector<int>& solution) {
    std::ofstream ofs;
    ofs.open(res_file, std::ofstream::out | std::ofstream::trunc);

    if (ofs.is_open()) {
        ofs << solution;
    } else {
        std::cerr << "Can't open output file" << std::endl;
    }

    ofs.close();
}

void local_search_first_improvement_run() {
    for (int i = 0; i < files.size(); ++i) {
        std::cout << files[i] << std::endl;

        int n;
        Matrix distance_matrix;
        Matrix flow_matrix;

        read_info(files[i], n, distance_matrix, flow_matrix);

        QAP qap(n, distance_matrix, flow_matrix);

        auto start = std::chrono::high_resolution_clock::now();

        qap.local_search_first_improvement();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::string res_file = "./results/local_search_first_improvement" + res_files[i];
        write_res_file(res_file, qap.get_best_solution());

        std::cout << "Best solution: ";
        std::cout << qap.get_best_solution() << std::endl;
        std::cout << "Best score: " << qap.get_best_score() << std::endl;
        std::cout << "TIME: " << elapsed.count() << "s" << std::endl;

        std::cout << std::endl;
    }
}

void iterated_local_search_run() {
    for (int i = 0; i < files.size(); ++i) {
        std::cout << files[i] << std::endl;

        int n;
        Matrix distance_matrix;
        Matrix flow_matrix;

        read_info(files[i], n, distance_matrix, flow_matrix);

        IteratedLocalSearch alg(n, distance_matrix, flow_matrix, 1000, 2);

        auto start = std::chrono::high_resolution_clock::now();

        alg.run();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        std::string res_file = "./results/iterated_local_search" + res_files[i];
        write_res_file(res_file, alg.get_best_solution());

        std::cout << "Best solution: ";
        std::cout << alg.get_best_solution() << std::endl;
        std::cout << "Best score: " << alg.get_best_score() << std::endl;
        std::cout << "TIME: " << elapsed.count() << "s" << std::endl;

        std::cout << std::endl;
    }
}

auto main() -> int {
    local_search_first_improvement_run();

    iterated_local_search_run();

    return 0;
}