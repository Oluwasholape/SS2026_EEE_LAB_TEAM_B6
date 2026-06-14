# M4 Scientific Communication: Paper and Talk: AI Usage Interaction Log

## Metadata
- **Student:** Soaib Ferdous
- **Student ID:** 1232368
- **Course:** HW/SW Co-Design
- **Topic:** High-Level Synthesis with Tabu Search Scheduling
- **Protocol version:** 0.2

## Milestone purpose
Scientific communication through final paper and talk. The focus is to synthesise M1-M3 into coherent communication while documenting AI support transparently.

## Expected milestone evidence
- Coherent explanation of the technical contribution
- Scientific context and related approaches and tradeoffs
- Critical evaluation and application discussion
- Clear citations and transparent AI usage protocol
- Talk motivation, core idea, assumptions, tradeoffs, limitations, and open issues

## How to read this file
This Markdown file is a human-readable version of the machine-readable AI usage protocol JSON. It documents only milestone-relevant AI-supported reasoning episodes. It is not a raw chat dump and it is not final paper prose. The important parts are the objective, verification actions, evaluation status, integration decision, and reflection for each interaction.

## Summary of AI usage

### Evaluation status counts
- `accepted`: 4
- `partially-accepted`: 2
- `revised`: 3

### Verification level counts
- `primary-source-check`: 3
- `secondary-source-check`: 3
- `methodological-check`: 2
- `empirical-check`: 1

### Usage type counts
- `text-revision`: 1
- `modeling-support`: 1
- `code-generation`: 1
- `argument-critique`: 1
- `comparison-structuring`: 1
- `idea-generation`: 1
- `verification-support`: 2
- `comparison-structuring`: 1

## Interaction overview table
| # | Objective | Verification | Status | Used in work | Section | Usage type |
|---:|---|---|---|---|---|---|
| 1 | Plan the final paper structure by mapping M1, M2, and M3 content to IEEE paper sections. | `secondary-source-check` | `accepted` | yes | M4 Paper outline | `comparison-structuring` |
| 2 | Write the introduction, motivation, and formal HLS scheduling problem section in scientific prose. | `primary-source-check` | `revised` | yes | Sections I and II | `text-revision` |
| 3 | Describe all three phases of TS-PRED and the ILP baseline in paper-quality scientific prose. | `primary-source-check` | `revised` | yes | Section III | `modeling-support` |
| 4 | Implement the main feature of TS-PRED in C++ without comments and verify the output. | `empirical-check` | `partially-accepted` | yes | Section IV | `code-generation` |
| 5 | Add runtime and overhead analysis and integrate the application example output explanation. | `methodological-check` | `partially-accepted` | yes | Sections V and VI | `argument-critique` |
| 6 | Integrate experimental results, TS-PRED vs TS-SS comparison, and M3 critical evaluation. | `primary-source-check` | `accepted` | yes | Sections VII and VIII | `comparison-structuring` |
| 7 | Plan the 10-minute presentation slides and prepare speaker notes for key technical terms. | `secondary-source-check` | `revised` | yes | Talk slides and speaker notes | `idea-generation` |
| 8 | Prepare possible examiner questions and brief answers for the 5-minute discussion. | `methodological-check` | `accepted` | yes | Talk discussion preparation | `verification-support` |
| 9 | Validate the final paper and presentation against the professor's elaboration checklist. | `secondary-source-check` | `accepted` | yes | M4 Final checklist validation | `verification-support` |

## Detailed interaction records

