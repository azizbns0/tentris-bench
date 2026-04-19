# tentris-bench

A SPARQL query benchmarking CLI for [Tentris](https://github.com/dice-group/tentris-research-project), written in C++17.

## Sample output
```text
tentris-bench v1.0
Endpoint  : http://localhost:9080/sparql
Queries   : queries/benchmark.sparql
Runs/query: 3
Loaded 8 queries.
Running query 1/8 [simple]...     0.62 ms,  9 rows
Running query 2/8 [simple]...     0.42 ms,  9 rows
Running query 3/8 [simple]...     0.34 ms,  3 rows
Running query 4/8 [simple]...     0.34 ms,  3 rows
Running query 5/8 [multi-join]... 0.39 ms,  3 rows
Running query 6/8 [multi-join]... 0.38 ms,  3 rows
Running query 7/8 [triangle]...   0.47 ms,  1 rows
Running query 8/8 [triangle]...   0.47 ms,  3 rows

========== BENCHMARK REPORT ==========
Class       Latency(ms)   Rows    Status
------------------------------------------
simple      0.62          9       OK
simple      0.42          9       OK
simple      0.34          3       OK
simple      0.34          3       OK
multi-join  0.39          3       OK
multi-join  0.38          3       OK
triangle    0.47          1       OK
triangle    0.47          3       OK
======================================
CSV written to results.csv
```
## Why I built this

Tentris is built around two core innovations: the **hypertrie** index structure and **Worst-Case Optimal Joins (WCOJs)**.

Traditional RDF databases use binary joins — they combine two triple patterns at a time, materializing intermediate results in memory. For graph queries with circular variable dependencies (triangle patterns), this becomes asymptotically expensive. A query like:

```sparql
SELECT * WHERE {
  ?r1 <:knows> ?r2 .
  ?r1 <:knows> ?r3 .
  ?r2 <:knows> ?r3 .
}
```

...forms a triangle: r1-r2, r1-r3, r2-r3. Binary join systems struggle here because intermediate results can be enormous before the final answer is produced. WCOJs, as proven by Ngo et al. and implemented in Tentris, evaluate all variables simultaneously and are asymptotically optimal — bounded by the actual result size, not intermediate blowup.

This tool classifies queries into three categories:

- **simple** — single triple pattern, no joins
- **multi-join** — two triple patterns sharing a variable
- **triangle** — three or more triple patterns with circular variable dependencies (the WCOJ sweet spot)

By measuring latency across these classes, the tool surfaces exactly the performance characteristics that make Tentris architecturally interesting.

## Technical design

Written in C++17 with three components:

**http_client.hpp** — sends SPARQL queries to the /sparql GET endpoint using libcurl, URL-encodes the query string, parses the JSON response with nlohmann/json, and measures wall-clock latency using std::chrono::high_resolution_clock. Each query runs N times and the average latency is reported.

**query_analyzer.hpp** — classifies each query by counting triple patterns and unique variable names. Queries with 3+ triple patterns and 3+ shared variables are flagged as triangle queries — the exact pattern where WCOJs outperform binary joins asymptotically.

**reporter.hpp** — formats results into an ASCII table and writes a CSV file for further analysis.

## Build

```bash
git clone https://github.com/YOUR_USERNAME/tentris-bench.git
cd tentris-bench
mkdir build && cd build
cmake ..
make
```

## Run

```bash
# Start Tentris first (research version)
./tentris_loader -s ./data -f your_data.nt --logstdout
./tentris_server -s ./data --logstdout

# Run the benchmark
./tentris-bench http://localhost:9080/sparql ../queries/benchmark.sparql
```

## Query file format

Separate queries with blank lines:

```sparql
SELECT * WHERE { ?s ?p ?o } LIMIT 10

SELECT * WHERE {
  ?r1 <:p> ?r2 .
  ?r1 <:p> ?r3 .
  ?r2 <:p> ?r3 .
}
```

## Requirements

- Linux (Ubuntu 22.04+)
- g++ with C++17 support
- cmake 3.14+
- libcurl (`sudo apt install libcurl4-openssl-dev`)
- A running Tentris instance

## References

- Bigerl et al. [Tentris - A Tensor-Based Triple Store](https://link.springer.com/chapter/10.1007/978-3-030-62419-4_4) — ISWC 2020
- Bigerl et al. [Hashing the Hypertrie: Space- and Time-Efficient Indexing for SPARQL in Tensors](https://link.springer.com/chapter/10.1007/978-3-031-19433-7_4) — ISWC 2022
- Bigerl et al. Efficient Updates for Worst-Case Optimal Join Triple Stores — ISWC 2025
- Ngo et al. Skew strikes back: New developments in the theory of join algorithms — PODS 2014

## Author

Aziz — Embedded Systems Engineer exploring graph database internals.
