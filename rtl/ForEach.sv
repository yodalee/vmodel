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

// busy: currently streaming 1..N to output
logic busy;
logic [7:0] counter;
logic [7:0] n_reg;

assign i_ready = !busy;
assign o_valid = busy;
assign o_data  = counter;

always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        busy    <= 1'b0;
        counter <= 8'h0;
        n_reg   <= 8'h0;
    end else begin
        if (!busy) begin
            // Accept input when not busy; skip if N==0
            if (i_valid && i_data != 8'h0) begin
                busy    <= 1'b1;
                n_reg   <= i_data;
                counter <= 8'h1;
            end
        end else begin
            // Stream counter until it reaches N
            if (o_ready) begin
                if (counter == n_reg) begin
                    busy    <= 1'b0;
                    counter <= 8'h0;
                end else begin
                    counter <= counter + 8'h1;
                end
            end
        end
    end
end

endmodule
