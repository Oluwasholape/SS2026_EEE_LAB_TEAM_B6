# M3 Critical Evaluation, Limitations, and Extensions: AI Usage Interaction Log

## Metadata
- **Student:** Soaib Ferdous
- **Student ID:** 1232368
- **Course:** HW/SW Co-Design
- **Topic:** High-Level Synthesis with Tabu Search Scheduling
- **Protocol version:** 0.2

## Milestone purpose
Critical evaluation of TS-PRED beyond surface-level description. The focus is to show that the strengths, limitations, hidden assumptions, scalability behaviour, suitable/unsuitable scenarios, and possible extensions were understood and critically assessed against the primary source.

## Expected milestone evidence
- Strengths of the approach with evidence from experimental results
- Limitations and weaknesses including degradation at scale
- Hidden assumptions and consequences if violated
- Scalability analysis based on Tables 2, 3, and 4 of the paper
- Suitable and unsuitable scenarios
- Proposed extensions including those suggested by the authors
- Open questions for further investigation

## How to read this file
This Markdown file is a human-readable version of the machine-readable AI usage protocol JSON. It documents only milestone-relevant AI-supported reasoning episodes. It is not a raw chat dump and it is not final paper prose. The important parts are the objective, verification actions, evaluation status, integration decision, and reflection for each interaction.

## Summary of AI usage

### Evaluation status counts
- `accepted`: 1
- `partially-accepted`: 2
- `revised`: 1

### Verification level counts
- `primary-source-check`: 2
- `methodological-check`: 1
- `argument-critique`: 1

### Usage type counts
- `argument-critique`: 2
- `summarization`: 1
- `idea-generation`: 1

## Interaction overview table
| # | Objective | Verification | Status | Used in work | Section | Usage type |
|---:|---|---|---|---|---|---|
| 1 | Identify the strengths of TS-PRED supported by experimental evidence from the paper. | `primary-source-check` | `accepted` | yes | Section 1 (Strengths) | `summarization` |
| 2 | Identify limitations, weaknesses and hidden assumptions of the TS-PRED model. | `methodological-check` | `partially-accepted` | yes | Sections 2 and 3 (Limitations and Hidden Assumptions) | `argument-critique` |
| 3 | Analyse scalability behaviour of TS-PRED across instance types and sizes from Tables 2, 3 and 4. | `primary-source-check` | `partially-accepted` | yes | Section 4 (Scalability Analysis) | `argument-critique` |
| 4 | Identify proposed extensions and open research questions beyond the scope of the paper. | `methodological-check` | `revised` | yes | Sections 5 and 6 (Extensions and Open Questions) | `idea-generation` |

## Detailed interaction records

### Interaction 1
- **Objective:** Identify the strengths of TS-PRED supported by experimental evidence from the paper.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
What are the main strengths of the TS-PRED Tabu Search algorithm for HLS scheduling based on Sevaux et al. (2011)? Support each strength with evidence from the paper's experimental results.
```

#### AI output summary
AI listed six strengths: handles precedence constraints uniquely among heuristics; always finds a feasible solution including n=1000 cases; near-optimal on small instances with deviation under 1.24%; faster than ILP with growing speed advantage; effective on real-life electronic circuit instances; preprocessing reduces search space.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Verified deviation figures (1.24% at n=100) against Table 2 of Sevaux et al. (2011), p. 209.
  - Verified real-life instance performance (27/27 optimal) against Table 4, p. 211.
  - Confirmed that TS-PRED is described as the only heuristic for instances with precedence in Section 1, p. 202.

#### Evaluation
- **Status:** `accepted`
- **Usefulness:** `high`
- **Issues:**
  - None — all strengths were traceable to specific sections and tables of the paper.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 1 (Strengths)
- **Usage type:** `summarization`
- **Direct text reused:** No

#### Reflection
Grounding each strength in a specific table or section of the paper made this section much more credible than a general list of claimed advantages. This approach should be applied to all evaluation sections.

---

### Interaction 2
- **Objective:** Identify limitations, weaknesses and hidden assumptions of the TS-PRED model.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
What are the limitations and hidden assumptions of TS-PRED for HLS scheduling? Consider the problem model, algorithm design, and experimental evaluation separately.
```

#### AI output summary
AI identified: no optimality guarantee; empirically tuned parameters with no theoretical justification; identical processor assumption excluding heterogeneous FUs; integer time model; single objective only; no sensitivity analysis reported. For hidden assumptions: DAG precedence structure, non-preemptive jobs, fixed release and due dates.

