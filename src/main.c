#pragma config FOSC = INTOSCIO  //Internal oscillator - I/O on RA6 and RA7
#pragma config WDTE = OFF       //Watchdog timer off
#pragma config PWRTE = ON       //Power-up timer enabled
#pragma config MCLRE = OFF      //Disable master clear
#pragma config BOREN = OFF      //Disable brown-out detection
#pragma config LVP = OFF        //Disable low voltage program - free RB4
#pragma config CPD = OFF        //No EEPROM protection
#pragma config CP = OFF         //No code protection

#include <xc.h>

#define _XTAL_FREQ 4000000      //Internal oscillator frequency - Needed for __delay_xs functions

unsigned char a = 83;
unsigned char b = 179;
unsigned char c = 11;
unsigned char x = 7;

volatile unsigned char spin = 0;

void initRand(unsigned char s1, unsigned char s2, unsigned char s3){
    x++;
    a ^= s1;
    b ^= s2;
    c ^= s3;
    a = (a^c^x);
    b = (b+a);
    c = (c+(b>>1)^a);
}

unsigned char rand(void){
    x++;
    a = (a^c^x);
    b = (b+a);
    c = (c+(b>>1)^a);
    return(c);
}

void click(void){//Easiest, simplest, cheapest way to get a clicking sound that I could find
    TRISA1 = 0;
    __delay_ms(1);//Turn on piezo speaker for 1 ms
    TRISA1 = 1;
}

void led(unsigned char led){
    PORTA = 0;
    PORTB = 0;
    switch (led){
        case 0://Lets us turn off the LEDs before sleeping
            break;
        case 1:
            RB1 = 1;
            break;
        case 2:
            RB2 = 1;
            break;
        case 3:
            RB3 = 1;
            break;
        case 4:
            RB4 = 1;
            break;
        case 5:
            RB5 = 1;
            break;
        case 6:
            RB6 = 1;
            break;
        case 7:
            RB7 = 1;
            break;
        case 8:
            RA6 = 1;
            break;
        case 9:
            RA7 = 1;
            break;
        case 10:
            RA0 = 1;
            break;
    }
}

void interrupt isr(void){
    if(INTF){
        GIE = 0;
        spin = 1;//No debounce function since nothing else can happen until the spin ends anyway.
        INTF = 0;
    }
}

void main(void) {
    unsigned int i = 0;
    unsigned char curLed = 0;
    unsigned int spinDelay = 1;
    unsigned char spinValue = 0;

    CMCON = 0b00000111;//Disable comparator
    T1CON = 0b00000101;
    INTCON = 0b10010000;
    OPTION_REG = 0b11001000;
    TRISA = 0b00111110;
    TRISB = 0b00000001;
    PORTA = 0b00000000;
    PORTB = 0b00000000;

    initRand(TMR1H, TMR0, TMR1L);//Prime PRNG

    while(1){
        if(spin){

            while(spinValue > 10 || spinValue == 0){//Keep trying for a value between 1 and 10
                initRand(TMR1L, TMR0, TMR1H);
                spinValue = rand() & 0b00001111;
            }

            do{
                if(++curLed == 11)
                    curLed = 1;

                led(curLed);
                click();

                for(i = 0; i < spinDelay; i++)//Workaround. Delay maco can't be varied at runtime.
                   __delay_ms(1); 

                spinDelay += 4;//Change to vary the spin decay rate and duration

            }while(spinDelay < 100 || curLed != spinValue);

            i = 0;
            spin = 0;
            spinDelay = 1;
            spinValue = 0;
            GIE = 1;

            while(++i < 5000 && spin == 0)//Display number for 5 seconds then turn off unless the spin button is pressed again
                __delay_ms(1);
        }else{
            led(0);
            asm("sleep");
            asm("nop");
        }
    }
}
