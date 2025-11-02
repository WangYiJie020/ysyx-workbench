parameter int WORD_BITWIDTH=32;
parameter int REG_ADDRWIDTH=5;

import "DPI-C" function void raise_break ();

localparam TypeN=0,TypeR=1,TypeI=2,TypeS=3,TypeU=4,TypeJ=5,TypeB=6;

module cpu(
    input [WORD_BITWIDTH-1:0] pc,
    output [WORD_BITWIDTH-1:0] nxt_pc
);
endmodule

module itype_decoder(
    input [6:0] opcode,
    output [3:0] itype
);
// opcode[1:0] should always be 11 for 32bit

wire [4:0] opcu=opcode[6:2];

wire isI=(opcu==5'b00100)||(opcu==5'b11001)||(opcu==5'b00000);
wire isR=(opcu==5'b01100);
wire isS=(opcu==5'b01000);

assign itype=
    isI?TypeI:
    isR?TypeR:
    isS?TypeS:
    TypeN;

endmodule

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

    itype_decoder _idc(.opcode(opcode),.itype(itype));

    wire [31:0] immI={{21{inst[31]}},inst[30:20]};

    always@(*)begin
        if(inst==32'h00100073)raise_break();

        case(itype)
            TypeI:imm=immI;
            TypeJ:imm=immI;
            default:imm=32'hcdcdcdcd;
        endcase
    end

endmodule



