#include <msp430.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "intrinsics.h"
#include "lcd.h"
#include "adc.h"
#include <math.h>

unsigned int ADC_Value;
float voltage;
float temp;
int time_index = 0;
int time = 0;

int main(void)
{
    
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mdoe to activate

    const char temp_str_A[] = "A:XX.X";
    size_t temp_str_l_A = sizeof(temp_str_A) - 1;

    const char temp_str_P[] = "P:XX.X";
    size_t temp_str_l_P = sizeof(temp_str_A) - 1;

    const char time_initial_str[] = "000s";

    set_lcd_ports();
    setup_ADC();
    lcd_setup();
    lcd_pos(0x8);
    lcd_string(temp_str_A, temp_str_l_A);
    lcd_char(0xDF);
    lcd_char('C');
    lcd_pos(0x48);
    lcd_string(temp_str_P, temp_str_l_P);
    lcd_char(0xDF);
    lcd_char('C');
    lcd_pos(0x42);
    lcd_string(time_initial_str, 4);


    while(1)
    {
        __delay_cycles(1000);
    }
}


#pragma vector = TIMER2_B0_VECTOR
__interrupt void ADC_read_ISR(void) {
    ADCCTL0 |= ADCENC | ADCSC;      // read ADC
    // update time

    time_index = (time_index + 1) % 2;

    if (time_index == 1)
    {
        time++;
        process_time(time);
    }
    TB2CCTL2 &= ~CCIFG;
}

#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void){
    
    ADC_Value = ADCMEM0;                
    // voltage formula
    voltage = ADC_Value * 3.3f / 4095.0f;
    // temp formula from ADC
    temp = -1481.96f + sqrt(2.1962e6f + ((1.8639f - voltage) / 3.88e-6f));

    moving_average(temp);
}