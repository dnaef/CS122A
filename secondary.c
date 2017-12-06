/*
 * Lab 2.c
 *
 * Created: 10/5/2017 12:54:48 PM
 * Author : ralme
 */ 
#define F_CPU 8000000UL  // 8 MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"
#include "nokia5110.c"

unsigned char nokia_num_blue = 0;
unsigned char nokia_num_green =0;
unsigned char nokia_num_red = 0;
unsigned char nokia_num_purp =0;
unsigned char nokia_speed = 1;
unsigned char nokia_status = 1;
char color_arr[5];
char input = 0;
char color = 0;
unsigned char temp=0;

unsigned char arr[5];
unsigned char cnt = 0;
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b)
{ return (b ? x | (0x01 << k) : x & ~(0x01 << k));}
unsigned char GetBit(unsigned char x, unsigned char k)
{ return ((x & (0x01 << k)) != 0);}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////--TIMER CODE--/////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


volatile unsigned char TimerFlag = 0;// TimerISR() sets this to 1. C programmer should clear to 0.

//internal variables for mapping AVR's ISR to our cleaner TimerISR model
unsigned long _avr_timer_M = 1; //Start count from here, down to 0. Default 1ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn(){   //AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0: CTC mode (Clear timer on compare)
	//bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	//SO, 8MHz clock or 8,000,000 /64 = 125,ticks/s
	//Thus, TCNT1 register will count at 125,000 tick/s


	// AVR output compare register OCR1A.
	OCR1A = 125; //Timer interrupt will be generated when TCNT1==OCR1A
	//We want a 1ms tick. -> 0.001s * 125,000 tick/s = 125
	//1 ms has passed. Thus, we compare to 125.


	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}
void TimerOff()
{
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}
void TimerISR()
{
	TimerFlag =1;
}
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0)
	{ // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////-- END TIMER CODE--/////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum LCD {INIT_LCD, UPDATE_LCD} LCD_switch;
void nokia_lcd_draw_data(){
    nokia_lcd_clear();
    nokia_lcd_set_cursor(0,0);
    nokia_lcd_write_string("BLUE: "  ,1);
    nokia_lcd_set_cursor(0,8);
    nokia_lcd_write_string("GREEN: "  ,1);
    nokia_lcd_set_cursor(0,16);
    nokia_lcd_write_string("RED: "  ,1);
    nokia_lcd_set_cursor(0,24);
    nokia_lcd_write_string("PURPLE: "  ,1);
    nokia_lcd_set_cursor(0,32);
    nokia_lcd_write_string("SPEED: "  ,1);
    nokia_lcd_set_cursor(0,40);
    nokia_lcd_write_string("STATUS: "  ,1);
    //nokia_lcd_render();
    //update blue
    int temp = nokia_num_blue;  


    itoa(temp,color_arr,10);
    nokia_lcd_set_cursor(43,0);
    nokia_lcd_write_string(color_arr ,1);

    temp = nokia_num_green;
    itoa(temp,color_arr,10);
    nokia_lcd_set_cursor(43,8);
    nokia_lcd_write_string(color_arr ,1);

    temp = nokia_num_red;
    itoa(temp,color_arr,10);
    nokia_lcd_set_cursor(43,16);
    nokia_lcd_write_string(color_arr ,1);

    temp = nokia_num_purp;
    itoa(temp,color_arr,10);
    nokia_lcd_set_cursor(43,24);
    nokia_lcd_write_string(color_arr ,1);

    temp = nokia_speed;
    itoa(temp,color_arr,10);
    nokia_lcd_set_cursor(43,32);
    nokia_lcd_write_string(color_arr ,1);

    temp = nokia_status;
    itoa(temp,color_arr,10);
    nokia_lcd_set_cursor(43,40);
    if(temp == 1){
        nokia_lcd_write_string("STANDBY" ,1);
    }
    else if(temp == 2){
        nokia_lcd_write_string("RUNNING" ,1);
    }

    nokia_lcd_render();
    _delay_ms(100);

}
void LCD_Tick(){
    //actions
    switch(LCD_switch){
        case INIT_LCD:
        nokia_lcd_draw_data();
        break;
        default:
        break;

    }
    //transitions
    switch(LCD_switch){
        default:
        LCD_switch = INIT_LCD;
        break;
    }
}
int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x1F; PORTC=0xE0;
	
	initUSART(0);
    nokia_lcd_init();
	TimerSet(1);
	TimerOn();
	unsigned char temp=0;
	char turn= 0x00;  
	LCD_switch = INIT_LCD;

	while (1)
	{	
	//second one
		
		 if ( USART_HasReceived(0) ) 
		 {
			 //...receive data...
			temp = USART_Receive(0);
            //cnt++;			 // write data received by USART0to temp
		}
        
        PORTA = temp;
                    if(temp == 0x01){nokia_num_blue++;}
                    else if(temp == 0x02){nokia_num_red++;}
                    else if(temp == 0x04){nokia_num_green++;}
                    else if(temp ==0x08){nokia_num_purp++;}
                    else if(temp == 0x10){nokia_speed++;}
                    else if(temp == 0x20){nokia_speed--;}
                    else if(temp == 0x40){nokia_status=1;}
                    else if(temp == 0x80){nokia_status=2;}
                    else if(temp == 0xF1){nokia_speed=0;nokia_status=1;}
                    temp = 0;
        USART_Flush(0);
       LCD_Tick();
	while(!TimerFlag);
	TimerFlag = 0;
		

	}
}
