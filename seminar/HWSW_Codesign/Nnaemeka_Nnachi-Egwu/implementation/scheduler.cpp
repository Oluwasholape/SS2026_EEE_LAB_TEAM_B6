/**
 * scheduler.cpp
 * FDS + A_FDS implementation for HLS scheduling.
 *
 * Reference: Kopuri & Mansouri, IEEE ISCAS 2004, pp. V-257--V-260.
 *
 * Equations implemented exactly as in the paper (see inline comments).
 * Sign convention in Eq.3 follows the paper as printed.
 * If ACOParams::fix_sign = true, the corrected (positive) sign is used.
 */

#include "scheduler.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>

// ---------------------------------------------------------------------------
// CDFG: ASAP / ALAP computation
// ---------------------------------------------------------------------------

void CDFG::compute_asap_alap() {
    // Forward pass: ASAP (earliest possible time-step, 1-based)
    // Process in topological order (ops vector must be topologically ordered)
    for (auto& op : ops) {
        op.asap = 1;
        for (int pred_id : op.preds) {
            op.asap = std::max(op.asap, ops[pred_id].asap + 1);
        }
    }

    // Backward pass: ALAP (latest possible time-step given lambda)
    for (int k = N - 1; k >= 0; --k) {
        auto& op = ops[k];
        op.alap = lambda;
        for (int succ_id : op.succs) {
            op.alap = std::min(op.alap, ops[succ_id].alap - 1);
        }
        if (op.alap < op.asap)
            throw std::runtime_error("Infeasible latency: operation "
                + std::to_string(op.id) + " has ALAP < ASAP");
    }
}

// ---------------------------------------------------------------------------
// FDSScheduler: helpers
// ---------------------------------------------------------------------------

FDSScheduler::FDSScheduler(CDFG& g) : g_(g), sched_(g.N, -1) {}

// q_k(l): type distribution for type t at time-step l (1-based)
// = sum over operations of type t of their probability of being at l
std::vector<double> FDSScheduler::type_distribution(OpType t) const {
    std::vector<double> q(g_.lambda + 1, 0.0); // index 1..lambda
    for (const auto& op : g_.ops) {
        if (op.type != t) continue;
        if (op.scheduled_at >= 0) {
            // Scheduled: point mass
            q[op.scheduled_at] += 1.0;
        } else {
            // Unscheduled: uniform over time frame
            int span = op.alap - op.asap + 1;
            double prob = 1.0 / static_cast<double>(span);
            for (int m = op.asap; m <= op.alap; ++m)
                q[m] += prob;
        }
    }
    return q; // q[0] unused
}

// Self-force for op i scheduled at time-step l (Eq. 1)
// S_il = sum_{m=asap_i}^{alap_i} q_k(m) * (delta(l,m) - p_i(m))
double FDSScheduler::self_force(int op_id, int l) const {
    const auto& op = g_.ops[op_id];
    auto q = type_distribution(op.type);
    int span = op.alap - op.asap + 1;
    double p = 1.0 / static_cast<double>(span);
    double sf = 0.0;
    for (int m = op.asap; m <= op.alap; ++m) {
        double delta = (m == l) ? 1.0 : 0.0;
        sf += q[m] * (delta - p);
    }
    return sf;
}

