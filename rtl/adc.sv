// No setear el mismo registro en 2 lugares distintos ante la misma condicion
// Crear los estados con ENUM
// div_cnt resetear inicialmente al valor en el cual toma la muestra para que esta sea instantanea -> Tambien resetear con señal start
// Agregar un bit para permitir que siga funcionando una vez terminada la primera muestra o que corte en ese instante
    // circular_mode

module adc #(
    parameter        DATA_WIDTH  = 32,   // Resolucion del ADC: 8, 16 o 32 bits
    parameter        N_SAMPLES   = 32,   // Cantidad de muestras a tomar
    parameter        SIGNAL_TYPE = 0,    // 0 = Senoidal | 1 = Triangular | 2 = Diente de sierra
    parameter real   VREF        = 3.3,  // Tension maxima
    parameter real   FRECUENCIA  = 1e6
)( 
    input  wire                    i_clk,
    input  wire                    i_rst_n,
    input  wire [1:0]              i_start, // i_start[1] configuracion del tipo de captura : 1 es circular , 0 es unica  , i_start[0] bit de inicio de captura
    input  wire [DATA_WIDTH-1:0]   i_clk_divider,
    output wire [1:0]              o_status,       // o_status[0]=half_done o_status[1]=acq_done
    output wire [DATA_WIDTH*N_SAMPLES-1:0]   o_sample_mem // vector con las muestras del lote actual
);

    reg [DATA_WIDTH-1:0] sample_mem [0:N_SAMPLES-1];

    localparam real    PI           = 3.14159265358979323846;
    localparam real    MAX_CODE_R   = (2.0 ** DATA_WIDTH) - 1.0;
    localparam integer HALF_SAMPLES = N_SAMPLES / 2;


    reg half_done;
    reg acq_done;
    assign o_status = {acq_done, half_done};
    reg start_d, posedge_start;


    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            start_d         <= 1'b0;
            posedge_start   <= 1'b0;
        end else begin
            start_d         <= i_start[0] ;
            posedge_start   <= (i_start[0]==1'b1) && (start_d==1'b0); 
        end
    end

    // assign posedge_start = i_start[0] && start_d


    //STATE MACHINE
    typedef enum reg [1:0] {ST_IDLE,ST_CAPT} states_enum;
    states_enum state, next_state ;

    //Logica de estado
    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            state <= ST_IDLE;
        end else begin
            state <= next_state; 
        end
    end

    //Logica de entrada
    always @(*) begin
        case (state)
            ST_IDLE: begin
                if (posedge_start == 1'b1 || i_start[1]==1'b1) begin
                    next_state = ST_CAPT;
                end else begin
                    next_state = ST_IDLE;
                end
            end
            ST_CAPT: begin
                if (samp_cnt == N_SAMPLES) begin
                    if (i_start[1]==1'b1) begin
                        next_state = ST_CAPT;
                    end else begin
                        next_state = ST_IDLE;
                    end
                end else begin
                    next_state = ST_CAPT;
                end
            end
            default: begin
                next_state = ST_IDLE;
            end
        endcase
    end

    //Logica de salida
    wire enter_capt = (state == ST_IDLE) && (next_state == ST_CAPT);




    // BLOQUE DIVISOR DE RELOJ
    reg [DATA_WIDTH-1:0] div_cnt;
    reg     sample_tick;

    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            div_cnt     <= i_clk_divider-1;
            sample_tick <= 1'b0;
        end else if (posedge_start) begin
            div_cnt     <= i_clk_divider-1;
            sample_tick <= 1'b0;
        end else begin
            if (state == ST_CAPT) begin
               if (div_cnt == i_clk_divider-1) begin
                    div_cnt     <= 0;
                    sample_tick <= 1'b1;
                end else begin
                    div_cnt     <= div_cnt + 1;
                    sample_tick <= 1'b0;
                end 
            end
        end
    end


    // CONTADORES DE MUESTRA DEL LOTE (samp_cnt)
    integer samp_cnt;  // muestras tomadas del lote actual

    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            acq_done  <= 1'b0;
            half_done <= 1'b0;
        end else begin
            if(posedge_start) begin
                acq_done  <= 1'b0;
                half_done <= 1'b0;
            end else begin
                if (state==ST_CAPT) begin
                    if (samp_cnt == N_SAMPLES) begin
                        acq_done <= 1'b1;
                        half_done <= 1'b0;
                    end else begin
                        if (samp_cnt >= HALF_SAMPLES-1) begin
                            half_done <= 1'b1;
                            acq_done  <= 1'b0;
                        end
                    end
                end    
            end
        end
    end

    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            samp_cnt <= 0;
        end else begin
            if ((state==ST_CAPT) && sample_tick) begin
                if (samp_cnt == N_SAMPLES) begin
                    samp_cnt <= 0;
                end else begin
                    samp_cnt<= samp_cnt + 1;
                end
            end
        end
    end



    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            for (int i = 0 ; i< N_SAMPLES; i ++) begin
               sample_mem[i] <= '0;
            end
        end else if (state == ST_CAPT && sample_tick) begin 
            sample_mem[samp_cnt]  <= quant_sample;
        end
    end

    genvar gv_sample_mem;
    generate
        for (gv_sample_mem = 0 ; gv_sample_mem < N_SAMPLES ; gv_sample_mem = gv_sample_mem + 1 ) begin : gen_out
            assign o_sample_mem[DATA_WIDTH*gv_sample_mem +: DATA_WIDTH] = sample_mem[gv_sample_mem];
            
        end
    endgenerate












    // CAPTURA
    logic [DATA_WIDTH-1:0] quant_sample;
    real analogica_signal, rtime;

    always @(posedge i_clk or negedge i_rst_n) begin
        if (!i_rst_n) begin
            rtime <= 0;
            analogica_signal <= 0;
            quant_sample <= '0;
        end else begin
            rtime <= $realtime;
            analogica_signal    <= gen_signal(rtime);
            quant_sample        <= quantize(analogica_signal);  
        end
    end



    //Calcular la señal analogica en base al timing de la simulacion
    function automatic real gen_signal(input real rtime);
        real phase, frac, val, x;
        begin
            phase = 2.0 * PI *  FRECUENCIA * rtime* 1e-9;
            x = rtime * 1e-9 * FRECUENCIA;
            frac = x - $rtoi(x); 
            case (SIGNAL_TYPE)
                0:val = (VREF/2.0) + (VREF/2.0)*$sin(phase);
                1:val = (frac < 0.5) ? VREF * 2.0 * frac : VREF - VREF * 2 * (frac-0.5);
                2:val =  VREF * frac;
                default: val = 0;
            endcase 
            gen_signal = val;
        end
    endfunction


// Funcion cuantizador
    function automatic [DATA_WIDTH-1:0] quantize(input real v);
        real code_r;
        begin
            if (v < 0) v = 0;
            if (v > VREF) v = VREF;
            code_r = (v/VREF) * MAX_CODE_R;
            code_r = code_r + 0.5; // redondeo al entero mas cercano
            if (code_r > MAX_CODE_R) code_r = MAX_CODE_R;
            /* verilator lint_off REALCVT */
            quantize = code_r;
            /* verilator lint_on REALCVT */
        end
    endfunction


endmodule