`include "rtl/config.vh"

module mem_mux #() (
    input  wire                      i_clk               ,
    input  wire                      i_mem_valid         ,
    input  wire [NB_MEM_ADDRESS-1:0] i_mem_addr          ,
    input  wire [NB_MEM_DATA   -1:0] i_mem_wdata         ,
    input  wire [3:0]                i_mem_wstrb         ,
    output wire [NB_MEM_DATA   -1:0] o_mem_rdata         ,
    output wire                      o_mem_ready         ,
    
    // register file connection
    output wire                      o_control_rf_trigger_reg         ,
    output wire [NB_MEM_ADDRESS-1:0] o_control_rf_reg_offset          ,
    output wire [NB_MEM_ADDRESS-1:0] o_control_rf_i_mem_addr          ,
    output wire [NB_MEM_DATA   -1:0] o_control_rf_i_mem_wdata         ,
    input  wire [NB_MEM_DATA   -1:0] i_control_rf_o_mem_rdata       
);
    reg [NB_MEM_DATA-1 : 0] memory_i  [0 : ((ADDR_SIZE_INST ) >> 2) - 1] ;
    reg [NB_MEM_DATA-1 : 0] memory_d  [0 : ((ADDR_SIZE_DATA ) >> 2) - 1] ;

    reg                 mem_ready                           ;
    reg  [NB_MEM_DATA-1:0]  mem_rdata                           ;
    wire  [NB_MEM_DATA-1:0]  reg_mem_rdata                       ;
    wire [NB_MEM_ADDRESS-1:0]  reg_offset                          ;

    wire   is_instruction_mem                                                                           ;
    wire   is_data_mem                                                                           ;
    wire   is_register_mem                                                                                   ;
    assign is_instruction_mem       = ((i_mem_addr >= ADDR_START_INST     ) && (i_mem_addr <= ADDR_START_INST     +  ADDR_SIZE_INST      - 1     ))   ;
    assign is_data_mem              = ((i_mem_addr >= ADDR_START_DATA     ) && (i_mem_addr <= ADDR_START_DATA     +  ADDR_SIZE_DATA      - 1     ))   ;
    assign is_register_mem          = ((i_mem_addr >= ADDR_START_REG_FILE ) && (i_mem_addr <= ADDR_START_REG_FILE +  ADDR_SIZE_REG_FILE  - 1     ))   ;


    always @(*) begin
        mem_ready = i_mem_valid && (is_instruction_mem || is_data_mem || is_register_mem);
        mem_rdata = 0;
        if (is_instruction_mem)
            mem_rdata = memory_i[(i_mem_addr-ADDR_START_INST) >> 2];
        else if (is_data_mem)
            mem_rdata = memory_d[(i_mem_addr-ADDR_START_DATA) >> 2];
        else if (is_register_mem)
            mem_rdata = i_control_rf_o_mem_rdata;
            `ifdef DEBUG_PRINTS
                $display(" reg access: reg_offset= %x", reg_offset);
                if(i_mem_wstrb)
                    $display("    reg WRITE = %x", i_mem_wdata);
                else
                    $display("    reg READ  = %x", i_control_rf_o_mem_rdata);
            `endif
    end

    always @(posedge i_clk) begin
        if (i_mem_valid && is_instruction_mem) begin
            if (i_mem_wstrb[0]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][ 7: 0] <= i_mem_wdata[ 7: 0];
            if (i_mem_wstrb[1]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][15: 8] <= i_mem_wdata[15: 8];
            if (i_mem_wstrb[2]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][23:16] <= i_mem_wdata[23:16];
            if (i_mem_wstrb[3]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][31:24] <= i_mem_wdata[31:24];
        end else if (i_mem_valid && is_data_mem) begin
            if (i_mem_wstrb[0]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][ 7: 0] <= i_mem_wdata[ 7: 0];
            if (i_mem_wstrb[1]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][15: 8] <= i_mem_wdata[15: 8];
            if (i_mem_wstrb[2]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][23:16] <= i_mem_wdata[23:16];
            if (i_mem_wstrb[3]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][31:24] <= i_mem_wdata[31:24];
        end
    end





    // always @(posedge i_clk) begin
    //     mem_ready <= 0;
    //     if (i_mem_valid && !mem_ready) begin
    //         if (is_instruction_mem) begin
    //             mem_ready <= 1;
    //             mem_rdata <= memory_i[(i_mem_addr-ADDR_START_INST) >> 2];
    //             if (i_mem_wstrb[0]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][ 7: 0] <= i_mem_wdata[ 7: 0];
    //             if (i_mem_wstrb[1]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][15: 8] <= i_mem_wdata[15: 8];
    //             if (i_mem_wstrb[2]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][23:16] <= i_mem_wdata[23:16];
    //             if (i_mem_wstrb[3]) memory_i[(i_mem_addr-ADDR_START_INST) >> 2][31:24] <= i_mem_wdata[31:24];
    //         end else if (is_data_mem) begin
    //             mem_ready <= 1;
    //             mem_rdata <= memory_d[(i_mem_addr-ADDR_START_DATA) >> 2];
    //             if (i_mem_wstrb[0]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][ 7: 0] <= i_mem_wdata[ 7: 0];
    //             if (i_mem_wstrb[1]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][15: 8] <= i_mem_wdata[15: 8];
    //             if (i_mem_wstrb[2]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][23:16] <= i_mem_wdata[23:16];
    //             if (i_mem_wstrb[3]) memory_d[(i_mem_addr-ADDR_START_DATA) >> 2][31:24] <= i_mem_wdata[31:24];
    //         end else if (is_register_mem  ) begin
    //             mem_ready <= 1;
    //             mem_rdata <= i_control_rf_o_mem_rdata;
    //             `ifdef DEBUG_PRINTS
    //                 $display(" reg access: reg_offset= %x", reg_offset);
    //                 if(i_mem_wstrb)
    //                     $display("    reg WRITE = %x", i_mem_wdata);
    //                 else
    //                     $display("    reg READ  = %x", i_control_rf_o_mem_rdata);
    //             `endif
    //         end
    //     end
    // end

    assign o_mem_rdata = mem_rdata;
    assign o_mem_ready = mem_ready;
    assign reg_offset  = i_mem_addr - ADDR_START_REG_FILE;

    assign o_control_rf_trigger_reg  = | i_mem_wstrb & is_register_mem &  i_mem_valid ;
    assign o_control_rf_reg_offset   = reg_offset    ;
    assign o_control_rf_i_mem_addr   = i_mem_addr    ;
    assign o_control_rf_i_mem_wdata  = i_mem_wdata   ;

//    register_file #(
//        .NB_ADDR (NB_MEM_ADDRESS) ,
//        .NB_DATA (NB_MEM_DATA)  
//    ) u_register_file (
//        .i_clk                    (i_clk         )      ,
//        .i_trigger_reg            (trigger_reg   )      ,
//        .i_reg_offset             (reg_offset    )      ,
//        .i_reg_address            (i_mem_addr    )      ,
//        .i_mem_wdata              (i_mem_wdata   )      ,
//        .o_mem_rdata              (reg_mem_rdata )      
//    );

endmodule