module ysyx_core_npc_top (
    input        clock,
    input        rst_n,

    input        tck,
    input        tms,
    input        tdi,
    output       tdo,
    output [3:0] tap_state,

    input        mtip_asyn
);

wire reset = ~rst_n;

wire        io_master_awready;
wire        io_master_awvalid;
wire [31:0] io_master_awaddr;
wire [3:0]  io_master_awid;
wire [7:0]  io_master_awlen;
wire [2:0]  io_master_awsize;
wire [1:0]  io_master_awburst;
wire        io_master_wready;
wire        io_master_wvalid;
wire [63:0] io_master_wdata;
wire [7:0]  io_master_wstrb;
wire        io_master_wlast;
wire        io_master_bready;
wire        io_master_bvalid;
wire [1:0]  io_master_bresp;
wire [3:0]  io_master_bid;
wire        io_master_arready;
wire        io_master_arvalid;
wire [31:0] io_master_araddr;
wire [3:0]  io_master_arid;
wire [7:0]  io_master_arlen;
wire [2:0]  io_master_arsize;
wire [1:0]  io_master_arburst;
wire        io_master_rready;
wire        io_master_rvalid;
wire [1:0]  io_master_rresp;
wire [63:0] io_master_rdata;
wire        io_master_rlast;
wire [3:0]  io_master_rid;

