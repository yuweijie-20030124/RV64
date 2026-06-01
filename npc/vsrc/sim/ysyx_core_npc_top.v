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

wire        MXR;
wire        SUM;
wire        MPRV;
wire [1:0]  MPP;
wire [3:0]  satp_mode;
wire [15:0] satp_asid;
wire [43:0] satp_ppn;

wire        ifu_arready;
wire        ifu_arvalid;
wire [63:0] ifu_araddr;
wire        ifu_rvalid;
wire        ifu_rready;
wire [1:0]  ifu_rresp;
wire [63:0] ifu_rdata;

wire        lsu_arvalid;
wire        lsu_arready;
wire        lsu_arlock;
wire [2:0]  lsu_arsize;
wire [63:0] lsu_araddr;
wire        lsu_rvalid;
wire        lsu_rready;
wire [1:0]  lsu_rresp;
wire [63:0] lsu_rdata;
wire        lsu_awvalid;
wire        lsu_awready;
wire        lsu_awlock;
wire [2:0]  lsu_awsize;
wire [63:0] lsu_awaddr;
wire        lsu_wvalid;
wire        lsu_wready;
wire [7:0]  lsu_wstrb;
wire [63:0] lsu_wdata;
wire        lsu_bvalid;
wire        lsu_bready;
wire [1:0]  lsu_bresp;

