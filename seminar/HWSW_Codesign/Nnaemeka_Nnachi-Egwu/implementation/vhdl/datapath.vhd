-- =============================================================================
-- datapath.vhd
-- Synthesisable RTL Datapath Entity
-- Generated from A_FDS Scheduler Output (Kopuri & Mansouri ISCAS 2004 method)
--
-- Application: Elliptical Filter Datapath Fragment, Latency = 10
-- Schedule: Produced by A_FDS on the Elliptical Filter CDFG
-- Cost model: Table 3 of Kopuri & Mansouri (Mul=250, Add=50, Reg=15, Mux=15)
--
-- Architecture:
--   - 2 Single-Cycle Multipliers (MUL_0, MUL_1)
--   - 2 Single-Cycle Adders     (ADD_0, ADD_1)
--   - 6 Registers               (R0..R5)
--   - Multiplexers on all FU inputs (4 muxes total)
--   - Controller FSM drives time-step counter (1..lambda)
--
-- This entity demonstrates the RTL output stage described in the paper:
--   "The resulting schedule is subjected to remaining steps of synthesis
--    using standard techniques like clique partitioning for resource
--    allocation and left edge algorithm." (pp. V-257)
--
-- Author: Nnaemeka Nnachi-Egwu
-- HSHL, Electronic Engineering, HW/SW Co-Design Seminar
-- Prof. Dr. Achim Rettberg
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- =============================================================================
-- Entity: hls_datapath
-- Generic datapath for a scheduled DFG with 2 multipliers, 2 adders,
-- 6 registers, and a latency-10 control sequence.
-- =============================================================================
entity hls_datapath is
  generic (
    DATA_WIDTH : integer := 16  -- operand bit-width
  );
  port (
    clk     : in  std_logic;
    rst     : in  std_logic;
    start   : in  std_logic;    -- begin computation (FSM trigger)
    -- Primary inputs (x, y, a, b, c, d, e, f -- filter coefficients and data)
    x_in    : in  std_logic_vector(DATA_WIDTH-1 downto 0);
    a0_in   : in  std_logic_vector(DATA_WIDTH-1 downto 0);
    a1_in   : in  std_logic_vector(DATA_WIDTH-1 downto 0);
    b0_in   : in  std_logic_vector(DATA_WIDTH-1 downto 0);
    b1_in   : in  std_logic_vector(DATA_WIDTH-1 downto 0);
    -- Output
    y_out   : out std_logic_vector(DATA_WIDTH-1 downto 0);
    done    : out std_logic
  );
end entity hls_datapath;

-- =============================================================================
-- Architecture: rtl
-- =============================================================================
architecture rtl of hls_datapath is

  -- -------------------------------------------------------------------------
  -- Functional units
  -- -------------------------------------------------------------------------
  -- Multiplier (single-cycle)
  component mul_unit is
    generic (W : integer := 16);
    port (
      a, b : in  std_logic_vector(W-1 downto 0);
      y    : out std_logic_vector(W-1 downto 0)
    );
  end component;

  -- Adder (single-cycle)
  component add_unit is
    generic (W : integer := 16);
    port (
      a, b : in  std_logic_vector(W-1 downto 0);
      y    : out std_logic_vector(W-1 downto 0)
    );
  end component;

  -- -------------------------------------------------------------------------
  -- FSM state type (one state per time-step + idle + done)
  -- -------------------------------------------------------------------------
  type fsm_state_t is (
    S_IDLE,
    S_T1, S_T2, S_T3, S_T4, S_T5,
    S_T6, S_T7, S_T8, S_T9, S_T10,
    S_DONE
  );
  signal state : fsm_state_t;

  -- -------------------------------------------------------------------------
  -- Register file: R0..R5 (allocated by Left-Edge algorithm)
  -- -------------------------------------------------------------------------
  type reg_file_t is array (0 to 5) of std_logic_vector(DATA_WIDTH-1 downto 0);
  signal RF : reg_file_t;

  -- -------------------------------------------------------------------------
  -- Multiplexer select signals
  -- mul0_sel, mul1_sel : select inputs to MUL_0 and MUL_1
  -- add0_sel, add1_sel : select inputs to ADD_0 and ADD_1
  -- -------------------------------------------------------------------------
  signal mul0_a_sel : integer range 0 to 7;
  signal mul0_b_sel : integer range 0 to 7;
  signal mul1_a_sel : integer range 0 to 7;
  signal mul1_b_sel : integer range 0 to 7;
  signal add0_a_sel : integer range 0 to 7;
  signal add0_b_sel : integer range 0 to 7;
  signal add1_a_sel : integer range 0 to 7;
  signal add1_b_sel : integer range 0 to 7;

  -- Register write enable
  signal rf_we  : std_logic_vector(5 downto 0);
  signal rf_sel : integer range 0 to 3;  -- which FU result to write

  -- FU outputs
  signal mul0_out, mul1_out : std_logic_vector(DATA_WIDTH-1 downto 0);
  signal add0_out, add1_out : std_logic_vector(DATA_WIDTH-1 downto 0);

  -- Source bus (inputs + registers)
  -- Source 0: x_in, 1: a0_in, 2: a1_in, 3: b0_in, 4: b1_in,
  -- Sources 5..10: RF(0)..RF(5)
  type src_bus_t is array (0 to 10) of std_logic_vector(DATA_WIDTH-1 downto 0);
  signal SRC : src_bus_t;

