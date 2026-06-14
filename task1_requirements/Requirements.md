# Cross-Traffic Management for Autonomous Vehicles: Requirements Specification

**Team:** Team B6

**Milestone:** Week 1, Requirements and Use-Cases

---

## 1. Purpose and Scope

This document specifies the requirements for an **Autonomous Intersection Management (AIM)** system that schedules autonomous vehicles through a four-way intersection without collisions.

The requirements are grounded, in order of precedence, first in the course brief and lecture slides [1], which set the topic of cross-traffic management, the queuing methodology (M/M/c), the communication of autonomous vehicles, and the UPPAAL [3] / C / freeRTOS [4] / ModelSim toolchain, and second in the reference paper by Chouhan and Banda [2] (*Sensors*, 2020), which provides the conflict-point reservation mechanism, the decomposition into vehicles, an intersection manager (IM), and collision checkers, and the verification approach.

The model is unified in two layers:

- A **macro queuing layer** that treats the intersection as an M/M/c server and answers the congestion question (RQ-2).
- A **micro scheduling and safety layer** that gives every conflict point its own FIFO reservation queue and guarantees no two vehicles meet at the same point.

## 2. System Overview

The system controls a **four-way intersection with three lanes per approach direction** (North, East, South, West). The team's intersection drawing identifies **16 conflict points (CPs)**, the locations where two vehicle routes cross. Right-turn movements are conflict-free and contribute no CPs.

**Macro layer (queuing).** The intersection is modelled as an M/M/c queue, where the number of servers c is the number of vehicles allowed to cross concurrently:

- **M/M/4** is the main model: up to four vehicles cross at once, one stream per approach. The four servers correspond to the four approaches of the intersection.
- **M/M/1** (serialize the intersection, one vehicle at a time) and **M/M/2** (two concurrent vehicles) are studied as comparison cases, as named in the slides.

Arrivals follow a Poisson process (rate lambda) and service (crossing) times are exponential (rate mu). M/M/4 is an idealized capacity model; the four servers are not fully independent, because two crossing routes can conflict. The micro layer below captures that coupling and provides the actual safety guarantee.

