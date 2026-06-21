# Implementation

This folder contains the full implementation deliverable: C++17 scheduler and a VHDL RTL application example.

## Build and Run (C++)

```bash
cd implementation/
make
./hls_scheduler
```

Requires g++ with C++17 support (`-std=c++17`). Tested on GCC 12+.

## What the C++ code implements

All five equations from Kopuri & Mansouri (ISCAS 2004) are implemented exactly as printed, with the sign convention from the paper. A `fix_sign` flag in `ACOParams` enables the corrected sign for comparison:

| Equation | Location | Notes |
|---|---|---|
| Eq. 1 – Self-force | `FDSScheduler::self_force()` | Type distribution + Kronecker delta |
| Eq. 2 – PS-force | `FDSScheduler::ps_force()` | Time-frame narrowing for predecessors/successors |
| Eq. 3 – Trail update | `AFDSScheduler::update_trail()` | **Sign inconsistency documented here** |
| Eq. 4 – Global update | `AFDSScheduler::update_global()` | Evaporation factor ρ |
| Eq. 5 – Modified force | `AFDSScheduler::force_il()` | Normalised self-force + trail + global |

## Sign convention note

The `main.cpp` driver runs two experiments:

1. **As-printed** (`fix_sign=false`): reproduces the paper's sign convention where improved decisions receive anti-reinforcement. Results still show A_FDS ≤ FDS.
2. **Corrected** (`fix_sign=true`): negates the Eq. 3 increment, giving positive trail for improvement. Converges faster on DCT benchmarks.

See M3 and M4 for full analysis of the inconsistency.

## Benchmark CDFGs

Four benchmarks are implemented as `CDFG` objects in `scheduler.cpp`:

| Benchmark | Latencies tested | Paper Table 4 result |
|---|---|---|
| Differential Equation | 4, 5 | 875 / 875 (no improvement) |
| Elliptical Filter | 10, 12 | 825→775, 775→775 |
| DCT 1 | 12, 14 | 1795→1730, 1795→1690 |
| DCT 2 | 13, 14 | 1210→1085, 1525→1390 |

Note: the CDFGs in this implementation are structural approximations of the standard HLS benchmark circuits. Exact numerical results will differ from the paper because the original CDFGs and random seeds are not available, but the improvement pattern (A_FDS ≤ FDS) is reproduced.

## VHDL (vhdl/ subfolder)

The `vhdl/` subfolder contains `datapath.vhd`, a synthesisable VHDL entity demonstrating the RTL output of the HLS process. See `vhdl/README.md` for details.
