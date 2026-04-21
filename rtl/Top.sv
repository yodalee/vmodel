// Top: Plus1 → ForEach → Repeat
// Input N, Plus1 computes N+1, ForEach outputs 1..N+1, Repeat outputs each
// value N+1 times.
module Top (
    input clk,
    input rst_n,
    input        i_valid,
    output logic i_ready,
    input  [7:0] i_data,
    output logic o_valid,
    input        o_ready,
    output logic [7:0] o_data
);

// Plus1 → ForEach
logic        p1_fe_valid;
logic        p1_fe_ready;
logic [7:0]  p1_fe_data;

// ForEach → Repeat
logic        fe_rp_valid;
logic        fe_rp_ready;
logic [7:0]  fe_rp_data;

Plus1 u_plus1 (
    .clk    (clk),
    .rst_n  (rst_n),
    .i_valid(i_valid),
    .i_ready(i_ready),
    .i_data (i_data),
    .o_valid(p1_fe_valid),
    .o_ready(p1_fe_ready),
    .o_data (p1_fe_data)
);

ForEach u_foreach (
    .clk    (clk),
    .rst_n  (rst_n),
    .i_valid(p1_fe_valid),
    .i_ready(p1_fe_ready),
    .i_data (p1_fe_data),
    .o_valid(fe_rp_valid),
    .o_ready(fe_rp_ready),
    .o_data (fe_rp_data)
);

Repeat u_repeat (
    .clk    (clk),
    .rst_n  (rst_n),
    .i_valid(fe_rp_valid),
    .i_ready(fe_rp_ready),
    .i_data (fe_rp_data),
    .o_valid(o_valid),
    .o_ready(o_ready),
    .o_data (o_data)
);

endmodule
