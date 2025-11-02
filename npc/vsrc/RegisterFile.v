module RegisterFile #(ADDR_WIDTH = 1, DATA_WIDTH = 1) (
  input clk,
  input [ADDR_WIDTH-1:0] waddr,
  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] raddr1,raddr2,
  output [DATA_WIDTH-1:0] rdata1,rdata2,
  input wen
);
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];

  assign rdata1=(raddr1!=0)?rf[raddr1]:0;
  assign rdata2=(raddr2!=0)?rf[raddr2]:0;

  always @(posedge clk) begin
    if (wen) rf[waddr] <= wdata;
  end
endmodule
