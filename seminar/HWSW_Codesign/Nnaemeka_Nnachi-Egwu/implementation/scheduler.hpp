/**
 * scheduler.hpp
 * High-Level Synthesis Scheduler: FDS + A_FDS with ACO
 *
 * Implements:
 *   - Force-Directed Scheduling (FDS) -- Paulin & Knight 1987
 *   - ACO-Enhanced Scheduler (A_FDS) -- Kopuri & Mansouri ISCAS 2004
 *   - Clique-partitioning resource allocation
 *   - Left-Edge register allocation
 *   - Hardware cost model from Table 3 of Kopuri & Mansouri
 *
 * Reference:
 *   S. Kopuri and N. Mansouri, "Enhancing Scheduling Solutions Through
 *   Ant Colony Optimization," IEEE ISCAS 2004, pp. V-257--V-260.
 *
 * Author: Nnaemeka Nnachi-Egwu
 * Institution: Hochschule Hamm-Lippstadt, Electronic Engineering
 * Seminar: HW/SW Co-Design (Prof. Dr. Achim Rettberg)
 */

#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>

// ---------------------------------------------------------------------------
// Operation types (functional unit types)
// ---------------------------------------------------------------------------
enum class OpType { MUL, ADD, SUB, CMP, OTHER };

// ---------------------------------------------------------------------------
// CDFG node: a single operation in the control-data flow graph
// ---------------------------------------------------------------------------
struct Operation {
    int   id;
    OpType type;
    int   asap;          // ASAP time-step (1-based)
    int   alap;          // ALAP time-step (1-based)
    int   scheduled_at;  // -1 if unscheduled
    std::vector<int> preds;  // predecessor op ids (data deps)
    std::vector<int> succs;  // successor op ids
};

// ---------------------------------------------------------------------------
// Schedule result
// ---------------------------------------------------------------------------
struct Schedule {
    int latency;                          // lambda
    std::vector<int> time_step;           // time_step[op_id] -> 1-based slot
    std::vector<std::vector<int>> slots;  // slots[t] -> list of op ids at t
};

// ---------------------------------------------------------------------------
// Synthesis result (post allocation + binding)
// ---------------------------------------------------------------------------
struct SynthResult {
    Schedule sched;
    int num_multipliers;
    int num_adders;
    int num_registers;
    int num_muxes;
    int total_cost;  // using Table 3 cost model
};

// ---------------------------------------------------------------------------
// CDFG: full graph
// ---------------------------------------------------------------------------
struct CDFG {
    int N;                      // total operations
    int lambda;                 // latency constraint
    std::vector<Operation> ops; // indexed 0..N-1 (id matches index)

    // Compute ASAP and ALAP via topological forward/backward passes
    void compute_asap_alap();
};

// ---------------------------------------------------------------------------
// FDS Scheduler
// ---------------------------------------------------------------------------
class FDSScheduler {
public:
    explicit FDSScheduler(CDFG& g);

    Schedule schedule();

    // Exposed for testing / inspection
    std::vector<double> type_distribution(OpType t) const;  // q_k[l], 1-based l
    double self_force(int op_id, int l) const;
    double ps_force(int op_id, int l) const;

protected:
    CDFG& g_;
    std::vector<int> sched_;  // working partial schedule

    void update_time_frames_after(int op_id, int l);
};

// ---------------------------------------------------------------------------
// ACO parameters
// ---------------------------------------------------------------------------
struct ACOParams {
    double eta   = 0.5;  // fraction of ants / candidate fraction
    double alpha = 1.0;  // self-force weight
    double beta  = 2.0;  // per-cycle trail weight
    double gamma = 2.0;  // global trail weight
    double rho   = 0.40; // evaporation factor
    int n_iter   = 20;   // iterations
    int n_ants   = 20;   // ants per iteration (overridden by eta*N if eta set)
    bool fix_sign = false; // if true, use corrected (positive) trail increment
};

// ---------------------------------------------------------------------------
// A_FDS: ACO-enhanced scheduler
// ---------------------------------------------------------------------------
class AFDSScheduler {
public:
    AFDSScheduler(CDFG& g, const ACOParams& p);

    SynthResult run();  // Run full Ant_sched outer loop, return best synthesis
    SynthResult synthesise(const Schedule& s);  // public for FDS result synthesis

    // Exposed for inspection
    const std::vector<std::vector<double>>& trail_matrix() const { return M_; }
    const std::vector<std::vector<double>>& global_matrix() const { return G_; }

private:
    CDFG&       g_;
    ACOParams   p_;
    int         N_, lambda_;

    // M_[i][l] = per-cycle trail; G_[i][l] = global experience (0-based i, l)
    std::vector<std::vector<double>> M_;
    std::vector<std::vector<double>> G_;

    // --- Inner scheduling procedure (Procedure A_FDS, Table 2) ---
    Schedule procedure_afds(int ant_start_op, int ant_start_l);

    // Modified force (Eq. 5)
    double force_il(int i, int l,
                    const std::vector<std::vector<double>>& type_dist,
                    double rms_norm) const;

    // Type distribution over current partial schedule
    std::vector<std::vector<double>> compute_type_dist(
        const std::vector<int>& sched) const;

    // RMS normalisation denominator
    double rms_norm_self_force(const CDFG& g) const;

    // Update trail matrix (Eq. 3)
    void update_trail(const Schedule& s, const SynthResult& best,
                      const SynthResult& current);

    // Update global matrix (Eq. 4)
    void update_global();

    // --- Downstream synthesis ---
    // (synthesise is declared public above)

    // Resource allocation (clique partitioning)
    int clique_partition_resources(const Schedule& s, OpType t);

    // Register allocation (Left-Edge)
    int left_edge_registers(const Schedule& s);

    // Mux count estimation
    int estimate_muxes(const Schedule& s,
                       int n_mul, int n_add);

    // Self-force (Eq. 1) -- recomputed in A_FDS context
    double self_force_afds(int i, int l,
                           const std::vector<std::vector<double>>& qk) const;
};

// ---------------------------------------------------------------------------
// Cost model (Table 3 of Kopuri & Mansouri 2004)
// ---------------------------------------------------------------------------
namespace CostModel {
    constexpr int MULTIPLIER = 250;
    constexpr int ADDER      = 50;
    constexpr int REGISTER   = 15;
    constexpr int MUX        = 15;

    inline int compute(int n_mul, int n_add, int n_reg, int n_mux) {
        return n_mul * MULTIPLIER + n_add * ADDER
             + n_reg * REGISTER   + n_mux * MUX;
    }
}

// ---------------------------------------------------------------------------
// Benchmark CDFG builders (from standard HLS literature)
// ---------------------------------------------------------------------------
namespace Benchmarks {
    CDFG differential_equation(int lambda);
    CDFG elliptical_filter(int lambda);
    CDFG dct1(int lambda);
    CDFG dct2(int lambda);
}