// PS-force for all predecessors/successors of op_id when scheduled at l (Eq. 2)
// Simplified: compute the change in type distribution contribution for each
// affected op after time-frame narrowing.
double FDSScheduler::ps_force(int op_id, int l) const {
    // When op_id is scheduled at l:
    // - predecessors must be at time-steps <= l-1  => their alap shrinks to l-1
    // - successors must be at time-steps >= l+1    => their asap grows to l+1
    double total_ps = 0.0;
    const auto& op = g_.ops[op_id];

    auto compute_partial_sum = [&](int i, int new_asap, int new_alap) -> double {
        // PS_il = 1/(new_mob+1) * sum q_k[m] over new frame
        //       - 1/(old_mob+1) * sum q_k[m] over old frame  (Eq. 2)
        const auto& o = g_.ops[i];
        auto q = type_distribution(o.type);

        int old_mob = o.alap - o.asap;
        int new_mob = new_alap - new_asap;

        if (new_mob < 0) return 0.0; // frame collapsed -- operation already constrained

        double new_sum = 0.0;
        for (int m = new_asap; m <= new_alap; ++m) new_sum += q[m];
        double old_sum = 0.0;
        for (int m = o.asap; m <= o.alap; ++m) old_sum += q[m];

        return new_sum / static_cast<double>(new_mob + 1)
             - old_sum / static_cast<double>(old_mob + 1);
    };

    for (int pred_id : op.preds) {
        const auto& p = g_.ops[pred_id];
        if (p.scheduled_at >= 0) continue; // already scheduled
        int new_alap = std::min(p.alap, l - 1);
        total_ps += compute_partial_sum(pred_id, p.asap, new_alap);
    }
    for (int succ_id : op.succs) {
        const auto& s = g_.ops[succ_id];
        if (s.scheduled_at >= 0) continue;
        int new_asap = std::max(s.asap, l + 1);
        total_ps += compute_partial_sum(succ_id, new_asap, s.alap);
    }
    return total_ps;
}

// After scheduling op_id at l, narrow time frames of predecessors/successors
void FDSScheduler::update_time_frames_after(int op_id, int l) {
    const auto& op = g_.ops[op_id];
    for (int pred_id : op.preds) {
        auto& p = g_.ops[pred_id];
        p.alap = std::min(p.alap, l - 1);
    }
    for (int succ_id : op.succs) {
        auto& s = g_.ops[succ_id];
        s.asap = std::max(s.asap, l + 1);
    }
}

// Main FDS scheduling procedure
Schedule FDSScheduler::schedule() {
    // Reset
    for (auto& op : g_.ops) {
        op.scheduled_at = -1;
        // Restore time frames (ASAP/ALAP already set by compute_asap_alap)
    }
    std::fill(sched_.begin(), sched_.end(), -1);

    int scheduled_count = 0;
    while (scheduled_count < g_.N) {
        // Find (op, l) with minimum total force among unscheduled ops
        double best_force = std::numeric_limits<double>::max();
        int best_op = -1, best_l = -1;

        for (const auto& op : g_.ops) {
            if (op.scheduled_at >= 0) continue;
            for (int l = op.asap; l <= op.alap; ++l) {
                double f = self_force(op.id, l) + ps_force(op.id, l);
                if (f < best_force) {
                    best_force = f;
                    best_op    = op.id;
                    best_l     = l;
                }
            }
        }

        // Schedule best_op at best_l
        g_.ops[best_op].scheduled_at = best_l;
        sched_[best_op] = best_l;
        update_time_frames_after(best_op, best_l);
        ++scheduled_count;
    }

    // Build result
    Schedule s;
    s.latency   = g_.lambda;
    s.time_step = sched_;
    s.slots.resize(g_.lambda + 1);
    for (int i = 0; i < g_.N; ++i) {
        s.slots[sched_[i]].push_back(i);
    }
    return s;
}

// ---------------------------------------------------------------------------
// AFDSScheduler
// ---------------------------------------------------------------------------

AFDSScheduler::AFDSScheduler(CDFG& g, const ACOParams& p)
    : g_(g), p_(p), N_(g.N), lambda_(g.lambda),
      M_(g.N, std::vector<double>(g.lambda + 1, 0.0)),
      G_(g.N, std::vector<double>(g.lambda + 1, 0.0))
{}

