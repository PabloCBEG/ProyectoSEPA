-- #################################################################################################
-- # << NEORV32 - Example setup including the bootloader, for the iCEBreaker (c) Board >>          #
-- # ********************************************************************************************* #
-- # BSD 3-Clause License                                                                          #
-- #                                                                                               #
-- # Copyright (c) 2021, Stephan Nolting. All rights reserved.                                     #
-- #                                                                                               #
-- # Redistribution and use in source and binary forms, with or without modification, are          #
-- # permitted provided that the following conditions are met:                                     #
-- #                                                                                               #
-- # 1. Redistributions of source code must retain the above copyright notice, this list of        #
-- #    conditions and the following disclaimer.                                                   #
-- #                                                                                               #
-- # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
-- #    conditions and the following disclaimer in the documentation and/or other materials        #
-- #    provided with the distribution.                                                            #
-- #                                                                                               #
-- # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
-- #    endorse or promote products derived from this software without specific prior written      #
-- #    permission.                                                                                #
-- #                                                                                               #
-- # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
-- # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
-- # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
-- # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
-- # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
-- # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
-- # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
-- # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
-- # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
-- # ********************************************************************************************* #
-- # The NEORV32 Processor - https://github.com/stnolting/neorv32              (c) Stephan Nolting #
-- #################################################################################################

-- Created by Hipolito Guzman-Miranda based on Unai Martinez-Corral's adaptation to the iCESugar board

-- Allow use of std_logic, signed, unsigned
--library myworks;
--myworks.my_package.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


-- Library where the processor is located
library neorv32;



entity neorv32_iCEBreaker_BoardTop_MinimalBoot is
  -- Top-level ports. Board pins are defined in setups/osflow/constraints/iCEBreaker.pcf
  port (
    -- 12MHz Clock input
    iCEBreakerv10_CLK                : in std_logic;
    -- UART0
    iCEBreakerv10_RX                 : in  std_logic;
    iCEBreakerv10_TX                 : out std_logic;
    -- Button inputs
    iCEBreakerv10_BTN_N              : in std_logic;
    iCEBreakerv10_PMOD2_9_Button_1   : in std_logic;
    iCEBreakerv10_PMOD2_10_Button_3  : in std_logic;

    --Pines de entrada
    iCEBreakerv10_PMOD1B_7           : in std_logic;
    iCEBreakerv10_PMOD1B_8           : in std_logic;
    iCEBreakerv10_PMOD1B_10          : in std_logic;
    iCEBreakerv10_PMOD1B_9           : in std_logic;
    --Pines de salida
    iCEBreakerv10_PMOD1B_1 : out std_logic;
    iCEBreakerv10_PMOD1B_2 : out std_logic;
    iCEBreakerv10_PMOD1B_3 : out std_logic;
    iCEBreakerv10_PMOD1B_4 : out std_logic;

    --Pines de la interfaz Wishbone


    -- LED outputs
    iCEBreakerv10_LED_R_N            : out std_logic;
    iCEBreakerv10_LED_G_N            : out std_logic;
    iCEBreakerv10_PMOD2_1_LED_left   : out std_logic;
    iCEBreakerv10_PMOD2_2_LED_right  : out std_logic;
    iCEBreakerv10_PMOD2_8_LED_up     : out std_logic;
    iCEBreakerv10_PMOD2_3_LED_down   : out std_logic;
    iCEBreakerv10_PMOD2_7_LED_center : out std_logic
  );
end entity;