#### Verification
- **Level:** `methodological-check`
- **Actions:**
  - Verified the identical processor assumption against Section 1 of the paper where identical parallel processors are stated explicitly.
  - Confirmed the empirical parameter tuning statement from Section 3 p. 208 of the paper.
  - Checked that no sensitivity analysis is reported — confirmed by absence in the paper.
  - Verified the DAG assumption from Section 2.1 where acyclicity is implied by the topological ordering scheme.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `high`
- **Issues:**
  - AI suggested a limitation about memory usage but this is not discussed in the paper at all. Removed as unverifiable.
  - AI did not identify the non-preemptive assumption explicitly — added manually since this is implicit in the model where each job runs continuously from start to completion.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Sections 2 and 3 (Limitations and Hidden Assumptions)
- **Usage type:** `argument-critique`
- **Direct text reused:** No

#### Reflection
AI correctly identified most structural limitations but missed implicit assumptions embedded in the mathematical model. Reading the ILP formulation carefully revealed the non-preemptive and DAG assumptions that the paper does not state explicitly.

---

### Interaction 3
- **Objective:** Analyse scalability behaviour of TS-PRED across instance types and sizes from Tables 2, 3 and 4.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
Analyse the scalability of TS-PRED based on the experimental results in Tables 2, 3 and 4 of Sevaux et al. (2011). What patterns emerge as n increases and how do instance types affect performance?
```

#### AI output summary
AI noted that without precedence, deviation grows slowly from 0% at n=20 to 1.24% at n=100 with fast runtimes. With precedence, performance degrades at n=250 and beyond, especially for Type 2. Type 4 performs better due to larger processing time leeway. For n=1000 ILP is unusable while TS-PRED still finds feasible solutions for all instances.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Verified all figures against Tables 2, 3 and 4 of Sevaux et al. (2011), pp. 209-211.
  - Confirmed Type 4 advantage explanation from the authors' own discussion on p. 210.
  - Checked n=1000 ILP failure from Table 3 where #Feas=0 for Types 1 and 3.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `high`
- **Issues:**
  - AI stated that TS-PRED always finds the optimal solution for n=1000 Type 4 — this is incorrect. Corrected after checking Table 3: #Opt=0 for n=1000 Type 4 (ILP also finds 0 optimal so comparison is unavailable, not that TS-PRED is optimal).

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 4 (Scalability Analysis)
- **Usage type:** `argument-critique`
- **Direct text reused:** No

#### Reflection
This interaction produced a factual error that required careful table reading to catch. AI confused feasible solutions with optimal solutions for the n=1000 Type 4 case. Always verify numerical claims against the actual tables.

---

### Interaction 4
- **Objective:** Identify proposed extensions and open research questions beyond the scope of the paper.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
What extensions to TS-PRED do the authors propose in Sevaux et al. (2011)? What additional research directions would be natural given the limitations of the current approach?
```

#### AI output summary
AI identified three author-proposed extensions: memetic algorithm with population management (MA|PM) using TS-PRED as local search; support for multiple processor types; improved preprocessing and lower bound computations. AI also suggested adaptive parameter control and multi-objective extensions as natural directions.

#### Verification
- **Level:** `methodological-check`
- **Actions:**
  - Verified MA|PM extension against Section 4 (Conclusions) of Sevaux et al. (2011), p. 211.
  - Confirmed multiple processor type extension from the same conclusions section.
  - Confirmed improved lower bounds direction from the conclusions section.
  - Adaptive parameter control and multi-objective extensions are not in the paper — kept as independently identified directions and labelled clearly as such.

#### Evaluation
- **Status:** `revised`
- **Usefulness:** `high`
- **Issues:**
  - AI presented all extensions as if they were from the paper. Revised to clearly separate author-proposed extensions (Section 4 of the paper) from independently identified extensions. This distinction is important for academic integrity.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 5 (Proposed Extensions)
- **Usage type:** `idea-generation`
- **Direct text reused:** No

#### Reflection
Distinguishing between what the authors themselves propose and what can be independently derived from the limitations analysis is an important scientific habit. AI blurred this boundary and it had to be corrected manually.

---

## Academic integrity note
The interactions documented here were used as support for understanding, structuring, critique, verification, or communication. The submitted milestone artifact does not reuse AI text directly. All experimental figures were verified against Tables 2, 3 and 4 of Sevaux et al. (2011). The distinction between author-proposed extensions and independently identified extensions is explicitly maintained in the submitted work.
