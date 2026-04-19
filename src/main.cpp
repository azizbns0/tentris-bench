#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <curl/curl.h>

#include "http_client.hpp"
#include "query_analyzer.hpp"
#include "reporter.hpp"

int main(int argc, char* argv[]) {
    std::string endpoint   = "http://localhost:9080/sparql";
    std::string query_file = "queries/benchmark.sparql";
    std::string csv_out    = "results.csv";
    int         runs       = 3;

    if (argc > 1) endpoint   = argv[1];
    if (argc > 2) query_file = argv[2];

    std::cout << "tentris-bench v1.0\n";
    std::cout << "Endpoint  : " << endpoint   << "\n";
    std::cout << "Queries   : " << query_file << "\n";
    std::cout << "Runs/query: " << runs       << "\n\n";

    auto queries = load_queries(query_file);
    if (queries.empty()) {
        std::cerr << "Error: no queries loaded from " << query_file << "\n";
        return 1;
    }
    std::cout << "Loaded " << queries.size() << " queries.\n";

    curl_global_init(CURL_GLOBAL_ALL);

    std::vector<QueryResult> all_results;
    for (size_t i = 0; i < queries.size(); i++) {
        const auto& q   = queries[i];
        std::string cls = classify_query(q);

        std::cout << "Running query " << (i+1) << "/" << queries.size()
                  << " [" << cls << "]... ";
        std::cout.flush();

        // Run each query `runs` times and take the average latency
        double total_ms = 0;
        QueryResult last;
        for (int r = 0; r < runs; r++) {
            last = send_sparql_query(endpoint, q, cls);
            total_ms += last.latency_ms;
        }
        last.latency_ms = total_ms / runs;
        all_results.push_back(last);

        if (last.success)
            std::cout << last.latency_ms << " ms, " << last.row_count << " rows\n";
        else
            std::cout << "FAILED: " << last.error_msg << "\n";
    }

    curl_global_cleanup();

    print_report(all_results);
    write_csv(all_results, csv_out);
    return 0;
}