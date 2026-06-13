# Cross-Traffic Management for Autonomous Vehicles: Requirements Specification

**Team:** Team B6

**Milestone:** Week 1, Requirements and Use-Cases

---

## 1. Purpose and Scope

This document specifies the requirements for an **Autonomous Intersection Management (AIM)** system that schedules autonomous vehicles through a road intersection without collisions.

The requirements are grounded, in order of precedence, first in the course brief and lecture slides [1], which set the topic of cross-traffic management, the queuing methodology (M/M/1 and M/M/2), the communication of autonomous vehicles, and the UPPAAL [3] / C / freeRTOS [4] / ModelSim toolchain, and second in the reference paper by Chouhan and Banda [2] (*Sensors*, 2020), which provides the modeling approach: a decomposition into **vehicles**, an **intersection manager (IM)**, and **collision checkers**, with requirements written so each maps onto a verifiable property.

Scope of this deliverable: functional, non-functional, verification, and constraint requirements, plus assumptions. The use-case diagram is the companion artifact for this milestone.

## 2. System Overview

The system controls a **four-way intersection with three lanes per approach direction** (North, East, South, West), following the reference paper's geometry.

Vehicles approach over time on these lanes and announce their route (their approach and intended turn) to the IM over a wireless link. The IM treats the intersection's crossing capacity as the server of a queuing system and assigns each vehicle a passage schedule so that no two vehicles occupy the same conflict point at the same time. Collision checkers continuously monitor lane and intersection occupancy and flag any overlap.

The queuing view, per the course methodology:

- **Arrivals (M):** vehicles arrive as a stochastic stream (Poisson process, mean rate λ).
- **Service (M):** service time is the time a vehicle occupies the intersection while crossing (exponential, mean rate μ).
- **Servers (1 or 2):** M/M/1 permits one vehicle in the intersection at a time; M/M/2 permits two non-conflicting vehicles in parallel.
- **Discipline:** first-come-first-served (FCFS).

Comparing M/M/1 against M/M/2 on waiting time, throughput, and queue length is the basis for the congestion study (RQ-2).

## 3. Definitions and Terminology

| Term | Meaning |
|---|---|
| AIM | Autonomous Intersection Management |
| IM | Intersection Manager, the central scheduler or "server" |
| CP | Conflict Point, a location where two routes cross inside the intersection |
| Approach | One of the four directions (N, E, S, W) from which vehicles arrive |
| Route | A vehicle's path: an approach plus a turn (straight / left / right) |
| Occupancy vector | Discrete 1-D grid representing which cells of a lane/route are occupied |
| FCFS / FIFS | First-come-first-served service discipline |
| λ | Mean arrival rate of vehicles |
| μ | Mean service rate (intersection crossing) |
| M/M/1 | Single-server queue: one vehicle in the intersection at a time |
| M/M/2 | Two-server queue: two non-conflicting vehicles in the intersection in parallel |

## 4. Assumptions and Design Decisions

- **AD-1: Intersection geometry.** A four-way intersection with **three lanes per approach** (12 inbound lanes total), per the reference paper.
- **AD-2: Lane discipline.** Standard turn-by-lane assignment: the **left lane serves left turns, the centre lane serves straight-through, the right lane serves right turns**. A vehicle's chosen turn therefore determines both its lane and its set of conflict points.
- **AD-3: Turn choice.** Each vehicle takes one of three routes: straight, left, or right.
- **AD-4: Conflict structure.** Right turns are (near) conflict-free; straight-through crosses the perpendicular flows; left turns cross the most flows and are the most conflict-heavy. This ordering determines which vehicle pairs may be served in parallel under M/M/2.
- **AD-5: Queuing model.** Arrivals follow a Poisson process (rate λ); service (crossing) times are exponential (rate μ); discipline is FCFS. The number of servers distinguishes the two cases studied:
  - **M/M/1:** at most one vehicle inside the intersection at any time.
  - **M/M/2:** two vehicles may be inside simultaneously only if their routes share no conflict point.
