module Plus1 (
    input clk,
    input rst_n,
    input i_valid,
    output logic i_ready,
    input [7:0] i_data,
    output logic o_valid,
    input o_ready,
    output logic [7:0] o_data
);

logic o_en;

Pipeline pipeline (
    .clk     (clk),
    .rst_n   (rst_n),
    .i_valid (i_valid),
    .i_ready (i_ready),
    .o_en    (o_en),
    .o_valid (o_valid),
    .o_ready (o_ready)
);

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        o_data <= 8'h00;
    end else if (o_en) begin
        o_data <= i_data + 8'd1;
    end
end

endmodule