architecture neorv32_iCEBreaker_BoardTop_MinimalBoot_rtl of neorv32_iCEBreaker_BoardTop_MinimalBoot is

  -- -------------------------------------------------------------------------------------------
  -- Constants for microprocessor configuration
  -- -------------------------------------------------------------------------------------------

  -- General config --
  constant CLOCK_FREQUENCY              : natural := 12_000_000;  -- Microprocessor clock frequency (Hz)
  constant INT_BOOTLOADER_EN            : boolean := true;        -- boot configuration: true = boot explicit bootloader; false = boot from int/ext (I)MEM
  constant HW_THREAD_ID                 : natural := 0;           -- hardware thread id (32-bit)

  -- RISC-V CPU Extensions --
  constant CPU_EXTENSION_RISCV_A        : boolean := true;        -- implement atomic extension?
  constant CPU_EXTENSION_RISCV_C        : boolean := true;        -- implement compressed extension?
  constant CPU_EXTENSION_RISCV_E        : boolean := false;       -- implement embedded RF extension?
  constant CPU_EXTENSION_RISCV_M        : boolean := true;        -- implement mul/div extension?
  constant CPU_EXTENSION_RISCV_U        : boolean := false;       -- implement user mode extension?
  constant CPU_EXTENSION_RISCV_Zfinx    : boolean := false;       -- implement 32-bit floating-point extension (using INT regs!)
  constant CPU_EXTENSION_RISCV_Zicsr    : boolean := true;        -- implement CSR system?
  constant CPU_EXTENSION_RISCV_Zifencei : boolean := false;       -- implement instruction stream sync.?

  -- Extension Options --
  constant FAST_MUL_EN                  : boolean := false;       -- use DSPs for M extension's multiplier
  constant FAST_SHIFT_EN                : boolean := false;       -- use barrel shifter for shift operations
  constant CPU_CNT_WIDTH                : natural := 34;          -- total width of CPU cycle and instret counters (0..64)

  -- Physical Memory Protection (PMP) --
  constant PMP_NUM_REGIONS              : natural := 0;            -- number of regions (0..64)
  constant PMP_MIN_GRANULARITY          : natural := 64*1024;      -- minimal region granularity in bytes, has to be a power of 2, min 8 bytes

  -- Hardware Performance Monitors (HPM) --
  constant HPM_NUM_CNTS                 : natural := 0;            -- number of implemented HPM counters (0..29)
  constant HPM_CNT_WIDTH                : natural := 40;           -- total size of HPM counters (0..64)

  -- Internal Instruction memory --
  constant MEM_INT_IMEM_EN              : boolean := true;         -- implement processor-internal instruction memory
  constant MEM_INT_IMEM_SIZE            : natural := 64*1024;      -- size of processor-internal instruction memory in bytes

  -- Internal Data memory --
  constant MEM_INT_DMEM_EN              : boolean := true;         -- implement processor-internal data memory
  constant MEM_INT_DMEM_SIZE            : natural := 8*1024;      -- size of processor-internal data memory in bytes

  -- Internal Cache memory --
  constant ICACHE_EN                    : boolean := false;       -- implement instruction cache
  constant ICACHE_NUM_BLOCKS            : natural := 4;           -- i-cache: number of blocks (min 1), has to be a power of 2
  constant ICACHE_BLOCK_SIZE            : natural := 64;          -- i-cache: block size in bytes (min 4), has to be a power of 2
  constant ICACHE_ASSOCIATIVITY         : natural := 1;           -- i-cache: associativity / number of sets (1=direct_mapped), has to be a power of 2

  -- Processor peripherals --
  constant IO_GPIO_EN                   : boolean := true;        -- implement general purpose input/output port unit (GPIO)?
  constant IO_MTIME_EN                  : boolean := true;        -- implement machine system timer (MTIME)?
  constant IO_UART0_EN                  : boolean := true;        -- implement primary universal asynchronous receiver/transmitter (UART0)?
  constant IO_PWM_NUM_CH                : natural := 3;           -- number of PWM channels to implement (0..60); 0 = disabled
  constant IO_WDT_EN                    : boolean := true;        -- implement watch dog timer (WDT)?

  -- -------------------------------------------------------------------------------------------
  -- Signals for internal IO connections
  -- -------------------------------------------------------------------------------------------

  --Senales p2
  signal gpio_o : std_ulogic_vector(63 downto 0);
  signal gpio_i : std_ulogic_vector(63 downto 0);
  signal gpio_i_int : std_ulogic_vector(63 downto 0);

  signal gpio_i_int_2 : std_ulogic_vector(63 downto 0);

  --Senales p3
  --Senales necesarias para el bus wishbone
  signal wb_adr_m2s_signal : std_ulogic_vector(31 downto 0);
  signal wb_dat_s2m_signal : std_ulogic_vector(31 downto 0);
  signal wb_dat_m2s_signal : std_ulogic_vector(31 downto 0);
  signal wb_we_m2s_signal : std_logic;
  signal wb_sel_m2s_signal : std_ulogic_vector(3 downto 0);
  signal wb_stb_m2s_signal : std_logic;
  signal wb_cyc_m2s_signal : std_logic;
  signal wb_lock_m2s_signal : std_logic;
  signal wb_ack_s2m_signal : std_logic;
  signal wb_err_s2m_signal : std_logic;
  ----------------------------------------------------------------------------------------------
  --Instancia de los componentes a usar

  --Debouncer:
  component debouncer is
    Port(
    clk : in std_logic;
    reset : in std_logic;
    button_input : in std_logic;
    button_out : out std_logic
    );

  end component;

  --Rising edge detector
  
  component rising_edge_detector is
    Port(
    clk : in std_logic;
    reset : in std_logic;
    button_input : in std_logic;
    button_output : out std_logic
    );
  end component;

  --Controlador escritura/lectura teclado
  component controlador_teclado is
    Generic(
      numero_pines   : integer; --Numero de pines sobre los que tenemos que escribir y leer
      numero_elementos_teclado : integer  --Numero de teclas en el teclado
    );
    Port ( 
      clk : in std_logic; --Puerto de reloj
      reset : in std_logic; --Puerto de reseteo

      --Pines para la comunicacion con el teclado
      pines_lectura_teclado   : in  std_logic_vector(numero_pines-1 downto 0);
      pines_escritura_teclado : out std_logic_vector(numero_pines-1 downto 0);

      --Pines para la comunicacion con el microprocesador

      pines_escritura_micro   : out std_logic_vector(numero_elementos_teclado-1 downto 0)

  );
  end component;

  --Periferico Wishbone

