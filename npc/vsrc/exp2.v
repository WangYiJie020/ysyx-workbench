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

module bcd7seg(
    input [3:0] bcd,
    output reg [7:0] seg
);
    always @(*) begin
        case (bcd)
            4'd0: seg = 8'b11000000; // 0
            4'd1: seg = 8'b11111001; // 1
            4'd2: seg = 8'b10100100; // 2
            4'd3: seg = 8'b10110000; // 3
            4'd4: seg = 8'b10011001; // 4
            4'd5: seg = 8'b10010010; // 5
            4'd6: seg = 8'b10000010; // 6
            4'd7: seg = 8'b11111000; // 7
            4'd8: seg = 8'b10000000; // 8
            4'd9: seg = 8'b10010000; // 9
            default: seg = 8'b11111111; // off
        endcase
    end
endmodule

module exp2(
    input [7:0] in,
    input en,
    output [7:0] seg_hasvalue,
    output [7:0] seg_out
);
    wire [2:0] out;
    wire has_value;
    bcd7seg seg1 (
        .bcd({1'b0, out}),
        .seg(seg_out)
    );
    bcd7seg seg2 (
        .bcd({3'b000, has_value}),
        .seg(seg_hasvalue)
    );
    priority_enc83 pe (
        .in(in),
        .en(en),
        .out(out),
        .has_value(has_value)
    );
endmodule

