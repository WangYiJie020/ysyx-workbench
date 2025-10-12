module exp6(
    input clk,
    output reg [7:0] out
);
    always @(posedge clk) begin
        if(out==0)
            out<=8'b00000001;
        else out<={
            out[4]^out[3]^out[2]^out[0],
            out[7:1]
        };
    end
endmodule
