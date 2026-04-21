module Repeat (
    input clk,
    input rst_n,
    input i_valid,
    output logic i_ready,
    input [7:0] i_data,
    output logic o_valid,
    input o_ready,
    output logic [7:0] o_data
);

logic [7:0] data_reg;
logic [7:0] count_reg;

logic pl_i_valid;
logic pl_i_ready;
logic pl_i_cen;
logic pl_o_cen;
logic pl_o_done;

// Skip N=0: accept the handshake but don't start a loop
assign pl_i_valid = i_valid && (i_data != 8'h00);
assign i_ready    = pl_i_ready;
assign o_data     = data_reg;
assign pl_o_done  = (count_reg == 8'd1);

PipelineLoop pl (
    .clk     (clk),
    .rst_n   (rst_n),
    .i_valid (pl_i_valid),
    .i_ready (pl_i_ready),
    .i_cen   (pl_i_cen),
    .o_valid (o_valid),
    .o_ready (o_ready),
    .o_done  (pl_o_done),
    .o_cen   (pl_o_cen)
);

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        data_reg  <= 8'h00;
        count_reg <= 8'h00;
    end else begin
        if (pl_i_cen) begin
            data_reg  <= i_data;
            count_reg <= i_data;
        end else if (pl_o_cen) begin
            count_reg <= count_reg - 8'd1;
        end
    end
end

endmodule