ysyx_23060081 #(
    .RST_PC(64'h8000_0000)
) u_ysyx_core (
    .clock             (clock),
    .reset             (reset),
    .io_interrupt      (mtip_asyn),
    .io_master_awready (io_master_awready),
    .io_master_awvalid (io_master_awvalid),
    .io_master_awaddr  (io_master_awaddr),
    .io_master_awid    (io_master_awid),
    .io_master_awlen   (io_master_awlen),
    .io_master_awsize  (io_master_awsize),
    .io_master_awburst (io_master_awburst),
    .io_master_wready  (io_master_wready),
    .io_master_wvalid  (io_master_wvalid),
    .io_master_wdata   (io_master_wdata),
    .io_master_wstrb   (io_master_wstrb),
    .io_master_wlast   (io_master_wlast),
    .io_master_bready  (io_master_bready),
    .io_master_bvalid  (io_master_bvalid),
    .io_master_bresp   (io_master_bresp),
    .io_master_bid     (io_master_bid),
    .io_master_arready (io_master_arready),
    .io_master_arvalid (io_master_arvalid),
    .io_master_araddr  (io_master_araddr),
    .io_master_arid    (io_master_arid),
    .io_master_arlen   (io_master_arlen),
    .io_master_arsize  (io_master_arsize),
    .io_master_arburst (io_master_arburst),
    .io_master_rready  (io_master_rready),
    .io_master_rvalid  (io_master_rvalid),
    .io_master_rresp   (io_master_rresp),
    .io_master_rdata   (io_master_rdata),
    .io_master_rlast   (io_master_rlast),
    .io_master_rid     (io_master_rid),

    .io_slave_awready  (),
    .io_slave_awvalid  (1'b0),
    .io_slave_awaddr   (32'h0),
    .io_slave_awid     (4'h0),
    .io_slave_awlen    (8'h0),
    .io_slave_awsize   (3'h0),
    .io_slave_awburst  (2'h0),
    .io_slave_wready   (),
    .io_slave_wvalid   (1'b0),
    .io_slave_wdata    (64'h0),
    .io_slave_wstrb    (8'h0),
    .io_slave_wlast    (1'b0),
    .io_slave_bready   (1'b0),
    .io_slave_bvalid   (),
    .io_slave_bresp    (),
    .io_slave_bid      (),
    .io_slave_arready  (),
    .io_slave_arvalid  (1'b0),
    .io_slave_araddr   (32'h0),
    .io_slave_arid     (4'h0),
    .io_slave_arlen    (8'h0),
    .io_slave_arsize   (3'h0),
    .io_slave_arburst  (2'h0),
    .io_slave_rready   (1'b0),
    .io_slave_rvalid   (),
    .io_slave_rresp    (),
    .io_slave_rdata    (),
    .io_slave_rlast    (),
    .io_slave_rid      ()
);

sim_sram_dpic #(
    .AXI_ADDR_W(64),
    .AXI_ID_W  (4),
    .AXI_DATA_W(64)
) u_sim_sram_dpic (
    .aclk         (clock),
    .arst_n       (rst_n),
    .mst_awvalid  (io_master_awvalid),
    .mst_awready  (io_master_awready),
    .mst_awaddr   ({32'h0, io_master_awaddr}),
    .mst_awlen    (io_master_awlen),
    .mst_awsize   (io_master_awsize),
    .mst_awburst  (io_master_awburst),
    .mst_awlock   (1'b0),
    .mst_awcache  (4'h0),
    .mst_awprot   (3'h0),
    .mst_awqos    (4'h0),
    .mst_awregion (4'h0),
    .mst_awid     (io_master_awid),
    .mst_wvalid   (io_master_wvalid),
    .mst_wready   (io_master_wready),
    .mst_wlast    (io_master_wlast),
    .mst_wdata    (io_master_wdata),
    .mst_wstrb    (io_master_wstrb),
    .mst_bvalid   (io_master_bvalid),
    .mst_bready   (io_master_bready),
    .mst_bid      (io_master_bid),
    .mst_bresp    (io_master_bresp),
    .mst_arvalid  (io_master_arvalid),
    .mst_arready  (io_master_arready),
    .mst_araddr   ({32'h0, io_master_araddr}),
    .mst_arlen    (io_master_arlen),
    .mst_arsize   (io_master_arsize),
    .mst_arburst  (io_master_arburst),
    .mst_arlock   (1'b0),
    .mst_arcache  (4'h0),
    .mst_arprot   (3'h0),
    .mst_arqos    (4'h0),
    .mst_arregion (4'h0),
    .mst_arid     (io_master_arid),
    .mst_rvalid   (io_master_rvalid),
    .mst_rready   (io_master_rready),
    .mst_rid      (io_master_rid),
    .mst_rresp    (io_master_rresp),
    .mst_rdata    (io_master_rdata),
    .mst_rlast    (io_master_rlast)
);

wire commit_valid = u_ysyx_core.u_core_top.u_wbu.LS_WB_reg_ls_valid;
wire [4:0] commit_rd = u_ysyx_core.u_core_top.u_wbu.LS_WB_reg_rd;
wire [63:0] commit_pc = (u_ysyx_core.u_core_top.u_wbu.WB_IF_jump_flag) ?
                        u_ysyx_core.u_core_top.u_wbu.WB_IF_jump_addr :
                        u_ysyx_core.u_core_top.u_wbu.LS_WB_reg_next_PC;

DifftestArchIntRegState u_DifftestArchIntRegState (
    .io_value_0  (64'h0),
    .io_value_1  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[1]),
    .io_value_2  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[2]),
    .io_value_3  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[3]),
    .io_value_4  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[4]),
    .io_value_5  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[5]),
    .io_value_6  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[6]),
    .io_value_7  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[7]),
    .io_value_8  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[8]),
    .io_value_9  (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[9]),
    .io_value_10 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[10]),
    .io_value_11 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[11]),
    .io_value_12 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[12]),
    .io_value_13 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[13]),
    .io_value_14 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[14]),
    .io_value_15 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[15]),
    .io_value_16 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[16]),
    .io_value_17 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[17]),
    .io_value_18 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[18]),
    .io_value_19 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[19]),
    .io_value_20 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[20]),
    .io_value_21 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[21]),
    .io_value_22 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[22]),
    .io_value_23 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[23]),
    .io_value_24 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[24]),
    .io_value_25 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[25]),
    .io_value_26 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[26]),
    .io_value_27 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[27]),
    .io_value_28 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[28]),
    .io_value_29 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[29]),
    .io_value_30 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[30]),
    .io_value_31 (u_ysyx_core.u_core_top.u_wbu.u_gpr.riscv_reg[31])
);

DifftestPerformRegState u_DifftestPerformRegState (
    .io_value_0  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[1]),
    .io_value_1  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[2]),
    .io_value_3  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[3]),
    .io_value_4  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[4]),
    .io_value_5  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[5]),
    .io_value_6  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[6]),
    .io_value_7  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[7]),
    .io_value_8  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[8]),
    .io_value_9  (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[9]),
    .io_value_10 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[10]),
    .io_value_11 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[11]),
    .io_value_12 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[12]),
    .io_value_13 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[13]),
    .io_value_14 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[14]),
    .io_value_15 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[15]),
    .io_value_16 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[16]),
    .io_value_17 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[17]),
    .io_value_18 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[18]),
    .io_value_19 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[19]),
    .io_value_20 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[20]),
    .io_value_21 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[21]),
    .io_value_22 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[22]),
    .io_value_23 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[23]),
    .io_value_24 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[24]),
    .io_value_25 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[25]),
    .io_value_26 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[26]),
    .io_value_27 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[27]),
    .io_value_28 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[28]),
    .io_value_29 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[29]),
    .io_value_30 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[30]),
    .io_value_31 (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[31])
);