### Interaction 1
- **Objective:** Plan the final paper structure by mapping M1, M2, and M3 content to IEEE paper sections.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Create a 5-page IEEE-style seminar paper outline for Tabu Search for HLS that integrates M1 mathematical foundations, M2 algorithm description, and M3 critical evaluation.
```
#### AI output summary
AI proposed eight sections: introduction, HLS scheduling problem, resolution approaches covering ILP and TS-PRED, C++ implementation, runtime analysis, application example, results and comparison, and critical evaluation with conclusion.
#### Verification — `secondary-source-check`
- Checked professor's elaboration requirements slide against all proposed sections.
- Confirmed every section maps back to M1-M3 content.
- Removed sections that would not fit within five pages.
#### Evaluation — `accepted` | Usefulness: `high` | Issues: None.
#### Integration — Used: Yes | Section: M4 Paper outline | Type: `comparison-structuring` | Direct text reused: No
#### Reflection
The paper must synthesise the milestones rather than being generated independently. Mapping each section back to M1-M3 confirmed that nothing was added beyond what was already understood.

---

### Interaction 2
- **Objective:** Write the introduction, motivation, and formal HLS scheduling problem section in scientific prose.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Draft the introduction and formal problem section for a seminar paper on Tabu Search for HLS. Cover why minimising functional units matters, why ILP fails for large instances, the job model with parameters r_i, d_i, p_i and PRED_i, all four constraints, and the NP-hardness argument.
```
#### AI output summary
AI produced an introduction connecting chip design cost to processor minimisation and NP-hardness, and a formal section with the job model, time horizon formula, binary decision variable, and all four constraints with numbered equations.
#### Verification — `primary-source-check`
- Checked all formal definitions and constraint formulations against Sections 1 and 2.1 of Sevaux et al. (2011), pp. 201-203.
- Verified NP-hardness reduction via bin packing from p. 202.
- Rewrote introduction manually to avoid direct reuse of AI wording.
#### Evaluation — `revised` | Usefulness: `high` | Issues: AI introduction was too broad. Binary constraint C4 was initially missing and added manually after checking the paper.
#### Integration — Used: Yes | Section: Sections I and II | Type: `text-revision` | Direct text reused: No
#### Reflection
Scientific prose requires more precision than AI naturally produces. Primary source verification caught a missing constraint and over-broad motivation claims.

---

### Interaction 3
- **Objective:** Describe all three phases of TS-PRED and the ILP baseline in paper-quality scientific prose including Algorithm 1 pseudocode.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Write the resolution approaches section for a seminar paper on Tabu Search for HLS. Cover the ILP baseline with GLPK and 600-second limit, then all three phases of TS-PRED: preprocessing via critical path, greedy initial solution in topological order, and the tabu search loop with perturbation, insertion, exchange, and tabu tenure formula max{3, ceil(n/m)}.
```
#### AI output summary
AI described ILP as minimising m via GLPK with 600-second limit, and covered all three TS-PRED phases including forward and backward pass preprocessing, topological greedy allocation, both perturbation methods, insertion via ECT/LST, exchange restricted to shorter jobs, and the tabu tenure formula.
#### Verification — `primary-source-check`
- Verified all three phases against Sections 2.2.1, 2.2.2, and 2.2.3 of Sevaux et al. (2011), pp. 204-206.
- Confirmed tabu tenure formula from p. 208.
- Verified exchange condition restricting to shorter processing times from p. 206.
- Confirmed GLPK and 600-second limit from Section 3, p. 208.
#### Evaluation — `revised` | Usefulness: `high` | Issues: AI omitted the exchange condition that only jobs with processing time strictly less than the unallocated job may be exchanged. Added manually after checking the paper.
#### Integration — Used: Yes | Section: Section III | Type: `modeling-support` | Direct text reused: No
#### Reflection
The exchange condition is the most important algorithmic detail distinguishing TS-PRED from TS-SS. Missing it would have made the paper description inaccurate.

---

### Interaction 4
- **Objective:** Implement the main feature of TS-PRED in C++ without comments and verify the output by running it.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Write a clean C++ implementation of TS-PRED for HLS multiprocessor scheduling with no comments. Include all three phases: preprocessing, greedy initial solution, and the tabu search loop with insertion, exchange, and tabu list.
```
#### AI output summary
AI produced a complete C++ implementation with Job, Processor, Solution, and TabuList structs, forward and backward preprocessing passes, greedy initial solution generator, try_insert and try_exchange functions, and the main tspred loop with both perturbation methods.
#### Verification — `empirical-check`
- Compiled and ran the implementation on an online C++ compiler.
- Verified output: 4-job HLS example produces 1 processor with schedule [J0 s=0 f=2][J1 s=2 f=4][J2 s=4 f=6][J3 s=6 f=8].
- Confirmed all precedence constraints respected in the output.
- Screenshot saved as implementation_result.png and included in the paper as Fig. 1.
#### Evaluation — `partially-accepted` | Usefulness: `high` | Issues: random_shuffle is deprecated in C++17. Noted as a compiler warning only. Code compiles and produces correct output.
#### Integration — Used: Yes | Section: Section IV | Type: `code-generation` | Direct text reused: No
#### Reflection
Running the code and seeing the correct output is the only reliable verification method. The screenshot confirms the schedule respects all precedence and time-window constraints.

