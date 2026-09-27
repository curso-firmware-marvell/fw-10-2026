
module mem_arbitrator #(
    parameter NB_ADDR          =             32 ,
    parameter NB_DATA          =             32 
)(
        input  wire               i_master_0_mem_valid ,
        input  wire [NB_ADDR-1:0] i_master_0_mem_addr  ,
        input  wire [NB_DATA-1:0] i_master_0_mem_wdata ,
        input  wire [3:0]         i_master_0_mem_wstrb ,
        output wire [NB_DATA-1:0] o_master_0_mem_rdata ,
        output wire               o_master_0_mem_ready ,

        input  wire               i_master_1_mem_valid ,
        input  wire [NB_ADDR-1:0] i_master_1_mem_addr  ,
        input  wire [NB_DATA-1:0] i_master_1_mem_wdata ,
        input  wire [3:0]         i_master_1_mem_wstrb ,
        output wire [NB_DATA-1:0] o_master_1_mem_rdata ,
        output wire               o_master_1_mem_ready ,

        output wire               o_salve_mem_valid ,
        output wire [NB_ADDR-1:0] o_salve_mem_addr  ,
        output wire [NB_DATA-1:0] o_salve_mem_wdata ,
        output wire [3:0]         o_salve_mem_wstrb ,
        input  wire [NB_DATA-1:0] i_salve_mem_rdata ,
        input  wire               i_salve_mem_ready ,

        input wire i_clk                            , 
        input wire i_reset 
    
);

wire master_0_valid = i_master_0_mem_valid;
wire master_1_valid = i_master_1_mem_valid & !i_master_0_mem_valid;



typedef enum reg [1:0] {
    IDLE          = 2'b00,
    MASTER_0_REQ  = 2'b01,
    MASTER_1_REQ  = 2'b10
} ms_sates_t;

ms_sates_t current_state ;
ms_sates_t next_state    ;

reg slave_ready_done ;
always @(posedge i_clk)begin
    if (i_reset)begin
        current_state <= MASTER_0_REQ ;
        slave_ready_done <= 0;
    end else begin
        current_state <= next_state;
        if(i_salve_mem_ready)begin
            if(!slave_ready_done)
                slave_ready_done <= 1'b1;
        end else if (i_master_0_mem_valid | i_master_1_mem_valid)begin
            slave_ready_done <= 1'b0;
        end
    end
end

always @(*)begin
    case (current_state)
        MASTER_0_REQ: begin
            if (master_1_valid & slave_ready_done)
                next_state = MASTER_1_REQ ;
            else 
                next_state = MASTER_0_REQ;
        end
        MASTER_1_REQ: begin
            if (master_0_valid & slave_ready_done)
                next_state = MASTER_0_REQ ;
            else 
                next_state = MASTER_1_REQ;
        end
        default     : begin 
            next_state = MASTER_1_REQ ;
        end
    endcase
end

reg               master_0_mem_ready ;
reg               master_1_mem_ready ;
reg               salve_mem_valid    ;
reg [NB_ADDR-1:0] salve_mem_addr     ;
reg [NB_DATA-1:0] salve_mem_wdata    ;
reg [3:0]         salve_mem_wstrb    ;

always @(*)begin
    case (current_state)
        MASTER_0_REQ: begin
            master_0_mem_ready = i_salve_mem_ready & i_master_0_mem_valid;
            master_1_mem_ready = 1'b0                 ;
            salve_mem_valid    = i_master_0_mem_valid ;
            salve_mem_addr     = i_master_0_mem_addr  ;
            salve_mem_wdata    = i_master_0_mem_wdata ;
            salve_mem_wstrb    = i_master_0_mem_wstrb ;
        end
        MASTER_1_REQ: begin
            master_0_mem_ready = 1'b0                 ;
            master_1_mem_ready = i_salve_mem_ready  & i_master_1_mem_valid  ;
            salve_mem_valid    = i_master_1_mem_valid ;
            salve_mem_addr     = i_master_1_mem_addr  ;
            salve_mem_wdata    = i_master_1_mem_wdata ;
            salve_mem_wstrb    = i_master_1_mem_wstrb ;
        end
    endcase
end

assign o_master_0_mem_ready = master_0_mem_ready ;
assign o_master_1_mem_ready = master_1_mem_ready ;
assign o_salve_mem_valid    = salve_mem_valid    ;
assign o_salve_mem_addr     = salve_mem_addr     ;
assign o_salve_mem_wdata    = salve_mem_wdata    ;
assign o_salve_mem_wstrb    = salve_mem_wstrb    ;

    //output wire [NB_DATA-1:0] o_master_1_mem_rdata ,
    //output wire [NB_DATA-1:0] o_master_0_mem_rdata ,

assign o_master_0_mem_rdata = i_salve_mem_rdata;
assign o_master_1_mem_rdata = i_salve_mem_rdata;



endmodule