# M2 – Scientific Contextualization and Tradeoff Analysis

**Student:** Nnaemeka Nnachi-Egwu  
**Programme:** Electronic Engineering, Hochschule Hamm-Lippstadt  
**Seminar:** HW/SW Co-Design (Prof. Dr. Achim Rettberg)  
**Main ref:** Kopuri & Mansouri, IEEE ISCAS 2004

> Rough notes. Not publication prose.

---

## 1. Where This Paper Sits

Three broad strategies for HLS scheduling:

1. **Exact methods** (ILP, SMT) — global optimum guaranteed, exponential worst-case, impractical above ~40 ops
2. **Constructive heuristics** (List scheduling, FDS) — polynomial, single pass, near-optimal
3. **Iterative meta-heuristics** (GA, SA, ACO, Tabu) — stochastic, population-based, iterate toward better solutions

This paper = intersection of (2) and (3): augments FDS with ACO iteration.

Related work is **sparse at the exact intersection** (ACO applied specifically to HLS scheduling) but **rich in adjacent areas** (ACO for combinatorial optimisation, GA for HLS, FDS variants).

---

## 2. Scheduling Algorithm Comparison

| Method | Strategy | Optimality | Complexity | Main weakness |
|---|---|---|---|---|
| ILP | Exact integer program | Global optimum | Exponential | Doesn't scale; infeasible N > ~40 |
| List Scheduling | Priority queue, greedy | Local heuristic | O(N log N) | Quality = quality of priority function |
| FDS [Paulin 1987] | Force minimisation, single pass | Near-optimal heuristic | O(N³) | PS-force expensive; no feedback |
| Graph Bisection [Park 1991] | Kernighan-Lin partition, iterative | Heuristic | O(N² log N) | Limited generality |
| GA / Simulated Evolution [Heijligers 1995] | Population crossover/mutation | Stochastic near-optimal | Population-dependent | High param count; slow convergence |
| InSyn [Sharma 1994] | Integrated sched for DSP | DSP-specific heuristic | Polynomial | Narrow domain applicability |
| Dynamic Ants [Keinprasit 2003] | Decision-path graph, niche sharing | Stochastic | Polynomial | Doesn't close full synthesis loop |
| **A_FDS (this paper)** | FDS self-force + ACO trail/global-exp | Stochastic, iterative | O(Nλ) per step | Sign inconsistency in Eq. 3; param sensitivity |

**Key differentiator of A_FDS:** Only method that (a) uses post-synthesis cost as the reinforcement signal AND (b) stays polynomial per iteration.

---

## 3. ACO Variant Comparison

| Variant | Key feature | Relation to this paper |
|---|---|---|
| Ant System (AS) [Dorigo 1992] | Original; global pheromone update after each cycle | Conceptual basis; paper follows stigmergy + evaporation idea |
| Ant Colony System (ACS) | Local + global update; best-ant reinforcement | Not used here |
| **Ant_sched (this paper)** | Per-ant trail M^n; global matrix G; FDS self-force replaces transition probability | Custom for HLS scheduling assignment problem |

**Critical structural difference from standard ACO:** Classical ACO places pheromone on *graph edges* (path choices → TSP, VRP). Here, pheromone is on `(operation, time-step)` pairs — an **assignment problem** structure. Less natural fit, but the paper makes it work.

---

## 4. Tradeoff Analysis

### 4.1 Optimality vs Runtime

- FDS: O(N³) single pass, no global-optimum guarantee
- A_FDS: n_iter × n_ants complete synthesis calls = 400 total evaluations for benchmark params
- ILP: global optimum but exponential → impractical
- **Tradeoff:** A_FDS accepts ~400× more synthesis calls for an iteratively improved schedule. Still runs in seconds on DSP benchmarks because N is small (≤ 50 ops, λ ≤ 20).

### 4.2 Cost Model Precision vs Fidelity

