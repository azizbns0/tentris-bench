#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "http_client.hpp"

void print_report(const std::vector<QueryResult>& results) {
    std::cout << "\n========== BENCHMARK REPORT ==========\n";
    std::cout << std::left
              << std::setw(12) << "Class"
              << std::setw(14) << "Latency(ms)"
              << std::setw(8)  << "Rows"
              << std::setw(8)  << "Status"
              << "\n";
    std::cout << std::string(42, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::setw(12) << r.query_class
                  << std::setw(14) << std::fixed << std::setprecision(2) << r.latency_ms
                  << std::setw(8)  << r.row_count
                  << std::setw(8)  << (r.success ? "OK" : "FAIL")
                  << "\n";
    }
    std::cout << "======================================\n";
}

void write_csv(const std::vector<QueryResult>& results,
               const std::string& filepath) {
    std::ofstream f(filepath);
    f << "class,latency_ms,row_count,success\n";
    for (const auto& r : results)
        f << r.query_class << ","
          << r.latency_ms  << ","
          << r.row_count   << ","
          << r.success     << "\n";
    std::cout << "CSV written to " << filepath << "\n";
}