**Micro layer (scheduling and safety).** Each of the 16 conflict points has its own **FIFO reservation queue** (the paper's reservation record, and the "queue for every critical situation" requested in review). Before entering, a vehicle reserves a time slot at every conflict point on its route, in first-enter-first-serve order. It crosses only when it holds a clear, non-overlapping slot at all of those points. Collision checkers monitor lane and conflict-point occupancy and flag any overlap.

**Communication.** Vehicles exchange route and timing information with the IM over a V2I (vehicle-to-infrastructure) link. The V2V (vehicle-to-vehicle) alternative shown in the course slides is out of scope, since the design is centralized around the IM.

## 3. Definitions and Terminology

| Term | Meaning |
|---|---|
| AIM | Autonomous Intersection Management |
| IM | Intersection Manager, the central coordinator that issues reservations |
| Approach | One of the four directions (N, E, S, W) from which vehicles arrive |
| Route | A vehicle's path: an approach plus a turn (straight / left / right) |
| Conflict point (CP) | One of the 16 points where two routes cross; a place a collision can occur |
| Critical situation | A conflict point; a configuration where two routes could collide |
| Reservation queue | The FIFO record of time-slot reservations held at one conflict point |
| FIFO / FEFS | First-in-first-out (first-enter-first-serve) service discipline |
| V2I | Vehicle-to-infrastructure communication (vehicle to IM) |
| lambda | Mean arrival rate of vehicles |
| mu | Mean service rate (intersection crossing) |
| M/M/1, M/M/2, M/M/4 | The intersection modelled with 1, 2, or 4 concurrent crossing servers |

## 4. Assumptions and Design Decisions

- **AD-1: Geometry.** A four-way intersection with three lanes per approach.
- **AD-2: Conflict points.** The intersection has 16 conflict points, enumerated in the team's intersection drawing. Right-turn movements contribute none. The count of 16 follows the standard lane-to-movement layout also used by the reference paper.
- **AD-3: Movements.** Each vehicle takes one of three routes: straight, left, or right. The set of routes produces the 16 conflict points in AD-2.
- **AD-4: Macro queuing model.** The intersection is an M/M/c queue with Poisson arrivals (rate lambda) and exponential service (rate mu). The main case is **M/M/4** (c = 4, one server per approach). **M/M/1** and **M/M/2** are studied as comparison cases. Increasing c is expected to reduce congestion, which is the basis for RQ-2.
- **AD-5: Micro scheduling and safety.** Each conflict point has one FIFO reservation queue. A vehicle reserves a slot at every conflict point on its route, in first-enter-first-serve order, and may cross only when all of those slots are clear and non-overlapping. This per-conflict-point reservation is what makes concurrent crossings collision-free.
- **AD-6: Communication.** Reliable, bounded-latency V2I between each vehicle and the IM in the baseline. V2V is out of scope. Packet loss and faults are reserved for error-injection testing.
- **AD-7: Safety scenario.** The verification stress test is one car per lane (the team's drawing), the worst-case simultaneous demand, covering the full set of turn combinations.
- **AD-8: Vehicle behaviour.** Vehicles obey IM reservations in the baseline; disobedient vehicles are an error-injection case for verification, not a baseline requirement.

## 5. Functional Requirements

### 5.1 Vehicle, route, and arrival handling
- **FR-1** The system shall accept vehicles arriving as a Poisson stream on any of the four approaches (N, E, S, W). *(must)*
- **FR-2** Each vehicle shall declare its approach and turn (straight / left / right) on arrival. *(must)*
- **FR-3** Each vehicle's route shall map to the set of conflict points it crosses. *(must)*
- **FR-4** The system shall support the one-car-per-lane safety scenario across all turn combinations. *(must)*
- **FR-5** The mean arrival rate lambda shall be configurable. *(should)*

### 5.2 Intersection Manager (conflict-point reservation)
- **FR-6** The IM shall determine the conflict points on each vehicle's route. *(must)*
- **FR-7** The IM shall maintain one FIFO reservation queue per conflict point (16 in total). *(must)*
- **FR-8** The IM shall reserve a time slot for each vehicle at every conflict point on its route, in first-enter-first-serve order. *(must)*
- **FR-9** A vehicle shall be admitted to the intersection only when it holds a clear, non-overlapping reservation at every conflict point on its route. *(must)*
- **FR-10** The IM shall allow up to four vehicles in the intersection concurrently (M/M/4), and shall support capping concurrency at one (M/M/1) or two (M/M/2) for the comparison study. *(must)*
- **FR-11** The IM shall release a conflict point's reservation as the vehicle clears that point. *(must)*

### 5.3 Collision detection
- **FR-12** A lane collision checker shall detect any continuous overlap of two vehicles within the same lane. *(must)*
- **FR-13** An intersection collision checker shall detect two vehicles occupying the same conflict point at the same time. *(must)*
- **FR-14** Collision events shall be counted in dedicated counters (`lane_collisions`, `int_collisions`) for verification. *(must)*

### 5.4 Communication
- **FR-15** Each vehicle shall send its route to the IM and receive its reservations or grant over the V2I link. *(must)*
- **FR-16** Message exchange shall complete within a bounded time before the vehicle reaches the intersection. *(should)*

### 5.5 Simulation and implementation
- **FR-17** The core reservation and collision logic shall be implemented in C and runnable as freeRTOS tasks (e.g. arrival-generator task, IM task, collision-checker task). *(must)*
- **FR-18** Simulation parameters (lambda, mu, the concurrency c in {1, 2, 4}, run length) shall be configurable without changing core logic. *(should)*
- **FR-19** The simulation shall log per-run metrics: throughput, mean waiting time, per-conflict-point queue length, and collision counts. *(should)*

## 6. Non-Functional Requirements

- **NFR-1: Safety (primary).** The intersection shall be free of collisions: across a run, `lane_collisions == 0` and `int_collisions == 0`, including the one-car-per-lane scenario for all turn combinations. No two vehicles shall hold overlapping slots at the same conflict point.
- **NFR-2: Liveness.** Every admitted vehicle shall eventually cross and terminate (no deadlock, no starvation).
- **NFR-3: Performance and congestion.** Increasing the concurrency from M/M/1 to M/M/2 to M/M/4 shall reduce mean waiting time and increase throughput, with M/M/4 the best case (basis for RQ-2).
- **NFR-4: Real-time.** freeRTOS tasks shall meet their deadlines; the IM shall issue reservations before the requesting vehicle reaches the stop line.
- **NFR-5: Verifiability.** Every functional requirement shall be expressible as a UPPAAL-SMC query.
- **NFR-6: Modularity.** Vehicle, IM, and collision-checker components shall be separable, enabling stage-wise (layered) verification.
- **NFR-7: Portability.** Core logic shall map from the UPPAAL model to freeRTOS C via a documented, well-defined mapping.

## 7. Verification Requirements (properties to check in UPPAAL-SMC)

- **VR-1 (safety, lane):** `Pr[<=K]([] lane_collisions == 0)` returns maximum confidence. *(verifies FR-12, NFR-1)*
- **VR-2 (safety, conflict points):** `Pr[<=K]([] int_collisions == 0)` returns maximum confidence; no two vehicles share a conflict-point slot. *(verifies FR-9, FR-13, NFR-1)*
- **VR-3 (liveness):** every vehicle template reaches a `Cross` or `Terminate` state. *(verifies NFR-2)*
- **VR-4 (reservation invariants):** at every conflict point, reservations stay FIFO-ordered and never overlap in time. *(verifies FR-7, FR-8)*
- **VR-5 (error injection):** injecting a fault (mis-issued reservation, or a disobedient vehicle) yields reduced no-collision confidence, confirming the checker detects unsafe behaviour. *(validates the model itself)*

## 8. Constraints

- **C-1** Formal model in UPPAAL (timed automata / UPPAAL-SMC).
- **C-2** Software in C with freeRTOS; ModelSim for the HW side in the co-design stage.
- **C-3** All work tracked in the team GitHub repository; profs and assistants invited; continuous uploads.
- **C-4** Final paper in IEEE LaTeX/BibTeX, 2 to 4 pages per member, references managed in Zotero/Citavi.
- **C-5** Weekly deliverables: W1 requirements and use-cases, W2 models and diagrams, W3 abstract implementation, W4 HW/SW co-design proposal, W5 co-design implementation.

## 9. Traceability to Research Questions

| Research question | Addressed by |
|---|---|
| RQ-1: Model intersection traffic as a queuing system | AD-4, AD-5, FR-7, FR-8, FR-9, FR-10 |
| RQ-2: Best scenario to reduce congestion | NFR-3, FR-18, FR-19, M/M/1 vs M/M/2 vs M/M/4 comparison |
| RQ-3: Simulate with C-code and freeRTOS, which parameters | FR-17, FR-18, FR-19, NFR-4 |

## 10. Use-Case Overview

The use-case diagram is delivered separately as `UseCaseDiagram.pdf` and will be regenerated to match this unified model. The actors and use cases are:

**Actors:**

- **Autonomous Vehicle** (primary): requests passage over V2I and crosses the intersection.
- **Simulation Controller** (secondary): configures parameters (including the concurrency c), runs the simulation, and views the logged metrics.

The Intersection Manager and Collision Checker are internal components that realize the use cases, not actors.

**Use cases, relationships, and traceability:**

| Use case | Actor or relationship | Requirements |
|---|---|---|
| Request Passage | Autonomous Vehicle | FR-1, FR-2, FR-15 |
| Choose Route | include of Request Passage | FR-2 |
| Communicate via V2I | include of Request Passage | FR-15, FR-16 |
| Reserve Conflict-Point Slots | include of Request Passage | FR-8 |
| Determine Route Conflict Points | include of Reserve Conflict-Point Slots | FR-6 |
| Wait for Conflict-Point Slot | extend of Reserve Conflict-Point Slots | FR-7, FR-9 |
| Cross Intersection | Autonomous Vehicle | FR-9, FR-11 |
| Detect Lane Collision | include of Cross Intersection | FR-12 |
| Detect Intersection Collision | include of Cross Intersection | FR-13 |
| Configure Simulation | Simulation Controller | FR-5, FR-18 |
| Run Simulation | Simulation Controller | FR-17 |
| Log Metrics | include of Run Simulation | FR-19 |
| View Metrics | Simulation Controller | FR-19 |

## 11. References

[1] Course brief and lecture slides, Embedded Electronic Engineering Lab A, University of Applied Sciences Hamm-Lippstadt, Summer Semester 2026.

[2] A. P. Chouhan and G. Banda, "Formal Verification of Heuristic Autonomous Intersection Management Using Statistical Model Checking," *Sensors*, vol. 20, no. 16, art. no. 4506, 2020, doi: 10.3390/s20164506.

[3] A. David, K. G. Larsen, A. Legay, M. Mikučionis, and D. B. Poulsen, "Uppaal SMC tutorial," *International Journal on Software Tools for Technology Transfer*, vol. 17, no. 4, pp. 397-415, 2015, doi: 10.1007/s10009-014-0361-y.

[4] *FreeRTOS Real-Time Kernel*, Amazon Web Services. [Online]. Available: https://www.freertos.org
