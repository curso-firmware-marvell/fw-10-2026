
#include "includes.h"

#define ITERATIONS 20

volatile int int_0 ;
volatile int int_1 ;
volatile int int_2 ;

void main(){
    REG_FW_STATUS = 0x0 ;
    REG_FW_REPORT = 0x0 ;

    print_str("-- running test_fw_status --");
    
    for(int i=0; i< ITERATIONS; i++){

        int_2 = int_0 + int_1;
        int_2 += 1;
        int_1 = int_2 + int_0;
        int_1 += 6;
        int_0 = int_2 + int_1;
        int_0 += 2;
        
//        int_2 = int_0 * int_1;
//        int_2 *= 3;
//        int_1 = int_2 * int_0;
//        int_1 *= 5;
//        int_0 = int_2 * int_1;
//        int_0 *= 9;

        print_dec(i);

        REG_FW_REPORT = i;
    } 

    print_str("-- test done! --");
    
    REG_FW_STATUS = 0xFF;

}
