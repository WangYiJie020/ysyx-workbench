parameter
    MCYCLE_ADDR = 12'hB00,
    MCYCLEH_ADDR= 12'hB80,
    MSTATUS_ADDR= 12'h300,
    MEPC_ADDR   = 12'h341,
    MCAUSE_ADDR = 12'h342,
    MTVEC_ADDR  = 12'h305;

parameter MVENDORID_ADDR = 12'hF11;
parameter MARCHID_ADDR   = 12'hF12;

parameter MVENDORID_VALUE = 32'h79737978; // "ysyx"
parameter MARCHID_VALUE   = 32'd25100261;

module ControlStatusRegister(
    input clk,
    input [31:0] pc,
    input [31:0] inst,
    input [11:0] addr,
    input [31:0] wdata,
    input wen,ren,
    output reg [31:0] rdata
);
    reg [31:0] rf[5];

    reg [63:0] mcycle;

    reg [2:0] idx;
    always@(*) begin case(addr)
        MSTATUS_ADDR: idx = 3'd0;
        MEPC_ADDR:    idx = 3'd1;
        MCAUSE_ADDR:  idx = 3'd2;
        MTVEC_ADDR:   idx = 3'd3;
        default:      idx = 3'd4; // unused
    endcase end

    always@(*) if(ren) begin
        case(addr)
            MCYCLE_ADDR:    rdata = mcycle[31:0];
            MCYCLEH_ADDR:   rdata = mcycle[63:32];
            MVENDORID_ADDR: rdata = MVENDORID_VALUE;
            MARCHID_ADDR:   rdata = MARCHID_VALUE;
            default:        rdata = rf[idx];
        endcase
    end else begin
        rdata = 32'd0;
    end

    always@(posedge clk) begin
        $display("CSR : addr %03X data %08X wen %b ren %b",addr,wdata,wen,ren);
        mcycle <= mcycle + 1;
        if(inst==INST_ECALL) begin
            rf[1] <= pc; // mepc
            rf[2] <= 32'd11; // mcause, environment call from M-mode
        end else begin
            if(wen) rf[idx] <= wdata;
        end
    end
endmodule