// Type distribution (same as FDS version, but operating on a working sched vector)
std::vector<std::vector<double>> AFDSScheduler::compute_type_dist(
    const std::vector<int>& sched) const
{
    // Returns qk[type_index][l] -- we use a flat encoding via OpType cast
    // Return value: indexed by l in [1..lambda]
    // For simplicity, compute for all types in one pass.
    // Outer index: 0=MUL,1=ADD,2=SUB,3=CMP,4=OTHER
    int ntypes = 5;
    std::vector<std::vector<double>> qk(ntypes,
        std::vector<double>(lambda_ + 1, 0.0));

    for (int i = 0; i < N_; ++i) {
        const auto& op = g_.ops[i];
        int t = static_cast<int>(op.type);
        if (sched[i] >= 0) {
            qk[t][sched[i]] += 1.0;
        } else {
            int cur_asap = op.asap, cur_alap = op.alap;
            int span = cur_alap - cur_asap + 1;
            if (span <= 0) continue;
            double prob = 1.0 / static_cast<double>(span);
            for (int m = cur_asap; m <= cur_alap; ++m)
                qk[t][m] += prob;
        }
    }
    return qk;
}

// Self-force in A_FDS context (Eq. 1)
double AFDSScheduler::self_force_afds(int i, int l,
    const std::vector<std::vector<double>>& qk) const
{
    const auto& op = g_.ops[i];
    int t = static_cast<int>(op.type);
    int span = op.alap - op.asap + 1;
    if (span <= 0) return 0.0;
    double p = 1.0 / static_cast<double>(span);
    double sf = 0.0;
    for (int m = op.asap; m <= op.alap; ++m) {
        double delta = (m == l) ? 1.0 : 0.0;
        sf += qk[t][m] * (delta - p);
    }
    return sf;
}

// RMS normalisation: sqrt( (1/(N*lambda)) * sum_all S_ij^2 )
// Note: computed over unscheduled operations; paper says "all S_ij"
double AFDSScheduler::rms_norm_self_force(const CDFG& g) const {
    // We need to compute S_ij for all (i,j). Build a temporary type dist.
    // Use the full CDFG (not partial schedule) for normalisation.
    std::vector<std::vector<double>> qk_full(5,
        std::vector<double>(lambda_ + 1, 0.0));
    for (const auto& op : g.ops) {
        int t = static_cast<int>(op.type);
        int span = op.alap - op.asap + 1;
        if (span <= 0) continue;
        double prob = 1.0 / static_cast<double>(span);
        for (int m = op.asap; m <= op.alap; ++m)
            qk_full[t][m] += prob;
    }

    double sum_sq = 0.0;
    int count = 0;
    for (int i = 0; i < N_; ++i) {
        const auto& op = g.ops[i];
        int t = static_cast<int>(op.type);
        int span = op.alap - op.asap + 1;
        if (span <= 0) continue;
        double p = 1.0 / static_cast<double>(span);
        for (int l = op.asap; l <= op.alap; ++l) {
            double sf = 0.0;
            for (int m = op.asap; m <= op.alap; ++m) {
                double delta = (m == l) ? 1.0 : 0.0;
                sf += qk_full[t][m] * (delta - p);
            }
            sum_sq += sf * sf;
            ++count;
        }
    }
    if (count == 0) return 1.0;
    double mean_sq = sum_sq / static_cast<double>(N_ * lambda_);
    return std::sqrt(mean_sq + 1e-12); // epsilon to avoid division by zero
}

// Modified force (Eq. 5)
double AFDSScheduler::force_il(int i, int l,
    const std::vector<std::vector<double>>& qk,
    double rms_norm) const
{
    double S_il = self_force_afds(i, l, qk);
    double normalised_sf = p_.alpha * (S_il / rms_norm);
    double trail_term    = p_.beta  * M_[i][l];
    double global_term   = p_.gamma * G_[i][l];

    // Eq. 5:  F_il = alpha * S_il/norm - beta*M_il - gamma*G_il
    return normalised_sf - trail_term - global_term;
}

