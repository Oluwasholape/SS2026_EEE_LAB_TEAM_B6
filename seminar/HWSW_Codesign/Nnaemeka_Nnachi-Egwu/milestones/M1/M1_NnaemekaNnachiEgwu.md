# M1 – Technical Understanding and Research Framing

**Student:** Nnaemeka Nnachi-Egwu  
**Programme:** Electronic Engineering, Hochschule Hamm-Lippstadt  
**Seminar:** HW/SW Co-Design (Prof. Dr. Achim Rettberg)  
**Topic:** High-Level Synthesis with Ant Colony Optimization Scheduling  
**Main ref:** Kopuri & Mansouri, IEEE ISCAS 2004, pp. V-257–V-260

> **Note:** These are rough working notes, not polished prose. Format is intentionally bullet-heavy, with pseudocode and annotated equations.

---

## 1. Problem Statement (my own words)

- **Core task:** In HLS, a behavioural spec (CDFG) → RTL netlist. *Scheduling* = assign each operation to a time-step under a fixed latency constraint λ, without violating data dependencies, while minimising hardware area.
- **Why hard:** NP-complete. Scheduling choices propagate downstream: concurrent ops → more functional units → more area. Wrong call here = bloated chip.
- **FDS problem:** Classical Force-Directed Scheduling is O(N³). Single pass. No feedback from what actually happens downstream (resource allocation, binding, register cost).
- **Paper's claim:** Replace the PS-force in FDS with two ACO-inspired experience matrices. Run full synthesis at each iteration. Feed real hardware cost back into scheduling. Converges to lower cost than FDS within ≤20 iterations.

---

## 2. HLS Flow Context

Four steps in datapath synthesis (from the paper introduction):

