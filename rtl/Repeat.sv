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

typedef enum logic [0:0] { IDLE, OUTPUTTING } state_t;

state_t state;
logic [7:0] data_reg;
logic [7:0] count_reg;

assign i_ready = (state == IDLE);
assign o_valid = (state == OUTPUTTING);
assign o_data  = data_reg;

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        state     <= IDLE;
        data_reg  <= 8'h00;
        count_reg <= 8'h00;
    end else begin
        case (state)
            IDLE: begin
                if (i_valid && i_data != 8'h00) begin
                    data_reg  <= i_data;
                    count_reg <= i_data;
                    state     <= OUTPUTTING;
                end
            end
            OUTPUTTING: begin
                if (o_ready) begin
                    count_reg <= count_reg - 8'd1;
                    if (count_reg == 8'd1) begin
                        state <= IDLE;
                    end
                end
            end
        endcase
    end
end

endmodule
