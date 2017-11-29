#define F_CPU 20000000UL  // 8 MHz

#include <stdint.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <string.h> 
#include <math.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/eeprom.h> 
#include <avr/portpins.h> 

#include "nokia5110.c"

void oneStep(char stepDir);
 
//FreeRTOS include files 
#include "FreeRTOS.h" 
#include "task.h" 
#include "croutine.h" 

#define left 0
#define right 1



char B0, B1, B2, B3, B4, B5, B6, B7 = 0;
char D0, D1, D2, D3, D4, D5, D6, D7 = 0;
char C1, C5, C6, C7 = 0;
char address = 0x00;// SPI address for controlling the digital pot that controls the speed of the motor
unsigned char curSpeed = 1;
const char increment = 10; // amount that the speed increases per press

unsigned char count = 0;

char red = 0x20;
char green = 0x40;
char blue = 0x80;
char purple = 0xA0;
char seen = 0;
unsigned char nokia_num_blue = 0;
unsigned char nokia_num_green =0;
unsigned char nokia_num_red = 0;
unsigned char nokia_num_purp =0;
unsigned char nokia_speed = 1;
unsigned char nokia_status = 1;
char color_arr[5];

unsigned char COLOR = 0x00;
#define posTrigger B7


char dir, dir2 = 0;

enum LCD {INIT_LCD, UPDATE_LCD} LCD_switch;
enum Sort {INIT_sort, WAIT_sort, GREEN, RED, BLUE, BLACK, PURPLE, CENTER} Sort_1;
enum SpeedControl {INIT_sp, WAIT_sp, RUN, STOP, INC, DEC} Speed;


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b)
{ return (b ? x | (0x01 << k) : x & ~(0x01 << k));}
unsigned char GetBit(unsigned char x, unsigned char k)
{ return ((x & (0x01 << k)) != 0);}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////SPI CODE///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Master code
void SPI_MasterInit(void) {
	
	/* Set MOSI and SCK output, all others input */
	//DDRB = (1 << DDB5) || (1 << DDB7); //SPI = (1<<DD_MOSI)|(1<<DD_SCK);
	DDRB = 0xB0;
	PORTB = 0x4F;
    	//DDRB = 0xBF;
    	//PORTB = 0x40;
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	// Make sure global interrupts are enabled on SREG register (pg. 9)
	SREG = 0x80; // 0x80: 1000000
	
}

void SPI_MasterTransmit(unsigned char cData) {
	/* Start transmission */
	SetBit(PORTB,4,0);
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	
	SetBit(PORTB,4,1);// set SS high
}

void SPI_DigiPot(unsigned char cData) {
    /* Start transmission */
    SetBit(PORTB,4,0);
    SPDR = address;
    /* Wait for transmission complete */
    while(!(SPSR & (1<<SPIF)));
    SPDR = cData;
    /* Wait for transmission complete */
    while(!(SPSR & (1<<SPIF)));
        
    SetBit(PORTB,4,1);// set SS high
    }

// Servant code
void SPI_ServantInit(void) {
	
	DDRB = 0x40;// set DDRB to have MISO line as output and MOSI, SCK, and SS as input
	PORTB = 0xBF;//
	SPCR |= (1<<SPE)|(1<<SPIE);// set SPCR register to enable SPI and enable SPI interrupt (pg. 168)
	SREG = 0x80; // 0x80: 1000000// make sure global interrupts are enabled on SREG register (pg. 9)
}
unsigned char receivedData = 0;
ISR(SPI_STC_vect) { // this is enabled in with the SPCR register’s “SPI
	// Interrupt Enable”
	// SPDR contains the received data, e.g. unsigned char receivedData = SPDR;
	receivedData = SPDR;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////END SPI CODE///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadInput(void){ // Gets the status of all of the inputs
	B0 = ~PINB & 0x01;
	B1 = ~PINB & 0x02;
	B2 = ~PINB & 0x04;
	B3 = ~PINB & 0x08;
	B4 = ~PINB & 0x10;
	B5 = ~PINB & 0x20;
	B6 = ~PINB & 0x40;
	B7 = ~PINB & 0x80;
	return;
}

