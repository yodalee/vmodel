// Verilog testbench for Top (Plus1 → ForEach → Repeat)
//
// Feeds inputs 1 and 3, verifies the output sequence:
//   N=1 → Plus1→2, ForEach→[1,2], Repeat→[1, 2,2]              => [1, 2, 2]
//   N=3 → Plus1→4, ForEach→[1,2,3,4],
//          Repeat→[1, 2,2, 3,3,3, 4,4,4,4]                      => [1,2,2,3,3,3,4,4,4,4]
// Combined: [1, 2, 2, 1, 2, 2, 3, 3, 3, 4, 4, 4, 4]
//
// Clock period : 10 ns
// Reset active : deasserts at a random cycle between 5 and 10
// Input feeding: first input presented at t = 50 ns

module tb_top;

    // ----------------------------------------------------------------
    // Signals
    // ----------------------------------------------------------------
    logic        clk;
    logic        rst_n;
    logic        i_valid;
    logic        i_ready;
    logic [7:0]  i_data;
    logic        o_valid;
    logic        o_ready;
    logic [7:0]  o_data;

    // ----------------------------------------------------------------
    // DUT
    // ----------------------------------------------------------------
    Top dut (
        .clk    (clk),
        .rst_n  (rst_n),
        .i_valid(i_valid),
        .i_ready(i_ready),
        .i_data (i_data),
        .o_valid(o_valid),
        .o_ready(o_ready),
        .o_data (o_data)
    );

    // ----------------------------------------------------------------
    // Clock: 10 ns period
    // ----------------------------------------------------------------
    initial clk = 1'b0;
    always #5 clk = ~clk;

    // ----------------------------------------------------------------
    // Expected output sequence
    // ----------------------------------------------------------------
    localparam int NUM_OUTPUTS = 13;
    logic [7:0] exp_out [NUM_OUTPUTS] = '{
        8'd1, 8'd2, 8'd2, 8'd3, 8'd3, 8'd3, 8'd4, 8'd4, 8'd4, 8'd4,
        8'd1, 8'd2, 8'd2
    };

    // ----------------------------------------------------------------
    // Input handshake task: drive i_valid/i_data, wait for i_ready
    // ----------------------------------------------------------------
    task automatic send_input(input logic [7:0] data);
        i_valid = 1'b1;
        i_data  = data;
        @(posedge clk);
        while (!i_ready) @(posedge clk);
        i_valid = 1'b0;
    endtask

    // ----------------------------------------------------------------
    // Reset + input stimulus
    // ----------------------------------------------------------------
    initial begin
        rst_n   = 1'b0;
        i_valid = 1'b0;
        i_data  = 8'h00;
        o_ready = 1'b1;
        repeat (5) @(posedge clk);
        rst_n = 1'b1;

        #50;

        send_input(8'd3);
        send_input(8'd1);
    end

    // ----------------------------------------------------------------
    // Output checker (int is 2-state, zero-initialized by default)
    // ----------------------------------------------------------------
    int out_idx;
    int errors;

    always @(posedge clk) begin
        if (o_valid && o_ready) begin
            if (o_data !== exp_out[out_idx]) begin
                $error("output[%0d]: got %3d, expected %3d",
                       out_idx, o_data, exp_out[out_idx]);
                errors <= errors + 1;
            end else begin
                $display("output[%2d]: got %3d  OK", out_idx, o_data);
            end
            out_idx <= out_idx + 1;
            if (out_idx == NUM_OUTPUTS) begin
                if (errors == 0)
                    $display("\nPASS: all %0d outputs matched.", NUM_OUTPUTS);
                else
                    $display("\nFAIL: %0d mismatch(es).", errors);
                $finish;
            end
        end
    end

    // ----------------------------------------------------------------
    // Watchdog
    // ----------------------------------------------------------------
    initial begin
        #200000;
        $finish;
    end

endmodule
