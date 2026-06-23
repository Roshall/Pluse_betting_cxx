/**
 * @file test_equivalence.cpp
 * @brief Equivalence tests between Python and C++ implementations.
 */

#include <catch2/catch_all.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "betting_by_time/framework.hpp"
#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"

using namespace betting;

struct TestCase {
    int test_id;
    float prior_mean;
    float delta;
    int grid_num;
    float expected_mean;
    int expected_used;
    std::vector<float> samples;
};

static std::vector<TestCase> read_csv(const std::string& path) {
    std::vector<TestCase> out;
    std::ifstream in(path);
    if (!in) {
        std::cerr << "[test_equivalence] failed to open CSV: " << path << "\n";
        return out;
    }
    std::string line;
    // header
    // Read all physical lines and coalesce logical records: continuation lines start with ','
    // Read entire file into a string, find header line, then tokenize remaining content by commas
    std::string content;
    std::string all;
    while (std::getline(in, line)) {
        all += line;
        all.push_back('\n');
    }
    auto pos = all.find('\n');
    if (pos == std::string::npos) {
        std::cerr << "[test_equivalence] malformed CSV: " << path << "\n";
        return out;
    }
    std::string header = all.substr(0, pos);
    std::cerr << "[test_equivalence] opened CSV: " << path << " header=" << header << "\n";
    content = all.substr(pos + 1);

    // tokenize by commas across the whole content (this handles physical line wraps)
    std::vector<std::string> toks;
    std::string cur;
    for (char c : content) {
        if (c == ',') {
            toks.push_back(cur);
            cur.clear();
        } else if (c == '\n') {
            // treat newline as a record separator only when cur is empty (skip)
            if (!cur.empty()) {
                toks.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) toks.push_back(cur);

    // parse tokens sequentially: test_id, prior_mean, delta, grid_num, expected_mean, expected_used, num_samples, samples...
    size_t idx = 0;
    while (idx + 7 <= toks.size()) {
        TestCase tc;
        try {
            tc.test_id = std::stoi(toks[idx++]);
            tc.prior_mean = std::stof(toks[idx++]);
            tc.delta = std::stof(toks[idx++]);
            tc.grid_num = std::stoi(toks[idx++]);
            tc.expected_mean = std::stof(toks[idx++]);
            tc.expected_used = std::stoi(toks[idx++]);
            int num_samples = std::stoi(toks[idx++]);
            tc.samples.reserve(num_samples);
            for (int i = 0; i < num_samples && idx < toks.size(); ++i) {
                tc.samples.push_back(std::stof(toks[idx++]));
            }
            out.push_back(std::move(tc));
        } catch (...) {
            break;
        }
    }
    return out;
}

// Optional: filter to a single test id for focused debugging
static int debug_test_id() {
    const char* env = std::getenv("DEBUG_TC");
    if (!env) return -1;
    try {
        return std::stoi(env);
    } catch (...) {
        return -1;
    }
}

TEST_CASE("Equivalence: vanilla_geo", "[equivalence][vanilla][geo]") {
    auto cases = read_csv(std::string(TEST_DATA_DIR) + "/test_data_vanilla_geo.csv");
    REQUIRE(!cases.empty());
        for (auto &tc : cases) {
        Vector32f samples(tc.samples.size());
        for (size_t i = 0; i < tc.samples.size(); ++i) samples(i) = tc.samples[i];
        // Submit samples one-by-one via breakpoints so gambler.s_ptr() counts individual samples
        std::vector<Int32> breakpoints;
        breakpoints.reserve(tc.samples.size() + 1);
        for (Int32 i = 0; i <= static_cast<Int32>(tc.samples.size()); ++i) breakpoints.push_back(i);
        auto [est, used] = vanilla_betting(samples, tc.prior_mean, tc.delta, tc.grid_num, breakpoints);
        CAPTURE(tc.test_id);
        CAPTURE(used);
        CAPTURE(tc.expected_used);
        CAPTURE(est);
        CAPTURE(tc.expected_mean);
        REQUIRE(used == tc.expected_used);
        REQUIRE(est == Catch::Approx(tc.expected_mean).margin(1e-5f));
    }
}

TEST_CASE("Equivalence: vanilla_seq", "[equivalence][vanilla][seq]") {
    auto cases = read_csv(std::string(TEST_DATA_DIR) + "/test_data_vanilla_seq.csv");
    REQUIRE(!cases.empty());
    for (auto &tc : cases) {
        int dbg = debug_test_id();
        if (dbg != -1 && tc.test_id != dbg) continue;
        Vector32f samples(tc.samples.size());
        for (size_t i = 0; i < tc.samples.size(); ++i) samples(i) = tc.samples[i];
        // Submit samples one-by-one via breakpoints so gambler.s_ptr() counts individual samples
        std::vector<Int32> breakpoints;
        breakpoints.reserve(tc.samples.size() + 1);
        for (Int32 i = 0; i <= static_cast<Int32>(tc.samples.size()); ++i) breakpoints.push_back(i);
        auto [est, used] = vanilla_betting_sequence(samples, tc.prior_mean, tc.delta, tc.grid_num, breakpoints);
        REQUIRE(used == tc.expected_used);
        REQUIRE(est == Catch::Approx(tc.expected_mean).margin(1e-5f));
    }
}

TEST_CASE("Equivalence: adaptive_geo", "[equivalence][adaptive][geo]") {
    auto cases = read_csv(std::string(TEST_DATA_DIR) + "/test_data_adaptive_geo.csv");
    REQUIRE(!cases.empty());
    for (auto &tc : cases) {
        int dbg = debug_test_id();
        if (dbg != -1 && tc.test_id != dbg) continue;
        Vector32f samples(tc.samples.size());
        for (size_t i = 0; i < tc.samples.size(); ++i) samples(i) = tc.samples[i];
        std::cerr << "[equivalence][adaptive][geo] running test_id=" << tc.test_id << "\n";
        // Submit samples one-by-one via breakpoints so gambler.s_ptr() counts individual samples
        std::vector<Int32> breakpoints;
        breakpoints.reserve(tc.samples.size() + 1);
        for (Int32 i = 0; i <= static_cast<Int32>(tc.samples.size()); ++i) breakpoints.push_back(i);
        auto [est, used] = adaptive_betting(samples, tc.prior_mean, tc.delta, tc.grid_num, breakpoints);
        REQUIRE(used == tc.expected_used);
        float grid_step = 1.0f / static_cast<float>(tc.grid_num);
        CAPTURE(grid_step);
        REQUIRE(est == Catch::Approx(tc.expected_mean).margin(grid_step + 1e-6f));
    }
}

TEST_CASE("Equivalence: adaptive_seq", "[equivalence][adaptive][seq]") {
    auto cases = read_csv(std::string(TEST_DATA_DIR) + "/test_data_adaptive_seq.csv");
    REQUIRE(!cases.empty());
    for (auto &tc : cases) {
        int dbg = debug_test_id();
        if (dbg != -1 && tc.test_id != dbg) continue;
        std::cerr << "[parsed_case] test_id=" << tc.test_id << " expected_used=" << tc.expected_used << " num_samples=" << tc.samples.size() << "\n";
        Vector32f samples(tc.samples.size());
        for (size_t i = 0; i < tc.samples.size(); ++i) samples(i) = tc.samples[i];
        // Submit samples one-by-one via breakpoints so gambler.s_ptr() counts individual samples
        std::vector<Int32> breakpoints;
        breakpoints.reserve(tc.samples.size() + 1);
        for (Int32 i = 0; i <= static_cast<Int32>(tc.samples.size()); ++i) breakpoints.push_back(i);
        auto [est, used] = adaptive_betting_sequence(samples, tc.prior_mean, tc.delta, tc.grid_num, breakpoints);
        REQUIRE(used == tc.expected_used);
        float grid_step = 1.0f / static_cast<float>(tc.grid_num);
        CAPTURE(grid_step);
        REQUIRE(est == Catch::Approx(tc.expected_mean).margin(grid_step + 1e-6f));
    }
}