DifftestCSRState u_DifftestCSRState (
    .io_privilegeMode ({62'h0, u_ysyx_core.u_core_top.u_wbu.u_csr.current_priv_status}),
    .io_mstatus       (u_ysyx_core.u_core_top.u_wbu.u_csr.mstatus),
    .io_sstatus       (u_ysyx_core.u_core_top.u_wbu.u_csr.sstatus),
    .io_mepc          (u_ysyx_core.u_core_top.u_wbu.u_csr.mepc),
    .io_sepc          (u_ysyx_core.u_core_top.u_wbu.u_csr.sepc),
    .io_mtval         (u_ysyx_core.u_core_top.u_wbu.u_csr.mtval),
    .io_stval         (u_ysyx_core.u_core_top.u_wbu.u_csr.stval),
    .io_mtvec         (u_ysyx_core.u_core_top.u_wbu.u_csr.mtvec),
    .io_stvec         (u_ysyx_core.u_core_top.u_wbu.u_csr.stvec),
    .io_mcause        (u_ysyx_core.u_core_top.u_wbu.u_csr.mcause),
    .io_scause        (u_ysyx_core.u_core_top.u_wbu.u_csr.scause),
    .io_satp          (u_ysyx_core.u_core_top.u_wbu.u_csr.satp),
    .io_mip           (u_ysyx_core.u_core_top.u_wbu.u_csr.mip),
    .io_mie           (u_ysyx_core.u_core_top.u_wbu.u_csr.mie),
    .io_mscratch      (u_ysyx_core.u_core_top.u_wbu.u_csr.mscratch),
    .io_sscratch      (u_ysyx_core.u_core_top.u_wbu.u_csr.sscratch),
    .io_mideleg       (u_ysyx_core.u_core_top.u_wbu.u_csr.mideleg),
    .io_medeleg       (u_ysyx_core.u_core_top.u_wbu.u_csr.medeleg)
);

DifftestInstrCommit u_DifftestInstrCommit (
    .clock      (clock),
    .io_valid   (commit_valid),
    .io_skip    (1'b0),
    .io_isRVC   (1'b0),
    .io_rfwen   (u_ysyx_core.u_core_top.u_wbu.LS_WB_reg_dest_wen),
    .io_fpwen   (1'b0),
    .io_vecwen  (1'b0),
    .io_wpdest  (commit_rd),
    .io_wdest   ({3'h0, commit_rd}),
    .io_pc      (commit_pc),
    .io_instr   (u_ysyx_core.u_core_top.u_wbu.LS_WB_reg_inst),
    .io_robIdx  (10'h0),
    .io_lqIdx   (7'h0),
    .io_sqIdx   (7'h0),
    .io_isLoad  (1'b0),
    .io_isStore (1'b0),
    .io_nFused  (8'h0),
    .io_special (8'h0),
    .io_coreid  (8'h0),
    .io_index   (8'h0)
);

DifftestTrapEvent u_DifftestTrapEvent (
    .clock       (clock),
    .enable      (u_ysyx_core.u_core_top.u_wbu.u_csr.u_trap_control.trap_m_interrupt),
    .io_hasTrap  (1'b0),
    .io_cycleCnt (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[1]),
    .io_instrCnt (u_ysyx_core.u_core_top.u_wbu.u_csr.Performance_Monitor[2]),
    .io_hasWFI   (1'b0),
    .io_code     (u_ysyx_core.u_core_top.u_wbu.u_csr.u_trap_control.cause),
    .io_pc       (u_ysyx_core.u_core_top.u_wbu.u_csr.u_trap_control.next_pc),
    .io_coreid   (8'h0)
);

assign tdo = tdi & tms & tck;
assign tap_state = 4'h0;

endmodule
