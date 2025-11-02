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

    output reg [WORD_BITWIDTH-1:0] pc,
    input [WORD_BITWIDTH-1:0] inst
);


    reg wen;
    wire [3:0] itype;
    wire [WORD_BITWIDTH-1:0] imm,src1,src2;
    reg [WORD_BITWIDTH-1:0] wdata,nxt_pc;
    wire [REG_ADDRWIDTH-1:0] rd,rs1,rs2;

    RegisterFile #(
        .ADDR_WIDTH(REG_ADDRWIDTH),
        .DATA_WIDTH(WORD_BITWIDTH)
    ) regs(
        .clk(clk),
        .waddr(rd),
        .wdata(wdata),
        .raddr1(rs1),.raddr2(rs2),
        .rdata1(src1),.rdata2(src2),
        .wen(wen)
    );

    decode_operand dec_opr(
        .inst(inst),
        .itype(itype),
        .imm(imm),
        .rd(rd),
        .rs1(rs1),
        .rs2(rs2)
    );

    wire [6:0] opcode=inst[6:0];
    wire [2:0] func3t=inst[14:12];

    always@(*)begin
        wen=0;wdata=0;nxt_pc=pc+4;
        case(opcode)
            7'b0010011: begin // ADDI
                if(func3t==0)begin
                    wdata=src1+imm;
                    wen=1;
                    $display("ADDI r%d=r%d(%d)+%d",rd,rs1,src1,imm);
                end
            end
            7'b1100111:begin
                if(func3t==0)begin
                    wdata=pc+4;
                    wen=1;
                    nxt_pc=(src1+imm)&~1;
                    $display("JALR %08X",nxt_pc);
                end
            end
        default:;
        endcase
    end

    always@(posedge clk,posedge rst)begin
        if(rst)begin
            pc<=0;
        end else begin
            pc<=nxt_pc;
        end
    end

endmodule