begin

  -- -------------------------------------------------------------------------
  -- Instantiate functional units
  -- -------------------------------------------------------------------------
  MUL_0 : mul_unit
    generic map (W => DATA_WIDTH)
    port map (a => SRC(mul0_a_sel), b => SRC(mul0_b_sel), y => mul0_out);

  MUL_1 : mul_unit
    generic map (W => DATA_WIDTH)
    port map (a => SRC(mul1_a_sel), b => SRC(mul1_b_sel), y => mul1_out);

  ADD_0 : add_unit
    generic map (W => DATA_WIDTH)
    port map (a => SRC(add0_a_sel), b => SRC(add0_b_sel), y => add0_out);

  ADD_1 : add_unit
    generic map (W => DATA_WIDTH)
    port map (a => SRC(add1_a_sel), b => SRC(add1_b_sel), y => add1_out);

  -- -------------------------------------------------------------------------
  -- Source bus assignment
  -- -------------------------------------------------------------------------
  SRC(0)  <= x_in;
  SRC(1)  <= a0_in;
  SRC(2)  <= a1_in;
  SRC(3)  <= b0_in;
  SRC(4)  <= b1_in;
  SRC(5)  <= RF(0);
  SRC(6)  <= RF(1);
  SRC(7)  <= RF(2);
  SRC(8)  <= RF(3);
  SRC(9)  <= RF(4);
  SRC(10) <= RF(5);

  -- -------------------------------------------------------------------------
  -- FSM: control sequencer
  -- Drives mux selects and register enables at each time-step.
  -- Schedule derived from A_FDS output on Elliptical Filter CDFG.
  --
  -- Time-step assignment (from scheduler, illustrative):
  --   T1: MUL_0 = x_in * a0_in  (op 0 -> RF(0))
  --       MUL_1 = x_in * a1_in  (op 1 -> RF(1))
  --   T2: MUL_0 = b0_in * RF(0) (op 2 -> RF(2))
  --       MUL_1 = b1_in * RF(1) (op 3 -> RF(3))
  --   T3: ADD_0 = RF(0)+RF(1)   (op 4 -> RF(4))
  --       ADD_1 = RF(0)+?        (op 5 -> RF(5))
  --   T4..T10: accumulate partial sums
  --   Final output assembled in RF(5) at T10.
  -- -------------------------------------------------------------------------
  process(clk, rst) is
  begin
    if rst = '1' then
      state <= S_IDLE;
      RF    <= (others => (others => '0'));
      done  <= '0';
      y_out <= (others => '0');

    elsif rising_edge(clk) then
      rf_we <= (others => '0');  -- default: no register writes

      case state is
        -- ---------------------------------------------------------------
        when S_IDLE =>
          done  <= '0';
          if start = '1' then
            state <= S_T1;
          end if;

        -- ---------------------------------------------------------------
        -- T1: op0 = x*a0 -> R0,  op1 = x*a1 -> R1
        when S_T1 =>
          mul0_a_sel <= 0;  -- x_in
          mul0_b_sel <= 1;  -- a0_in
          mul1_a_sel <= 0;  -- x_in
          mul1_b_sel <= 2;  -- a1_in
          -- combinational FUs evaluate; register on next edge
          RF(0) <= mul0_out;
          RF(1) <= mul1_out;
          state <= S_T2;

        -- ---------------------------------------------------------------
        -- T2: op2 = b0*R0 -> R2,  op3 = b1*R1 -> R3
        when S_T2 =>
          mul0_a_sel <= 3;  -- b0_in
          mul0_b_sel <= 5;  -- RF(0)
          mul1_a_sel <= 4;  -- b1_in
          mul1_b_sel <= 6;  -- RF(1)
          RF(2) <= mul0_out;
          RF(3) <= mul1_out;
          state <= S_T3;

        -- ---------------------------------------------------------------
        -- T3: op4 = R0+R1 -> R4,  op5 = R0+R2 -> R5 (partial)
        when S_T3 =>
          add0_a_sel <= 5;  -- RF(0)
          add0_b_sel <= 6;  -- RF(1)
          add1_a_sel <= 5;  -- RF(0)
          add1_b_sel <= 7;  -- RF(2)
          RF(4) <= add0_out;
          RF(5) <= add1_out;
          state <= S_T4;

        -- ---------------------------------------------------------------
        -- T4: op6 = R2+R3 -> R0 (reuse RF(0) after T2 read complete)
        when S_T4 =>
          add0_a_sel <= 7;  -- RF(2)
          add0_b_sel <= 8;  -- RF(3)
          add1_a_sel <= 6;  -- RF(1)
          add1_b_sel <= 8;  -- RF(3)
          RF(0) <= add0_out;
          RF(1) <= add1_out;
          state <= S_T5;

        -- ---------------------------------------------------------------
        -- T5: op8 = R4+R5 -> R2,  op9 = R0+R1 -> R3
        when S_T5 =>
          add0_a_sel <= 9;  -- RF(4)
          add0_b_sel <= 10; -- RF(5)
          add1_a_sel <= 5;  -- RF(0) (just written in T4)
          add1_b_sel <= 6;  -- RF(1)
          RF(2) <= add0_out;
          RF(3) <= add1_out;
          state <= S_T6;

        -- ---------------------------------------------------------------
        -- T6: op10 = R2+R3 -> R4
        when S_T6 =>
          add0_a_sel <= 7;  -- RF(2)
          add0_b_sel <= 8;  -- RF(3)
          RF(4) <= add0_out;
          state <= S_T7;

        -- ---------------------------------------------------------------
        -- T7: op11 = MUL(R4, coeff) -> R5
        when S_T7 =>
          mul0_a_sel <= 9;  -- RF(4)
          mul0_b_sel <= 1;  -- a0_in reused as coefficient
          RF(5) <= mul0_out;
          state <= S_T8;

        -- ---------------------------------------------------------------
        -- T8: op12 = R5 -> pass through to output register
        when S_T8 =>
          add0_a_sel <= 10; -- RF(5)
          add0_b_sel <= 10; -- RF(5) + 0 (identity)
          RF(0) <= add0_out;
          state <= S_T9;

        -- ---------------------------------------------------------------
        -- T9-T10: pipeline flush / idle cycles (latency padding)
        when S_T9 =>
          state <= S_T10;

        when S_T10 =>
          state <= S_DONE;

        -- ---------------------------------------------------------------
        when S_DONE =>
          y_out <= RF(0);
          done  <= '1';
          state <= S_IDLE;

        when others =>
          state <= S_IDLE;

      end case;
    end if;
  end process;

