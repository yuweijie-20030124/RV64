module rv64_mem_arb #(
    parameter AXI_ADDR_W = 64,
    parameter AXI_ID_W   = 8,
    parameter AXI_DATA_W = 64
) (
    input                           clk,
    input                           rst_n,

    input                           ifu_arvalid,
    output                          ifu_arready,
    input  [AXI_ADDR_W-1:0]         ifu_araddr,
    output                          ifu_rvalid,
    input                           ifu_rready,
    output [1:0]                    ifu_rresp,
    output [AXI_DATA_W-1:0]         ifu_rdata,

    input                           lsu_arvalid,
    output                          lsu_arready,
    input                           lsu_arlock,
    input  [2:0]                    lsu_arsize,
    input  [AXI_ADDR_W-1:0]         lsu_araddr,
    output                          lsu_rvalid,
    input                           lsu_rready,
    output [1:0]                    lsu_rresp,
    output [AXI_DATA_W-1:0]         lsu_rdata,

    input                           lsu_awvalid,
    output                          lsu_awready,
    input                           lsu_awlock,
    input  [2:0]                    lsu_awsize,
    input  [AXI_ADDR_W-1:0]         lsu_awaddr,
    input                           lsu_wvalid,
    output                          lsu_wready,
    input  [AXI_DATA_W/8-1:0]       lsu_wstrb,
    input  [AXI_DATA_W-1:0]         lsu_wdata,
    output                          lsu_bvalid,
    input                           lsu_bready,
    output [1:0]                    lsu_bresp,

    output                          mst_awvalid,
    input                           mst_awready,
    output [AXI_ADDR_W-1:0]         mst_awaddr,
    output [7:0]                    mst_awlen,
    output [2:0]                    mst_awsize,
    output [1:0]                    mst_awburst,
    output                          mst_awlock,
    output [3:0]                    mst_awcache,
    output [2:0]                    mst_awprot,
    output [3:0]                    mst_awqos,
    output [3:0]                    mst_awregion,
    output [AXI_ID_W-1:0]           mst_awid,
    output                          mst_wvalid,
    input                           mst_wready,
    output                          mst_wlast,
    output [AXI_DATA_W-1:0]         mst_wdata,
    output [AXI_DATA_W/8-1:0]       mst_wstrb,
    input                           mst_bvalid,
    output                          mst_bready,
    input  [AXI_ID_W-1:0]           mst_bid,
    input  [1:0]                    mst_bresp,
    output                          mst_arvalid,
    input                           mst_arready,
    output [AXI_ADDR_W-1:0]         mst_araddr,
    output [7:0]                    mst_arlen,
    output [2:0]                    mst_arsize,
    output [1:0]                    mst_arburst,
    output                          mst_arlock,
    output [3:0]                    mst_arcache,
    output [2:0]                    mst_arprot,
    output [3:0]                    mst_arqos,
    output [3:0]                    mst_arregion,
    output [AXI_ID_W-1:0]           mst_arid,
    input                           mst_rvalid,
    output                          mst_rready,
    input  [AXI_ID_W-1:0]           mst_rid,
    input  [1:0]                    mst_rresp,
    input  [AXI_DATA_W-1:0]         mst_rdata,
    input                           mst_rlast
);

localparam REQ_IFU_READ  = 2'h1;
localparam REQ_LSU_READ  = 2'h2;
localparam REQ_LSU_WRITE = 2'h3;

reg [1:0] active;

wire ifu_read_fire  = ifu_arvalid & ifu_arready;
wire lsu_read_fire  = lsu_arvalid & lsu_arready;
wire lsu_write_fire = lsu_awvalid & lsu_awready;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        active <= 2'h0;
    end
    else begin
        case(active)
            2'h0: begin
                if(lsu_write_fire)begin
                    active <= REQ_LSU_WRITE;
                end
                else if(lsu_read_fire)begin
                    active <= REQ_LSU_READ;
                end
                else if(ifu_read_fire)begin
                    active <= REQ_IFU_READ;
                end
            end
            REQ_IFU_READ: begin
                if(mst_rvalid & mst_rready & mst_rlast) active <= 2'h0;
            end
            REQ_LSU_READ: begin
                if(mst_rvalid & mst_rready & mst_rlast) active <= 2'h0;
            end
            REQ_LSU_WRITE: begin
                if(mst_bvalid & mst_bready) active <= 2'h0;
            end
            default: begin
                active <= 2'h0;
            end
        endcase
    end
end

wire idle = (active == 2'h0);
wire sel_lsu_write = idle & lsu_awvalid;
wire sel_lsu_read  = idle & (!sel_lsu_write) & lsu_arvalid;
wire sel_ifu_read  = idle & (!sel_lsu_write) & (!sel_lsu_read) & ifu_arvalid;

assign lsu_awready = sel_lsu_write & mst_awready;
assign lsu_arready = sel_lsu_read  & mst_arready;
assign ifu_arready = sel_ifu_read  & mst_arready;

assign mst_awvalid = sel_lsu_write;
assign mst_awaddr  = lsu_awaddr;
assign mst_awlen   = 8'h0;
assign mst_awsize  = lsu_awsize;
assign mst_awburst = 2'h1;
assign mst_awlock  = lsu_awlock;
assign mst_awcache = 4'h0;
assign mst_awprot  = 3'h0;
assign mst_awqos   = 4'h0;
assign mst_awregion= 4'h0;
assign mst_awid    = 8'h20;

assign mst_wvalid  = (active == REQ_LSU_WRITE) & lsu_wvalid;
assign lsu_wready  = (active == REQ_LSU_WRITE) & mst_wready;
assign mst_wlast   = 1'b1;
assign mst_wdata   = lsu_wdata;
assign mst_wstrb   = lsu_wstrb;

assign lsu_bvalid  = (active == REQ_LSU_WRITE) & mst_bvalid;
assign mst_bready  = (active == REQ_LSU_WRITE) & lsu_bready;
assign lsu_bresp   = mst_bresp;

assign mst_arvalid = sel_lsu_read | sel_ifu_read;
assign mst_araddr  = sel_lsu_read ? lsu_araddr : ifu_araddr;
assign mst_arlen   = 8'h0;
assign mst_arsize  = sel_lsu_read ? lsu_arsize : 3'h3;
assign mst_arburst = 2'h1;
assign mst_arlock  = sel_lsu_read ? lsu_arlock : 1'b0;
assign mst_arcache = 4'h0;
assign mst_arprot  = 3'h0;
assign mst_arqos   = 4'h0;
assign mst_arregion= 4'h0;
assign mst_arid    = sel_lsu_read ? 8'h20 : 8'h10;

assign ifu_rvalid  = (active == REQ_IFU_READ) & mst_rvalid;
assign lsu_rvalid  = (active == REQ_LSU_READ) & mst_rvalid;
assign mst_rready  = ((active == REQ_IFU_READ) & ifu_rready) |
                     ((active == REQ_LSU_READ) & lsu_rready);
assign ifu_rresp   = mst_rresp;
assign lsu_rresp   = mst_rresp;
assign ifu_rdata   = mst_rdata;
assign lsu_rdata   = mst_rdata;

wire unused_mst_bid = |mst_bid;
wire unused_mst_rid = |mst_rid;

endmodule
