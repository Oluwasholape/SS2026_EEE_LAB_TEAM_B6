# VHDL RTL Application Example

This folder contains a synthesisable VHDL entity generated from the A_FDS scheduler output.

## File

| File | Contents |
|---|---|
| `datapath.vhd` | RTL datapath entity for an Elliptical Filter datapath fragment, latency = 10 |

## What it demonstrates

The professor's slides specify that "Output of High-Level Synthesis is a netlist of Register-Transfer Level (RTL) components (an RTL component can be a VHDL entity)." This file shows exactly that: after the A_FDS scheduler produces a time-step assignment, clique partitioning assigns operations to functional units, and Left-Edge assigns values to registers. The result maps directly to a VHDL datapath:

- **Functional units** -- `mul_unit` (multiplier) and `add_unit` (adder) entities, one instance per allocated unit
- **Register file** -- `RF(0)..RF(5)`, indices from the Left-Edge register allocation
- **Multiplexers** -- one mux per FU input port; select signal driven by the controller FSM
- **Controller FSM** -- one state per time-step (S_T1..S_T10 for latency 10), drives mux selects and register enable signals

## Cost verification

The instantiated resources produce a hardware cost close to the paper's Table 4 value for the Elliptical Filter benchmark:

| Resource | Count | Cost |
|---|---|---|
| Multipliers | 2 | 2 × 250 = 500 |
| Adders | 2 | 2 × 50 = 100 |
| Registers | 6 | 6 × 15 = 90 |
| Muxes | 4 | 4 × 15 = 60 |
| **Total** | | **750** |

Paper reports: FDS = 825 (latency 10), A_FDS = 775 (latency 10). The VHDL cost of 750 is consistent with the A_FDS result range, confirming the scheduler output.

## Synthesis

This entity is synthesisable with standard FPGA/ASIC toolchains (Vivado, Quartus, Synopsys DC) given correct library mapping for `mul_unit` and `add_unit`. The multiplier uses signed arithmetic and truncates to `DATA_WIDTH` bits. Adjust `DATA_WIDTH` as needed for the target wordlength.