void ReadUpperB(void){ // Gets the status of all of the inputs
    B0 = ~PINB & 0x01;
    B1 = ~PINB & 0x02;
    B2 = ~PINB & 0x04;
    B3 = ~PINB & 0x08;
    //B7 = ~PINB & 0x80;
    return;
}

void ReadD(void){ // Gets the status of all of the inputs
	D0 = ~PIND & 0x01;
	D1 = ~PIND & 0x02;
	D2 = ~PIND & 0x04;
	D3 = PIND & 0x08;
	D4 = PIND & 0x10;
	D5 = PIND & 0x20;
	D6 = PIND & 0x40;
	D7 = PIND & 0x80;
	return;
}

void ReadC(void){ // Gets the status of all of the inputs
    //TODO: change to standard input
    C5 = PINC & 0x20;
    C6 = PINC & 0x40;
    C7 = PINC & 0x80;    
    return;
}

void LCD_INIT(){
    LCD_switch = INIT_LCD;
}

void SortINIT(){
	Sort_1 = INIT_sort;
}

void SpeedINIT(){
    Speed = INIT_sp;
}

short RotationDeg(short degree) {
	return floor(degree / 1.8 * 2);
}

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
}

void moveTo(unsigned short curPos, unsigned short targetPos){//renove crpos
    if (curPos != targetPos){
        if (curPos < targetPos){
            oneStep(right);
            curPos++;
        }
        else if (curPos > targetPos){
            oneStep(left);
            curPos--;
        }
    }
}

unsigned long accelerate = 0;
short deg90,deg180 = 0;
char step = 0;
char step2 = 0;
short curPos = 0; //the current position of the paddles
unsigned short targetPos = 0; // the position that the paddle need to move to 
char input = 0;
char color = 0;

unsigned short greenPos = 0;
unsigned short redPos = 120;
unsigned short bluePos = 240;
unsigned short purpPos = 360;
char temp = 0;

void setDegrees(){
    purpPos = RotationDeg(purpPos);
    redPos = RotationDeg(redPos);
    bluePos = RotationDeg(bluePos);
    greenPos = 0;
}

void oneStep(char stepDir){
    PORTA = SetBit(PORTA,1,stepDir); // set the direction that the motors are moving 
    PORTA = SetBit(PORTA,3,stepDir);
    step = ~step;
    PORTA = SetBit(PORTA,0,step); // move each motor one step
    PORTA = SetBit(PORTA,2,step);
    
}

void LCD_Tick(){
    //actions
    switch(LCD_switch){
        case INIT_LCD:
            nokia_lcd_draw_data();
        break;
        case UPDATE_LCD:
            nokia_lcd_draw_data();
        break;
        default:
           // no action
        break;

    }
    //transitions
    switch(LCD_switch){
        case INIT_LCD:
           LCD_switch = UPDATE_LCD;
        break;
        case UPDATE_LCD:
            LCD_switch = UPDATE_LCD;
        break;
        default:
            LCD_switch = INIT_LCD;
        break;
    }
}

void SortTick(){
	//Actions
    
    switch(Sort_1){
        case INIT_sort: 
            Sort_1 = WAIT_sort;
        break; 
        case WAIT_sort:
            ReadC();            
            if(!C5 && !C6 && !C7){
                
            }
            else{
                if(C5 && C7){
                    Sort_1 = PURPLE;
                    if (seen == 0){
                        nokia_num_purp++;
                        seen = 1;
                    }                    
                    targetPos = RotationDeg(purpPos);
                }
                else if (C6){//c7 BLUE 
                    targetPos = RotationDeg(greenPos);
                    if (seen == 0){
                        nokia_num_green++;
                        seen = 1;
                    }                    
                    Sort_1 = GREEN;
                }            
                else if(C7){ // c6 gREEN
                    Sort_1 = BLUE;
                    if (seen == 0){
                        nokia_num_blue++;
                        seen = 1;
                    }
                    targetPos = RotationDeg(bluePos);
                }
                else if(C5){ // C5 RED
                    Sort_1 = RED;
                    if (seen == 0){
                        nokia_num_red++;
                        seen = 1;
                    }
                    targetPos = RotationDeg(redPos);
                }

            }
        break;
        case GREEN:
            if(curPos >targetPos){
                oneStep(left);
                curPos--;
            }
            else{
                Sort_1 = WAIT_sort;
            }
        break;
        case RED: 
            if (curPos < targetPos){
                oneStep(right);
                curPos++;
            }
            else if (curPos > targetPos){
                oneStep(left);
                curPos--;
            }
            if (curPos == targetPos){
                Sort_1 = WAIT_sort;
            }            
        break;
        case BLUE:
            if (curPos < targetPos){
                oneStep(right);
                curPos++;
            }
            else if (curPos > targetPos){
                oneStep(left);
                curPos--;
            }
            if (curPos == targetPos){
                Sort_1 = WAIT_sort;
            }
        break;
        case PURPLE:
            if (curPos < targetPos){
                oneStep(right);
                curPos++;
            }
            else if (curPos > targetPos){
                oneStep(left);
                curPos--;
            }
            if (curPos == targetPos){
                Sort_1 = WAIT_sort;
            }            
        break;
        default:
            
        break;
    }
}

