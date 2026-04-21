module ForEach (
    input clk,
    input rst_n,
    input i_valid,
    output logic i_ready,
    input [7:0] i_data,
    output logic o_valid,
    input o_ready,
    output logic [7:0] o_data
);

logic i_cen, o_cen, o_done;
logic [7:0] counter;
logic [7:0] n_reg;

// Gate i_valid so N==0 is silently consumed without starting the loop
PipelineLoop loop_ctrl (
    .clk(clk),
    .rst_n(rst_n),
    .i_valid(i_valid && i_data != 8'h0),
    .i_ready(i_ready),
    .i_cen(i_cen),
    .o_valid(o_valid),
    .o_ready(o_ready),
    .o_done(o_done),
    .o_cen(o_cen)
);

assign o_done = (counter == n_reg);
assign o_data = counter;

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        counter <= 8'h0;
        n_reg   <= 8'h0;
    end else begin
        if (i_cen) begin
            n_reg   <= i_data;
            counter <= 8'h1;
        end else if (o_cen) begin
            counter <= counter + 8'h1;
        end
    end
end

endmodule
