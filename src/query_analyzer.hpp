#pragma once
#include <string>
#include <set>
#include <regex>
#include <sstream>

std::string classify_query(const std::string& query) {
    // Count unique variable names
    std::regex var_regex(R"(\?[a-zA-Z0-9_]+)");
    std::set<std::string> vars;
    auto it  = std::sregex_iterator(query.begin(), query.end(), var_regex);
    auto end = std::sregex_iterator();
    for (; it != end; ++it)
        vars.insert((*it)[0].str());

    // Count triple patterns by counting occurrences of " . "
    // This works whether the query is single-line or multi-line
    int triple_count = 0;
    size_t pos = 0;
    while ((pos = query.find(" . ", pos)) != std::string::npos) {
        triple_count++;
        pos++;
    }
    // Also count the final triple which ends with " }" not " . "
    triple_count += 1;

    if (triple_count >= 3 && vars.size() >= 3) return "triangle";
    if (triple_count >= 2)                      return "multi-join";
    return "simple";
}
