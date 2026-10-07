
#include "includes.h"

#define ITERATIONS 20

volatile int int_0 ;
volatile int int_1 ;
volatile int int_2 ;

void delay(int loops){
    for(int i=0; i<loops; i++){
        REG_AUX_0 ++;
        REG_AUX_0 --;
    }
}

void main(){
    REG_FW_STATUS = 0x0 ;
    REG_FW_REPORT = 0x0 ;

    REG_UART0_START_TRANSMIT = 0 ;
    
    REG_ARRAY_UART0_BUFF_OUT[0] = (unsigned int)'h';
    REG_ARRAY_UART0_BUFF_OUT[1] = (unsigned int)'e';
    REG_ARRAY_UART0_BUFF_OUT[2] = (unsigned int)'l';
    REG_ARRAY_UART0_BUFF_OUT[3] = (unsigned int)'l';
    REG_ARRAY_UART0_BUFF_OUT[4] = (unsigned int)'o';
    REG_ARRAY_UART0_BUFF_OUT[5] = (unsigned int)' ';
    REG_ARRAY_UART0_BUFF_OUT[6] = (unsigned int)'L';
    REG_ARRAY_UART0_BUFF_OUT[7] = (unsigned int)'u';
    REG_ARRAY_UART0_BUFF_OUT[8] = (unsigned int)'i';
    REG_ARRAY_UART0_BUFF_OUT[9] = (unsigned int)'g';
    REG_ARRAY_UART0_BUFF_OUT[10] = (unsigned int)'i';
    REG_ARRAY_UART0_BUFF_OUT[11] = (unsigned int)'!';
    
    REG_UART0_SIZE_TO_TRANSMIT = 12;
    REG_UART0_START_TRANSMIT = 1 ;
    REG_UART0_START_TRANSMIT = 0 ;

    while (!REG_UART0_TRANSMISION_DONE){}

    REG_AUX_0 = 0x5ED5ED ; // to start testbench transmision 
    REG_AUX_0 = 0x0      ; // to start testbench transmision 

    while (!REG_UART0_DATA_AVAILABLE){}
    delay(4);

    int size_in = REG_UART0_INDEX_IN;
    for(int i=0; i<size_in; i++){
        REG_ARRAY_UART0_BUFF_OUT[i] = REG_ARRAY_UART0_BUFF_IN[i];
    }
    REG_UART0_SIZE_TO_TRANSMIT = size_in;
    REG_UART0_START_TRANSMIT = 1 ;
    REG_UART0_START_TRANSMIT = 0 ;




    REG_FW_STATUS = 0xFF;

}