---

### Interaction 5
- **Objective:** Add runtime and overhead analysis and integrate the application example output explanation into the paper.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Write a runtime and overhead analysis for TS-PRED covering all three phases, and explain what the 4-job application example output means in terms of HLS hardware scheduling.
```
#### AI output summary
AI described preprocessing as O(n + |E|), initial solution as O(n*m), and the tabu loop as bounded by the stopping condition. Explained that 1 processor is optimal for the example because the precedence chain forces serial execution. Cited runtimes under 2 seconds at n=100 and under 105 seconds at n=1000.
#### Verification — `methodological-check`
- Verified runtime figures against Tables 2 and 3 of Sevaux et al. (2011).
- Confirmed O(n*H) memory claim is consistent with Section 2.2.3.
- Manually verified the 4-job schedule: J2 starts at cycle 4 after both J0 and J1 complete, J3 starts at cycle 6 after J2 completes, all within due dates.
#### Evaluation — `partially-accepted` | Usefulness: `high` | Issues: AI gave a precise Big-O for the tabu loop not derivable from the paper — replaced with a bounded-by-stopping-condition statement. AI also initially claimed J0 and J1 run in parallel which is incorrect — corrected to sequential.
#### Integration — Used: Yes | Section: Sections V and VI | Type: `argument-critique` | Direct text reused: No
#### Reflection
Two errors in one interaction: an invented complexity formula and an incorrect parallelism claim. Both required manual correction against the paper and the actual code output.

---

### Interaction 6
- **Objective:** Integrate experimental results, TS-PRED vs TS-SS comparison, and M3 critical evaluation into the paper.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Write the results, comparison, and critical evaluation sections for a seminar paper on Tabu Search for HLS. Include TS-PRED vs TS-SS differences and performance figures, scalability behaviour across instance types, strengths, limitations, hidden assumptions, and proposed extensions.
```
#### AI output summary
AI listed five algorithmic differences between TS-PRED and TS-SS, cited deviation figures of 1.24% vs 5.45% at n=100, described scalability degradation at n=250 and ILP failure at n=1000, and identified strengths, limitations, and three extensions: MA|PM, heterogeneous processors, multi-objective.
#### Verification — `primary-source-check`
- Verified all performance figures against Tables 2, 3 and 4 of Sevaux et al. (2011), pp. 209-211.
- Confirmed MA|PM and heterogeneous processor extensions from Section 4, p. 211.
- Verified identical processor assumption from Section 1, p. 201.
#### Evaluation — `accepted` | Usefulness: `high` | Issues: None.
#### Integration — Used: Yes | Section: Sections VII and VIII | Type: `comparison-structuring` | Direct text reused: No
#### Reflection
Grounding each claim in a specific table entry made the results section credible. The critical evaluation draws directly from M3 so no new analysis was needed.

---

