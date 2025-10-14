module scpu(
    input [7:0] code,
    input [3:0] pc,
    output [3:0] nxt_pc,
    output [7:0] out,

    input [31:0] rinbus,
    output [31:0] routbus
);
    parameter ADD=2'b00,LI=2'b10,BNER0=2'b11;
    parameter OUT=2'b01;

    wire [7:0] rin[0:3];
    assign rin[0] = rinbus[7:0];
    assign rin[1] = rinbus[15:8];
    assign rin[2] = rinbus[23:16];
    assign rin[3] = rinbus[31:24];
endmodule
