# M1 Technical Understanding and Research Framing: AI Usage Interaction Log

## Metadata
- **Student:** Soaib Ferdous
- **Student ID:** 1232368
- **Course:** HW/SW Co-Design
- **Topic:** High-Level Synthesis with Tabu Search Scheduling
- **Protocol version:** 0.2

## Milestone purpose
Technical understanding and research framing before polished prose. The focus is to show that the technical contribution, assumptions, problem setting, algorithmic model, and open questions were understood.

## Expected milestone evidence
- Problem statement and motivation
- Core technical idea
- Relevant formalism, algorithm, or model
- Assumptions and scope limitations
- Runtime, memory, scalability, or resource aspects
- Unclear points and questions for feedback

## How to read this file
This Markdown file is a human-readable version of the machine-readable AI usage protocol JSON. It documents only milestone-relevant AI-supported reasoning episodes. It is not a raw chat dump and it is not final paper prose. The important parts are the objective, verification actions, evaluation status, integration decision, and reflection for each interaction.

## Summary of AI usage

### Evaluation status counts
- `accepted`: 1
- `partially-accepted`: 2

### Verification level counts
- `primary-source-check`: 2
- `secondary-source-check`: 1

### Usage type counts
- `summarization`: 1
- `terminology-clarification`: 2

## Interaction overview table
| # | Objective | Verification | Status | Used in work | Section | Usage type |
|---:|---|---|---|---|---|---|
| 1 | Identify the formal problem formulation for multiprocessor scheduling in HLS from Sevaux et al. (2011). | `primary-source-check` | `partially-accepted` | yes | Section 4 (Formal Problem Definition) | `summarization` |
| 2 | Establish the NP-hardness argument and understand why exact methods are insufficient for large HLS instances. | `primary-source-check` | `accepted` | yes | Section 5 (Complexity) | `terminology-clarification` |
| 3 | Map the abstract scheduling formulation to the HLS hardware context and clarify what jobs, processors and time steps represent. | `secondary-source-check` | `partially-accepted` | yes | Section 2 (Context: High-Level Synthesis) | `terminology-clarification` |

## Detailed interaction records

### Interaction 1
- **Objective:** Identify the formal problem formulation for multiprocessor scheduling in HLS from Sevaux et al. (2011).
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
What is the formal problem definition for multiprocessor scheduling in High-Level Synthesis as described by Sevaux, Singh and Rossi (2011)? Include job parameters, constraints and objective.
```

#### AI output summary
AI described the scheduling problem with n jobs on identical parallel processors, each job having release date r_i, due date d_i, processing time p_i and predecessor set PRED_i. Objective is to minimise number of processors m. Outlined time-window, capacity, precedence and binary constraints.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Checked all parameters and constraint formulations directly against Section 1 and Section 2.1 of Sevaux et al. (2011), pp. 201–203.
  - Confirmed job parameter definitions and ILP constraint structure match the paper exactly.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `high`
- **Issues:**
  - AI initially omitted the binary variable constraint C4 and did not include the time horizon formula H = max(d_j - p_j). Both were added manually after checking the paper.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 4 (Formal Problem Definition)
- **Usage type:** `summarization`
- **Direct text reused:** No

#### Reflection
Checking the paper directly revealed two missing constraints. This confirmed that AI output must always be validated against the primary source before use in formal notation.

---

### Interaction 2
- **Objective:** Establish the NP-hardness argument and understand why exact methods are insufficient for large HLS instances.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
Explain why the multiprocessor scheduling problem for HLS is NP-hard, including the reduction from bin packing. Also explain why no PTAS is known.
```

#### AI output summary
AI explained that setting all release dates to zero and all due dates to a common value reduces the problem to 1D bin packing (NP-hard per Garey and Johnson 1979). Version 2 with precedence constraints is also NP-hard as Version 1 is a special case. Stated no known PTAS or performance-guaranteed algorithm exists.

#### Verification
- **Level:** `primary-source-check`
- **Actions:**
  - Verified reduction argument against Section 1 of Sevaux et al. (2011), p. 202.
  - Checked that Garey and Johnson (1979) citation is correctly attributed.
  - Confirmed the statement about no PTAS matches the paper's own claim.

#### Evaluation
- **Status:** `accepted`
- **Usefulness:** `high`
- **Issues:**
  - None — argument matched the paper precisely.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 5 (Complexity)
- **Usage type:** `terminology-clarification`
- **Direct text reused:** No

#### Reflection
The NP-hardness reduction is the key justification for using Tabu Search instead of exact methods. Understanding this argument is essential for defending design choices in later milestones.

---

### Interaction 3
- **Objective:** Map the abstract scheduling formulation to the HLS hardware context and clarify what jobs, processors and time steps represent.
- **AI tool:** Claude
- **AI model:** claude-sonnet-4-6

#### Prompt
```text
How does the abstract multiprocessor scheduling formulation map onto High-Level Synthesis? What do jobs, processors and time steps represent in the HLS context?
```

#### AI output summary
AI explained that operations in a behavioural description map to jobs, functional units such as adders and multipliers map to processors, and clock cycles map to discrete time steps. Output of HLS is an RTL netlist where each component can be a VHDL entity.

#### Verification
- **Level:** `secondary-source-check`
- **Actions:**
  - Verified HLS context description against the course slides (Prof. Rettberg, HSHL, 03.06.26) and the introduction of Sevaux et al. (2011), p. 201.
  - RTL and VHDL mapping confirmed against the course elaboration requirements slide.

#### Evaluation
- **Status:** `partially-accepted`
- **Usefulness:** `medium`
- **Issues:**
  - AI described processors as general compute units without specifying that in HLS they are identical parallel functional units. Corrected this in the submitted text to match the paper's assumption.

#### Integration
- **Used in submitted work:** Yes
- **Section:** Section 2 (Context: High-Level Synthesis)
- **Usage type:** `terminology-clarification`
- **Direct text reused:** No

#### Reflection
Verifying against both the paper and course slides clarified the identical-processor assumption which is a key constraint of the model and must be stated explicitly in the milestone.

---

## Academic integrity note
The interactions documented here were used as support for understanding, structuring, critique, verification, or communication. The submitted milestone artifact does not reuse AI text directly. Scientific claims were checked against the assigned reference (Sevaux et al. 2011), course lecture material, and primary sources before being included in the submitted work.