### Interaction 7
- **Objective:** Plan the 10-minute presentation slides and prepare speaker notes for key technical terms.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Create a 10-minute presentation outline for Tabu Search for HLS that does not repeat the paper. Also give speaker notes for: tabu search, tabu tenure, perturbation, insertion vs exchange, and why TS-PRED outperforms ILP for large instances.
```
#### AI output summary
AI proposed slides for chip cost motivation, HLS scheduling problem, NP-hardness and ILP failure, TS-PRED three phases, application example with output screenshot, TS-PRED vs TS-SS comparison, limitations and extensions, and discussion questions. Speaker notes explained each term in plain language.
#### Verification — `secondary-source-check`
- Checked professor's M4 requirement of 10 minutes presentation and 5 minutes discussion.
- Verified speaker note definitions against Sections 2.2.2 and 2.2.3 of Sevaux et al. (2011).
- Removed informal analogies and replaced tabu tenure explanation with the formula max{3, ceil(n/m)}.
#### Evaluation — `revised` | Usefulness: `high` | Issues: AI speaker note for tabu tenure used an informal analogy. AI slide outline had too much text per slide — reduced to bullet points and diagrams.
#### Integration — Used: Yes | Section: Talk slides and speaker notes | Type: `idea-generation` | Direct text reused: No
#### Reflection
The presentation must teach the central idea quickly, not reproduce the paper. Showing the implementation output screenshot is more effective than reading equations from slides.

---

### Interaction 8
- **Objective:** Prepare possible examiner questions and brief answers for the 5-minute discussion.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
What questions might the professor ask about Tabu Search for HLS scheduling during the 5-minute discussion, and how should I answer them briefly?
```
#### AI output summary
AI suggested questions about: why the problem is NP-hard, why ILP fails at n=1000, what tabu tenure controls, why the exchange condition restricts to shorter jobs, what the MA|PM extension would add, and whether TS-PRED could handle heterogeneous processors.
#### Verification — `methodological-check`
- Checked that each question maps to a weak point identified in M3 critical evaluation.
- Prepared concise answers from own understanding of the paper.
- Marked the heterogeneous processor question for further review as it goes beyond the paper scope.
#### Evaluation — `accepted` | Usefulness: `high` | Issues: None.
#### Integration — Used: Yes | Section: Talk discussion preparation | Type: `verification-support` | Direct text reused: No
#### Reflection
Preparing for examiner questions validates whether the topic is understood deeply enough to defend. The exchange condition question is the most likely difficult question because it is a subtle but important algorithmic detail.

---

### Interaction 9
- **Objective:** Validate the final paper and presentation against the professor's elaboration checklist.
- **AI tool:** Claude | **Model:** claude-sonnet-4-6

#### Prompt
```text
Check whether my final paper and presentation satisfy all elaboration requirements: mathematical basics, formal topic description, formal algorithm description, runtime and overhead, C++ implementation, application example, and RTL output context.
```
#### AI output summary
AI mapped all seven elaboration requirements to specific sections of the paper and confirmed all are covered. Confirmed that runtime analysis, C++ implementation with output figure, and RTL context in the introduction are all present.
#### Verification — `secondary-source-check`
- Checked the professor's elaboration slide directly against the submitted paper sections.
- Confirmed runtime analysis is in Section V.
- Confirmed C++ implementation is in Section IV with code listing and Fig. 1 showing the output.
- Confirmed RTL and VHDL context is stated in the Introduction.
#### Evaluation — `accepted` | Usefulness: `high` | Issues: None.
#### Integration — Used: Yes | Section: M4 Final checklist validation | Type: `verification-support` | Direct text reused: No
#### Reflection
This final check confirmed that every elaboration requirement from the professor's slides is covered by a specific section in the paper. M4 is a synthesis milestone — nothing was added that was not already understood from M1 to M3.

---

## Academic integrity note
The interactions documented here were used as support for understanding, structuring, critique, verification, or communication. The submitted milestone artifact does not reuse AI text directly. Technical claims were verified against the assigned reference (Sevaux et al. 2011) before inclusion. The C++ implementation was written with AI assistance and verified by compilation and execution.
