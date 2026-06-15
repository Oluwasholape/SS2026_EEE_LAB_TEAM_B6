#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <ctime>

struct Job {
    int id;
    int r;
    int d;
    int p;
    std::vector<int> pred;
    std::vector<int> succ;
};

struct ScheduledJob {
    int job_id;
    int start;
    int finish;
};

struct Processor {
    std::vector<ScheduledJob> jobs;
};

struct Solution {
    std::vector<Processor> processors;
    int m() const { return (int)processors.size(); }
};

struct TabuList {
    std::vector<int> tenure;
    int n;
    explicit TabuList(int n) : n(n), tenure(n, 0) {}
    void make_tabu(int job_id, int tau) { tenure[job_id] = tau; }
    bool is_tabu(int job_id) const { return tenure[job_id] > 0; }
    void step() { for (int i = 0; i < n; i++) if (tenure[i] > 0) tenure[i]--; }
    void reset() { std::fill(tenure.begin(), tenure.end(), 0); }
};

int tabu_tenure(int n, int m) {
    return std::max(3, (n + m - 1) / m);
}

void forward_pass(std::vector<Job>& jobs) {
    int n = (int)jobs.size();
    std::vector<int> est(n);
    for (int i = 0; i < n; i++) {
        est[i] = jobs[i].r;
        for (int pred_id : jobs[i].pred)
            est[i] = std::max(est[i], est[pred_id] + jobs[pred_id].p);
        if (est[i] > jobs[i].r)
            jobs[i].r = est[i];
    }
}

void backward_pass(std::vector<Job>& jobs) {
    int n = (int)jobs.size();
    std::vector<int> lct(n);
    for (int i = 0; i < n; i++) lct[i] = jobs[i].d;
    for (int i = n - 1; i >= 0; i--) {
        for (int succ_id : jobs[i].succ)
            lct[i] = std::min(lct[i], lct[succ_id] - jobs[i].p);
        if (lct[i] < jobs[i].d)
            jobs[i].d = lct[i];
    }
}

bool preprocess(std::vector<Job>& jobs) {
    forward_pass(jobs);
    backward_pass(jobs);
    for (const auto& j : jobs)
        if (j.r > j.d - j.p) return false;
    return true;
}

int earliest_start(const Job& job, const Processor& proc, const std::vector<int>& est_global) {
    int t = std::max(job.r, est_global[job.id]);
    if (!proc.jobs.empty())
        t = std::max(t, proc.jobs.back().finish);
    return t;
}

Solution generate_initial_solution(const std::vector<Job>& jobs, const std::vector<int>& est_global) {
    Solution sol;
    sol.processors.push_back(Processor());
    for (const auto& job : jobs) {
        int best_proc = -1;
        int best_start = INT_MAX;
        int min_idle = INT_MAX;
        for (int k = 0; k < sol.m(); k++) {
            int t = earliest_start(job, sol.processors[k], est_global);
            if (t + job.p > job.d) continue;
            int idle = t - (sol.processors[k].jobs.empty() ? 0 : sol.processors[k].jobs.back().finish);
            if (idle < min_idle) {
                min_idle = idle;
                best_proc = k;
                best_start = t;
            }
        }
        if (best_proc == -1) {
            Processor np;
            int t = std::max(job.r, est_global[job.id]);
            np.jobs.push_back({job.id, t, t + job.p});
            sol.processors.push_back(np);
        } else {
            sol.processors[best_proc].jobs.push_back({job.id, best_start, best_start + job.p});
        }
    }
    return sol;
}

std::vector<int> perturb_method1(Solution& sol) {
    std::vector<int> unallocated;
    int remove_count = (sol.m() >= 7) ? 2 : 1;
    remove_count = std::min(remove_count, sol.m() - 1);
    for (int r = 0; r < remove_count; r++) {
        if (sol.processors.empty()) break;
        int idx = rand() % sol.m();
        for (const auto& sj : sol.processors[idx].jobs)
            unallocated.push_back(sj.job_id);
        sol.processors.erase(sol.processors.begin() + idx);
    }
    return unallocated;
}