--  component wb_regs is
--    generic (
--      WB_ADDR_BASE : std_ulogic_vector(31 downto 0); -- module base address, size-aligned
--      WB_ADDR_SIZE : positive                        -- module address space in bytes, has to be a power of two, min 4
--    );
--    port (
--      -- wishbone host interface --
--      wb_clk_i  : in  std_ulogic;                     -- clock
--      wb_rstn_i : in  std_ulogic;                     -- reset, async, low-active
--      wb_adr_i  : in  std_ulogic_vector(31 downto 0); -- address
--     wb_dat_i  : in  std_ulogic_vector(31 downto 0); -- read data
--      wb_dat_o  : out std_ulogic_vector(31 downto 0); -- write data
--      wb_we_i   : in  std_ulogic;                     -- read/write
--      wb_sel_i  : in  std_ulogic_vector(03 downto 0); -- byte enable
--      wb_stb_i  : in  std_ulogic;                     -- strobe
--      wb_cyc_i  : in  std_ulogic;                     -- valid cycle
--      wb_ack_o  : out std_ulogic;                     -- transfer acknowledge
--      wb_err_o  : out std_ulogic                      -- transfer error
--    );
--  end component;
  ----------------------------------------------------------------------------------------------

begin

  ----------------------------------------------------------------------------------------------

  --Instancia del controlador
  --Entidad del controlador
controlador_teclado_inst : entity neorv32.controlador_teclado
  Generic map(
      numero_pines   => 4, --Numero de pines sobre los que tenemos que escribir y leer
      numero_elementos_teclado => 16 --Numero de teclas en el teclado
  )
  Port map( 
  clk   => std_ulogic(iCEBreakerv10_CLK), --Puerto de reloj
  reset => std_ulogic(iCEBreakerv10_BTN_N), --Puerto de reseteo

  --Pines para la comunicacion con el teclado
  pines_lectura_teclado(0)   => iCEBreakerv10_PMOD1B_10,
  pines_lectura_teclado(1)   => iCEBreakerv10_PMOD1B_9,
  pines_lectura_teclado(2)   => iCEBreakerv10_PMOD1B_8,
  pines_lectura_teclado(3)   => iCEBreakerv10_PMOD1B_7,

  pines_escritura_teclado(0) => iCEBreakerv10_PMOD1B_4,
  pines_escritura_teclado(1) => iCEBreakerv10_PMOD1B_3,
  pines_escritura_teclado(2) => iCEBreakerv10_PMOD1B_2,
  pines_escritura_teclado(3) => iCEBreakerv10_PMOD1B_1,

  --Pines para la comunicacion con el microprocesador

  pines_escritura_micro      => gpio_i(16-1 downto 0)

  );


  --Instancia del debouncer

  -- debouncer_inst_0: entity neorv32.debouncer
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    button_input => iCEBreakerv10_PMOD1B_10,
      --button_out => gpio_i(0)
  --    button_out => gpio_i_int(0)
  --  );

  --debouncer_inst_1: entity neorv32.debouncer
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    button_input => iCEBreakerv10_PMOD1B_9,
  --    button_out => gpio_i(1)
      --button_out => gpio_i_int(1)
  --  );

  --debouncer_inst_2: entity neorv32.debouncer
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    button_input => iCEBreakerv10_PMOD1B_8,
  --    button_out => gpio_i(2)
      --button_out => gpio_i_int(2)
  --  );

  --debouncer_inst_3: entity neorv32.debouncer
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    button_input => iCEBreakerv10_PMOD1B_7,
  --    button_out => gpio_i(3)
      --button_out => gpio_i_int(3)
  --  );

  --Instancia del rising_edge_detector 
  --edge_detector_inst_0: entity neorv32.rising_edge_detector
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    button_input => gpio_i_int(0),
  --    button_output => gpio_i_int_2(0)
      --button_output => gpio_i(0)
  --  );

  --edge_detector_inst_1: entity neorv32.rising_edge_detector
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    input => gpio_i_int(1),
  --    output => gpio_i(1)
  --  );

  --edge_detector_inst_2: entity neorv32.rising_edge_detector
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    input => gpio_i_int(2),
  --    output => gpio_i(2)
  --  );

  --edge_detector_inst_3: entity neorv32.rising_edge_detector
  --  Port map(
  --    clk => std_ulogic(iCEBreakerv10_CLK),
  --    reset => std_ulogic(iCEBreakerv10_BTN_N),
  --    input => gpio_i_int(3),
  --    output => gpio_i(3)
  --  );


  --Instancia del periferioc Wishbone