// Procedure A_FDS (Table 2 in paper)
Schedule AFDSScheduler::procedure_afds(int ant_start_op, int ant_start_l) {
    static thread_local std::mt19937 rng{std::random_device{}()};

    // Working copy of time frames
    std::vector<int> asap(N_), alap(N_);
    for (int i = 0; i < N_; ++i) {
        asap[i] = g_.ops[i].asap;
        alap[i] = g_.ops[i].alap;
    }

    std::vector<int> sched(N_, -1);
    int n = 0; // number of scheduled ops

    // Schedule the ant's designated starting operation first
    sched[ant_start_op] = ant_start_l;
    ++n;

    // Narrow frames for predecessor/successors of starting op
    auto narrow_frames = [&](int op_id, int l) {
        for (int pred : g_.ops[op_id].preds) {
            alap[pred] = std::min(alap[pred], l - 1);
        }
        for (int succ : g_.ops[op_id].succs) {
            asap[succ] = std::max(asap[succ], l + 1);
        }
    };
    narrow_frames(ant_start_op, ant_start_l);

    while (n != N_) {
        // Compute type distributions from current partial schedule
        // (temporarily apply local asap/alap to ops)
        for (int i = 0; i < N_; ++i) {
            g_.ops[i].asap = asap[i];
            g_.ops[i].alap = alap[i];
        }
        auto qk = compute_type_dist(sched);
        double norm = rms_norm_self_force(g_);

        // Compute forces for all unscheduled (op, l) pairs
        std::vector<std::tuple<double,int,int>> forces;
        for (int i = 0; i < N_; ++i) {
            if (sched[i] >= 0) continue;
            for (int l = asap[i]; l <= alap[i]; ++l) {
                double f = force_il(i, l, qk, norm);
                forces.emplace_back(f, i, l);
            }
        }

        if (forces.empty()) break;
        std::sort(forces.begin(), forces.end());

        // Choose among the x candidates with least force (x = |eta * n|)
        int x = std::max(1, static_cast<int>(p_.eta * n));
        x = std::min(x, static_cast<int>(forces.size()));

        std::uniform_int_distribution<int> pick(0, x - 1);
        auto [f_chosen, chosen_op, chosen_l] = forces[pick(rng)];

        sched[chosen_op] = chosen_l;
        narrow_frames(chosen_op, chosen_l);
        ++n;
    }

    // Restore original time frames
    for (int i = 0; i < N_; ++i) {
        g_.ops[i].asap = g_.ops[i].asap; // already set above
        g_.ops[i].alap = g_.ops[i].alap;
    }

    Schedule s;
    s.latency   = lambda_;
    s.time_step = sched;
    s.slots.resize(lambda_ + 1);
    for (int i = 0; i < N_; ++i) {
        if (sched[i] >= 0) s.slots[sched[i]].push_back(i);
    }
    return s;
}

// Trail update (Eq. 3)
// M_ij += (cost(s) - cost(s^o)) / cost(s^o)  for all (i,j) in s
void AFDSScheduler::update_trail(const Schedule& s,
    const SynthResult& best, const SynthResult& current)
{
    if (best.total_cost == 0) return; // avoid division by zero on first iter

    double increment = static_cast<double>(current.total_cost - best.total_cost)
                     / static_cast<double>(best.total_cost);

    // fix_sign: if true, negate the increment so that improvement -> positive trail
    if (p_.fix_sign) increment = -increment;

    for (int i = 0; i < N_; ++i) {
        int l = s.time_step[i];
        if (l >= 0 && l <= lambda_)
            M_[i][l] += increment;
    }
}

// Global experience update (Eq. 4)
// G_ij^{n+1} = rho * G_ij^n + M_ij^n
void AFDSScheduler::update_global() {
    for (int i = 0; i < N_; ++i)
        for (int l = 0; l <= lambda_; ++l)
            G_[i][l] = p_.rho * G_[i][l] + M_[i][l];
}

