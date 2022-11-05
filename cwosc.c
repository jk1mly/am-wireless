/*
 * AM wireless morse osc  for PIC12F1612
 *
 *      JK1MLY:Hidekazu Inaba
 *
 *  (C)2022 JK1MLY All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

// CONFIG1
#pragma config FOSC     = INTOSC   // Oscillator Selection Bits->INTOSC oscillator: I/O function on CLKIN pin
#pragma config PWRTE    = OFF      // Power-up Timer Enable->PWRT disabled
#pragma config MCLRE    = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input)
#pragma config CP       = OFF      // Flash Program Memory Code Protection->Program memory code protection is disabled
#pragma config BOREN    = ON       // Brown-out Reset Enable->Brown-out Reset enabled
#pragma config CLKOUTEN = OFF      // Clock Out Enable->CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin

// CONFIG2
#pragma config WRT      = OFF      // Flash Memory Self-Write Protection->Write protection off
#pragma config ZCD      = OFF      // Zero Cross Detect Disable Bit->ZCD disable.  ZCD can be enabled by setting the ZCDSEN bit of ZCDCON
#pragma config PLLEN    = OFF      // PLL Enable Bit->4x PLL is enabled when software sets the SPLLEN bit
#pragma config STVREN   = ON       // Stack Overflow/Underflow Reset Enable->Stack Overflow or Underflow will cause a Reset
#pragma config BORV     = LO       // Brown-out Reset Voltage Selection->Brown-out Reset Voltage (Vbor), low trip point selected.
#pragma config LPBOR    = OFF      // Low-Power Brown Out Reset->Low-Power BOR is disabled
#pragma config LVP      = ON       // Low-Voltage Programming Enable->Low-voltage programming enabled

// CONFIG3
#pragma config WDTCPS   = WDTCPS1F // WDT Period Select->Software Control (WDTPS)
#pragma config WDTE     = OFF      // Watchdog Timer Enable->WDT disabled
#pragma config WDTCWS   = WDTCWSSW // WDT Window Select->Software WDT window size control (WDTWS bits)
#pragma config WDTCCS   = SWC      // WDT Input Clock Selector->Software control, controlled by WDTCS bits


#define _XTAL_FREQ 8000000
// Pin list
//          GND     VSS       Pin 8
#define		F_UP	RA0     //Pin 7 AN0 DA1 CCP2
#define		Key		RA1     //Pin 6 AN1 REF
#define		Ant		RA2     //Pin 5 AN2     CCP1
#define		F_DN	RA3     //Pin 4 MCLR
#define		SPX 	RA4     //Pin 3 AN3 OUT
#define		SPK 	RA5     //Pin 2     IN
//          VCC     VDD       Pin 1

void port_init(void) {

    // Configure GPIO
    LATA     = 0b00000000;    //Output
    TRISA    = 0b00001011;    //In(1) /Out(0)
    ANSELA   = 0b00000000;    //Analog(1)
    WPUA     = 0b00001011;    //PupOn(1)
    OPTION_REGbits.nWPUEN = 0;
    ODCONA   = 0b00000000;
    SLRCONA  = 0b00110111;
    INLVLA   = 0b00111111;
    APFCON   = 0b00000000;    //ALTERNATE PIN FUNCTION

    // SCS FOSC; SPLLEN disabled; IRCF 8MHz_HF;
    OSCCON   = 0b01110000;    //PLL IRCF3:0 1110;8M 0 SCS1:0 00;config
    // TUN 0;
    OSCTUNE = 0x00;
    // SBOREN disabled; BORFS disabled;
    BORCON = 0x00;

    // Configure CCP
    CCP1CON  = 0b11001111;    //EN OE OUT FMT MODE3:0 11xx=PWM
    CCPR1H   = 0b00000000;
    CCPR1L   = 0b00000011;
    CCPTMRSbits.CCP1TSEL = 0x0; // Selecting Timer 2

    // Timer configuration
    // 8MHz/4(T2CS)/2(toggle))
    T2CLKCON = 0b00000000;    //00000 T2CS 000;FOSC/4 001;FOSC
    T2HLT    = 0b00000000;    //PSYN POL SYN 0 MODE3:0 0000;freerun
    T2RST    = 0b00000000;    //T2RSEL T2IN
    T2PR     = 0b00000001;    //PR2 1
    T2TMR    = 0b00000001;    //TMR2 1
    PIR1bits.TMR2IF = 0;      //Clearing IF flag.
    T2CON    = 0b00000000;    //ON CKPS2:0=1/1 OUTPS3:0=1/1
//  T2CONbits.TMR2ON = 1;
}

void send_dot(void){
    for(uint8_t lp=0; lp<60; lp++){
        T2CONbits.T2ON = 1;
        SPK = 1;
        SPX = 0;
        __delay_us(830);
        T2CONbits.T2ON = 0;
        SPK = 0;
        SPX = 1;
        __delay_us(830);
    }
    __delay_ms(100);
}

void send_dash(void){
    for(uint8_t lp=0; lp<180; lp++){
        T2CONbits.T2ON = 1;
        SPK = 1;
        SPX = 0;
        __delay_us(830);
        T2CONbits.T2ON = 0;
        SPK = 0;
        SPX = 1;
        __delay_us(830);
    }
    __delay_ms(100);
}

void send_v(void){
    send_dot();
    send_dot();
    send_dot();
    send_dash();
}

void send_vvv(void){
    for(uint8_t lp=0; lp<3; lp++){
        send_v();
        __delay_ms(200);
    }
    __delay_ms(700);
}

void main(void){
    uint8_t flag = 0;

    port_init();
// switch check
    for(uint8_t lp=0; lp<5; lp++){
        if(Key == 0){
            flag = 1;
            break;
        }
        __delay_ms(200);
    }

// oscillator turn on
    T2CONbits.TMR2ON = 1;
    
// frequency adjust
    if(F_DN == 0){
        OSCTUNE = 0x3D;
            SPK = 1;
        __delay_ms(500);
            SPK = 0;
        __delay_ms(200);
    } else if(F_UP == 0){
        __delay_ms(300);
        OSCTUNE = 0x03;
            SPK = 1;
        __delay_ms(500);
            SPK = 0;
        __delay_ms(200);
    } else {
        OSCTUNE = 0x00;
    }

// test mode
    while(flag == 1){
        T2CONbits.TMR2ON = 1;
        __delay_ms(100);
        send_vvv();

// tuning
        while(Key == 0){
            if(F_DN == 0){
                OSCTUNE = 0x3D;
            } else if(F_UP == 0){
                OSCTUNE = 0x03;
            } else {
                OSCTUNE = 0x00;
            }
            __delay_ms(500);
        }

        T2CONbits.TMR2ON = 0;
// switch check
        for(uint8_t lp=0; lp<10; lp++){
            __delay_ms(100);
            if(Key == 0){
                flag = 0;
                break;
            }
        }
    }

// normal operation
    T2CONbits.TMR2ON = 1;
    __delay_ms(100);
    while (1)
    {
        // key down tone
        if (Key == 0) {
            T2CONbits.T2ON = 1;
            SPK = 1;
            SPX = 0;
            __delay_us(830);
            T2CONbits.T2ON = 0;
            SPK = 0;
            SPX = 1;
            __delay_us(830);
        } else {
            __delay_us(830);
        }
    }
}