--  periph_wishbone : entity neorv32.wb_regs 
--    Generic map (
--      WB_ADDR_BASE => x"90000000",--to_stdulogicvector( 1001 0000 0000 0000 0000 0000 0000 0000), -- module base address, size-aligned
--      WB_ADDR_SIZE => 16                        -- module address space in bytes, has to be a power of two, min 4
--    )
--    Port map (
      -- wishbone host interface --
      --  signal wb_adr_m2s_signal : std_ulogic_vector(31 downto 0);
      --  signal wb_dat_s2m_signal : std_ulogic_vector(31 downto 0);
      --  signal wb_dat_m2s_signal : std_ulogic_vector(31 downto 0);
      --  signal wb_we_m2s_signal : std_logic;
      --  signal wb_sel_m2s_signal : std_ulogic_vector(3 downto 0);
      --  signal wb_stb_m2s_signal : std_logic;
      --  signal wb_cyc_m2s_signal : std_logic;
      --  signal wb_lock_m2s_signal : std_logic;
      --  signal wb_ack_s2m_signal : std_logic;
      --  signal wb_err_s2m_signal : std_logic;
--      wb_clk_i  => std_ulogic(iCEBreakerv10_CLK),                     -- clock
--      wb_rstn_i => std_ulogic(iCEBreakerv10_BTN_N),                     -- reset, async, low-active
--      wb_adr_i  => wb_adr_m2s_signal, -- address
--      wb_dat_i  => wb_dat_m2s_signal, -- read data
--      wb_dat_o  => wb_dat_s2m_signal ,
       -- write data
--      wb_we_i   => wb_we_m2s_signal,                     -- read/write
--      wb_sel_i  => wb_sel_m2s_signal, -- byte enable
--      wb_stb_i  => wb_stb_m2s_signal,                     -- strobe
--      wb_cyc_i  => wb_cyc_m2s_signal,                     -- valid cycle
--      wb_ack_o  => wb_ack_s2m_signal,                     -- transfer acknowledge
--      wb_err_o  => wb_err_s2m_signal                      -- transfer error
--    );

  ----------------------------------------------------------------------------------------------

  -- -------------------------------------------------------------------------------------------
  -- Instance the microprocessor
  -- -------------------------------------------------------------------------------------------

  neorv32_inst: entity neorv32.neorv32_top
  generic map (
    -- General --
    CLOCK_FREQUENCY              => CLOCK_FREQUENCY,               -- clock frequency of clk_i in Hz
    INT_BOOTLOADER_EN            => INT_BOOTLOADER_EN,             -- boot configuration: true = boot explicit bootloader; false = boot from int/ext (I)MEM
    HW_THREAD_ID                 => HW_THREAD_ID,                  -- hardware thread id (32-bit)

    -- On-Chip Debugger (OCD) --
    ON_CHIP_DEBUGGER_EN          => false,                         -- implement on-chip debugger?

    -- RISC-V CPU Extensions --
    CPU_EXTENSION_RISCV_A        => CPU_EXTENSION_RISCV_A,         -- implement atomic extension?
    CPU_EXTENSION_RISCV_C        => CPU_EXTENSION_RISCV_C,         -- implement compressed extension?
    CPU_EXTENSION_RISCV_E        => CPU_EXTENSION_RISCV_E,         -- implement embedded RF extension?
    CPU_EXTENSION_RISCV_M        => CPU_EXTENSION_RISCV_M,         -- implement mul/div extension?
    CPU_EXTENSION_RISCV_U        => CPU_EXTENSION_RISCV_U,         -- implement user mode extension?
    CPU_EXTENSION_RISCV_Zfinx    => CPU_EXTENSION_RISCV_Zfinx,     -- implement 32-bit floating-point extension (using INT regs!)
    CPU_EXTENSION_RISCV_Zicsr    => CPU_EXTENSION_RISCV_Zicsr,     -- implement CSR system?
    CPU_EXTENSION_RISCV_Zicntr   => true,                          -- implement base counters?
    CPU_EXTENSION_RISCV_Zifencei => CPU_EXTENSION_RISCV_Zifencei,  -- implement instruction stream sync.?

    -- Extension Options --
    FAST_MUL_EN                  => FAST_MUL_EN,                   -- use DSPs for M extension's multiplier
    FAST_SHIFT_EN                => FAST_SHIFT_EN,                 -- use barrel shifter for shift operations
    CPU_CNT_WIDTH                => CPU_CNT_WIDTH,                 -- total width of CPU cycle and instret counters (0..64)

    -- Physical Memory Protection (PMP) --
    PMP_NUM_REGIONS              => PMP_NUM_REGIONS,       -- number of regions (0..64)
    PMP_MIN_GRANULARITY          => PMP_MIN_GRANULARITY,   -- minimal region granularity in bytes, has to be a power of 2, min 8 bytes

    -- Hardware Performance Monitors (HPM) --
    HPM_NUM_CNTS                 => HPM_NUM_CNTS,          -- number of implemented HPM counters (0..29)
    HPM_CNT_WIDTH                => HPM_CNT_WIDTH,         -- total size of HPM counters (1..64)

    -- Internal Instruction memory --
    MEM_INT_IMEM_EN              => MEM_INT_IMEM_EN,       -- implement processor-internal instruction memory
    MEM_INT_IMEM_SIZE            => MEM_INT_IMEM_SIZE,     -- size of processor-internal instruction memory in bytes

    -- Internal Data memory --
    MEM_INT_DMEM_EN              => MEM_INT_DMEM_EN,       -- implement processor-internal data memory
    MEM_INT_DMEM_SIZE            => MEM_INT_DMEM_SIZE,     -- size of processor-internal data memory in bytes

    -- Internal Cache memory --
    ICACHE_EN                    => ICACHE_EN,             -- implement instruction cache
    ICACHE_NUM_BLOCKS            => ICACHE_NUM_BLOCKS,     -- i-cache: number of blocks (min 1), has to be a power of 2
    ICACHE_BLOCK_SIZE            => ICACHE_BLOCK_SIZE,     -- i-cache: block size in bytes (min 4), has to be a power of 2
    ICACHE_ASSOCIATIVITY         => ICACHE_ASSOCIATIVITY,  -- i-cache: associativity / number of sets (1=direct_mapped), has to be a power of 2

    -- External memory interface --
    MEM_EXT_EN                   => true,                 -- implement external memory bus interface?
    MEM_EXT_TIMEOUT              => 0,                     -- cycles after a pending bus access auto-terminates (0 = disabled)

    -- Processor peripherals --
    IO_GPIO_EN                   => IO_GPIO_EN,    -- implement general purpose input/output port unit (GPIO)?
    IO_MTIME_EN                  => IO_MTIME_EN,   -- implement machine system timer (MTIME)?
    IO_UART0_EN                  => IO_UART0_EN,   -- implement primary universal asynchronous receiver/transmitter (UART0)?
    IO_UART1_EN                  => false,         -- implement secondary universal asynchronous receiver/transmitter (UART1)?
    IO_SPI_EN                    => false,         -- implement serial peripheral interface (SPI)?
    IO_TWI_EN                    => false,         -- implement two-wire interface (TWI)?
    IO_PWM_NUM_CH                => IO_PWM_NUM_CH, -- number of PWM channels to implement (0..60); 0 = disabled
    IO_WDT_EN                    => IO_WDT_EN,     -- implement watch dog timer (WDT)?
    IO_TRNG_EN                   => false,         -- implement true random number generator (TRNG)?
    IO_CFS_EN                    => false,         -- implement custom functions subsystem (CFS)?
    IO_CFS_CONFIG                => x"00000000",   -- custom CFS configuration generic
    IO_CFS_IN_SIZE               => 32,            -- size of CFS input conduit in bits
    IO_CFS_OUT_SIZE              => 32,            -- size of CFS output conduit in bits
    IO_NEOLED_EN                 => false          -- implement NeoPixel-compatible smart LED interface (NEOLED)?
  )
  port map (
    -- Global control --
    clk_i       => std_ulogic(iCEBreakerv10_CLK),   -- global clock, rising edge
    rstn_i      => std_ulogic(iCEBreakerv10_BTN_N), -- global reset, low-active, async

    -- JTAG on-chip debugger interface (available if ON_CHIP_DEBUGGER_EN = true) --
    jtag_trst_i => '0',                          -- low-active TAP reset (optional)
    jtag_tck_i  => '0',                          -- serial clock
    jtag_tdi_i  => '0',                          -- serial data input
    jtag_tdo_o  => open,                         -- serial data output
    jtag_tms_i  => '0',                          -- mode select

    -- Wishbone bus interface (available if MEM_EXT_EN = true) --
    --IMPORTANTE PRACTICA 3

  --  signal wb_adr_m2s_signal : std_ulogic_vector(31 downto 0);
  --  signal wb_dat_s2m_signal : std_ulogic_vector(31 downto 0);
  --  signal wb_dat_m2s_signal : std_ulogic_vector(31 downto 0);
  --  signal wb_we_m2s_signal : std_logic;
  --  signal wb_sel_m2s_signal : std_ulogic_vector(3 downto 0);
  --  signal wb_stb_m2s_signal : std_logic;
  --  signal wb_cyc_m2s_signal : std_logic;
  --  signal wb_lock_m2s_signal : std_logic;
  --  signal wb_ack_s2m_signal : std_logic;
  --  signal wb_err_s2m_signal : std_logic;

    wb_tag_o    => open,                         -- request tag
    wb_adr_o    => open,                         -- address
  --  wb_adr_o    => wb_adr_m2s_signal,                         -- address
    wb_dat_i    => (others => '0'),              -- read data
  --  wb_dat_i    => wb_dat_s2m_signal,              -- read data
    wb_dat_o    => open,                         -- write data
  --  wb_dat_o    => wb_dat_m2s_signal,                         -- write data
    wb_we_o     => open,                         -- read/write
  --  wb_we_o     => wb_we_m2s_signal,                         -- read/write
    wb_sel_o    => open,                         -- byte enable
  --  wb_sel_o    => wb_sel_m2s_signal,                         -- byte enable
    wb_stb_o    => open,                         -- strobe
  --  wb_stb_o    => wb_stb_m2s_signal,                         -- strobe
    wb_cyc_o    => open,                         -- valid cycle
  --  wb_cyc_o    => wb_cyc_m2s_signal,                         -- valid cycle
    wb_lock_o   => open,                         -- exclusive access request
  --  wb_lock_o   => wb_lock_m2s_signal,                         -- exclusive access request
    wb_ack_i    => '0',                          -- transfer acknowledge
  --  wb_ack_i    => wb_ack_s2m_signal,                          -- transfer acknowledge
    wb_err_i    => '0',                          -- transfer error
  --  wb_err_i    => wb_err_s2m_signal,                          -- transfer error

    -- Advanced memory control signals (available if MEM_EXT_EN = true) --
    fence_o     => open,                         -- indicates an executed FENCE operation
    fencei_o    => open,                         -- indicates an executed FENCEI operation

    -- GPIO (available if IO_GPIO_EN = true) --
    gpio_o      => gpio_o,                       -- parallel output
    gpio_i      => gpio_i,                       -- parallel input

    -- primary UART0 (available if IO_UART0_EN = true) --
    uart0_txd_o => iCEBreakerv10_TX,             -- UART0 send data
    uart0_rxd_i => iCEBreakerv10_RX,             -- UART0 receive data
    uart0_rts_o => open,                         -- hw flow control: UART0.RX ready to receive ("RTR"), low-active, optional
    uart0_cts_i => '0',                          -- hw flow control: UART0.TX allowed to transmit, low-active, optional

    -- secondary UART1 (available if IO_UART1_EN = true) --
    uart1_txd_o => open,                         -- UART1 send data
    uart1_rxd_i => '0',                          -- UART1 receive data
    uart1_rts_o => open,                         -- hw flow control: UART1.RX ready to receive ("RTR"), low-active, optional
    uart1_cts_i => '0',                          -- hw flow control: UART1.TX allowed to transmit, low-active, optional

    -- SPI (available if IO_SPI_EN = true) --
    spi_sck_o   => open,                         -- SPI serial clock
    spi_sdo_o   => open,                         -- controller data out, peripheral data in
    spi_sdi_i   => '0',                          -- controller data in, peripheral data out
    spi_csn_o   => open,                         -- SPI CS

    -- TWI (available if IO_TWI_EN = true) --
    twi_sda_io  => open,                         -- twi serial data line
    twi_scl_io  => open,                         -- twi serial clock line

    -- PWM (available if IO_PWM_NUM_CH > 0) --
    pwm_o       => open,                         -- pwm channels

    -- Custom Functions Subsystem IO --
    cfs_in_i    => (others => '0'),              -- custom CFS inputs conduit
    cfs_out_o   => open,                         -- custom CFS outputs conduit

    -- NeoPixel-compatible smart LED interface (available if IO_NEOLED_EN = true) --
    neoled_o    => open,                         -- async serial data line

    -- System time --
    mtime_i     => (others => '0'),              -- current system time from ext. MTIME (if IO_MTIME_EN = false)
    mtime_o     => open,                         -- current system time from int. MTIME (if IO_MTIME_EN = true)

    -- Interrupts --
    mtime_irq_i => '0',                          -- machine timer interrupt, available if IO_MTIME_EN = false
    msw_irq_i   => '0',                          -- machine software interrupt
    mext_irq_i  => '0'                           -- machine external interrupt
  );

  -- -------------------------------------------------------------------------------------------
  -- IO Connections
  -- -------------------------------------------------------------------------------------------

  --iCEBreakerv10_PMOD2_1_LED_left   <= gpio_o(0);
  --iCEBreakerv10_PMOD2_2_LED_right  <= gpio_o(1);
  --iCEBreakerv10_PMOD2_8_LED_up     <= gpio_o(2);
  --iCEBreakerv10_PMOD2_3_LED_down   <= gpio_o(3);
  --iCEBreakerv10_PMOD2_7_LED_center <= gpio_o(4);

  --Salidas
  --iCEBreakerv10_PMOD1B_9 <= gpio_o(0);
  --iCEBreakerv10_PMOD1B_3 <= gpio_o(1);
  --iCEBreakerv10_PMOD1B_10 <= gpio_o(2);
  --iCEBreakerv10_PMOD1B_4 <= gpio_o(3);
  --  iCEBreakerv10_PMOD1B_4 <= gpio_o(0);
  --  iCEBreakerv10_PMOD1B_3 <= gpio_o(1);
  --  iCEBreakerv10_PMOD1B_2 <= gpio_o(2);
  --  iCEBreakerv10_PMOD1B_1 <= gpio_o(3);

  --Entradas

  --gpio_i(0) <= iCEBreakerv10_PMOD1B_7;
  --gpio_i(1) <= iCEBreakerv10_PMOD1B_1;
  --gpio_i(2) <= iCEBreakerv10_PMOD1B_8;
  --gpio_i(3) <= iCEBreakerv10_PMOD1B_2;
  --  gpio_i(0) <= iCEBreakerv10_PMOD1B_10;
  --  gpio_i(0) <= gpio_i_int(0);
  --  gpio_i(0) <= gpio_i_int_2(0);
  --  gpio_i(1) <= iCEBreakerv10_PMOD1B_9;
  --  gpio_i(2) <= iCEBreakerv10_PMOD1B_8;
  --  gpio_i(3) <= iCEBreakerv10_PMOD1B_7;


  --Salidas
  --pines 28 y 31 son de alimentacion
  --pines 32 y 34 son de tierra
  --Entradas
  --42 -> row 0
  --43 -> row 1
  --36 -> row 2
  --38 -> row 3

  --Salidas
  --32 -> col 0
  --34 -> col 1
  --28 -> col 2 
  --31 -> col 3
  


end architecture;