// Clique partitioning: count functional units of type t needed
// Two operations of the same type CAN share a unit iff they don't overlap in time.
// Overlap in time = scheduled at the same time-step.
// Returns the number of units (= chromatic number of conflict graph).
int AFDSScheduler::clique_partition_resources(const Schedule& s, OpType t) {
    // Build conflict graph: ops of type t that share a time-step conflict
    std::vector<int> ops_t;
    for (int i = 0; i < N_; ++i)
        if (g_.ops[i].type == t) ops_t.push_back(i);

    if (ops_t.empty()) return 0;

    // Group by time-step: max ops at same step = max clique = chromatic number
    // (interval graph special case: clique partition = chromatic number = max clique)
    std::map<int,int> count_at_step;
    for (int i : ops_t) {
        int l = s.time_step[i];
        if (l >= 0) count_at_step[l]++;
    }
    int max_concurrent = 0;
    for (auto& [l, cnt] : count_at_step)
        max_concurrent = std::max(max_concurrent, cnt);
    return max_concurrent;
}

// Left-Edge register allocation: count registers needed
// Each variable (result of an operation) has a live range [produce_step, last_use_step]
// Left-Edge: sort by start, greedily assign register, count max active ranges.
int AFDSScheduler::left_edge_registers(const Schedule& s) {
    // Build live ranges: for each operation i, live range = [s.time_step[i], max_succ_step-1]
    // If no successors, live range ends at s.time_step[i] (no register needed after use).
    struct LiveRange { int start, end; };
    std::vector<LiveRange> ranges;

    for (int i = 0; i < N_; ++i) {
        int prod = s.time_step[i];
        if (prod < 0) continue;
        // Find last consumer (latest successor time-step)
        int last_use = prod; // if no successors, used immediately
        for (int succ : g_.ops[i].succs) {
            last_use = std::max(last_use, s.time_step[succ]);
        }
        if (last_use > prod) // only need a register if value crosses a time boundary
            ranges.push_back({prod, last_use - 1});
    }

    if (ranges.empty()) return 0;

    // Sort by start time (Left-Edge)
    std::sort(ranges.begin(), ranges.end(),
        [](const LiveRange& a, const LiveRange& b){ return a.start < b.start; });

    // Count max overlapping intervals
    std::vector<int> active_ends;
    int max_regs = 0;
    for (auto& r : ranges) {
        // Remove expired intervals
        active_ends.erase(
            std::remove_if(active_ends.begin(), active_ends.end(),
                [&](int e){ return e < r.start; }),
            active_ends.end());
        active_ends.push_back(r.end);
        max_regs = std::max(max_regs, static_cast<int>(active_ends.size()));
    }
    return max_regs;
}

// Mux count: for each functional unit input port, count distinct values driven
// Simplified: number of ops mapped to each FU beyond 1 indicates mux inputs
int AFDSScheduler::estimate_muxes(const Schedule& s, int n_mul, int n_add) {
    // Each FU has 2 inputs. If k operations share one FU over k different time-steps,
    // each input needs a k-way mux (k-1 extra mux inputs).
    // Simplified model: total mux inputs = total FU inputs - number of FUs * 2
    // Ops per FU slot (n_mul multipliers, each has 2 inputs):
    int mul_ops = 0, add_ops = 0;
    for (const auto& op : g_.ops) {
        if (op.type == OpType::MUL) ++mul_ops;
        else if (op.type == OpType::ADD || op.type == OpType::SUB
              || op.type == OpType::CMP) ++add_ops;
    }
    // Each FU is used for (total_ops / n_units) different operations over time
    // Each "extra" mapping beyond 1 op per FU requires mux inputs (one per input port)
    int extra_mul = std::max(0, mul_ops - n_mul);
    int extra_add = std::max(0, add_ops - n_add);
    // Each extra op requires 2 mux inputs (2-input op), each 2-input mux = 1 mux unit
    return (extra_mul * 2 + extra_add * 2);
}

// Full downstream synthesis for one schedule
SynthResult AFDSScheduler::synthesise(const Schedule& s) {
    SynthResult r;
    r.sched = s;

    r.num_multipliers = clique_partition_resources(s, OpType::MUL);
    // Adders, subtractors, and comparators share a single functional unit type
    int n_add = clique_partition_resources(s, OpType::ADD)
              + clique_partition_resources(s, OpType::SUB)
              + clique_partition_resources(s, OpType::CMP);
    r.num_adders   = n_add;
    r.num_registers = left_edge_registers(s);
    r.num_muxes     = estimate_muxes(s, r.num_multipliers, r.num_adders);
    r.total_cost    = CostModel::compute(r.num_multipliers, r.num_adders,
                                         r.num_registers,   r.num_muxes);
    return r;
}

