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
#include <avr/pgmspace.h> 
 
//FreeRTOS include files 
#include "FreeRTOS.h" 
#include "task.h" 
#include "croutine.h" 

#define left 0
#define right 1

char B0, B1, B2, B3, B4, B5, B6, B7 = 0;
char D0, D1, D2, D3, D4, D5, D6, D7 = 0;



char dir, dir2 = 0;

enum Stepper1 {INIT,WAIT,WAIT_REL,Deg90,Deg180} Stepper_1;
enum Stepper2 {INIT2,WAIT2,WAIT_REL2,Deg90_2,Deg180_2} Stepper_2;
enum Sort {INIT_sort, WAIT_sort, GREEN, RED, BLUE, BLACK, PURPLE} Sort_1;

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
	DDRB = 0xBF;
	PORTB = 0x40;
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

void SpinUP(unsigned short count) {
	unsigned short i = 0;
	while (i < count )
	{
	 i++;
	}
}

void SpinDOWN(unsigned short count) {
	unsigned short i = 0;
	while (i < count )
	{
		i++;
	}
}
void Stepper1INIT(){
	Stepper_1 = INIT;
}

void Stepper2INIT(){
	Stepper_2 = INIT2;
}

void SortINIT(){
	Sort_1 = INIT_sort;
}


short RotationDeg(short degree) {
	return floor(degree / 1.8 * 2);
}

void moveTo(unsigned short curPos, unsigned short targetPos){
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
}
static unsigned short cnt = 0;
static unsigned short cnt2 = 0;
unsigned long accelerate = 0;
short deg90,deg180 = 0;
char step = 0;
char step2 = 0;
short curPos = 0; //the current position of the paddles
short targetPos = 0; // the position that the paddle need to move to 
char input = 0;

unsigned short greenPos = 0;
unsigned short redPos = 30;
unsigned short bluePos = 60;
unsigned short purpPos = 90;


void oneStep(char stepDir){
    PORTA = SetBit(PORTA,1,stepDir); // set the direction that the motors are moving 
    PORTA = SetBit(PORTA,3,stepDir);
    step = ~step;
    PORTA = SetBit(PORTA,0,step); // move each motor one step
    PORTA = SetBit(PORTA,2,step);
    
}

void StepTick1(){
	//Actions
	switch(Stepper_1){
		case INIT:
			step = 0;
			accelerate = 500;
		break;
		case WAIT:
			//ReadInput();
		break;
		case WAIT_REL:

		break;
		case Deg90:
			
		break;		
		case Deg180:
			
		break;
		default:
			step = 0;
		break;
	}
	//Transitions
	switch(Stepper_1){
		case INIT:
			Stepper_1 = WAIT;
		break;
		case WAIT:
			//ReadInput();
			if (!B0 && !B1)	{
				Stepper_1 = WAIT_REL;
			}
		break;
		case WAIT_REL:
			if (B0)	{
				Stepper_1 = Deg90;
				dir = B2;
			}
			else if (B1) {
				Stepper_1 = Deg180;
				dir = B2;
			}
			else {
				Stepper_1 = WAIT_REL;
			}
		break;
		case Deg90:
			
			if (cnt< deg90){
				step = ~step;
				//SpinUP(accelerate);
				if (accelerate > 0){
	//				accelerate--;
				}
				cnt++;
				Stepper_1 = Deg90;
			}
			else{
				Stepper_1 = WAIT;
				cnt = 0;
			//	accelerate = 500;
			}
		break;
				case Deg180:
				if (cnt<deg180){
					step = ~step;
					cnt++;
					Stepper_1 = Deg180;
				}
				else{
					Stepper_1 = WAIT;
					cnt = 0;
				}
				break;
		default:
			Stepper_1 = INIT;
		break;
	}
}

void StepTick2(){
	//Actions
	switch(Stepper_2){
		case INIT2:
			step2 = 0;
		break;
		case WAIT2:
		
		break;
		case Deg90_2:
		
		break;
		case Deg180_2:
		
		break;
		default:
			step2 = 0;
		break;
	}
	//Transitions
	switch(Stepper_2){
		case INIT2:
			Stepper_2 = WAIT2;
		break;
		case WAIT2:
			ReadInput();
			if (!B3 && !B4)	{
				Stepper_2 = WAIT_REL2;
			}
		break;
		case WAIT_REL2:
			if (B3)	{
				Stepper_2 = Deg90_2;
				dir2 = B5;
			}
			else if (B4) {
				Stepper_2 = Deg180_2;
				dir2 = B5;
			}
			else {
				Stepper_2 = WAIT_REL2;
			}
		break;
		case Deg90_2:		
			if (cnt2 < deg90){
				step2 = ~step2;
				cnt2++;
				Stepper_2 = Deg90_2;
			}
			else{
				Stepper_2 = WAIT2;
				cnt2 = 0;
			}
		break;
		case Deg180_2:
			if (cnt2 < deg180){
				step2 = ~step2;
				cnt2++;
				Stepper_2 = Deg180_2;
			}
			else{
				Stepper_2 = WAIT2;
				cnt2 = 0;
			}
		break;
		default:
			Stepper_2 = INIT2;
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
            ReadD();
            if (!D0){
                targetPos = greenPos;
                Sort_1 = GREEN;
            }            
            else if(!D1){
                Sort_1 = RED;
                targetPos = RotationDeg(redPos);
            }
            else if(!D2){
                Sort_1 = RED;
                targetPos = RotationDeg(bluePos);
            }
            else if(!D0 && !D2){
                Sort_1 = PURPLE;
                targetPos = RotationDeg(purpPos);
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

void StepperTask1(){	
	Stepper1INIT();
   for(;;){ 	
	StepTick1();
	vTaskDelay(5);
	
	PORTA = SetBit(PORTA,0,step); 
	PORTA = SetBit(PORTA,1,dir); 
	ReadInput();
		
   } 
}

void StepperTask2(){
	Stepper2INIT();
	for(;;){
		StepTick2();
		vTaskDelay(5);
		
		PORTA = SetBit(PORTA,2,step2);
		PORTA = SetBit(PORTA,3,dir2);
		ReadInput();
		
	}
}


void SortTask(){
    SortINIT();
    for(;;){
        SortTick();
        vTaskDelay(5);
        
    }
}
void StartStepPulse1(unsigned portBASE_TYPE Priority){
xTaskCreate(StepperTask1, (signed portCHAR *)"StepperTask1", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void StartStepPulse2(unsigned portBASE_TYPE Priority){
	xTaskCreate(StepperTask2, (signed portCHAR *)"StepperTask2", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void StartSortPulse(unsigned portBASE_TYPE Priority){
    xTaskCreate(SortTask, (signed portCHAR *)"SortTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}
 
int main(void) { 
	DDRA = 0xFF; PORTA=0x00;
	DDRB = 0x00; PORTB=0xFF;
	DDRD = 0x00; PORTD=0xFF;


	deg90 = RotationDeg(90);
	deg180 = RotationDeg(180);

	//Start Tasks  
	//StartStepPulse1(1);
	//StartStepPulse2(1);
	StartSortPulse(1);
    //RunSchedular 
	ReadD();
    vTaskStartScheduler(); 
	return 0; 
}
