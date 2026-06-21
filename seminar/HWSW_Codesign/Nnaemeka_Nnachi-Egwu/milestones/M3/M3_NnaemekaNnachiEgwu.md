# M3 – Critical Evaluation and Transfer

**Student:** Nnaemeka Nnachi-Egwu  
**Programme:** Electronic Engineering, Hochschule Hamm-Lippstadt  
**Seminar:** HW/SW Co-Design (Prof. Dr. Achim Rettberg)  
**Main ref:** Kopuri & Mansouri, IEEE ISCAS 2004

> Rough notes. Focus: what's strong, what's weak, what the math actually implies, and where this breaks down.

---

## 1. Strengths (Concrete)

- **Closed synthesis loop** — cost is evaluated *after* full synthesis (allocate + bind + register), not a surrogate. Unlike Keinprasit 2003. This is the real contribution.
- **Linear per-step complexity** — Eq. 5 is O(Nλ) vs O(N³) PS-force in FDS. Genuine speed-per-step improvement even though total evaluations (400) go up.
- **Parameter-driven cost model** — Table 3 cost can be swapped without changing the scheduling algorithm. Decouples scheduler from cost metric.
- **Never worse than FDS** — all 8 benchmark/latency pairs show A_FDS ≤ FDS. No regression.
- **Fast empirically** — "few seconds" on a Pentium 3 / 800 MHz. Even 400 synthesis evaluations are cheap for small CDFGs.
- **Uses established downstream tools** — clique partitioning + Left-Edge are well-understood; risk is in the scheduler, not in the allocation steps.

---

## 2. Critical Finding: Sign-Convention Inconsistency in Eq. 3

This is the most significant finding from close reading.

### Eq. 3 as printed (p. V-259):

$$M^n_{ij} = M^n_{ij} + \frac{\text{cost}(s) - \text{cost}(s^o)}{\text{cost}(s^o)}, \quad \forall (i,j) \in s$$

### Numerical trace:

Suppose `cost(s^o) = 1000`, `cost(s) = 900` (10% improvement):

```
increment = (900 - 1000) / 1000 = -0.10
M^n_ij becomes more negative.
```

In Eq. 5: `F_il = α·S_il/norm - β·M^n_il - γ·G^n_il`

```
With β = 2:
  -β·M^n_il = -2 × (-0.10) = +0.20 → F_il increases
A_FDS selects MINIMUM force.
→ The improved (i,l) decision now has HIGHER force → LESS likely to be re-chosen.
```

### Verdict:

**Improved scheduling decisions are anti-reinforced, not reinforced. The math contradicts the prose.**

Paper prose (p. V-259): *"The trail laid by an ant is positive or negative depending on whether improved synthesis results are obtained or not."*  
Math: trail increment is **always negative** for improvement.

### Two self-consistent corrections:

- **Correction A:** Negate Eq. 3: use `−(cost(s) − cost(s^o)) / cost(s^o)` → positive for improvement
- **Correction B:** Change Eq. 5 to `+β·M^n_il + γ·G^n_il` and select *maximum* force (or equivalently flip the minimum selection)

### Why benchmarks still improve (three hypotheses):

1. **Self-force dominance:** α·S_il/norm dominates for small trail values (start at 0); trail terms are weak perturbations around FDS behaviour
2. **Diversification:** anti-reinforcement spreads ants across different schedules → samples more of the search space than FDS's single greedy path
3. **Lucky parameter values:** β=2, γ=2 with small increments → trail contribution is too small to matter much; results are FDS + random noise

**Status: Cannot be resolved without the authors' source code.**  
My C++ implementation with `fix_sign=true` (Correction A) converges faster on DCT benchmarks, which suggests hypothesis 1 or 2 is partially right.

---

## 3. Other Limitations

- **No statistical variance** — single run per configuration. ACO is stochastic. Need ≥30 runs, mean ± std, and a Wilcoxon signed-rank test to establish significance. Paper predates this reporting standard in EDA.
- **No ablation** — which matters more, M^n or G? Unknown. No experiment separates their contributions.
- **Narrow benchmark suite** — four DSP kernels, all arithmetic-intensive with regular CDFGs. No control-flow, no irregular graphs, no industrial-scale circuits.
- **No comparison to other meta-heuristics** — SA and Tabu search are natural baselines for iterative HLS scheduling. Not compared.
- **Fixed-latency only** — minimum-latency synthesis (find minimum λ satisfying resource constraint) not addressed.
- **Missing implementation details:**
  - Exact clique-partitioning variant not specified
  - Left-Edge tie-breaking rule not given
  - How randomness in A_FDS candidate selection is seeded
  - No source code → reproducibility limited by the sign ambiguity

### Parameter sensitivity concerns:
- α=1, β=2, γ=2 stated as empirical best values but no sensitivity sweep provided
- η=0.5 controls both n_ants (outer loop) and candidate set (inner loop) — these are conceptually separate decisions; single value may not be optimal for both
- ρ tested in [0.35, 0.50] only — coarse grid