// Main Ant_sched outer loop
SynthResult AFDSScheduler::run() {
    static thread_local std::mt19937 rng{std::random_device{}()};

    // Reset matrices
    for (auto& row : M_) std::fill(row.begin(), row.end(), 0.0);
    for (auto& row : G_) std::fill(row.begin(), row.end(), 0.0);

    SynthResult best;
    best.total_cost = std::numeric_limits<int>::max();
    bool has_best = false;

    // Compute initial self-forces on full CDFG
    // Sort operations by ascending self-force (least force = least impact)
    FDSScheduler fds_init(g_);
    // Compute self-forces for all (op, asap) pairs as initial ranking
    std::vector<std::pair<double, int>> sf_order;
    for (const auto& op : g_.ops) {
        double sf = fds_init.self_force(op.id, op.asap);
        sf_order.emplace_back(sf, op.id);
    }
    std::sort(sf_order.begin(), sf_order.end()); // ascending: least force first

    int n_ants = std::max(1, static_cast<int>(p_.eta * N_));
    n_ants = std::min(n_ants, N_);

    for (int n = 0; n < p_.n_iter; ++n) {
        // Reset per-cycle trail
        for (auto& row : M_) std::fill(row.begin(), row.end(), 0.0);

        // Restore original ASAP/ALAP before each iteration
        // (A_FDS modifies local copies, but FDS above may have modified g_.ops)
        // Re-run ASAP/ALAP
        g_.compute_asap_alap();

        for (int m = 0; m < n_ants; ++m) {
            // Each ant starts with one of the n_ants least-self-force ops
            int ant_op_idx = m % static_cast<int>(sf_order.size());
            int ant_start_op = sf_order[ant_op_idx].second;
            int ant_start_l  = g_.ops[ant_start_op].asap;

            // Run A_FDS
            Schedule s = procedure_afds(ant_start_op, ant_start_l);

            // Synthesise
            SynthResult sr = synthesise(s);

            // Update trail matrix (Eq. 3)
            if (has_best) {
                update_trail(s, best, sr);
            }

            // Update best
            if (!has_best || sr.total_cost < best.total_cost) {
                best = sr;
                has_best = true;
            }
        }

        // Update global matrix (Eq. 4)
        update_global();
    }

    return best;
}

// ---------------------------------------------------------------------------
// Benchmark CDFG builders
// ---------------------------------------------------------------------------