1. **Scheduling** – assign ops to time-steps (this paper's focus)
2. **Resource Allocation & Binding** – pick FU instances; paper uses *clique partitioning*
3. **Register Allocation & Binding** – assign values to registers; paper uses *Left-Edge algorithm*
4. **Steering logic** – mux/bus interconnect generation

Input: CDFG. ASAP and ALAP scheduling on the CDFG defines the **time frame** `[t_i^S, t_i^L]` for each operation `i`.

---

## 3. FDS Baseline – Key Equations

### Type distribution

`q_k(l)` = expected concurrency of type-`k` ops at time-step `l`  
= sum over unscheduled type-k ops of their uniform scheduling probability

For an unscheduled op with time frame `[asap, alap]`:  
`p_i(m) = 1 / (alap - asap + 1)` for `m ∈ [asap, alap]`, 0 elsewhere.

### Eq. 1 – Self-force (p. V-258)

$$S_{il} = \sum_{m=t_i^S}^{t_i^L} q_k(m)\bigl(\delta_{lm} - p_i(m)\bigr)$$

- `δ_lm` = Kronecker delta (1 if l=m, else 0)
- Interpretation: scheduling `i` at `l` spikes `q_k(l)` → increases concurrency pressure there
- Negative self-force = relieves a peak → preferred

### Eq. 2 – PS-force (p. V-258)

When scheduling `i` at `l` narrows a predecessor/successor's frame from `[t_i^S, t_i^L]` to `[t̃_i^S, t̃_i^L]`:

$$PS_{il} = \frac{1}{\bar{\mu}_i+1}\sum_{m=\tilde{t}_i^S}^{\tilde{t}_i^L} q_k(m) - \frac{1}{\mu_i+1}\sum_{m=t_i^S}^{t_i^L} q_k(m)$$

- `μ_i` = mobility (frame width − 1) before; `μ̄_i` = mobility after
- This is what costs O(N³) — visits all predecessors/successors

**FDS total force:** `F_il = S_il + Σ PS` → pick (i, l) with minimum.

---

## 4. ACO Modification – Algorithms and New Equations

### Algorithm Ant_sched (Table 1, p. V-258)

```
Initialise G[N][λ] = 0;   s_best = ∅
FOR n = 1 to n_iter:
    Initialise M^n[N][λ] = 0
    n_ants = ⌊η·N⌋,   η < 1
    Compute S_ij for all (i, j);  sort ascending
    Assign n_ants ants to ops with least self-forces as starting points
    FOR m = 1 to n_ants:
        s  = procedure A_FDS()
        Compute cost(s)                   ← full downstream synthesis
        Update M^n_ij via Eq. 3
        IF cost(s) < cost(s_best):
            s_best = s
    END FOR
    Update G_ij via Eq. 4               ← after all ants in this cycle
END FOR
```

### Algorithm A_FDS (Table 2, p. V-259)

```
n = 0
WHILE n ≠ N:
    Compute S_ij,   i ∈ [1,N],  l ∈ [1,λ]
    Compute F_ij    via Eq. 5
    x = ⌊η·n⌋,   η < 1
    Pick (i, l) uniformly at random from the x candidates with least force
    Schedule and update time frames
    n = n + 1
```

### Eq. 3 – Trail update (p. V-259)

$$M^n_{ij} = M^n_{ij} + \frac{\text{cost}(s) - \text{cost}(s^o)}{\text{cost}(s^o)}, \quad \forall (i,j) \in s$$

> ⚠️ **OPEN QUESTION / SIGN ISSUE:** If `s` is *better* than `s^o`, then `cost(s) < cost(s^o)`, so the increment is **negative**. In Eq. 5, `F_il` subtracts `β·M_il`. A more-negative `M_il` increases `F_il`. A_FDS picks *minimum* force → improved decisions get *higher* force → *less likely* to be re-chosen. This is **anti-reinforcement**, not reinforcement. Contradicts the prose on p. V-259: "trail is positive or negative depending on whether improved results are obtained." **Status: unresolved – flag in class.**

### Eq. 4 – Global experience update (p. V-259)

$$\forall (i,j): \quad G^{n+1}_{ij} = \rho\, G^n_{ij} + M^n_{ij}$$

- `ρ ∈ [0,1]` = evaporation factor; paper best: 0.35–0.50

### Eq. 5 – Modified force (p. V-259)

$$F_{il} = \alpha \cdot \frac{S_{il}}{\sqrt{\dfrac{1}{N\lambda}\displaystyle\sum_{(i,j)=(1,1)}^{N,\lambda} S_{ij}}} - \beta M^n_{il} - \gamma G^n_{il}$$

- Term 1: normalised self-force (denominator = scaled sum of all S_ij, NOT true RMS since S_ij is not squared)
- Term 2: current-cycle experience (per-ant trail)
- Term 3: cross-cycle accumulated experience
- All three terms: **O(Nλ)** — matrix lookups only
- α, β, γ: empirical constants; paper uses α=1, β=2, γ=2

---

## 5. Cost Model (Table 3, p. V-259)

| Hardware Unit | Cost |
|---|---|
| Single Cycle Multiplier | 250 |
| Single Cycle Adder/Subtractor/Comparator | 50 |
| Register | 15 |
| 2-input Mux | 15 |

Cost = sum over instantiated units. Evaluated **after full synthesis** (allocate + bind + register), not during scheduling.

---

## 6. Benchmark Results (Table 4, p. V-260)

| Benchmark | Latency | FDS | A_FDS | Δ |
|---|---|---|---|---|
| Differential Eq. | 4 | 875 | 875 | 0% |
| Differential Eq. | 5 | 875 | 875 | 0% |
| Elliptical Filter | 10 | 825 | 775 | −6.1% |
| Elliptical Filter | 12 | 775 | 775 | 0% |
| DCT 1 | 12 | 1795 | 1730 | −3.6% |
| DCT 1 | 14 | 1795 | 1690 | −5.9% |
| DCT 2 | 13 | 1210 | 1085 | **−10.3%** |
| DCT 2 | 14 | 1525 | 1390 | −8.9% |

Observations:
- A_FDS ≤ FDS in **all 8 cases** (never worse)
- Biggest gain: DCT 2 at latency 13 (−10.3%)
- Zero improvement on Differential Equation (both latencies) — why?
- Convergence within ≤20 iterations; 20 ants per iteration
- Runtime: "few seconds" on Pentium 3 / 800 MHz / 320 MB — **no quantitative timing figures given**

---

## 7. Parameters (from Section 4 of paper)

| Param | Meaning | Value |
|---|---|---|
| η | ant fraction / candidate fraction | 0.5 |
| α | self-force weight | 1 |
| β | trail weight | 2 |
| γ | global experience weight | 2 |
| ρ | evaporation | 0.35–0.50 |
| n_iter | iterations | ≤ 20 |
| n_ants | ants per iteration | 20 |

Total synthesis evaluations per experiment = 20 × 20 = **400**

---

## 8. Complexity Analysis

| Component | Complexity |
|---|---|
| FDS | O(N³) — PS-force visits all predecessor/successor pairs |
| A_FDS per scheduling step | O(Nλ) — all Eq. 5 terms are O(1) matrix lookups |
| A_FDS total (one ant, one iter) | O(N²λ) |
| Ant_sched full | O(n_iter · n_ants · N²λ) |
| Memory (M^n and G matrices) | O(Nλ) |

Note: "all terms in the modified force equation can be calculated in linear time" (p. V-259) ← directly quoted claim.

---

## 9. Assumptions and Scope Limitations

- Fixed latency λ given as input (not minimum-latency synthesis)
- Single resource type per op type (all multipliers identical, single-cycle)
- Cost model is count-based — no timing, power, or wire delay
- Clique partitioning and Left-Edge are greedy heuristics, not optimal
- Parameters (α, β, γ, ρ, η) empirical — no analytical derivation
- No statistical variance reported — single run per configuration
- No pipelining or loop unrolling

---

## 10. Open Questions / Unclear Points

1. **Sign convention (Eq. 3 vs prose):** As shown above in §4.3, math anti-reinforces improved decisions. Plausible fix: negate Eq. 3, or flip signs in Eq. 5 to `+β·M + γ·G`. Which was intended? → Flag in class.
2. **p_i(m) exactly:** Paper implies uniform over time frame but no explicit formula written.
3. **Stochastic selection in A_FDS:** "at random from x candidates" — what distribution? Uniform assumed.
4. **Convergence guarantee:** "optimal schedules found within 20 iterations" — empirical observation only, no proof.
5. **Differential Equation zero improvement:** CDFG too small? FDS already at optimum? Sign issue more harmful for small N?
6. **What is the exact clique-partitioning variant used?** Paper doesn't specify.

---

## 11. Why Relevant for HW/SW Co-Design

- Scheduling is in the core HLS loop → directly shapes HW side of a co-design
- Better schedule → fewer FUs → less silicon → lower cost and power
- ACO is population-based: can naturally extend to multi-objective (area + timing + power) by changing cost model
- DSM concern raised by paper: in deep sub-micron, wire delay dominates → count-based cost is insufficient → extension opportunity

---

## References

- [1] S. Kopuri and N. Mansouri, "Enhancing scheduling solutions through ant colony optimization," *IEEE ISCAS 2004*, pp. V-257–V-260.
- [2] P. G. Paulin and J. P. Knight, "Force-directed scheduling in automatic data path synthesis," *DAC 1987*, pp. 195–202.
- [3] M. Dorigo, *Optimization, Learning and Natural Algorithms*, Ph.D. thesis, Politecnico di Milano, 1992.
