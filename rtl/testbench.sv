`include "rtl/config.vh"

module testbench;


    reg clk = 1;
    reg resetn = 0;
    wire trap;
    reg [63:0]clock_counts ;
    always #5 clk = ~clk;

    top  u_top (
        .i_clk    (clk    ),
        .i_resetn (resetn )
    );

    initial begin
        u_top.fake_uart0_data_input       = 32'b0;
        u_top.fake_uart0_data_input_valid = 1'b0;
        u_top.fake_uart1_data_input       = 32'b0;
        u_top.fake_uart1_data_input_valid = 1'b0;
    end

    initial begin
        $dumpfile("wave.vcd");
        // $dumpfile("wave.fst");
        $dumpvars;

        repeat (10) @(posedge clk);
        init_memory();
        repeat (10) @(posedge clk);
		uart_sniffer();
		uart_trnasmiter();
		led_monitor();

        repeat (10) @(posedge clk);
        resetn <= 1;
        
        count_mem_access_read   = 64'b0   ;
        count_mem_access_write  = 64'b0   ;
        count_ins_access_read   = 64'b0   ;
        count_ins_access_write  = 64'b0   ;
        clock_counts            = 64'b0   ;
        while(u_top.u_register_file.REGISTER_FW_STATUS != 32'hFF && trap == 0 )begin
            @(posedge clk);
            clock_counts = clock_counts + 64'b1;
        end
        repeat (100) @(posedge clk);
        if (trap)begin
            repeat(30) @(posedge clk);
            $display("------ TRAP! ------- clock_counts = %d",clock_counts);
        end else begin
            $display("ending simulation, u_top.u_register_file.REGISTER_FW_STATUS = %x",u_top.u_register_file.REGISTER_FW_STATUS);
            $display("total clocks: %d", clock_counts);
        end
        $display("--- mem report ---");
        $display("-- total mem access = %d",count_mem_access_write+count_mem_access_read);
        $display(" READ = %d, WRITE = %d",count_mem_access_read,count_mem_access_write);
        $display("-- total instruction access = %d",count_ins_access_write+count_ins_access_read);
        $display(" READ = %d, WRITE = %d",count_ins_access_read,count_ins_access_write);
        
        $finish;
    end


`ifdef DEBUG_PRINTS
    always @(posedge clk) begin
        if (u_top.mem_valid && u_top.mem_ready) begin
            if (u_top.mem_instr) begin
                if (u_top.mem_wstrb)begin
                    $display("INSTRUCTION write  0x%08x: 0x%08x (wstrb=%b)", u_top.mem_addr, u_top.mem_wdata, u_top.mem_wstrb);
                end else begin
                    $display("INSTRUCTION read   0x%08x: 0x%08x", u_top.mem_addr, u_top.mem_rdata);
                end
            end else begin
                if (u_top.mem_wstrb)begin
                    $display("MEM write  0x%08x: 0x%08x (wstrb=%b)", u_top.mem_addr, u_top.mem_wdata, u_top.mem_wstrb);
                end else begin
                    $display("MEM read   0x%08x: 0x%08x", u_top.mem_addr, u_top.mem_rdata);
                end
            end 
        end
    end
`endif

    reg [63:0] count_mem_access_read;
    reg [63:0] count_mem_access_write;
    reg [63:0] count_ins_access_read;
    reg [63:0] count_ins_access_write;
    always @(posedge clk) begin
        if (u_top.mem_valid && u_top.mem_ready) begin
            if (u_top.mem_instr) begin
                if (u_top.mem_wstrb)begin
                    count_ins_access_write <= count_ins_access_write + 64'b1;
                end else begin
                    count_ins_access_read <= count_ins_access_read + 64'b1;
                end
            end else begin
                if (u_top.mem_wstrb)begin
                    count_mem_access_write <= count_mem_access_write + 64'b1;
                end else begin
                    count_mem_access_read <= count_mem_access_read + 64'b1;
                end
            end 
        end
    end

    ///////////////// load mem /////////////////
    function init_memory();
    	integer fd;
    	string line;
    	logic [31:0] data;
    	int addr;
        fd = $fopen("fw/fw.hex", "r");
        if (fd == 0) begin $fatal("Cannot open fw.hex"); end
        addr = 0;
        while (!$feof(fd)) begin
            void'($fgets(line, fd));
            if (line.len() == 0) continue;
            $sscanf(line, "%x", data);
            $display("loading: %x -> %x", addr, data);
            u_top.u_mem_mux.memory_i[addr] <= data;
            addr += 1; // byte address
        end
    endfunction
    ////////////////////////////////////////////

	/////////////// print UART buffer //////////
	task uart_sniffer();
		reg [8*256-1:0] uart_buffer ;
		integer size_msg, cnt;
		fork
			while(1)begin
                uart_buffer = 256*8'b0;
                wait (u_top.u_uart0.i_start_transmit == 1'b1);
                size_msg = u_top.u_uart0.i_size_to_transmit;
                cnt = 0;
                while(cnt < size_msg)begin
                    @(posedge clk);
                    if(u_top.u_uart0.o_valid_output)begin
                        uart_buffer[8*((255-cnt)+1)-1 -: 8] = u_top.u_uart0.o_output_data[7:0];
                        cnt = cnt + 1;
                    end
                end
				$write("\033[36mUART: ");
                for (int i = 0; i < size_msg; i++) begin  $write("%c", uart_buffer[8*((255-i)+1)-1 -: 8]); end
                $write("\033[0m\n");
			end
		join_none
	endtask
    ////////////////////////////////////////////

    byte data_array[] = '{8'd72,8'd79,8'd76,8'd65,8'd33};
	/////////////// uart input transmiter //////////
	task uart_trnasmiter();
		fork
			while(1)begin
                wait (u_top.u_register_file.REGISTER_AUX_0 == 32'h5ED5ED);
                @(posedge clk);
                for(int i=0; i< data_array.size() ; i++)begin
                    force u_top.fake_uart0_data_input_valid = 1'b1;
                    force u_top.fake_uart0_data_input = data_array[i];
                    @(posedge clk);
                end
                force u_top.fake_uart0_data_input_valid = 1'b0;

			end
		join_none
	endtask
    ////////////////////////////////////////////

	/////////////// monitor LED register ////////
	task led_monitor();
		fork
			while(1)begin
				@(u_top.u_register_file.REGISTER_LED);
				$display("\033[32mLED: 0b%08b\033[0m", u_top.u_register_file.REGISTER_LED[7:0]);
			end
		join_none
	endtask
    ////////////////////////////////////////////

    // task uart0_send(input [31:0] data);
    //     begin
    //         u_top.fake_uart0_data_input       = data;
    //         u_top.fake_uart0_data_input_valid = 1'b1;
    //         @(posedge clk);
    //         u_top.fake_uart0_data_input_valid = 1'b0;
    //     end
    // endtask


    // task uart1_send(input [31:0] data);
    //     begin
    //         u_top.fake_uart1_data_input       = data;
    //         u_top.fake_uart1_data_input_valid = 1'b1;
    //         @(posedge clk);
    //         u_top.fake_uart1_data_input_valid = 1'b0;
    //     end
    // endtask

endmodule