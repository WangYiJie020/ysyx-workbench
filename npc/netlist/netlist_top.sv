module NetlistMemory (
  input         clock,
  input         reset,
  input         ifuReqValid,
  input  [31:0] ifuAddr,
  output reg    ifuRespValid,
  output reg [31:0] ifuRdata,
  input         lsuReqValid,
  input  [31:0] lsuAddr,
  input         lsuWen,
  input  [31:0] lsuWdata,
  input  [3:0]  lsuWmask,
  output reg    lsuRespValid,
  output reg [31:0] lsuRdata
);
`ifndef __ICARUS__
  import "DPI-C" function int pmem_read(input int addr);
  import "DPI-C" function void pmem_write(input int addr, input int data, input byte mask);
`endif

  always @(posedge clock) begin
    if (reset) begin
      ifuRespValid <= 1'b0;
      lsuRespValid <= 1'b0;
    end else begin
      ifuRespValid <= ifuReqValid;
      lsuRespValid <= lsuReqValid;
`ifdef __ICARUS__
      if (ifuReqValid) $pmem_read(ifuAddr, ifuRdata);
      if (lsuReqValid && lsuWen) $pmem_write(lsuAddr, lsuWdata, {4'b0, lsuWmask});
      if (lsuReqValid && !lsuWen) $pmem_read(lsuAddr, lsuRdata);
`else
      if (ifuReqValid) ifuRdata <= pmem_read(ifuAddr);
      if (lsuReqValid && lsuWen) pmem_write(lsuAddr, lsuWdata, {4'b0, lsuWmask});
      if (lsuReqValid && !lsuWen) lsuRdata <= pmem_read(lsuAddr);
`endif
    end
  end
endmodule

module NetlistTop (
  input  clock,
  input  reset,
  output reg halted
);
  wire        ifuReqValid;
  wire [31:0] ifuAddr;
  wire        ifuRespValid;
  wire [31:0] ifuRdata;
  wire        lsuReqValid;
  wire [31:0] lsuAddr;
  wire [1:0]  lsuSize;
  wire        lsuWen;
  wire [31:0] lsuWdata;
  wire [3:0]  lsuWmask;
  wire        lsuRespValid;
  wire [31:0] lsuRdata;

  ysyx_25100261 cpu (
    .clock(clock),
    .reset(reset),
    .io_ifu_reqValid(ifuReqValid),
    .io_ifu_addr(ifuAddr),
    .io_ifu_respValid(ifuRespValid),
    .io_ifu_rdata(ifuRdata),
    .io_lsu_reqValid(lsuReqValid),
    .io_lsu_addr(lsuAddr),
    .io_lsu_size(lsuSize),
    .io_lsu_wen(lsuWen),
    .io_lsu_wdata(lsuWdata),
    .io_lsu_wmask(lsuWmask),
    .io_lsu_respValid(lsuRespValid),
    .io_lsu_rdata(lsuRdata)
  );

  NetlistMemory memory (
    .clock(clock),
    .reset(reset),
    .ifuReqValid(ifuReqValid),
    .ifuAddr(ifuAddr),
    .ifuRespValid(ifuRespValid),
    .ifuRdata(ifuRdata),
    .lsuReqValid(lsuReqValid),
    .lsuAddr(lsuAddr),
    .lsuWen(lsuWen),
    .lsuWdata(lsuWdata),
    .lsuWmask(lsuWmask),
    .lsuRespValid(lsuRespValid),
    .lsuRdata(lsuRdata)
  );

  always @(posedge clock) begin
    if (reset) begin
      halted <= 1'b0;
    end else if (ifuRespValid && ifuRdata === 32'h00100073) begin
      $display("NPC netlist fetched ebreak at 0x%08x", ifuAddr);
      halted <= 1'b1;
    end
  end
endmodule

module iverilog_netlist_top;
  reg clock = 1'b0;
  reg reset = 1'b1;
  wire halted;
  integer cycles = 0;
  integer maxCycles = 1000000000;
  reg [1023:0] waveFile;

  NetlistTop dut (
    .clock(clock),
    .reset(reset),
    .halted(halted)
  );

  always #1 clock = ~clock;

  initial begin
    if ($value$plusargs("max-cycles=%d", maxCycles)) begin end
    if ($value$plusargs("wave=%s", waveFile)) begin
      $dumpfile(waveFile);
      $dumpvars(0, iverilog_netlist_top);
    end
    #10 reset = 1'b0;
  end

  always @(posedge clock) begin
    if (!reset) begin
      cycles <= cycles + 1;
      if (halted) $finish;
      if (cycles >= maxCycles) $fatal(1, "netlist timeout after %0d cycles", cycles);
    end
  end
endmodule
