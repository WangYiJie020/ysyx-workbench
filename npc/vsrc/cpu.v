parameter int WORD_BITWIDTH=32;
parameter int REG_ADDRWIDTH=5;

localparam TypeN=0,TypeR=1,TypeI=2,TypeS=3,TypeU=4,TypeJ=5,TypeB=6;

module decode_operand(
    input [WORD_BITWIDTH-1:0] inst,
    output [3:0] itype,
    output reg [WORD_BITWIDTH-1:0] imm,
    output [REG_ADDRWIDTH-1:0] rd,rs1,rs2
);
    wire [6:0] opcode=inst[6:0];

    assign rd=inst[11:7];
    assign rs1=inst[19:15];
    assign rs2=inst[24:20];
    assign itype=(opcode==7'b0010011)?TypeI:TypeN;

    wire [31:0] immI={{21{inst[31]}},inst[30:20]};

    always@(*)begin
        case(itype)
            TypeI:imm=immI;
            TypeJ:imm=immI;
            default:imm=32'hcdcdcdcd;
        endcase
    end

endmodule


module cpu(
    input [WORD_BITWIDTH-1:0] pc,
    output [WORD_BITWIDTH-1:0] nxt_pc
);

endmodule
