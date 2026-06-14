# M2 Algorithm Description, ILP Formulation, and Comparison: AI Usage Interaction Log

## Metadata
- **Student:** Soaib Ferdous
- **Student ID:** 1232368
- **Course:** HW/SW Co-Design
- **Topic:** High-Level Synthesis with Tabu Search Scheduling
- **Protocol version:** 0.2

## Milestone purpose
Formal and algorithmic understanding of the TS-PRED Tabu Search approach and its ILP baseline. The focus is to show that the ILP model, all three phases of TS-PRED (preprocessing, initial solution, tabu search), and the comparison with TS-SS were understood at a technical level.

## Expected milestone evidence
- Complete ILP formulation with all constraints
- Preprocessing phase via critical path tightening
- Initial solution generation procedure
- Full Tabu Search procedure (perturbation, insertion, exchange, tabu tenure)
- Comparison of TS-PRED vs TS-SS
- Runtime and parameter discussion

## How to read this file
This Markdown file is a human-readable version of the machine-readable AI usage protocol JSON. It documents only milestone-relevant AI-supported reasoning episodes. It is not a raw chat dump and it is not final paper prose. The important parts are the objective, verification actions, evaluation status, integration decision, and reflection for each interaction.

## Summary of AI usage

### Evaluation status counts
- `accepted`: 1
- `partially-accepted`: 3

### Verification level counts
- `primary-source-check`: 3
- `methodological-check`: 1

### Usage type counts
- `summarization`: 2
- `comparison-structuring`: 1
- `terminology-clarification`: 1

## Interaction overview table
| # | Objective | Verification | Status | Used in work | Section | Usage type |
|---:|---|---|---|---|---|---|
| 1 | Understand the three phases of TS-PRED and how preprocessing tightens release and due dates. | `primary-source-check` | `partially-accepted` | yes | Section 2 (TS-PRED Algorithm) | `summarization` |
| 2 | Understand the initial solution generation procedure and how topological ordering is used. | `primary-source-check` | `accepted` | yes | Section 2 (Phase 2) | `summarization` |
| 3 | Clarify the tabu search procedure: perturbation methods, insertion, exchange operations and tabu tenure formula. | `primary-source-check` | `partially-accepted` | yes | Section 2 (Phase 3) | `terminology-clarification` |
| 4 | Compare TS-PRED with TS-SS and identify the key algorithmic differences and performance improvements. | `methodological-check` | `partially-accepted` | yes | Section 3 (Comparison) | `comparison-structuring` |

## Detailed interaction records

### Interaction 1
- **Objective:** Understand the three phases of TS-PRED and how preprocessing tightens release and due dates.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
Explain the preprocessing phase of the TS-PRED Tabu Search algorithm for HLS scheduling as described by Sevaux et al. (2011). How does it tighten release dates and due dates using precedence constraints?
```

#### AI output summary
AI described a two-pass critical path computation. Forward pass computes earliest start times; if EST exceeds release date, release date is updated. Backward pass computes latest completion times; if LCT is less than due date, due date is updated. This reduces the feasible search space and identifies infeasible instances early.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Verified forward and backward pass logic against Section 2.2.1 of Sevaux et al. (2011), p. 204.
  - Confirmed that the method is described as analogous to critical path computations in the paper.
  - Checked that the benefit of identifying infeasible instances early is explicitly stated in the paper.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `high`
- **Issues:**
  - AI did not clearly distinguish the forward and backward pass directions. Restructured manually into two explicit passes with update rules for clarity in the milestone.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 2 (Phase 1 — Preprocessing)
- **Usage type:** `summarization`
- **Direct text reused:** No

#### Reflection
The preprocessing phase is not just an optimisation detail — it is essential for making TS-PRED practical on large instances. Understanding the critical path analogy helped clarify why this step significantly reduces the search space.

---

### Interaction 2
- **Objective:** Understand the initial solution generation procedure and how topological ordering is used.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
How does TS-PRED generate its initial feasible solution? Explain the role of topological ordering of the precedence DAG and how jobs are allocated to processors iteratively.
```

