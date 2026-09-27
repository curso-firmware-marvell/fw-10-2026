
module dma #(
    parameter NB_ADDR          =             32 ,
    parameter NB_DATA          =             32 
)(
        output wire               o_bus_valid ,
        output wire [NB_ADDR-1:0] o_bus_addr  ,
        output wire [NB_DATA-1:0] o_bus_wdata ,
        output wire [3:0]         o_bus_wstrb ,
        input  wire [NB_DATA-1:0] i_bus_rdata ,
        input  wire               i_bus_ready ,

        input  wire [NB_ADDR-1:0] i_dma_source ,
        input  wire [NB_ADDR-1:0] i_dma_dest   ,
        input  wire [NB_ADDR-1:0] i_dma_size   ,
        input  wire               i_dma_start  ,
        output wire               o_dma_done   ,

        input wire i_clk     ,
        input wire i_reset
    
);

reg               dma_done      ;
reg [NB_ADDR-1:0] address_count ;
reg               dma_start_reg ;

typedef enum reg [1:0] {
    IDLE      = 2'b00,
    READ_REQ  = 2'b01,
    WRITE_REQ = 2'b10
} ms_sates_t;

ms_sates_t current_state ;
ms_sates_t next_state ;

reg [NB_ADDR-1:0] dma_source ;
reg [NB_ADDR-1:0] dma_dest   ;
reg [NB_ADDR-1:0] dma_size   ;

reg [NB_ADDR-1:0] bus_addr  ;
reg               bus_valid ;
reg [NB_DATA-1:0] bus_wdata ;
reg [3:0]         bus_wstrb ;

reg [NB_DATA-1:0] current_data_read ;
always @(posedge i_clk)begin
    if (i_reset)begin
        dma_source <= 32'b0 ;
        dma_dest   <= 32'b0 ;
        dma_size   <= 32'b0 ;
        address_count <= 32'b0 ;
        dma_start_reg <= 1'b0;
    end else begin
        dma_start_reg <= i_dma_start;
        if(!dma_start_reg && i_dma_start)begin // posedge dma start
            dma_source <= i_dma_source  ;
            dma_dest   <= i_dma_dest    ;
            dma_size   <= i_dma_size    ;
        end
    end
end

always @(posedge i_clk)begin
    if (i_reset)begin
        current_state <= IDLE ;
        bus_wstrb <= 4'b0;
        address_count <= 32'b0 ;
    end else begin
        if(!dma_start_reg && i_dma_start)begin // posedge dma start
            address_count <= 32'b0;
            dma_done <= 1'b0;
        end else if (!dma_done) begin
            current_state <= next_state;
            case (current_state)
                IDLE: begin
                    if(dma_start_reg & !dma_done)begin // go to read req
                        bus_addr   <= dma_source + (address_count<<2);
                        bus_valid  <= 1'b1;
                        bus_wstrb <= 4'b0000;
                    end else begin
                        bus_valid  <= 1'b0;
                    end
                end
                READ_REQ: begin
                    if(i_bus_ready)begin // go to write req
                        bus_addr   <= dma_dest + (address_count<<2);
                        current_data_read <= i_bus_rdata;
                        bus_valid  <= 1'b0;
                        bus_wstrb <= 4'b1111;
                    end else begin
                        bus_valid  <= 1'b1;
                    end
                end
                WRITE_REQ: begin
                    if(i_bus_ready)begin // go to idle status
                        bus_valid  <= 1'b0;
                        bus_wstrb <= 4'b0000;
                        if(address_count <= dma_size)begin
                            address_count <= address_count + 32'b1;
                        end else begin
                            dma_done <= 1'b1;
                            address_count <= 32'b0;
                        end
                    end else begin
                        bus_wdata <= current_data_read;
                        bus_valid  <= 1'b1;
                    end
                end
                default: begin 
                end
            endcase
        end
    end
end

always @(*)begin
        case (current_state)
            IDLE: begin
                if(dma_start_reg)begin
                    next_state = READ_REQ;
                end else begin
                    next_state = IDLE;
                end
            end
            READ_REQ: begin
                if(i_bus_ready)begin
                    next_state = WRITE_REQ ;
                end else begin
                    next_state = READ_REQ ;
                end
            end
            WRITE_REQ: begin
                if(i_bus_ready)begin
                    next_state = IDLE ;
                end else begin
                    next_state = WRITE_REQ ;
                end
            end
            default: begin 
                next_state = IDLE ;
            end
        endcase
end

assign o_bus_wstrb = bus_wstrb ;
assign o_bus_addr  = bus_addr  ;
assign o_bus_valid = bus_valid ;
assign o_bus_wdata = bus_wdata ;

assign o_dma_done = dma_done;

endmodule