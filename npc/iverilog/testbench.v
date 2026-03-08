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

NPCTestSoC dut (
		.clock(clk),
		.reset(rst)
);

wire exu_out_valid;
wire [31:0] exu_code;
wire [31:0] gpr_a0;

`ifdef SIM_NETLIST
	// `ifdef IS_CI_ENV
	// 	assign exu_out_valid = dut.cpu.\core/_exu_io_out_valid ;
	// 	assign exu_code = dut.cpu.\core/exu/_stageCalc_io_out_bits_dinst_code ;
	// 	assign gpr_a0 = dut.cpu.\core/gprs/reg_10 ;
	// `else
		assign exu_out_valid = dut.cpu.\core._exu_io_out_valid ;
		assign exu_code = dut.cpu.\core.exu._stageCalc_io_out_bits_dinst_code ;
		assign gpr_a0 = dut.cpu.\core.gprs.reg_10 ;
	// `endif
`else
	assign exu_out_valid = dut.cpu.core.exu.io_out_valid;
	assign exu_code = dut.cpu.core.exu.io_in_bits_code;
	assign gpr_a0 = dut.cpu.core.gprs.reg_10;
`endif

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
  //   $dumpfile("wave.fst");
  //   $dumpvars(0, testbench);
		// #1000;
		// $finish;
end

endmodule


/*
assign exu_out_valid = dut.cpu.core._exu_io_out_valid;
assign exu_code = {
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_31_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_30_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_29_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_28_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_27_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_26_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_25_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_24_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_23_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_22_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_21_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_20_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_19_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_18_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_17_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_16_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_15_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_14_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_13_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_12_ ,
	5'b0 , // rd was optimized out
	// dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_11_ ,
	// dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_10_ ,
	// dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_9_ ,
	// dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_8_ ,
	// dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_7_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_6_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_5_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_4_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_3_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_2_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_1_ ,
	dut.cpu.core.\exu._stageCalc_io_out_bits_dinst_code_0_
};
assign gpr_a0 = {
	dut.cpu.core.\gprs.reg_10_31_ ,
	dut.cpu.core.\gprs.reg_10_30_ ,
	dut.cpu.core.\gprs.reg_10_29_ ,
	dut.cpu.core.\gprs.reg_10_28_ ,
	dut.cpu.core.\gprs.reg_10_27_ ,
	dut.cpu.core.\gprs.reg_10_26_ ,
	dut.cpu.core.\gprs.reg_10_25_ ,
	dut.cpu.core.\gprs.reg_10_24_ ,
	dut.cpu.core.\gprs.reg_10_23_ ,
	dut.cpu.core.\gprs.reg_10_22_ ,
	dut.cpu.core.\gprs.reg_10_21_ ,
	dut.cpu.core.\gprs.reg_10_20_ ,
	dut.cpu.core.\gprs.reg_10_19_ ,
	dut.cpu.core.\gprs.reg_10_18_ ,
	dut.cpu.core.\gprs.reg_10_17_ ,
	dut.cpu.core.\gprs.reg_10_16_ ,
	dut.cpu.core.\gprs.reg_10_15_ ,
	dut.cpu.core.\gprs.reg_10_14_ ,
	dut.cpu.core.\gprs.reg_10_13_ ,
	dut.cpu.core.\gprs.reg_10_12_ ,
	dut.cpu.core.\gprs.reg_10_11_ ,
	dut.cpu.core.\gprs.reg_10_10_ ,
	dut.cpu.core.\gprs.reg_10_9_ ,
	dut.cpu.core.\gprs.reg_10_8_ ,
	dut.cpu.core.\gprs.reg_10_7_ ,
	dut.cpu.core.\gprs.reg_10_6_ ,
	dut.cpu.core.\gprs.reg_10_5_ ,
	dut.cpu.core.\gprs.reg_10_4_ ,
	dut.cpu.core.\gprs.reg_10_3_ ,
	dut.cpu.core.\gprs.reg_10_2_ ,
	dut.cpu.core.\gprs.reg_10_1_ ,
	dut.cpu.core.\gprs.reg_10_0_
};
*/