- Model: multiplier=250, adder=50, register=15, mux=15 (count-based, Table 3)
- DSM reality: wire delay often dominates gate delay → placement-aware cost needed
- Paper itself notes: "in the DSM realm the cost of resources might not necessarily be dominant"
- **Tradeoff:** Simple cost = fast evaluation (seconds). Realistic cost = slower, requires placement info.

### 4.3 Memory vs Accuracy

- Two matrices M^n and G: each O(Nλ)
- For N=50, λ=20: 1000 doubles each → negligible
- ρ (evaporation): too low = slow learning; too high = no forgetting → premature convergence
- **Tradeoff:** ρ controls the memory horizon of the algorithm.

### 4.4 Offline vs Online

- A_FDS is entirely **offline** (compile-time). All 400 iterations before any schedule is deployed.
- Cannot adapt to runtime changes (dynamic reconfiguration, run-time resource constraints).
- Appropriate for static HLS. Irrelevant to dynamic partial reconfiguration.

### 4.5 Formal Guarantees vs Practical Deployability

- No convergence proof given or cited for this specific ACO variant
- ACO convergence to optimal proven for some AS variants in the infinite-iteration limit [Dorigo 1992] — not the same setup
- Parameters (α, β, γ, ρ, η) are empirical — must be tuned per circuit class
- **Practical:** runs in seconds, integrates into existing HLS flow, no change to downstream tools needed

---

## 5. Embedded Systems Constraints Perspective

| Constraint | How A_FDS addresses it |
|---|---|
| **Area** | Primary objective — cost model directly minimises unit count |
| **Timing / Clock period** | λ imposed as hard constraint, but critical-path length not modelled |
| **Power** | Not addressed — switching activity ignored |
| **Determinism** | RTL output is deterministic; scheduler is stochastic but offline |
| **Resource constraints** | Fixed-λ model is correct for area-constrained HLS |
| **DSP applications** | Benchmarks (Elliptical Filter, DCT) confirm good match |
| **Memory-bandwidth-bound** | Not modelled — DRAM latency absent from cost function |

---

## 6. Why Related Work is Sparse at This Intersection

- ACO applied to **full** HLS (scheduling + alloc + bind + register) is rare as of 2004
- Most ACO work addresses graph traversal (TSP, VRP, graph colouring) — pheromone on edges is natural there
- HLS scheduling is an **assignment problem** (N ops × λ time-steps) — the ACO formulation is less obvious
- The one direct predecessor [Keinprasit 2003] uses "dynamic ants" but does not close the full synthesis loop; cost model is unclear
- **Conclusion:** Paper is genuinely novel at the intersection (ACO + closed-loop HLS synthesis) but builds entirely on established components (FDS, clique partitioning, Left-Edge)

---

## 7. Justification: Included vs Excluded References

**Included / directly relevant:**
- Paulin & Knight 1987 — FDS baseline, paper modifies this directly
- Dorigo 1992 — ACO conceptual foundation
- Keinprasit & Chongstitvatana 2003 — closest prior ACO-in-HLS work
- Heijligers et al. 1995 — GA for HLS (nearest meta-heuristic comparison)
- Park & Kyung 1991 — iterative HLS scheduling (cited in paper)

**Excluded / less relevant:**
- Simulated annealing for HLS — same category but not cited or compared by authors
- Tabu search — neighbouring paradigm, not discussed
- Power-optimised HLS — extends cost model but outside paper scope

---

## References

- S. Kopuri and N. Mansouri, *IEEE ISCAS 2004*, pp. V-257–V-260
- P. G. Paulin and J. P. Knight, *DAC 1987*, pp. 195–202
- M. Dorigo, Ph.D. thesis, Politecnico di Milano, 1992
- I. C. Park and C. M. Kyung, *DAC 1991*, pp. 650–685
- M. G. M. Heijligers et al., *ASP-DAC 1995*, pp. 61–66
- A. Sharma and R. Jain, *Int. Symp. High-Level Synthesis 1994*, pp. 96–103
- R. Keinprasit and P. Chongstitvatana, *Int. J. Intell. Syst.*, 2003
