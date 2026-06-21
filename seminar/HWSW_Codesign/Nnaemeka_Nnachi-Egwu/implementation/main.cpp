/**
 * main.cpp
 * Driver program for FDS and A_FDS HLS schedulers.
 *
 * Runs all four benchmarks from Kopuri & Mansouri ISCAS 2004 (Table 4)
 * and prints cost comparison.
 *
 * Usage: ./hls_scheduler
 *
 * Author: Nnaemeka Nnachi-Egwu
 * HSHL, Electronic Engineering, HW/SW Co-Design Seminar
 */

#include "scheduler.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>

// -------------------------------------------------------------------------
// Utility: print one result row
// -------------------------------------------------------------------------
static void print_row(const std::string& bench, int latency,
                      int cost_fds, int cost_afds)
{
    double improvement = (cost_fds > 0)
        ? 100.0 * (cost_fds - cost_afds) / static_cast<double>(cost_fds)
        : 0.0;
    std::cout << std::left  << std::setw(28) << bench
              << std::right << std::setw(8)  << latency
              << std::setw(8)  << cost_fds
              << std::setw(8)  << cost_afds
              << std::fixed << std::setprecision(1)
              << std::setw(8)  << improvement << "%\n";
}

// -------------------------------------------------------------------------
// Run one FDS + A_FDS experiment
// -------------------------------------------------------------------------
static void run_experiment(const std::string& name,
                           std::function<CDFG(int)> bench_fn,
                           int latency,
                           const ACOParams& params)
{
    // FDS
    CDFG g_fds = bench_fn(latency);
    FDSScheduler fds(g_fds);
    Schedule s_fds = fds.schedule();

    // Synthesise FDS result
    AFDSScheduler syn_fds(g_fds, params);
    SynthResult r_fds = syn_fds.synthesise(s_fds);

    // A_FDS
    CDFG g_afds = bench_fn(latency);
    AFDSScheduler afds(g_afds, params);
    SynthResult r_afds = afds.run();

    print_row(name, latency, r_fds.total_cost, r_afds.total_cost);
}

int main() {
    ACOParams params;
    params.eta    = 0.5;
    params.alpha  = 1.0;
    params.beta   = 2.0;
    params.gamma  = 2.0;
    params.rho    = 0.40;
    params.n_iter = 20;
    params.n_ants = 20;
    params.fix_sign = false; // as-printed sign convention

    std::cout << "\n=== HLS Scheduler: FDS vs A_FDS ===\n";
    std::cout << "(Reference: Kopuri & Mansouri, IEEE ISCAS 2004, Table 4)\n\n";
    std::cout << std::left  << std::setw(28) << "Benchmark"
              << std::right << std::setw(8)  << "Latency"
              << std::setw(8)  << "FDS"
              << std::setw(8)  << "A_FDS"
              << std::setw(9)  << "Improve\n";
    std::cout << std::string(61, '-') << "\n";

    // Paper Table 4 experiments:
    run_experiment("Differential Equation",
                   Benchmarks::differential_equation, 4, params);
    run_experiment("Differential Equation",
                   Benchmarks::differential_equation, 5, params);
    run_experiment("Elliptical Filter",
                   Benchmarks::elliptical_filter, 10, params);
    run_experiment("Elliptical Filter",
                   Benchmarks::elliptical_filter, 12, params);
    run_experiment("DCT 1",
                   Benchmarks::dct1, 12, params);
    run_experiment("DCT 1",
                   Benchmarks::dct1, 14, params);
    run_experiment("DCT 2",
                   Benchmarks::dct2, 13, params);
    run_experiment("DCT 2",
                   Benchmarks::dct2, 14, params);

    std::cout << std::string(61, '-') << "\n";
    std::cout << "\nNote: exact numerical values may differ from the paper\n";
    std::cout << "due to CDFG approximation and stochastic A_FDS.\n";
    std::cout << "Pattern should match: A_FDS <= FDS in all cases.\n\n";

    // Demonstrate corrected sign convention
    std::cout << "\n=== Corrected sign convention (fix_sign=true) ===\n";
    std::cout << std::left  << std::setw(28) << "Benchmark"
              << std::right << std::setw(8)  << "Latency"
              << std::setw(8)  << "FDS"
              << std::setw(8)  << "A_FDS*"
              << std::setw(9)  << "Improve\n";
    std::cout << std::string(61, '-') << "\n";

    params.fix_sign = true;
    run_experiment("Elliptical Filter",
                   Benchmarks::elliptical_filter, 10, params);
    run_experiment("DCT 1",
                   Benchmarks::dct1, 12, params);
    run_experiment("DCT 2",
                   Benchmarks::dct2, 13, params);
    std::cout << std::string(61, '-') << "\n";
    std::cout << "* A_FDS* uses corrected positive trail increment.\n\n";

    return 0;
}
