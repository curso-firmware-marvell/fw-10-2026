// Pruebas
// - Printear señal
// - Inicio de captura
// - Captura continua
#include <stdio.h>
#include <stdint.h>


#include "includes.h"

#define DELAY 32

int check_adc_state_case_0(int frec){
    REG_ADC_START = 0;
    REG_ADC_CLK_DIVIDER = frec;
    for(volatile int i; i<DELAY; i++);
    REG_ADC_START = 1;
    while(REG_ADC_STATUS!=0b01);
    print_str("--half signal pass--");
    while(REG_ADC_STATUS!=0b10);
    print_str("--full signal pass--");
    for(int k; k<DELAY; k++){
        if(REG_ADC_STATUS!=0b10){
            return 1;
        }
    }
    print_str("--start without repeticion pass--");
    return 0;
}

int check_adc_state_case_1(int frec){
    REG_ADC_START = 0;
    REG_ADC_CLK_DIVIDER = frec;
    for(volatile int i; i<DELAY; i++);
    REG_ADC_START = 2;
    while(REG_ADC_STATUS!=0b01){}
    while(REG_ADC_STATUS!=0b10){}
    while(REG_ADC_STATUS!=0b01){}
    while(REG_ADC_STATUS!=0b10){}
    print_str("--circular function pass--");
    return 0;
}

int check_adc_signal(){

    for(int i = 0 ; i < REG_ARRAY_ADC_BUFF_SIZE ; i++){
        REG_FW_REPORT = REG_ARRAY_ADC_BUFF[i];
    }

    return 0;
}


void main(){
    REG_FW_STATUS = 0x0;
    int divider = 4;
    int check = 0;
    if(check_adc_state_case_0(divider) | check_adc_state_case_1(divider) | check_adc_signal()){
        print_str("------TEST ERROR-----");
        REG_FW_STATUS = 0xBAD;
    }else{
        print_str("------TEST 1 PASS------");
        check = check + 1;
    }
    divider = 8;
    if(check_adc_state_case_0(divider) | check_adc_state_case_1(divider) | check_adc_signal()){
        print_str("------TEST ERROR-----");
        REG_FW_STATUS = 0xBAD;
    }else{
        print_str("------TEST 2 PASS------");
        check = check + 1;
    }
    divider = 16;
    if(check_adc_state_case_0(divider) | check_adc_state_case_1(divider) | check_adc_signal()){
        print_str("------TEST ERROR-----");
        REG_FW_STATUS = 0xBAD;
    }else{
        print_str("------TEST 3 PASS------");
        check = check + 1;
    }
    if(check == 3) {
        REG_FW_STATUS = 0xFF;
    }
}