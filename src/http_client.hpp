#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include "json.hpp"
using json = nlohmann::json;

struct QueryResult {
    std::string query_text;
    std::string query_class;
    double      latency_ms;
    int         row_count;
    bool        success;
    std::string error_msg;
};

static size_t write_callback(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::vector<std::string> load_queries(const std::string& filepath) {
    std::vector<std::string> queries;
    std::ifstream file(filepath);
    std::string line, current;
    while (std::getline(file, line)) {
        if (line.empty()) {
            if (!current.empty()) {
                queries.push_back(current);
                current.clear();
            }
        } else {
            current += line + "\n";
        }
    }
    if (!current.empty()) queries.push_back(current);
    return queries;
}

QueryResult send_sparql_query(const std::string& endpoint,
                               const std::string& query,
                               const std::string& query_class) {
    QueryResult result;
    result.query_text  = query;
    result.query_class = query_class;
    result.success     = false;
    result.row_count   = 0;

    CURL* curl = curl_easy_init();
    if (!curl) { result.error_msg = "curl init failed"; return result; }

    char* encoded = curl_easy_escape(curl, query.c_str(), query.size());
    std::string full_url = endpoint + "?query=" + std::string(encoded);
    curl_free(encoded);

    std::string response_body;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/sparql-results+json");

    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    auto t_start = std::chrono::high_resolution_clock::now();
    CURLcode res  = curl_easy_perform(curl);
    auto t_end   = std::chrono::high_resolution_clock::now();

    result.latency_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    if (res == CURLE_OK) {
        try {
            auto j = json::parse(response_body);
            result.row_count = j["results"]["bindings"].size();
            result.success   = true;
        } catch (...) {
            result.error_msg = "JSON parse error: " + response_body.substr(0, 120);
        }
    } else {
        result.error_msg = curl_easy_strerror(res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}
