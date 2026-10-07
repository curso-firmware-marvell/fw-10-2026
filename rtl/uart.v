
module uart #(
    parameter        NB_DATA       = 32,
    parameter        SIZE_BUFF_IN  = 100,
    parameter        SIZE_BUFF_OUT = 100
)( 
    // input path
    input wire                                i_flush_data       ,
    output wire                               o_data_available   ,
    output wire [NB_DATA*SIZE_BUFF_IN-1 : 0]  o_buff_in          ,
    output wire [31:0]                        o_index_in         ,
    
    // output path 
    input wire                                i_start_transmit   ,
    input wire  [31:0]                        i_size_to_transmit ,
    input wire  [NB_DATA*SIZE_BUFF_OUT-1 : 0] i_buff_out         ,
    output wire                               o_transmision_done ,
    
    // fake world data
    input wire                                i_valid_input      ,
    input wire  [NB_DATA-1:0]                 i_input_data       ,
    output wire                               o_valid_output     ,
    output wire [NB_DATA-1:0]                 o_output_data      ,

    input  wire                               i_clk              ,
    input  wire                               i_rst_n            
);

wire reset = ~i_rst_n;

/////////////////
// output path //
/////////////////
reg                transmiting, done;
wire [NB_DATA-1:0] output_buffer [0:SIZE_BUFF_OUT-1] ;
reg  [31:0]        current_output_index              ;


// load incoming data
always @(posedge i_clk) begin
    if(reset)begin
        transmiting             <= 1'b0   ;
        done                    <= 1'b1   ;   
        current_output_index    <= 31'b0  ;
    end else begin
        if(i_start_transmit && !transmiting)begin
            transmiting          <= 1'b1  ;
            current_output_index <= 32'b0 ;
            done                 <= 1'b0  ;  
        end else if (transmiting) begin
            if(current_output_index >= i_size_to_transmit - 1)begin
                current_output_index <= 32'b0 ;
                transmiting          <= 1'b0  ;
                done                 <= 1'b1  ;
            end else begin
                current_output_index <= current_output_index + 32'b1        ;
                transmiting          <= 1'b1  ;
                done                 <= 1'b0  ;
            end
        end
    end
end
// assigns 
genvar gv_ou;
generate
    for (gv_ou = 0 ; gv_ou < SIZE_BUFF_OUT ; gv_ou = gv_ou + 1 ) begin : gen_out
        assign output_buffer[gv_ou] = i_buff_out[NB_DATA*gv_ou +: NB_DATA];
    end
endgenerate
assign o_valid_output = transmiting;
assign o_output_data = output_buffer[current_output_index] ;
assign o_transmision_done = done;

////////////////
// input path //
////////////////
reg [31:0]        current_input_index             ;
reg [NB_DATA-1:0] input_buffer [0:SIZE_BUFF_IN-1] ;

// load incoming data
always @(posedge i_clk) begin
    if(reset)begin
        current_input_index <= 32'b0 ;
    end else begin
        if(i_flush_data)begin
            current_input_index <= 32'b0 ;
        end else if(i_valid_input)begin
            input_buffer[current_input_index] <= i_input_data                ;
            if( current_input_index < SIZE_BUFF_IN ) begin
                current_input_index               <= current_input_index + 32'b1 ;
            end
        end
    end
end
// assigns
genvar gv_in; 
generate
    for (gv_in = 0 ; gv_in < SIZE_BUFF_IN ; gv_in = gv_in + 1 ) begin : gen_in
        assign o_buff_in[NB_DATA*gv_in +: NB_DATA] = input_buffer[gv_in];
    end
endgenerate
assign o_data_available = (current_input_index != 0);
assign o_index_in       = current_input_index;


endmodule