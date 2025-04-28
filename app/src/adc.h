/*

    This header file contains all the needed logic to communicate with the LM19 chip

*/

#ifndef ACD_H
#define ACD_H

#include "intrinsics.h"
#include <stddef.h>
#include <msp430.h>
#include "lcd.h"

#define WINDOW_SIZE 3
int temp_index = 0;
int sample_count = 0;

float temp_buffer[WINDOW_SIZE];

inline void setup_ADC(void) {

    P1SEL1 |= BIT1;                         // Configure P1.2 Pin for A1
    P1SEL0 |= BIT1; 

    ADCCTL0 &= ~ADCSHT;                     // Clear ADCSHT from def. of ADCSHT=01
    ADCCTL0 |= ADCSHT_2;                    // Conversion Cycles = 16 (ADCSHT=10)
    ADCCTL0 |= ADCON;                       // Turn ADC ON
    ADCCTL1 |= ADCSSEL_2;                   // ADC Clock Source = SMCLK
    ADCCTL1 |= ADCSHP;                      // Sample signal source = sampling timer
    ADCCTL2 &= ~ADCRES;                     // Clear ADCRES from def. of ADCRES=01
    ADCCTL2 |= ADCRES_2;                    // Resolution = 12-bit (ADCRES = 10)
    ADCMCTL0 |= ADCINCH_1;                  // ADC Input Channel = A1 (P1.1)
    ADCIE |= ADCIE0;                        // Enable ADC Conv Complete IRQ

    // Configure Timer B2
    TB2CTL |= (TBSSEL__ACLK | MC__UP | TBCLR);  // Use ACLK, up mode, clear
    TB2CCR0 = 16320;                             // 0.5s
    
    // Enable and clear interrupts                           
    TB2CCTL0 |= CCIE;                          
    TB2CCTL0 &= ~CCIFG; 

    __enable_interrupt();                   // Enable interrupts
    return;
}

inline void moving_average(float new_temp) {
    // Store new sample in circular buffer
    temp_buffer[temp_index] = new_temp;
    temp_index = (temp_index + 1) % WINDOW_SIZE;

    // Track how many samples we have received (up to WINDOW_SIZE)
    if (sample_count < WINDOW_SIZE) {
        sample_count++;
        return;  // not enough data yet
    }

    // Compute average
    float sum = 0.0f;
    int i;
    for (i = 0; i < WINDOW_SIZE; i++) {
        sum += temp_buffer[i];
    }

    float avg = sum / WINDOW_SIZE;
    process_temp(avg);
    return;

}

#endif