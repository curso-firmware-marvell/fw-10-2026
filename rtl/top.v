`include "rtl/config.vh"

module top #() (
        input wire i_clk     ,
        input wire i_resetn
);

    wire mem_valid;
    wire mem_instr;
    reg mem_ready;
    wire [31:0] mem_addr;
    wire [31:0] mem_wdata;
    wire [3:0]  mem_wstrb;
    reg  [31:0] mem_rdata;
    wire        mcu_reset_rf ;

    picorv32 #(
        .ENABLE_COUNTERS      (   ), //default: 1
        .ENABLE_COUNTERS64    (   ), //default: 1
        .ENABLE_REGS_16_31    (   ), //default: 1
        .ENABLE_REGS_DUALPORT (   ), //default: 1
        .LATCHED_MEM_RDATA    (   ), //default: 0
        .TWO_STAGE_SHIFT      (   ), //default: 1
        .BARREL_SHIFTER       (   ), //default: 0
        .TWO_CYCLE_COMPARE    (   ), //default: 0
        .TWO_CYCLE_ALU        (   ), //default: 0
        .COMPRESSED_ISA       (   ), //default: 0
        .CATCH_MISALIGN       (   ), //default: 1
        .CATCH_ILLINSN        (   ), //default: 1
        .ENABLE_PCPI          (   ), //default: 0
        .ENABLE_MUL           (   ), //default: 0
        .ENABLE_FAST_MUL      (   ), //default: 0
        .ENABLE_DIV           (   ), //default: 0
        .ENABLE_IRQ           (   ), //default: 0
        .ENABLE_IRQ_QREGS     (   ), //default: 1
        .ENABLE_IRQ_TIMER     (   ), //default: 1
        .ENABLE_TRACE         (   ), //default: 0
        .REGS_INIT_ZERO       (   ), //default: 0
        .MASKED_IRQ           ( ),   // default [31:0] 32'h 0000_0000 
        .LATCHED_IRQ          ( ),   // default [31:0] 32'h ffff_ffff 
        .PROGADDR_RESET       ( ),   // default [31:0] 32'h 0000_0000 
        .PROGADDR_IRQ         ( ),   // default [31:0] 32'h 0000_0010 
        .STACKADDR            ( )    // default [31:0] 32'h ffff_ffff 
    ) u_picorv32 (
        .clk         (i_clk        ),
        .resetn      (i_resetn & ~mcu_reset_rf    ),
        .trap        (trap       ),
        .mem_valid   (mem_valid  ),
        .mem_instr   (mem_instr  ),
        .mem_ready   (mem_ready  ),
        .mem_addr    (mem_addr   ),
        .mem_wdata   (mem_wdata  ),
        .mem_wstrb   (mem_wstrb  ),
        .mem_rdata   (mem_rdata  )
    );

    wire        control_rf_trigger_reg   ;
    wire [31:0] control_rf_reg_offset    ;
    wire [31:0] control_rf_i_mem_addr    ;
    wire [31:0] control_rf_i_mem_wdata   ;
    wire [31:0] control_rf_reg_mem_rdata ;

    wire        dma_valid ;
    wire [31:0] dma_addr  ;
    wire [31:0] dma_wdata ;
    wire [3:0]  dma_wstrb ;
    wire [31:0] dma_rdata ;
    wire        dma_ready ;

    wire        to_mem_mux_valid ;
    wire [31:0] to_mem_mux_addr  ;
    wire [31:0] to_mem_mux_wdata ;
    wire [3:0]  to_mem_mux_wstrb ;
    wire [31:0] to_mem_mux_rdata ;
    wire        to_mem_mux_ready ;
    wire [1:0]                                                  adc_status;
    wire [1:0]                                                  adc_start;
    wire [NB_MEM_ADDRESS-1:0]                                   adc_clk_divider;            
    wire [REG_ARRAY_READ_WRITE_TEST_SIZE*NB_MEM_ADDRESS-1:0]    adc_data_mem;

    mem_arbitrator #(
    ) u_mem_arbitrator(
        .i_master_0_mem_valid (   mem_valid ) ,
        .i_master_0_mem_addr  (   mem_addr  ) ,
        .i_master_0_mem_wdata (   mem_wdata ) ,
        .i_master_0_mem_wstrb (   mem_wstrb ) ,
        .o_master_0_mem_rdata (   mem_rdata ) ,
        .o_master_0_mem_ready (   mem_ready ) ,

        .i_master_1_mem_valid (   dma_valid ) ,
        .i_master_1_mem_addr  (   dma_addr  ) ,
        .i_master_1_mem_wdata (   dma_wdata ) ,
        .i_master_1_mem_wstrb (   dma_wstrb ) ,
        .o_master_1_mem_rdata (   dma_rdata ) ,
        .o_master_1_mem_ready (   dma_ready ) ,

        .o_salve_mem_valid (   to_mem_mux_valid ) ,
        .o_salve_mem_addr  (   to_mem_mux_addr  ) ,
        .o_salve_mem_wdata (   to_mem_mux_wdata ) ,
        .o_salve_mem_wstrb (   to_mem_mux_wstrb ) ,
        .i_salve_mem_rdata (   to_mem_mux_rdata ) ,
        .i_salve_mem_ready (   to_mem_mux_ready ) ,

        .i_clk   (i_clk)  ,
        .i_reset (~i_resetn   )
    );

    wire        dma_done;
    wire [31:0] dma_size;
    wire [31:0] dma_source;
    wire [31:0] dma_dest;
    wire        dma_start;
    dma #(
    ) u_dma(
        .o_bus_valid   (dma_valid )  ,
        .o_bus_addr    (dma_addr  )  ,
        .o_bus_wdata   (dma_wdata )  ,
        .o_bus_wstrb   (dma_wstrb )  ,
        .i_bus_rdata   (dma_rdata )  ,
        .i_bus_ready   (dma_ready )  ,

        .i_dma_source  (dma_source ) ,
        .i_dma_dest    (dma_dest   ) ,
        .i_dma_size    (dma_size   ) ,
        .i_dma_start   (dma_start  ) ,
        .o_dma_done    (dma_done   ) ,

        .i_clk         (i_clk        ) ,
        .i_reset       (~i_resetn    )
    );

    mem_mux #(
    ) u_mem_mux (
        .i_clk       (   i_clk              ) ,
        .i_mem_valid (   to_mem_mux_valid ) ,
        .i_mem_addr  (   to_mem_mux_addr  ) ,
        .i_mem_wdata (   to_mem_mux_wdata ) ,
        .i_mem_wstrb (   to_mem_mux_wstrb ) ,
        .o_mem_rdata (   to_mem_mux_rdata ) ,
        .o_mem_ready (   to_mem_mux_ready ) ,

        .o_control_rf_trigger_reg   (control_rf_trigger_reg    ) ,
        .o_control_rf_reg_offset    (control_rf_reg_offset     ) ,
        .o_control_rf_i_mem_addr    (control_rf_i_mem_addr     ) ,
        .o_control_rf_i_mem_wdata   (control_rf_i_mem_wdata    ) ,
        .i_control_rf_o_mem_rdata   (control_rf_reg_mem_rdata  ) 
    ); 

    register_file #(
        .NB_ADDR (NB_MEM_ADDRESS) ,
        .NB_DATA (NB_MEM_DATA)  
    ) u_register_file (
        .i_reg_dma_done           (dma_done)             ,
        .i_reg_adc_status         (adc_status)           ,       
        .i_reg_array_adc_buff     (adc_data_mem)         ,

        .o_reg_fw_status          ()                     ,
        .o_reg_fw_report          ()                     ,
        .o_reg_uart_data          ()                     ,
        .o_reg_uart_command       ()                     ,
        .o_reg_aux_0              ()                     ,
        .o_reg_mcu_reset          (mcu_reset_rf)         ,
        .o_reg_dma_source_addr    (dma_source)           ,
        .o_reg_dma_dest_addr      (dma_dest)             ,
        .o_reg_dma_size           (dma_size)             ,
        .o_reg_dma_start          (dma_start)            ,
        .o_reg_adc_start          (adc_start)            ,                 
        .o_reg_adc_clk_divider    (adc_clk_divider)      ,   
        .o_reg_led                ()                     ,
        
        .i_clk                    (i_clk         )         ,

        .i_trigger_reg            (control_rf_trigger_reg   )      ,
        .i_reg_offset             (control_rf_reg_offset    )      ,
        .i_reg_address            (control_rf_i_mem_addr    )      ,
        .i_mem_wdata              (control_rf_i_mem_wdata   )      ,
        .o_mem_rdata              (control_rf_reg_mem_rdata )      
    );

    adc #(
        .DATA_WIDTH  (NB_MEM_DATA),
        .N_SAMPLES   (REG_ARRAY_READ_WRITE_TEST_SIZE)
    ) u_adc (
        .i_clk          (i_clk),
        .i_rst_n        (i_resetn),
        .i_start        (adc_start),
        .i_clk_divider  (adc_clk_divider),
        .o_status       (adc_status),
        .o_sample_mem   (adc_data_mem)
    );

endmodule
