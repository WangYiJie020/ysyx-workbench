`timescale 1ns/1ps

module testbench;
reg clk;
reg rst;

initial begin
		clk = 0;
		rst = 1;
		#30 rst = 0;
		forever #5 clk = ~clk;
end

ysyx_25100261 dut (
		.clock(clk),
		.reset(rst)
);

initial begin
    $dumpfile("wave.fst");
    $dumpvars(0, testbench);
		#3000000000 $finish;
end

endmodule