std::vector<int> perturb_method2(Solution& sol, double p_rem = 0.2) {
    std::vector<int> unallocated;
    for (auto& proc : sol.processors) {
        std::vector<ScheduledJob> remaining;
        for (const auto& sj : proc.jobs) {
            if ((double)rand() / RAND_MAX < p_rem)
                unallocated.push_back(sj.job_id);
            else
                remaining.push_back(sj);
        }
        proc.jobs = remaining;
    }
    return unallocated;
}

bool try_insert(const Job& job, Solution& sol, const std::vector<Job>& jobs, const std::vector<int>& est_global) {
    int best_proc = -1;
    int best_pos = -1;
    int best_start = -1;
    int min_idle = INT_MAX;
    for (int k = 0; k < sol.m(); k++) {
        Processor& proc = sol.processors[k];
        for (int pos = 0; pos <= (int)proc.jobs.size(); pos++) {
            int t = (pos == 0)
                ? std::max(job.r, est_global[job.id])
                : std::max({job.r, est_global[job.id], proc.jobs[pos-1].finish});
            if (t + job.p > job.d) continue;
            if (pos < (int)proc.jobs.size()) {
                int next_start = std::max(proc.jobs[pos].start, t + job.p);
                if (next_start + jobs[proc.jobs[pos].job_id].p > jobs[proc.jobs[pos].job_id].d)
                    continue;
            }
            int idle = t - (pos == 0 ? 0 : proc.jobs[pos-1].finish);
            if (idle < min_idle) {
                min_idle = idle;
                best_proc = k;
                best_pos = pos;
                best_start = t;
            }
        }
    }
    if (best_proc == -1) return false;
    ScheduledJob sj = {job.id, best_start, best_start + job.p};
    sol.processors[best_proc].jobs.insert(sol.processors[best_proc].jobs.begin() + best_pos, sj);
    for (int i = best_pos + 1; i < (int)sol.processors[best_proc].jobs.size(); i++) {
        auto& cur = sol.processors[best_proc].jobs[i];
        int new_start = std::max(cur.start, sol.processors[best_proc].jobs[i-1].finish);
        cur.start = new_start;
        cur.finish = new_start + jobs[cur.job_id].p;
    }
    return true;
}

bool try_exchange(const Job& job, Solution& sol, const std::vector<Job>& jobs,
                  std::vector<int>& unallocated, const std::vector<int>& est_global) {
    int best_proc = -1;
    int best_start = -1;
    int min_idle = INT_MAX;
    std::vector<int> best_exchanged;
    for (int k = 0; k < sol.m(); k++) {
        Processor& proc = sol.processors[k];
        for (int i = 0; i < (int)proc.jobs.size(); i++) {
            const auto& sj = proc.jobs[i];
            if (sj.finish <= job.r || sj.start >= job.d - job.p) continue;
            if (jobs[sj.job_id].p >= job.p) continue;
            int t = std::max(job.r, est_global[job.id]);
            if (i > 0) t = std::max(t, proc.jobs[i-1].finish);
            if (t + job.p > job.d) continue;
            int idle = t - (i == 0 ? 0 : proc.jobs[i-1].finish);
            if (idle < min_idle) {
                min_idle = idle;
                best_proc = k;
                best_start = t;
                best_exchanged = {sj.job_id};
            }
        }
    }
    if (best_proc == -1) return false;
    Processor& proc = sol.processors[best_proc];
    std::vector<ScheduledJob> new_jobs;
    for (const auto& sj : proc.jobs) {
        bool exchanged = false;
        for (int eid : best_exchanged)
            if (sj.job_id == eid) { exchanged = true; break; }
        if (exchanged)
            unallocated.push_back(sj.job_id);
        else
            new_jobs.push_back(sj);
    }
    ScheduledJob sj_new = {job.id, best_start, best_start + job.p};
    auto it = std::lower_bound(new_jobs.begin(), new_jobs.end(), sj_new,
        [](const ScheduledJob& a, const ScheduledJob& b){ return a.start < b.start; });
    new_jobs.insert(it, sj_new);
    proc.jobs = new_jobs;
    return true;
}