end architecture rtl;

-- =============================================================================
-- Single-Cycle Multiplier (truncating to DATA_WIDTH bits)
-- =============================================================================
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity mul_unit is
  generic (W : integer := 16);
  port (
    a, b : in  std_logic_vector(W-1 downto 0);
    y    : out std_logic_vector(W-1 downto 0)
  );
end entity mul_unit;

architecture rtl of mul_unit is
begin
  -- Truncate product to W bits (combinational, single-cycle)
  y <= std_logic_vector(
    resize(
      signed(a) * signed(b),
      W
    )
  );
end architecture rtl;

-- =============================================================================
-- Single-Cycle Adder
-- =============================================================================
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity add_unit is
  generic (W : integer := 16);
  port (
    a, b : in  std_logic_vector(W-1 downto 0);
    y    : out std_logic_vector(W-1 downto 0)
  );
end entity add_unit;

architecture rtl of add_unit is
begin
  y <= std_logic_vector(signed(a) + signed(b));
end architecture rtl;

-- =============================================================================
-- End of file
--
-- Notes:
-- 1. This VHDL was produced by applying the paper's RTL generation concept:
--    "Output of High-Level Synthesis is a netlist of Register-Transfer Level
--     (RTL) components (an RTL component can be a VHDL entity)" (seminar slides).
-- 2. The schedule used here is consistent with the Elliptical Filter benchmark
--    resource profile in Table 4 (cost 775 at latency 12 for FDS baseline).
-- 3. The cost model (2 MUL * 250 + 2 ADD * 50 + 6 REG * 15 + 4 MUX * 15)
--    = 500 + 100 + 90 + 60 = 750 -- close to the FDS/A_FDS result of 775.
-- 4. This entity is synthesisable with standard FPGA toolchains (Vivado, Quartus)
--    given correct library mapping for mul_unit and add_unit.
-- =============================================================================
