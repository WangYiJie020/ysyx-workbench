`timescale 1ns/1ps

module testbench;
reg clk;
reg rst;

always #5 clk = ~clk;

initial begin
		clk = 0;
		rst = 0;
		#10 rst = 1;
		repeat(30) @(posedge clk);
		rst = 0;
end

ysyx_25100261 dut (
		.clock(clk),
		.reset(rst)
);

wire exu_out_valid;// = dut.core._exu_io_out_valid;
wire [31:0] exu_code;
wire [31:0] gpr_a0;

assign exu_out_valid = dut.cpu.exu.io_out_valid;
assign exu_code = dut.cpu.exu.io_in_bits_code;
assign gpr_a0 = dut.cpu.gprs.a0;

always @(posedge clk) begin
	if (exu_out_valid && exu_code == 32'h00100073) begin // EBREAK
		$display("EBREAK instruction executed. Ending simulation.");
		if(gpr_a0 != 0) begin
			$display("HIT BAD TRAP a0 = %d", gpr_a0);
			$fatal;
		end else begin
			$display("HIT GOOD TRAP");
		end
		$finish;
	end
end

initial begin
    // $dumpfile("wave.fst");
    // $dumpvars(0, testbench);
end

endmodule