#### AI output summary
AI explained that jobs are numbered in topological order so that each job's number is higher than all its predecessors. At each iteration, the lowest-numbered job with all predecessors allocated is selected. It is inserted at the position minimising idle time. If no processor can accommodate it, a new one is added. Start time is always the earliest feasible time.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Verified all steps against Section 2.2.2 of Sevaux et al. (2011), p. 205.
  - Confirmed that once allocated, a job cannot be moved during initial solution generation — this constraint is explicitly stated in the paper and was correctly captured by the AI.

#### Evaluation
- **Status:** `accepted`
- **Usefulness:** `high`
- **Issues:**
  - None — description matched the paper precisely.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 2 (Phase 2 — Initial Solution Generation)
- **Usage type:** `summarization`
- **Direct text reused:** No

#### Reflection
The topological ordering ensures that precedence constraints are never violated during construction. This is a simple but important design choice that makes the greedy construction always produce a valid initial solution.

---

### Interaction 3
- **Objective:** Clarify the tabu search procedure including perturbation methods, insertion, exchange operations and tabu tenure formula.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
Explain the main Tabu Search procedure of TS-PRED in detail: the two perturbation methods, how unallocated jobs are reinserted via insertion and exchange operations, and the tabu tenure formula.
```

#### AI output summary
AI described two perturbation methods (random processor removal and probabilistic job removal with p_rem=0.2). For reallocation: insertion tries to place the job by pushing others using ECT and LST; exchange swaps the job with shorter consecutive jobs whose schedules overlap. Tabu tenure is max{3, ceil(n/m)}. If neither insertion nor exchange works, a new processor is added.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Verified both perturbation methods against Section 2.2.3 of Sevaux et al. (2011), pp. 205-206.
  - Confirmed tabu tenure formula max{3, ceil(n/m)} from p. 208 of the paper.
  - Verified the exchange condition (only when processing time of consecutive job is less than unallocated job) from p. 206.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `high`
- **Issues:**
  - AI initially omitted the condition that exchange is only performed when the processing time of each consecutively allocated job is less than the unallocated job. This is a key difference from TS-SS and was added manually after checking the paper.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 2 (Phase 3 — Tabu Search Procedure)
- **Usage type:** `terminology-clarification`
- **Direct text reused:** No

#### Reflection
The exchange condition (shorter processing time only) is a deliberate design choice motivated by the idea that shorter jobs are easier to reschedule. Missing this detail would have made the algorithm description inaccurate compared to the paper.

---

### Interaction 4
- **Objective:** Compare TS-PRED with TS-SS and identify the key algorithmic differences and performance improvements.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
What are the key algorithmic differences between TS-PRED and TS-SS (Sevaux and Sorensen 2007)? How does TS-PRED improve solution quality and runtime compared to TS-SS?
```

#### AI output summary
AI identified five main differences: TS-PRED supports precedence constraints while TS-SS does not; TS-PRED uses a reverse approach starting from a feasible solution rather than building up from a lower bound; TS-PRED uses a move-based tabu criterion rather than sum of processing times; TS-PRED adds an exchange condition restricting to shorter jobs; TS-PRED includes preprocessing. Performance: TS-PRED deviation 1.24% vs 5.45% for TS-SS at n=100.

#### Verification
- **Level:** `methodological-check`
- **Actions:**
  - Verified performance figures against Table 2 of Sevaux et al. (2011), p. 209.
  - Checked the algorithmic differences against Sections 2.2 and 4 of the paper.
  - Confirmed that TS-SS data in Table 2 is taken from Sevaux and Sorensen (2007) as stated in the paper.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `high`
- **Issues:**
  - AI stated TS-PRED is always faster without qualification. Corrected to state that speed advantage grows with n, as reported in the paper's conclusions.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 3 (Comparison: TS-PRED vs TS-SS)
- **Usage type:** `comparison-structuring`
- **Direct text reused:** No

#### Reflection
Comparing the two methods clarified why TS-PRED is the contribution of this paper and not just an incremental change. The reverse approach and precedence support are the two most important novelties.

---

## Academic integrity note
The interactions documented here were used as support for understanding, structuring, critique, verification, or communication. The submitted milestone artifact does not reuse AI text directly. Scientific claims were checked against the assigned reference (Sevaux et al. 2011), specifically Sections 2.2.1 through 2.2.3 and Tables 2 and 3, before being included in the submitted work.
