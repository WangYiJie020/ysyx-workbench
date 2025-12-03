module alu(
    input [3:0] a,b,
    output [3:0] sub,
    output lt
);
    assign sub = a - b;
    assign lt = (a<b);
endmodule
