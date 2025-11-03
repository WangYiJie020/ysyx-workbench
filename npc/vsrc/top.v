import "DPI-C" function void raise_break();
import "DPI-C" function int pmem_read(input int raddr);
import "DPI-C" function void pmem_write(
  input int waddr, input int wdata, input byte wmask);

module top(
    input clk,
    input rst,
    input [4:0] btn,
    input [15:0] sw,
    input ps2_clk,
    input ps2_data,
    input uart_rx,
    output uart_tx,
    output [15:0] ledr,
    output VGA_CLK,
    output VGA_HSYNC,
    output VGA_VSYNC,
    output VGA_BLANK_N,
    output [7:0] VGA_R,
    output [7:0] VGA_G,
    output [7:0] VGA_B,
    output reg [7:0] seg0,
    output reg [7:0] seg1,
    output [7:0] seg2,
    output [7:0] seg3,
    output [7:0] seg4,
    output [7:0] seg5,
    output [7:0] seg6,
    output [7:0] seg7,

    output reg [WORD_BITWIDTH-1:0] pc
);

    wire [WORD_BITWIDTH-1:0] inst=pmem_read(pc);

    reg wen;
    wire [3:0] itype;
    wire [WORD_BITWIDTH-1:0] imm,src1,src2,alu_s1,alu_s2,alu_res;
    reg [WORD_BITWIDTH-1:0] wdata,nxt_pc;
    wire [REG_ADDRWIDTH-1:0] rd,rs1,rs2;

    wire is_arithmetic, is_load, is_jalr;
    wire is_lui,is_auipc;

    RegisterFile #(
        .ADDR_WIDTH(REG_ADDRWIDTH),
        .DATA_WIDTH(WORD_BITWIDTH)
    ) regs(
        .clk(clk),
        .waddr(rd),
        .wdata(wdata),
        .raddr1(rs1),.raddr2(rs2),
        .rdata1(src1),.rdata2(src2),
        .wen(wen),
        .dump_info(inst==INST_EBREAK)
    );

    decode_operand dec_opr(
        .inst(inst),
        .itype(itype),
        .imm(imm),
        .rd(rd),
        .rs1(rs1),
        .rs2(rs2),

        .is_jalr(is_jalr),
        .is_arithmetic(is_arithmetic),
        .is_load(is_load),
        .is_lui(is_lui),
        .is_auipc(is_auipc)
    );

    wire [6:0] opcode=inst[6:0];
    wire [2:0] func3t=inst[14:12];
    wire [6:0] func7t=inst[31:25];

    // use alu only when TypeR/TypeI
    assign alu_s1=src1;
    assign alu_s2=(itype==TypeR)?src2:imm;
    wire is_imm;
    assign is_imm=(itype==TypeI);

    alu _alu(
        .is_imm(is_imm),
        .func3t(func3t),.func7t(func7t),
        .src1(alu_s1),.src2(alu_s2),.res(alu_res)
    );


    assign nxt_pc=is_jalr?((src1+imm)&~1):(pc+4);

    assign wen=(itype!=TypeS)&&(itype!=TypeN);

    always@(*)begin
        wdata=32'hCDCDCDCD;
        case(itype)
            TypeI:begin
                if(is_jalr)begin
                    wdata=pc+4;
                end else if(is_arithmetic)begin
                    wdata=alu_res;
                end else if(is_load)begin
                end
            end
            TypeR:wdata=alu_res;
            TypeU:begin
                wdata=imm;
            end
            default:;

        endcase

    end

    always@(posedge clk,posedge rst)begin
        $display("pc %08x: inst %08X",pc,inst);
        if(inst==INST_EBREAK)begin
            raise_break();
        end
        $display("rs1(r%d)=%08X(%d) rs2(r%d)=%08X(%d) imm=%08X(%d)",
            rs1,src1,src1,
            rs2,src2,src2,
            imm,imm);
        if(is_jalr)$display("JALR %08X",nxt_pc);
        if(is_arithmetic)$display("Write arithemtic result");
        if(is_load)$display("Load");
        if(is_lui)$display("LUI");

        if(wen)$display("update r%d <- %08X(%d)",rd,wdata,wdata);

        if(rst)begin
            pc<=0;
        end else begin
            pc<=nxt_pc;
        end
    end

endmodule
