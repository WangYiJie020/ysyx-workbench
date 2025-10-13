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
    output [7:0] seg0,
    output [7:0] seg1,
    output [7:0] seg2,
    output [7:0] seg3,
    output [7:0] seg4,
    output [7:0] seg5,
    output [7:0] seg6,
    output [7:0] seg7
);

    wire [7:0] ps2_out;
    wire notidle,idle;

    ps2test _keyboard(
        .clk(ps2_clk),
        .d(ps2_data),
        .data(ps2_out),
        .ready(),
        .idle(idle)
    );
assign ledr[0]=idle;
    assign notidle = ~idle;
    bcd7seg seghigh(
        .bcd(notidle?ps2_out[7:4]:0),
        .seg(seg1)
    );
    bcd7seg seglow(
        .bcd(notidle?ps2_out[3:0]:0),
        .seg(seg0)
    );
endmodule