void SpeedControlTick(){
    //Actions
    switch(Speed){
        case INIT_sp:
            curSpeed = 1;
            SPI_DigiPot(curSpeed);
        break;
        case WAIT_sp:
        break;
        case RUN:
        break;
        case INC:
        break;
        case DEC:
        break;
        case STOP:
            curSpeed = 0;
            SPI_DigiPot(1);
        break;
        default:
            curSpeed = 0;
            SPI_DigiPot(curSpeed);
        break;
    }
    //Transitions
    switch(Speed){
        case INIT_sp:
            Speed = WAIT_sp;
        break;
        case WAIT_sp:
            curSpeed = 1;
            if(B1){
                nokia_status = 2;
                Speed = RUN;
            }
        break;
        case RUN:
            if(B0){
                nokia_status = 1;
                nokia_speed = 0;
                Speed = STOP;
                curSpeed = 0;
                SPI_DigiPot(curSpeed);
            }
            if (B2){
                Speed = INC;
                if(curSpeed < 128){
                    nokia_speed++;
                    curSpeed += increment;
                    SPI_DigiPot(curSpeed);
                }
            }
            if (B3) {
                Speed = DEC;
                if(curSpeed >increment){
                    nokia_speed--;
                    curSpeed -= increment;
                    SPI_DigiPot(curSpeed);
                }
            }

        break;
        case INC:
            if(!B2) {
                Speed = RUN;
            }
        break;
        case DEC:
            if(!B3) {
                Speed = RUN;
            }
        break;
        case STOP:
            Speed = WAIT_sp;
        break;
        default:
            Speed = INIT_sp;
        break;
    }
}

void SortTask(){
    SortINIT();
    for(;;){
        ReadC();
        SortTick();
        vTaskDelay(20);        
    }
}

void LCDTask(){
    LCD_INIT();
    for(;;){
        LCD_Tick();
        vTaskDelay(1000);
    }
}

void SpeedControlTask(){
    SpeedINIT();
    for(;;){
        ReadUpperB();
        SpeedControlTick();
        vTaskDelay(200);
    }
}

void StartSortPulse(unsigned portBASE_TYPE Priority){
    xTaskCreate(SortTask, (signed portCHAR *)"SortTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void StartSpeedPulse(unsigned portBASE_TYPE Priority){
    xTaskCreate(SpeedControlTask, (signed portCHAR *)"SpeedControlTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void StartLCD_Pulse(unsigned portBASE_TYPE Priority){
    xTaskCreate(LCDTask, (signed portCHAR *)"LCDTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}
 
int main(void) { 
	DDRA = 0xFF; PORTA=0x00;
	DDRB = 0x00; PORTB=0xFF; //used for nokia screen CHANGE
	DDRD = 0x00; PORTD=0xFF;
    DDRC = 0x1F; PORTC=0xE0; // sets the highest 3 bits as input and the rest to output
    //DDRC =0xFF; PORTC=0x00;
    //DDRC =0x00; PORTC=0xFF;
    SPI_MasterInit();
    nokia_lcd_init();
    
   

    SPI_DigiPot(0);

    StartSpeedPulse(1);
    StartSortPulse(2);
    //StartLCD_Pulse(3);
    //RunSchedular 
	ReadD();
    vTaskStartScheduler(); 
	return 0; 
}