### Cost model mismatch:
- Register cost = 15 per register regardless of bit-width
- Mux cost = 15 per 2-input mux regardless of data width
- No timing model → can't check timing closure
- No FPGA-specific resources (LUT count, DSP blocks, routing)

---

## 4. Assumptions That May Be Unrealistic

- **Homogeneous FUs** — all multipliers identical, single-cycle. Real libraries have multi-cycle, pipelined, area/speed variants.
- **No chaining** — ops that could execute in same clock cycle (add→compare) are not modelled.
- **Complete synthesis each iteration** — 400 calls to clique-partitioning + Left-Edge. Feasible for N≤50. For N=1000 with large λ, this blows up.
- **Trail initialised to zero** — no warm start from FDS solution. Could seed G with the FDS schedule to accelerate convergence.

---

## 5. Scalability and Implementation Implications

| Scale | Memory (M^n + G) | Runtime |
|---|---|---|
| N=50, λ=20 (paper) | 2 × 50×20 = 2000 doubles ≈ 16 KB | "few seconds" |
| N=500, λ=50 | 2 × 500×50 = 50,000 doubles ≈ 400 KB | Dominated by 400 × clique-partition (O(N²)) calls |
| N=5000, λ=100 | 2 × 5M doubles ≈ 40 MB | 400 × O(N²) = 400 × 25M = 10B ops — minutes to hours |

**Parallelism opportunity:** Ants in each iteration are independent (G is read-only during the inner loop). Trivially parallelisable across n_ants threads → 20× speedup for free.

---

## 6. Application Suitability

### Where A_FDS works:

- ✅ ASIC datapath synthesis for DSP (filter cores, FFT engines, linear algebra accelerators)
- ✅ Latency-constrained designs where λ is fixed by a real-time protocol
- ✅ Area-optimisation pass in an existing HLS toolchain (plug-in compatible)
- ✅ Regular arithmetic CDFGs with N ≤ ~200 ops

### Where A_FDS fails or is inappropriate:

- ❌ FPGA targets — need LUT-level cost, routing congestion, DSP-block inference
- ❌ Control-flow-heavy designs — FSMs, conditional branches violate fixed-latency model
- ❌ Memory-bandwidth-bound applications — DRAM latency absent from cost function
- ❌ Hard real-time with WCET guarantees — stochastic scheduler cannot prove bounds
- ❌ Online / runtime reconfiguration — algorithm is entirely offline
- ❌ Large N (>500) without parallelisation — synthesis evaluations dominate runtime

---

## 7. Critical Assessment of Experimental Methodology

### Statistical validity:
- Each data point = single run. Stochastic algorithm → multi-run reporting is essential.
- No confidence intervals, no variance analysis.
- Cannot determine if improvements are statistically significant or attributable to lucky seeds.

### Benchmark selection:
- All four are classical DSP kernels from HLS literature — by design arithmetic-intensive.
- **Zero improvement on Differential Equation:** possibly CDFG too small (≤10 ops) for ACO to accumulate useful signal. Not discussed by authors.
- No large or irregular CDFGs tested.

### Missing implementation details (reproducibility):
- No pseudocode for clique partitioning variant used
- No specification of how ASAP/ALAP are initialised
- Sign inconsistency in Eq. 3 means the algorithm as printed is not reproducible as intended

---

## 8. Transfer to Related Problem Domains

- **Multi-processor HLS:** trail matrix extends to 3D (op × time-step × processor). ACO iteration structure remains valid.
- **Reconfigurable computing:** FPGA reconfiguration windows = time intervals where modules load. FDS "concurrency pressure" concept maps to "reconfiguration pressure."
- **Task scheduling in embedded OS:** fixed-latency precedence scheduling is isomorphic to periodic task scheduling with dependencies. But real-time OS requires determinism → stochastic ACO not appropriate.
- **Packet scheduling in networks:** type distribution concept ≈ traffic load balancing across time slots. Trail matrix ≈ congestion experience.

---

## 9. Proposed Extensions

1. **Correct and statistically re-evaluate Eq. 3** — implement both sign conventions, 30 runs each, Wilcoxon test
2. **Multi-objective cost model** — area + timing + power via Pareto-dominance trail update
3. **Parallel ants** — thread-per-ant, trivially independent inner loop
4. **Adaptive parameter tuning** — Bayesian optimisation over (α, β, γ, ρ, η) per circuit class
5. **Warm start from FDS** — seed G with FDS schedule's positive trail to accelerate convergence
6. **FPGA-specific cost model** — replace unit counts with LUT estimates from library characterisation

---

## References

- S. Kopuri and N. Mansouri, *IEEE ISCAS 2004*, pp. V-257–V-260
- P. G. Paulin and J. P. Knight, *DAC 1987*, pp. 195–202
- M. Dorigo, Ph.D. thesis, Politecnico di Milano, 1992
- R. Keinprasit and P. Chongstitvatana, *Int. J. Intell. Syst.*, 2003
- M. G. M. Heijligers et al., *ASP-DAC 1995*, pp. 61–66