- **AD-6: Communication.** Reliable, bounded-latency wireless V2I (vehicle-to-infrastructure) between each vehicle and the IM in the baseline. Packet loss and faults are reserved for error-injection testing.
- **AD-7: Vehicle behaviour.** Vehicles obey IM schedules in the baseline; "disobedient" vehicles are an error-injection case for verification, not a baseline requirement.

## 5. Functional Requirements

### 5.1 Vehicle, route, and arrival handling
- **FR-1** The system shall accept vehicles arriving as a stochastic stream on any of the four approaches (N, E, S, W). *(must)*
- **FR-2** Each vehicle shall declare its approach and turn (straight / left / right) on arrival. *(must)*
- **FR-3** Each vehicle shall occupy the lane corresponding to its turn per the lane discipline (AD-2). *(must)*
- **FR-4** The system shall support a configurable mean arrival rate λ. *(should)*
- **FR-5** Vehicles that cannot be served immediately shall wait in a per-approach FCFS queue. *(must)*

### 5.2 Intersection Manager (scheduling)
- **FR-6** The IM shall determine each vehicle's set of conflict points from its route. *(must)*
- **FR-7** The IM shall assign each vehicle a passage schedule (velocity or time slot) before it enters the intersection. *(must)*
- **FR-8** The IM shall grant passage in first-come-first-served order. *(must)*
- **FR-9** In M/M/1 mode, the IM shall permit at most one vehicle inside the intersection at a time. *(must)*
- **FR-10** In M/M/2 mode, the IM shall permit two vehicles inside simultaneously only if their routes share no conflict point. *(must)*
- **FR-11** The IM shall update the lane and intersection occupancy representation as vehicles enter, traverse, and leave. *(must)*
- **FR-12** The IM shall release the server(s) when a vehicle clears the intersection and admit the next queued vehicle. *(must)*

### 5.3 Collision detection
- **FR-13** A lane collision checker shall detect any continuous overlap of two vehicles within the same lane. *(must)*
- **FR-14** An intersection collision checker shall detect simultaneous occupancy of any conflict point by two vehicles. *(must)*
- **FR-15** Collision events shall be counted in dedicated counters (`lane_collisions`, `int_collisions`) for verification. *(must)*

### 5.4 Communication
- **FR-16** Each vehicle shall send its route request to the IM and receive a schedule or grant over the V2I link. *(must)*
- **FR-17** Message exchange shall complete within a bounded time before the vehicle reaches the intersection. *(should)*

### 5.5 Simulation and implementation
- **FR-18** The core scheduling logic shall be implemented in C and runnable as freeRTOS tasks (e.g. arrival-generator task, IM task, collision-checker task). *(must)*
- **FR-19** Simulation parameters (λ, μ, M/M/1 vs M/M/2, run length) shall be configurable without changing core logic. *(should)*
- **FR-20** The simulation shall log per-run metrics: throughput, mean waiting time, queue length, and collision counts. *(should)*

## 6. Non-Functional Requirements

- **NFR-1: Safety (primary).** The intersection shall be free of collisions: across a bounded run, `lane_collisions == 0` and `int_collisions == 0`.
- **NFR-2: Liveness.** Every admitted vehicle shall eventually cross the intersection and terminate (no deadlock, no starvation).
- **NFR-3: Performance and congestion.** Under equal arrival load λ, the M/M/2 policy shall achieve higher throughput and lower mean waiting time than M/M/1 (basis for RQ-2).
- **NFR-4: Real-time.** freeRTOS tasks shall meet their deadlines; the IM shall produce a schedule before the requesting vehicle reaches the stop line.
- **NFR-5: Verifiability.** Every functional requirement shall be expressible as a UPPAAL-SMC query.
- **NFR-6: Modularity.** Vehicle, IM, and collision-checker components shall be separable, enabling stage-wise (layered) verification.
- **NFR-7: Portability.** Core logic shall map from the UPPAAL model to freeRTOS C via a documented, well-defined mapping.

## 7. Verification Requirements (properties to check in UPPAAL-SMC)

