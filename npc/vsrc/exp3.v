
module exp3 #(WIDTH=8) (
    input [2:0] op,
    input signed [WIDTH-1:0] a,
    input signed [WIDTH-1:0] b,
    output [WIDTH-1:0] out,
    output overflow,
    output carry,
    output zero
);
parameter ADD = 3'b000;
parameter SUB = 3'b001;
parameter NOT = 3'b010;
parameter AND = 3'b011;
parameter OR  = 3'b100;
parameter XOR = 3'b101;
parameter ALESSB = 3'b110;
parameter AEQB = 3'b111;

    wire signed [WIDTH:0] add_res;
    wire signed [WIDTH:0] sub_res;
    wire signed [WIDTH-1:0] not_res;
    wire signed [WIDTH-1:0] and_res;
    wire signed [WIDTH-1:0] or_res;
    wire signed [WIDTH-1:0] xor_res;

    wire is_alessb;
    wire is_aeqb;

    assign add_res = a + b;
    assign sub_res = a - b;
    assign not_res = ~a;
    assign and_res = a & b;
    assign or_res = a | b;
    assign xor_res = a ^ b;

    assign is_alessb = (a < b) ? 1'b1 : 1'b0;
    assign is_aeqb = (a == b) ? 1'b1 : 1'b0;

    assign overflow = (op == ADD) ? ((a[WIDTH-1] == b[WIDTH-1]) && (add_res[WIDTH-1] != a[WIDTH-1])) :
                      (op == SUB) ? ((a[WIDTH-1] != b[WIDTH-1]) && (sub_res[WIDTH-1] != a[WIDTH-1])) :
                      1'b0;
    assign carry = ((op == ADD) & add_res[WIDTH]) | 
        ((op == SUB) & sub_res[WIDTH]);

    wire [WIDTH*8+7:0] out_table;
    assign out_table={
        add_res[WIDTH-1:0],
        sub_res[WIDTH-1:0],
        not_res,
        and_res,
        or_res,
        xor_res,
        {7'b0,is_alessb},
        {7'b0,is_aeqb}
    };
    assign out = out_table[(7-op)*WIDTH +: WIDTH];

    assign zero = (out == 0);
endmodule

