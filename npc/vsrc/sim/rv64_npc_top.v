import "DPI-C" function void pc_inst_end(input int thepc_data, input int the_inst, input int diff_skip_flag);
import "DPI-C" function void set_npc_exit(input int pc, input int halt_ret);

module rv64_npc_top (
    input clock,
    input reset
);

wire rst_n = !reset;

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
    .MHARTID ( 0 ),
    .RST_PC  ( `RST_PC )
) u_core_top (
    .clk         ( clock ),
    .rst_n       ( rst_n ),
    .stip_asyn   ( 1'b0  ),
    .seip_asyn   ( 1'b0  ),
    .ssip_asyn   ( 1'b0  ),
    .mtip_asyn   ( 1'b0  ),
    .meip_asyn   ( 1'b0  ),
    .msip_asyn   ( 1'b0  ),
    .halt_req    ( 1'b0  ),
    .MXR         ( MXR ),
    .SUM         ( SUM ),
    .MPRV        ( MPRV ),
    .MPP         ( MPP ),
    .satp_mode   ( satp_mode ),
    .satp_asid   ( satp_asid ),
    .satp_ppn    ( satp_ppn ),
    .ifu_arready ( ifu_arready ),
    .ifu_arvalid ( ifu_arvalid ),
    .ifu_araddr  ( ifu_araddr ),
    .ifu_rvalid  ( ifu_rvalid ),
    .ifu_rready  ( ifu_rready ),
    .ifu_rresp   ( ifu_rresp ),
    .ifu_rdata   ( ifu_rdata ),
    .lsu_arvalid ( lsu_arvalid ),
    .lsu_arready ( lsu_arready ),
    .lsu_arlock  ( lsu_arlock ),
    .lsu_arsize  ( lsu_arsize ),
    .lsu_araddr  ( lsu_araddr ),
    .lsu_rvalid  ( lsu_rvalid ),
    .lsu_rready  ( lsu_rready ),
    .lsu_rresp   ( lsu_rresp ),
    .lsu_rdata   ( lsu_rdata ),
    .lsu_awvalid ( lsu_awvalid ),
    .lsu_awready ( lsu_awready ),
    .lsu_awlock  ( lsu_awlock ),
    .lsu_awsize  ( lsu_awsize ),
    .lsu_awaddr  ( lsu_awaddr ),
    .lsu_wvalid  ( lsu_wvalid ),
    .lsu_wready  ( lsu_wready ),
    .lsu_wstrb   ( lsu_wstrb ),
    .lsu_wdata   ( lsu_wdata ),
    .lsu_bvalid  ( lsu_bvalid ),
    .lsu_bready  ( lsu_bready ),
    .lsu_bresp   ( lsu_bresp )
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
    .clk          ( clock ),
    .rst_n        ( rst_n ),
    .ifu_arvalid  ( ifu_arvalid ),
    .ifu_arready  ( ifu_arready ),
    .ifu_araddr   ( ifu_araddr ),
    .ifu_rvalid   ( ifu_rvalid ),
    .ifu_rready   ( ifu_rready ),
    .ifu_rresp    ( ifu_rresp ),
    .ifu_rdata    ( ifu_rdata ),
    .lsu_arvalid  ( lsu_arvalid ),
    .lsu_arready  ( lsu_arready ),
    .lsu_arlock   ( lsu_arlock ),
    .lsu_arsize   ( lsu_arsize ),
    .lsu_araddr   ( lsu_araddr ),
    .lsu_rvalid   ( lsu_rvalid ),
    .lsu_rready   ( lsu_rready ),
    .lsu_rresp    ( lsu_rresp ),
    .lsu_rdata    ( lsu_rdata ),
    .lsu_awvalid  ( lsu_awvalid ),
    .lsu_awready  ( lsu_awready ),
    .lsu_awlock   ( lsu_awlock ),
    .lsu_awsize   ( lsu_awsize ),
    .lsu_awaddr   ( lsu_awaddr ),
    .lsu_wvalid   ( lsu_wvalid ),
    .lsu_wready   ( lsu_wready ),
    .lsu_wstrb    ( lsu_wstrb ),
    .lsu_wdata    ( lsu_wdata ),
    .lsu_bvalid   ( lsu_bvalid ),
    .lsu_bready   ( lsu_bready ),
    .lsu_bresp    ( lsu_bresp ),
    .mst_awvalid  ( mst_awvalid ),
    .mst_awready  ( mst_awready ),
    .mst_awaddr   ( mst_awaddr ),
    .mst_awlen    ( mst_awlen ),
    .mst_awsize   ( mst_awsize ),
    .mst_awburst  ( mst_awburst ),
    .mst_awlock   ( mst_awlock ),
    .mst_awcache  ( mst_awcache ),
    .mst_awprot   ( mst_awprot ),
    .mst_awqos    ( mst_awqos ),
    .mst_awregion ( mst_awregion ),
    .mst_awid     ( mst_awid ),
    .mst_wvalid   ( mst_wvalid ),
    .mst_wready   ( mst_wready ),
    .mst_wlast    ( mst_wlast ),
    .mst_wdata    ( mst_wdata ),
    .mst_wstrb    ( mst_wstrb ),
    .mst_bvalid   ( mst_bvalid ),
    .mst_bready   ( mst_bready ),
    .mst_bid      ( mst_bid ),
    .mst_bresp    ( mst_bresp ),
    .mst_arvalid  ( mst_arvalid ),
    .mst_arready  ( mst_arready ),
    .mst_araddr   ( mst_araddr ),
    .mst_arlen    ( mst_arlen ),
    .mst_arsize   ( mst_arsize ),
    .mst_arburst  ( mst_arburst ),
    .mst_arlock   ( mst_arlock ),
    .mst_arcache  ( mst_arcache ),
    .mst_arprot   ( mst_arprot ),
    .mst_arqos    ( mst_arqos ),
    .mst_arregion ( mst_arregion ),
    .mst_arid     ( mst_arid ),
    .mst_rvalid   ( mst_rvalid ),
    .mst_rready   ( mst_rready ),
    .mst_rid      ( mst_rid ),
    .mst_rresp    ( mst_rresp ),
    .mst_rdata    ( mst_rdata ),
    .mst_rlast    ( mst_rlast )
);

sim_sram_dpic u_sim_sram_dpic (
    .aclk         ( clock ),
    .arst_n       ( rst_n ),
    .mst_awvalid  ( mst_awvalid ),
    .mst_awready  ( mst_awready ),
    .mst_awaddr   ( mst_awaddr ),
    .mst_awlen    ( mst_awlen ),
    .mst_awsize   ( mst_awsize ),
    .mst_awburst  ( mst_awburst ),
    .mst_awlock   ( mst_awlock ),
    .mst_awcache  ( mst_awcache ),
    .mst_awprot   ( mst_awprot ),
    .mst_awqos    ( mst_awqos ),
    .mst_awregion ( mst_awregion ),
    .mst_awid     ( mst_awid ),
    .mst_wvalid   ( mst_wvalid ),
    .mst_wready   ( mst_wready ),
    .mst_wlast    ( mst_wlast ),
    .mst_wdata    ( mst_wdata ),
    .mst_wstrb    ( mst_wstrb ),
    .mst_bvalid   ( mst_bvalid ),
    .mst_bready   ( mst_bready ),
    .mst_bid      ( mst_bid ),
    .mst_bresp    ( mst_bresp ),
    .mst_arvalid  ( mst_arvalid ),
    .mst_arready  ( mst_arready ),
    .mst_araddr   ( mst_araddr ),
    .mst_arlen    ( mst_arlen ),
    .mst_arsize   ( mst_arsize ),
    .mst_arburst  ( mst_arburst ),
    .mst_arlock   ( mst_arlock ),
    .mst_arcache  ( mst_arcache ),
    .mst_arprot   ( mst_arprot ),
    .mst_arqos    ( mst_arqos ),
    .mst_arregion ( mst_arregion ),
    .mst_arid     ( mst_arid ),
    .mst_rvalid   ( mst_rvalid ),
    .mst_rready   ( mst_rready ),
    .mst_rid      ( mst_rid ),
    .mst_rresp    ( mst_rresp ),
    .mst_rdata    ( mst_rdata ),
    .mst_rlast    ( mst_rlast )
);

wire commit_valid = u_core_top.u_wbu.LS_WB_reg_ls_valid;
wire [31:0] commit_inst = u_core_top.u_wbu.LS_WB_reg_inst;
wire [63:0] commit_pc = u_core_top.u_wbu.LS_WB_reg_PC;
wire [63:0] commit_a0 = u_core_top.u_wbu.u_gpr.riscv_reg[10];

always @(posedge clock) begin
    if(rst_n & commit_valid)begin
        pc_inst_end(commit_pc[31:0], commit_inst, 0);
        if(commit_inst == 32'h00100073)begin
            set_npc_exit(commit_pc[31:0], commit_a0[31:0]);
        end
    end
end

wire unused_status = MXR | SUM | MPRV | (|MPP) | (|satp_mode) | (|satp_asid) | (|satp_ppn);

endmodule