core_top #(
    .MHARTID (0),
    .RST_PC  (64'h8000_0000)
) u_core_top (
    .clk         (clock),
    .rst_n       (rst_n),
    .stip_asyn   (1'b0),
    .seip_asyn   (1'b0),
    .ssip_asyn   (1'b0),
    .mtip_asyn   (mtip_asyn),
    .meip_asyn   (1'b0),
    .msip_asyn   (1'b0),
    .halt_req    (1'b0),
    .MXR         (MXR),
    .SUM         (SUM),
    .MPRV        (MPRV),
    .MPP         (MPP),
    .satp_mode   (satp_mode),
    .satp_asid   (satp_asid),
    .satp_ppn    (satp_ppn),
    .ifu_arready (ifu_arready),
    .ifu_arvalid (ifu_arvalid),
    .ifu_araddr  (ifu_araddr),
    .ifu_rvalid  (ifu_rvalid),
    .ifu_rready  (ifu_rready),
    .ifu_rresp   (ifu_rresp),
    .ifu_rdata   (ifu_rdata),
    .lsu_arvalid (lsu_arvalid),
    .lsu_arready (lsu_arready),
    .lsu_arlock  (lsu_arlock),
    .lsu_arsize  (lsu_arsize),
    .lsu_araddr  (lsu_araddr),
    .lsu_rvalid  (lsu_rvalid),
    .lsu_rready  (lsu_rready),
    .lsu_rresp   (lsu_rresp),
    .lsu_rdata   (lsu_rdata),
    .lsu_awvalid (lsu_awvalid),
    .lsu_awready (lsu_awready),
    .lsu_awlock  (lsu_awlock),
    .lsu_awsize  (lsu_awsize),
    .lsu_awaddr  (lsu_awaddr),
    .lsu_wvalid  (lsu_wvalid),
    .lsu_wready  (lsu_wready),
    .lsu_wstrb   (lsu_wstrb),
    .lsu_wdata   (lsu_wdata),
    .lsu_bvalid  (lsu_bvalid),
    .lsu_bready  (lsu_bready),
    .lsu_bresp   (lsu_bresp)
);

wire        mst_awvalid;
wire        mst_awready;
wire [63:0] mst_awaddr;
wire [7:0]  mst_awlen;
wire [2:0]  mst_awsize;
wire [1:0]  mst_awburst;
wire        mst_awlock;
wire [3:0]  mst_awcache;
wire [2:0]  mst_awprot;
wire [3:0]  mst_awqos;
wire [3:0]  mst_awregion;
wire [7:0]  mst_awid;
wire        mst_wvalid;
wire        mst_wready;
wire        mst_wlast;
wire [63:0] mst_wdata;
wire [7:0]  mst_wstrb;
wire        mst_bvalid;
wire        mst_bready;
wire [7:0]  mst_bid;
wire [1:0]  mst_bresp;
wire        mst_arvalid;
wire        mst_arready;
wire [63:0] mst_araddr;
wire [7:0]  mst_arlen;
wire [2:0]  mst_arsize;
wire [1:0]  mst_arburst;
wire        mst_arlock;
wire [3:0]  mst_arcache;
wire [2:0]  mst_arprot;
wire [3:0]  mst_arqos;
wire [3:0]  mst_arregion;
wire [7:0]  mst_arid;
wire        mst_rvalid;
wire        mst_rready;
wire [7:0]  mst_rid;
wire [1:0]  mst_rresp;
wire [63:0] mst_rdata;
wire        mst_rlast;

rv64_mem_arb u_mem_arb (
    .clk          (clock),
    .rst_n        (rst_n),
    .ifu_arvalid  (ifu_arvalid),
    .ifu_arready  (ifu_arready),
    .ifu_araddr   (ifu_araddr),
    .ifu_rvalid   (ifu_rvalid),
    .ifu_rready   (ifu_rready),
    .ifu_rresp    (ifu_rresp),
    .ifu_rdata    (ifu_rdata),
    .lsu_arvalid  (lsu_arvalid),
    .lsu_arready  (lsu_arready),
    .lsu_arlock   (lsu_arlock),
    .lsu_arsize   (lsu_arsize),
    .lsu_araddr   (lsu_araddr),
    .lsu_rvalid   (lsu_rvalid),
    .lsu_rready   (lsu_rready),
    .lsu_rresp    (lsu_rresp),
    .lsu_rdata    (lsu_rdata),
    .lsu_awvalid  (lsu_awvalid),
    .lsu_awready  (lsu_awready),
    .lsu_awlock   (lsu_awlock),
    .lsu_awsize   (lsu_awsize),
    .lsu_awaddr   (lsu_awaddr),
    .lsu_wvalid   (lsu_wvalid),
    .lsu_wready   (lsu_wready),
    .lsu_wstrb    (lsu_wstrb),
    .lsu_wdata    (lsu_wdata),
    .lsu_bvalid   (lsu_bvalid),
    .lsu_bready   (lsu_bready),
    .lsu_bresp    (lsu_bresp),
    .mst_awvalid  (mst_awvalid),
    .mst_awready  (mst_awready),
    .mst_awaddr   (mst_awaddr),
    .mst_awlen    (mst_awlen),
    .mst_awsize   (mst_awsize),
    .mst_awburst  (mst_awburst),
    .mst_awlock   (mst_awlock),
    .mst_awcache  (mst_awcache),
    .mst_awprot   (mst_awprot),
    .mst_awqos    (mst_awqos),
    .mst_awregion (mst_awregion),
    .mst_awid     (mst_awid),
    .mst_wvalid   (mst_wvalid),
    .mst_wready   (mst_wready),
    .mst_wlast    (mst_wlast),
    .mst_wdata    (mst_wdata),
    .mst_wstrb    (mst_wstrb),
    .mst_bvalid   (mst_bvalid),
    .mst_bready   (mst_bready),
    .mst_bid      (mst_bid),
    .mst_bresp    (mst_bresp),
    .mst_arvalid  (mst_arvalid),
    .mst_arready  (mst_arready),
    .mst_araddr   (mst_araddr),
    .mst_arlen    (mst_arlen),
    .mst_arsize   (mst_arsize),
    .mst_arburst  (mst_arburst),
    .mst_arlock   (mst_arlock),
    .mst_arcache  (mst_arcache),
    .mst_arprot   (mst_arprot),
    .mst_arqos    (mst_arqos),
    .mst_arregion (mst_arregion),
    .mst_arid     (mst_arid),
    .mst_rvalid   (mst_rvalid),
    .mst_rready   (mst_rready),
    .mst_rid      (mst_rid),
    .mst_rresp    (mst_rresp),
    .mst_rdata    (mst_rdata),
    .mst_rlast    (mst_rlast)
);

sim_sram_dpic #(
    .AXI_ADDR_W(64),
    .AXI_ID_W  (8),
    .AXI_DATA_W(64)
) u_sim_sram_dpic (
    .aclk         (clock),
    .arst_n       (rst_n),
    .mst_awvalid  (mst_awvalid),
    .mst_awready  (mst_awready),
    .mst_awaddr   (mst_awaddr),
    .mst_awlen    (mst_awlen),
    .mst_awsize   (mst_awsize),
    .mst_awburst  (mst_awburst),
    .mst_awlock   (mst_awlock),
    .mst_awcache  (mst_awcache),
    .mst_awprot   (mst_awprot),
    .mst_awqos    (mst_awqos),
    .mst_awregion (mst_awregion),
    .mst_awid     (mst_awid),
    .mst_wvalid   (mst_wvalid),
    .mst_wready   (mst_wready),
    .mst_wlast    (mst_wlast),
    .mst_wdata    (mst_wdata),
    .mst_wstrb    (mst_wstrb),
    .mst_bvalid   (mst_bvalid),
    .mst_bready   (mst_bready),
    .mst_bid      (mst_bid),
    .mst_bresp    (mst_bresp),
    .mst_arvalid  (mst_arvalid),
    .mst_arready  (mst_arready),
    .mst_araddr   (mst_araddr),
    .mst_arlen    (mst_arlen),
    .mst_arsize   (mst_arsize),
    .mst_arburst  (mst_arburst),
    .mst_arlock   (mst_arlock),
    .mst_arcache  (mst_arcache),
    .mst_arprot   (mst_arprot),
    .mst_arqos    (mst_arqos),
    .mst_arregion (mst_arregion),
    .mst_arid     (mst_arid),
    .mst_rvalid   (mst_rvalid),
    .mst_rready   (mst_rready),
    .mst_rid      (mst_rid),
    .mst_rresp    (mst_rresp),
    .mst_rdata    (mst_rdata),
    .mst_rlast    (mst_rlast)
);

wire [63:0] commit_pc = (u_core_top.u_wbu.WB_IF_jump_flag) ?
                        u_core_top.u_wbu.WB_IF_jump_addr :
                        u_core_top.u_wbu.LS_WB_reg_next_PC;

DifftestArchIntRegState u_DifftestArchIntRegState (
    .io_value_0  (64'h0),
    .io_value_1  (u_core_top.u_wbu.u_gpr.riscv_reg[1]),
    .io_value_2  (u_core_top.u_wbu.u_gpr.riscv_reg[2]),
    .io_value_3  (u_core_top.u_wbu.u_gpr.riscv_reg[3]),
    .io_value_4  (u_core_top.u_wbu.u_gpr.riscv_reg[4]),
    .io_value_5  (u_core_top.u_wbu.u_gpr.riscv_reg[5]),
    .io_value_6  (u_core_top.u_wbu.u_gpr.riscv_reg[6]),
    .io_value_7  (u_core_top.u_wbu.u_gpr.riscv_reg[7]),
    .io_value_8  (u_core_top.u_wbu.u_gpr.riscv_reg[8]),
    .io_value_9  (u_core_top.u_wbu.u_gpr.riscv_reg[9]),
    .io_value_10 (u_core_top.u_wbu.u_gpr.riscv_reg[10]),
    .io_value_11 (u_core_top.u_wbu.u_gpr.riscv_reg[11]),
    .io_value_12 (u_core_top.u_wbu.u_gpr.riscv_reg[12]),
    .io_value_13 (u_core_top.u_wbu.u_gpr.riscv_reg[13]),
    .io_value_14 (u_core_top.u_wbu.u_gpr.riscv_reg[14]),
    .io_value_15 (u_core_top.u_wbu.u_gpr.riscv_reg[15]),
    .io_value_16 (u_core_top.u_wbu.u_gpr.riscv_reg[16]),
    .io_value_17 (u_core_top.u_wbu.u_gpr.riscv_reg[17]),
    .io_value_18 (u_core_top.u_wbu.u_gpr.riscv_reg[18]),
    .io_value_19 (u_core_top.u_wbu.u_gpr.riscv_reg[19]),
    .io_value_20 (u_core_top.u_wbu.u_gpr.riscv_reg[20]),
    .io_value_21 (u_core_top.u_wbu.u_gpr.riscv_reg[21]),
    .io_value_22 (u_core_top.u_wbu.u_gpr.riscv_reg[22]),
    .io_value_23 (u_core_top.u_wbu.u_gpr.riscv_reg[23]),
    .io_value_24 (u_core_top.u_wbu.u_gpr.riscv_reg[24]),
    .io_value_25 (u_core_top.u_wbu.u_gpr.riscv_reg[25]),
    .io_value_26 (u_core_top.u_wbu.u_gpr.riscv_reg[26]),
    .io_value_27 (u_core_top.u_wbu.u_gpr.riscv_reg[27]),
    .io_value_28 (u_core_top.u_wbu.u_gpr.riscv_reg[28]),
    .io_value_29 (u_core_top.u_wbu.u_gpr.riscv_reg[29]),
    .io_value_30 (u_core_top.u_wbu.u_gpr.riscv_reg[30]),
    .io_value_31 (u_core_top.u_wbu.u_gpr.riscv_reg[31])
);

DifftestPerformRegState u_DifftestPerformRegState (
    .io_value_0  (u_core_top.u_wbu.u_csr.Performance_Monitor[1]),
    .io_value_1  (u_core_top.u_wbu.u_csr.Performance_Monitor[2]),
    .io_value_3  (u_core_top.u_wbu.u_csr.Performance_Monitor[3]),
    .io_value_4  (u_core_top.u_wbu.u_csr.Performance_Monitor[4]),
    .io_value_5  (u_core_top.u_wbu.u_csr.Performance_Monitor[5]),
    .io_value_6  (u_core_top.u_wbu.u_csr.Performance_Monitor[6]),
    .io_value_7  (u_core_top.u_wbu.u_csr.Performance_Monitor[7]),
    .io_value_8  (u_core_top.u_wbu.u_csr.Performance_Monitor[8]),
    .io_value_9  (u_core_top.u_wbu.u_csr.Performance_Monitor[9]),
    .io_value_10 (u_core_top.u_wbu.u_csr.Performance_Monitor[10]),
    .io_value_11 (u_core_top.u_wbu.u_csr.Performance_Monitor[11]),
    .io_value_12 (u_core_top.u_wbu.u_csr.Performance_Monitor[12]),
    .io_value_13 (u_core_top.u_wbu.u_csr.Performance_Monitor[13]),
    .io_value_14 (u_core_top.u_wbu.u_csr.Performance_Monitor[14]),
    .io_value_15 (u_core_top.u_wbu.u_csr.Performance_Monitor[15]),
    .io_value_16 (u_core_top.u_wbu.u_csr.Performance_Monitor[16]),
    .io_value_17 (u_core_top.u_wbu.u_csr.Performance_Monitor[17]),
    .io_value_18 (u_core_top.u_wbu.u_csr.Performance_Monitor[18]),
    .io_value_19 (u_core_top.u_wbu.u_csr.Performance_Monitor[19]),
    .io_value_20 (u_core_top.u_wbu.u_csr.Performance_Monitor[20]),
    .io_value_21 (u_core_top.u_wbu.u_csr.Performance_Monitor[21]),
    .io_value_22 (u_core_top.u_wbu.u_csr.Performance_Monitor[22]),
    .io_value_23 (u_core_top.u_wbu.u_csr.Performance_Monitor[23]),
    .io_value_24 (u_core_top.u_wbu.u_csr.Performance_Monitor[24]),
    .io_value_25 (u_core_top.u_wbu.u_csr.Performance_Monitor[25]),
    .io_value_26 (u_core_top.u_wbu.u_csr.Performance_Monitor[26]),
    .io_value_27 (u_core_top.u_wbu.u_csr.Performance_Monitor[27]),
    .io_value_28 (u_core_top.u_wbu.u_csr.Performance_Monitor[28]),
    .io_value_29 (u_core_top.u_wbu.u_csr.Performance_Monitor[29]),
    .io_value_30 (u_core_top.u_wbu.u_csr.Performance_Monitor[30]),
    .io_value_31 (u_core_top.u_wbu.u_csr.Performance_Monitor[31])
);

DifftestCSRState u_DifftestCSRState (
    .io_privilegeMode ({62'h0, u_core_top.u_wbu.u_csr.current_priv_status}),
    .io_mstatus       (u_core_top.u_wbu.u_csr.mstatus),
    .io_sstatus       (u_core_top.u_wbu.u_csr.sstatus),
    .io_mepc          (u_core_top.u_wbu.u_csr.mepc),
    .io_sepc          (u_core_top.u_wbu.u_csr.sepc),
    .io_mtval         (u_core_top.u_wbu.u_csr.mtval),
    .io_stval         (u_core_top.u_wbu.u_csr.stval),
    .io_mtvec         (u_core_top.u_wbu.u_csr.mtvec),
    .io_stvec         (u_core_top.u_wbu.u_csr.stvec),
    .io_mcause        (u_core_top.u_wbu.u_csr.mcause),
    .io_scause        (u_core_top.u_wbu.u_csr.scause),
    .io_satp          (u_core_top.u_wbu.u_csr.satp),
    .io_mip           (u_core_top.u_wbu.u_csr.mip),
    .io_mie           (u_core_top.u_wbu.u_csr.mie),
    .io_mscratch      (u_core_top.u_wbu.u_csr.mscratch),
    .io_sscratch      (u_core_top.u_wbu.u_csr.sscratch),
    .io_mideleg       (u_core_top.u_wbu.u_csr.mideleg),
    .io_medeleg       (u_core_top.u_wbu.u_csr.medeleg)
);

DifftestInstrCommit u_DifftestInstrCommit (
    .clock      (clock),
    .io_valid   (u_core_top.u_wbu.LS_WB_reg_ls_valid),
    .io_skip    (1'b0),
    .io_isRVC   (1'b0),
    .io_rfwen   (u_core_top.u_wbu.LS_WB_reg_dest_wen),
    .io_fpwen   (1'b0),
    .io_vecwen  (1'b0),
    .io_wpdest  (u_core_top.u_wbu.LS_WB_reg_rd),
    .io_wdest   ({3'h0, u_core_top.u_wbu.LS_WB_reg_rd}),
    .io_pc      (commit_pc),
    .io_instr   (u_core_top.u_wbu.LS_WB_reg_inst),
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
    .enable      (u_core_top.u_wbu.u_csr.u_trap_control.trap_m_interrupt),
    .io_hasTrap  (1'b0),
    .io_cycleCnt (u_core_top.u_wbu.u_csr.Performance_Monitor[1]),
    .io_instrCnt (u_core_top.u_wbu.u_csr.Performance_Monitor[2]),
    .io_hasWFI   (1'b0),
    .io_code     (u_core_top.u_wbu.u_csr.u_trap_control.cause),
    .io_pc       (u_core_top.u_wbu.u_csr.u_trap_control.next_pc),
    .io_coreid   (8'h0)
);

assign tdo = tdi & tms & tck;
assign tap_state = 4'h0;

wire unused_debug = MXR | SUM | MPRV | (|MPP) | (|satp_mode) | (|satp_asid) |
                    (|satp_ppn);

endmodule
