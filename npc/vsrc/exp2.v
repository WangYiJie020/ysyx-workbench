module priority_enc83(
    input [7:0] in,
    input en,
    output reg [2:0] out,
    output has_value
);

    always @(*) begin
        if (!en) begin
            out = 3'bxxx; // output undefined when not enabled
        end
        else begin
            casez (in)
                8'b1???????: out = 3'b111; // highest priority
                8'b01??????: out = 3'b110;
                8'b001?????: out = 3'b101;
                8'b0001????: out = 3'b100;
                8'b00001???: out = 3'b011;
                8'b000001??: out = 3'b010;
                8'b0000001?: out = 3'b001;
                8'b00000001: out = 3'b000; // lowest priority
                8'b00000000: out = 3'bxxx;
            endcase
        end
    end
    assign has_value = en & (|in);
endmodule

module exp2(
    input [7:0] in,
    output [2:0] out
);
    priority_enc83 pe (
        .in(in),
        .en(1'b1),
        .out(out),
        .has_value() // unused output
    );
endmodule

