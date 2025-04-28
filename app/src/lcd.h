/*

    This header file contains all the needed logic to communicate with the lcd screen

*/

#ifndef LCD_H
#define LCD_H

// includes
#include "intrinsics.h"
#include <stddef.h>

// constants
#define RS BIT0
#define RW BIT1
#define EN BIT2

// globals
int busy = 0;

// functions

inline void set_lcd_ports(void)
{
    P3DIR |= 0xF;
    P3OUT &= ~0xF;
    P4DIR |= 0x7;
    P4OUT &= ~0x7;
    return;
}

inline void write_data(int upper, int lower, int rs, int rw)
{
     check_busy();
    if (rs == 0)
    {
        P4OUT &= ~RS;
    }
    else 
    {
        P4OUT |= RS;
    }

    if (rw == 0)
    {
        P4OUT &= ~RW;
    }
    else 
    {
        P4OUT |= RW;
    }

    P6OUT &= ~0xF;
    P6OUT |= upper;
    P4OUT |= EN;
    __delay_cycles(4);
    P4OUT &= ~EN;
    __delay_cycles(4);
    P6OUT &= ~0xF;
    P6OUT |= lower;
    P4OUT |= EN;
    __delay_cycles(4);
    P4OUT &= ~EN;
    __delay_cycles(20);
    return;
}

inline void check_busy(void)
{
    P6DIR &= ~0xF;
    P6REN |= 0xF;
    P6OUT &= ~0xF;

    P4OUT |= RW;
    P4OUT &= ~RS;
    busy = 1;

    while(busy != 0)
    {
        //enable high
        P4OUT |= EN;
        busy = P6IN;
        busy &= BIT3;

        __delay_cycles(40);

        P4OUT &= ~EN;

        __delay_cycles(40);

        P4OUT |= EN;

        __delay_cycles(40);

        P4OUT &= ~EN;
    }

    P6DIR |= 0xF;
    P6OUT &= ~0xF;
    P4OUT &= ~(RS | RW | EN);

    return;
}

inline void lcd_setup(void)
{
    // Function set
    write_data(0b0010, 0b1000, 0, 0);
    // Clear Display
    write_data(0b0000, 0b0001, 0, 0);
    // Return Home
    write_data(0b0000, 0b0010, 0, 0);
    // display on/off controls cursor and stuff see datasheet
    write_data(0b0000, 0b1100, 0, 0);
    return;
}

inline void lcd_char(char c)
{
    int lower = c & 0xF;
    int upper = (c>>4) & 0xF;

    write_data(upper,lower,1,0);

    return;
}

inline void lcd_pos(int addr)
{
    // does not work look at later
    int lower = addr & 0xF;
    int upper = ((addr>>4) & 0xF) | 0x8;

    write_data(upper,lower,0,0);

    return;
}

inline void lcd_string(const char *str, size_t len)
{
    if (str == NULL) return; // simple null check

    size_t i;
    for (i = 0; i < len; i++) 
    {
        lcd_char(str[i]);
        __delay_cycles(10);
    }

    return;
}

inline void process_temp(float temp)
{
     // Extract integer and decimal parts
    int int_part = (int)temp;
    int decimal_digit = (int)((temp- int_part) * 10.0f + 0.5f); // round to nearest

    char temp_num_str[5] = "  . ";

    if (int_part >= 10) 
    {
        temp_num_str[0] = '0' + (int_part / 10);
        temp_num_str[1] = '0' + (int_part % 10);
    } 
    else 
    {
        temp_num_str[1] = '0' + int_part;
    }

    temp_num_str[3] = '0' + (decimal_digit % 10);

    lcd_pos(0xA);
    lcd_string(temp_num_str, 4);
    return;
}

inline void process_time(int time)
{
    char time_str[4] = "000";

    if (time >= 100)
    {
        time_str[0] = '0' + (time / 100);
        time_str[1] = '0' + ((time / 10) % 10);
        time_str[2] = '0' + (time % 10);
    }
    else if (time >= 10)
    {
        time_str[1] = '0' + (time / 10);
        time_str[2] = '0' + (time % 10);
    }
    else
    {
        time_str[2] = '0' + time;
    }
    lcd_pos(0x42);
    lcd_string(time_str, 3);
    
    return;
}

#endif