Solution tspred(const std::vector<Job>& jobs_in, int max_no_improve = 7500, int max_iter = 50000) {
    std::vector<Job> jobs = jobs_in;
    int n = (int)jobs.size();
    for (auto& j : jobs) j.succ.clear();
    for (const auto& j : jobs)
        for (int pred_id : j.pred)
            jobs[pred_id].succ.push_back(j.id);
    if (!preprocess(jobs)) {
        std::cerr << "Infeasible instance.\n";
        return Solution();
    }
    std::vector<int> est_global(n);
    for (int i = 0; i < n; i++) est_global[i] = jobs[i].r;
    Solution best = generate_initial_solution(jobs, est_global);
    Solution current = best;
    std::cout << "Initial solution: " << best.m() << " processors\n";
    int no_improve = 0;
    int iter = 0;
    while (no_improve < max_no_improve && iter < max_iter) {
        iter++;
        Solution perturbed = current;
        std::vector<int> unallocated;
        bool use_method1 = (current.m() >= 25) || ((double)rand() / RAND_MAX < 0.95);
        if (use_method1)
            unallocated = perturb_method1(perturbed);
        else
            unallocated = perturb_method2(perturbed);
        std::random_shuffle(unallocated.begin(), unallocated.end());
        TabuList tabu(n);
        int tau = tabu_tenure(n, current.m());
        for (int job_id : unallocated) {
            const Job& job = jobs[job_id];
            if (tabu.is_tabu(job_id)) {
                Processor np;
                int t = job.r;
                np.jobs.push_back({job_id, t, t + job.p});
                perturbed.processors.push_back(np);
                continue;
            }
            if (try_insert(job, perturbed, jobs, est_global))
                tabu.make_tabu(job_id, tau);
            else if (try_exchange(job, perturbed, jobs, unallocated, est_global))
                tabu.make_tabu(job_id, tau);
            else {
                Processor np;
                int t = job.r;
                np.jobs.push_back({job_id, t, t + job.p});
                perturbed.processors.push_back(np);
            }
            tabu.step();
        }
        if (perturbed.m() <= current.m()) {
            current = perturbed;
            if (current.m() < best.m()) {
                best = current;
                no_improve = 0;
                std::cout << "Improved: " << best.m() << " processors (iter " << iter << ")\n";
            } else {
                no_improve++;
            }
        } else {
            no_improve++;
        }
    }
    std::cout << "TS-PRED finished after " << iter << " iterations.\n";
    std::cout << "Best solution: " << best.m() << " processors\n";
    return best;
}

void print_solution(const Solution& sol, const std::vector<Job>& jobs) {
    std::cout << "\n=== Schedule (" << sol.m() << " processors) ===\n";
    for (int k = 0; k < sol.m(); k++) {
        std::cout << "Processor " << k << ": ";
        for (const auto& sj : sol.processors[k].jobs)
            std::cout << "[J" << sj.job_id << " s=" << sj.start << " f=" << sj.finish << "] ";
        std::cout << "\n";
    }
}

int main() {
    srand((unsigned)time(nullptr));
    std::vector<Job> jobs = {
        {0, 0, 4, 2, {},    {}},
        {1, 0, 4, 2, {},    {}},
        {2, 0, 6, 2, {0,1}, {}},
        {3, 2, 8, 2, {2},   {}}
    };
    std::cout << "=== TS-PRED Application Example ===\n";
    std::cout << "4 HLS operations, precedence: J0->J2, J1->J2, J2->J3\n\n";
    Solution sol = tspred(jobs, 7500, 50000);
    print_solution(sol, jobs);
    return 0;
}
