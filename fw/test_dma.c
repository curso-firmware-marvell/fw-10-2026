
#include "includes.h"

#define ITERATIONS 32

volatile unsigned int * const u_int_array      = (volatile unsigned int *)(ADDR_START_DATA + 0x1000);
volatile unsigned int * const dest_u_int_array = (volatile unsigned int *)(ADDR_START_DATA + 0x1100);

int check_dma_d_mem(){
    print_str("-- checking dma d mem to d mem --");
    volatile int int_0 ;
    volatile int int_1 ;
    volatile int int_2 ;
    REG_FW_REPORT = 0x0 ;
    REG_DMA_SOURCE_ADDR = (unsigned int)u_int_array;
    REG_DMA_DEST_ADDR   = (unsigned int)dest_u_int_array;
    for(int i=1; i< ITERATIONS; i++){
        
        REG_DMA_SIZE = i;

        print_str("iteration");
        print_dec(i);
        // write a value
        for(int j=0; j< i; j++){
            u_int_array[j] = i+j;
        }

        // start dma copy
        REG_DMA_START = 0;
        REG_DMA_START = 1;
        
        // wait for dma done
        while(!REG_DMA_DONE){
            int_2 = int_0 + int_1;
            int_2++;
            int_1 = int_2 + int_0;
            int_1++;
            int_0 = int_2 + int_1;
            int_0++;
            int_2 = int_0 * int_1;
            int_2*=3;
            int_1 = int_2 * int_0;
            int_1*=3;
            int_0 = int_2 * int_1;
            int_0*=3;
        }

        // verify destination content
        for(int j=0; j< i; j++){
            if(dest_u_int_array[j] != i+j)
                return(1);
        }

        REG_FW_REPORT ++;
    } 
    print_str("-- data mem PASS ! --");
    return(0);
}

volatile unsigned int * const u_int_array_instruction = (volatile unsigned int *)(ADDR_START_INST);


int check_dma_i_d_mem(){
    print_str("-- checking dma i mem to d mem --");
    REG_FW_REPORT = 0x0 ;
    for(int i=1; i< ITERATIONS; i++){
        REG_DMA_SIZE = i;
        print_str("iteration");
        print_dec(i);

        // start dma copy
        REG_DMA_SOURCE_ADDR = (unsigned int)u_int_array_instruction;
        REG_DMA_DEST_ADDR   = (unsigned int)u_int_array;
        REG_DMA_START = 0;
        REG_DMA_START = 1;

        // wait for dma done
        while(!REG_DMA_DONE);
        // verify destination content
        for(int j=0; j< i; j++){
            if(u_int_array[j] != u_int_array_instruction[j])
                return(1);
        }

        REG_FW_REPORT ++;
    } 
    print_str("-- instruction to data mem check PASS ! --");
    return(0);
}

volatile unsigned int * const register_array = (volatile unsigned int *)(REG_ARRAY_READ_WRITE_TEST_BASE);

int check_dma_registers(){
    REG_FW_REPORT = 0x0 ;
    print_str("-- checking dma to registers --");
    for(int i=1; i< ITERATIONS; i++){
        REG_DMA_SIZE = i;
        print_str("iteration");
        print_dec(i);

        // write a value in register
        for(int j=0; j< i; j++){
            register_array[j] = i+j+0xDD;
        }
        // start dma copy
        REG_DMA_SOURCE_ADDR = (unsigned int)register_array;
        REG_DMA_DEST_ADDR   = (unsigned int)u_int_array;
        REG_DMA_START = 0;
        REG_DMA_START = 1;
        // wait for dma done
        while(!REG_DMA_DONE);
        // verify destination content
        for(int j=0; j< i; j++){
            if(u_int_array[j] != i+j+0xDD)
                return(1);
        }


        // write a value in memory
        for(int j=0; j< i; j++){
            u_int_array[j] = i+j+0xEE;
        }
        // start dma copy
        REG_DMA_SOURCE_ADDR = (unsigned int)u_int_array;
        REG_DMA_DEST_ADDR   = (unsigned int)register_array;
        REG_DMA_START = 0;
        REG_DMA_START = 1;
        // wait for dma done
        while(!REG_DMA_DONE);
        // verify destination content
        for(int j=0; j< i; j++){
            if(register_array[j] != i+j+0xEE)
                return(1);
        }

        REG_FW_REPORT ++;
    } 
    print_str("-- register check PASS ! --");
    return(0);
}

void main(){
    REG_FW_STATUS = 0x0 ;
    if(check_dma_d_mem() | check_dma_i_d_mem() | check_dma_registers())
        REG_FW_STATUS = 0xBAD;
    else{
        print_str("-- test PASS ! --");
        REG_FW_STATUS = 0xFF;
    }
}
