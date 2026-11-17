module iverilog_top;
  reg clock = 1'b0;
  reg reset = 1'b1;
  integer cycles = 0;
  integer max_cycles = 1000000000;
  reg [1023:0] wave_file;

  TestSoC dut (
    .clock(clock),
    .reset(reset)
  );

  always #1 clock = ~clock;

  initial begin
    if ($value$plusargs("max-cycles=%d", max_cycles)) begin end
    if ($value$plusargs("wave=%s", wave_file)) begin
      $dumpfile(wave_file);
      $dumpvars(0, iverilog_top);
    end
    #10 reset = 1'b0;
  end

  always @(posedge clock) begin
    if (!reset) begin
      cycles <= cycles + 1;
      if (cycles >= max_cycles) begin
        $fatal(1, "iverilog timeout after %0d cycles", cycles);
      end
    end
  end
endmodule