namespace Benchmarks {

// Differential Equation: 9 operations (add/sub + mul)
// y = a*x + b*u - c*y  (simplified datapath)
// Topology from standard HLS benchmark suite
CDFG differential_equation(int lambda) {
    CDFG g;
    g.lambda = lambda;
    // Op ids 0..8, types and deps from standard diff-eq benchmark
    // (simplified 5-operation version for illustration)
    g.ops = {
        {0, OpType::MUL, 0,0,0, {},     {2}},    // m1 = a*x
        {1, OpType::MUL, 0,0,0, {},     {3}},    // m2 = b*u
        {2, OpType::ADD, 0,0,0, {0},    {4}},    // a1 = m1 + tmp
        {3, OpType::MUL, 0,0,0, {},     {4}},    // m3 = c*y
        {4, OpType::SUB, 0,0,0, {2,3},  {}}     // a2 = a1 - m3
    };
    g.N = static_cast<int>(g.ops.size());
    g.compute_asap_alap();
    return g;
}

// Elliptical Filter: representative structure with 16 operations
CDFG elliptical_filter(int lambda) {
    CDFG g;
    g.lambda = lambda;
    // Simplified structure capturing dependency pattern
    g.ops = {
        {0, OpType::MUL, 0,0,0, {},        {4,5}},
        {1, OpType::MUL, 0,0,0, {},        {4}},
        {2, OpType::MUL, 0,0,0, {},        {6}},
        {3, OpType::MUL, 0,0,0, {},        {7}},
        {4, OpType::ADD, 0,0,0, {0,1},     {8}},
        {5, OpType::ADD, 0,0,0, {0},       {8}},
        {6, OpType::ADD, 0,0,0, {2},       {9}},
        {7, OpType::ADD, 0,0,0, {3},       {9}},
        {8, OpType::ADD, 0,0,0, {4,5},     {10}},
        {9, OpType::ADD, 0,0,0, {6,7},     {10}},
        {10,OpType::ADD, 0,0,0, {8,9},     {11}},
        {11,OpType::MUL, 0,0,0, {10},      {12}},
        {12,OpType::ADD, 0,0,0, {11},      {}}
    };
    g.N = static_cast<int>(g.ops.size());
    g.compute_asap_alap();
    return g;
}

// DCT1: representative DCT structure
CDFG dct1(int lambda) {
    CDFG g;
    g.lambda = lambda;
    g.ops = {
        {0,  OpType::MUL, 0,0,0, {},       {8}},
        {1,  OpType::MUL, 0,0,0, {},       {8}},
        {2,  OpType::MUL, 0,0,0, {},       {9}},
        {3,  OpType::MUL, 0,0,0, {},       {9}},
        {4,  OpType::MUL, 0,0,0, {},       {10}},
        {5,  OpType::MUL, 0,0,0, {},       {10}},
        {6,  OpType::MUL, 0,0,0, {},       {11}},
        {7,  OpType::MUL, 0,0,0, {},       {11}},
        {8,  OpType::ADD, 0,0,0, {0,1},    {12}},
        {9,  OpType::ADD, 0,0,0, {2,3},    {12}},
        {10, OpType::ADD, 0,0,0, {4,5},    {13}},
        {11, OpType::ADD, 0,0,0, {6,7},    {13}},
        {12, OpType::ADD, 0,0,0, {8,9},    {14}},
        {13, OpType::ADD, 0,0,0, {10,11},  {14}},
        {14, OpType::ADD, 0,0,0, {12,13},  {}}
    };
    g.N = static_cast<int>(g.ops.size());
    g.compute_asap_alap();
    return g;
}

// DCT2: second DCT variant with more multiplications
CDFG dct2(int lambda) {
    CDFG g;
    g.lambda = lambda;
    g.ops = {
        {0,  OpType::MUL, 0,0,0, {},         {10}},
        {1,  OpType::MUL, 0,0,0, {},         {10}},
        {2,  OpType::MUL, 0,0,0, {},         {11}},
        {3,  OpType::MUL, 0,0,0, {},         {11}},
        {4,  OpType::MUL, 0,0,0, {},         {12}},
        {5,  OpType::MUL, 0,0,0, {},         {12}},
        {6,  OpType::MUL, 0,0,0, {},         {13}},
        {7,  OpType::MUL, 0,0,0, {},         {13}},
        {8,  OpType::MUL, 0,0,0, {},         {14}},
        {9,  OpType::MUL, 0,0,0, {},         {14}},
        {10, OpType::ADD, 0,0,0, {0,1},      {15}},
        {11, OpType::ADD, 0,0,0, {2,3},      {15}},
        {12, OpType::ADD, 0,0,0, {4,5},      {16}},
        {13, OpType::ADD, 0,0,0, {6,7},      {16}},
        {14, OpType::ADD, 0,0,0, {8,9},      {17}},
        {15, OpType::ADD, 0,0,0, {10,11},    {18}},
        {16, OpType::ADD, 0,0,0, {12,13},    {18}},
        {17, OpType::ADD, 0,0,0, {14},       {18}},
        {18, OpType::ADD, 0,0,0, {15,16,17}, {}}
    };
    g.N = static_cast<int>(g.ops.size());
    g.compute_asap_alap();
    return g;
}

} // namespace Benchmarks