- **VR-1 (safety, lane):** `Pr[<=K]([] lane_collisions == 0)` returns maximum confidence. *(verifies FR-13, NFR-1)*
- **VR-2 (safety, intersection):** `Pr[<=K]([] int_collisions == 0)` returns maximum confidence for the complete model. *(verifies FR-9, FR-10, FR-14, NFR-1)*
- **VR-3 (liveness):** every vehicle template reaches a `Cross` or `Terminate` state. *(verifies NFR-2)*
- **VR-4 (invariants):** occupancy and assigned-velocity invariants hold throughout a run. *(verifies FR-11, sane modeling)*
- **VR-5 (error injection):** injecting a fault (mis-scheduling IM, or a disobedient vehicle) yields reduced no-collision confidence, confirming the checker detects unsafe behaviour. *(validates the model itself)*

## 8. Constraints

- **C-1** Formal model in UPPAAL (timed automata / UPPAAL-SMC).
- **C-2** Software in C with freeRTOS; ModelSim for the HW side in the co-design stage.
- **C-3** All work tracked in the team GitHub repository; profs and assistants invited; continuous uploads.
- **C-4** Final paper in IEEE LaTeX/BibTeX, 2 to 4 pages per member, references managed in Zotero/Citavi.
- **C-5** Weekly deliverables: W1 requirements and use-cases, W2 models and diagrams, W3 abstract implementation, W4 HW/SW co-design proposal, W5 co-design implementation.

## 9. Traceability to Research Questions

| Research question | Addressed by |
|---|---|
| RQ-1: Model intersection traffic as a queuing system | AD-5, FR-1, FR-5, FR-9, FR-10 |
| RQ-2: Best scenario to reduce congestion | NFR-3, FR-20, M/M/1 vs M/M/2 comparison |
| RQ-3: Simulate with C-code and freeRTOS, which parameters | FR-18, FR-19, FR-20, NFR-4 |

## 10. Use-Case Overview

The use-case diagram is delivered separately as `UseCaseDiagram_Week1.pdf`. This section states the same model in text so the two documents agree.

**Actors:**

- **Autonomous Vehicle** (primary): requests passage over the V2I link and crosses the intersection.
- **Simulation Controller** (secondary): configures parameters, runs the simulation, and views the logged metrics (supports RQ-3).

The Intersection Manager and Collision Checker are internal components that realize the use cases, not actors. Their structure and interactions are modeled in the Week 2 component, sequence, and activity diagrams.

**Use cases, relationships, and traceability:**

| Use case | Actor or relationship | Requirements |
|---|---|---|
| Request Passage | Autonomous Vehicle | FR-1, FR-2, FR-16 |
| Choose Route | include of Request Passage | FR-2, FR-3 |
| Communicate via V2I | include of Request Passage | FR-16, FR-17 |
| Assign Passage Schedule | include of Request Passage | FR-7 |
| Determine Conflict Points | include of Assign Passage Schedule | FR-6 |
| Apply Server Policy (M/M/1 or M/M/2) | include of Assign Passage Schedule | FR-8, FR-9, FR-10 |
| Queue When Busy | extend of Assign Passage Schedule | FR-5 |
| Cross Intersection | Autonomous Vehicle | FR-11, FR-12 |
| Detect Lane Collision | include of Cross Intersection | FR-13 |
| Detect Intersection Collision | include of Cross Intersection | FR-14 |
| Configure Simulation | Simulation Controller | FR-4, FR-19 |
| Run Simulation | Simulation Controller | FR-18 |
| Log Metrics | include of Run Simulation | FR-20 |
| View Metrics | Simulation Controller | FR-20 |

## 11. References

[1] Course brief and lecture slides, Embedded Electronic Engineering Lab A, University of Applied Sciences Hamm-Lippstadt, Summer Semester 2026.

[2] A. P. Chouhan and G. Banda, "Formal Verification of Heuristic Autonomous Intersection Management Using Statistical Model Checking," *Sensors*, vol. 20, no. 16, art. no. 4506, 2020, doi: 10.3390/s20164506.

[3] A. David, K. G. Larsen, A. Legay, M. Mikučionis, and D. B. Poulsen, "Uppaal SMC tutorial," *International Journal on Software Tools for Technology Transfer*, vol. 17, no. 4, pp. 397-415, 2015, doi: 10.1007/s10009-014-0361-y.

[4] *FreeRTOS Real-Time Kernel*, Amazon Web Services. [Online]. Available: https://www.freertos.org
