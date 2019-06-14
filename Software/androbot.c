/*
* @file androbot.c
* @author Atharva Mahindrakar
* @author Prasanna Kulkarni
* @Surya Ravikumar
* @author Kanna Lakshamanan
* @copyright Portland State University 2019
*   
*   The software application for this project is developed on Xilinx’s Standalone OS board support package. 
*   This software application is responsible for polling the Firebase data base which is done with the help 
*   of ESP8266 NodeMCU . An UART channel is established between both to send Firebase data from ESP8266 NodeMCU  
*   to Nexys4 board and to get heading data in the opposite direction. Depending upon the mode of operation the 
*   bot is either driven by the directions given by the user from mobile app or it runs in automated obstacle detection mode. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "microblaze_sleep.h"
#include "nexys4IO.h"
#include "xintc.h"
#include "stdbool.h"
#include "math.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xtmrctr.h"
#include "Pmod_HB3.h"
#include "xuartlite.h"
#include "xgpio.h"

/*
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
 */



// AXI timer parameters 1 : Nexys4IO
#define AXI_TIMER1_DEVICE_ID	XPAR_AXI_TIMER_1_DEVICE_ID
#define AXI_TIMER1_BASEADDR		XPAR_AXI_TIMER_1_BASEADDR
#define AXI_TIMER1_HIGHADDR		XPAR_AXI_TIMER_1_HIGHADDR
#define TmrCtrNumber1			0

// Definitions for peripheral NEXYS4IO
#define NX4IO_DEVICE_ID		XPAR_NEXYS4IO_0_DEVICE_ID
#define NX4IO_BASEADDR		XPAR_NEXYS4IO_0_S00_AXI_BASEADDR
#define NX4IO_HIGHADDR		XPAR_NEXYS4IO_0_S00_AXI_HIGHADDR


//UARTLite
#define UARTLITE_DEVICE_ID	XPAR_AXI_UARTLITE_1_DEVICE_ID
#define TEST_BUFFER_SIZE    	784
#define TEST_BUFFER_SIZE_1    	10
#define TEST_BUFFER_SIZE_2    	1

// Definitions for peripheral PMODHB3
#define PMODHB3_0_BASEADDR		0x44A10000              // PMODHB3 for right motor
#define PMODHB3_1_BASEADDR		0x44A20000              // PMODHB3 for left motor

#define INTC_DEVICE_ID				XPAR_INTC_0_DEVICE_ID   // Interruot device ID

// Definitions for peripheral GPIO 0
#define GPIO_0_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_0_INPUT_0_CHANNEL		1



//Create Instances

XTmrCtr		AXITimerInst;               // AXI timer instance
XIntc 		IntrptCtlrInst;				// Interrupt Controller instance
XUartLite   UartLite;                   // UARTlite instance
XGpio		GPIOInst0;					// GPIO instance for GPIO 0

/*
*    The structure is defined to store the Bot's parameters like
*    current ON/OFF switch data, mode of operation and the headings.
*    Defining a structure of these parameters helps access these values
*    throughout the code. 
*/


typedef struct{
	u32 uart_in[2];             // uart's RX data
	u8	on_off_status;          // ON/OFF switch data
	u8	mode_operation;         // mode of operation data
	u8	direction_from_app;     // direction commands received from app when in accelerometer mode
	u8	headding;               // current heading
	u8	prev_heading;           // previous heading
}UART;

UART uart;

/*
*   UART parameters, data and its significance on the operation of the software
*   on_off_status -> if 1 then bot is stopped and turned off
*                    if 2 then botis turned on
*   mode_operation ->  if 1 then in accelerometer mode
*                      if 2 then in obstacle detection and mapping mode
*
*   direction_from_app -> if 1 then stop
*                         if 2 then go forward
*                         if 3 then go reverse
*                         if 4 then turn right
*                         if 5 then turn left
*
*   heading ->  if 1 then current heading is EAST
*               if 2 then current heading is NORTH
*               IF 3 then current heading is WEST
*               IF 4 then current heading is SOUTH
*
*   Now for example if we want to run out bot in acceleromer mode and it should move forward, 
*   then UART data sent from NodeMCU would be like - > 212
*   it is decoded as hundredth place's "2" gives us ON/OFF switch status, for here "2" means bot is turned on
*   the tenth place's "1" denotes mode of operation, here it means bot is in acceleromer mode
*   the unit place's "2" denotes direction of movement, here it is "2" it means move forward. The same algorithm is 
*   followed for rest of the directions too.
*
*   Now for Obstacle detection mode, the hundredth and tenth place's data is interpreted the same way as explained above.
*   for example, we would receive data -> 22x from node MCU. 
*   But here any directional data sent from NOdeMCU which is at unit's place is discarded. And after each iteration of the 
*   program when in obstacle detection mode the current heading is sent to NodeMCU via UART. It is further interpreted 
*   in NOdeMCU's code for updating the next X, Y cordinates of the Bot's movement.
*/                      



void androbot();                                // Projects iterative function
void reset_bot();                               // resets structure
void Decode_uart();                             // decodes the UART data as explained in above comment section
void accelerometer_mode();                      // accelerometer's mode function
void obstacle_detection_mode();                 // obstacle detecction's mode function
int calculate_heading(int heading,int turn);    // function to calculate next heading when in obstacle detection mode
u8 obstacle[10];
u8 heading[5];
u8 count = 0;



int main(void)
{

	uint32_t sts;

	sts = do_init();                // initialises the  HW
	if (XST_SUCCESS != sts)
	{
		exit(1);
	}


	reset_bot();                    // resets the Bot's parameters to default values
	heading[0] = uart.headding;

    androbot();                     // starts the bot 

	

	return 0;
}


/*
*   This is a iterative function. In this function, UART's data is read by polling it at every iteration.
*   Then decode function is called to decode the data and after this, the proper function having correct 
*   mode of operation is called.
*/

void androbot()
{
    u8 data[10];
	u8 datarcv[2];
	u16 gpio_in = 0;

    while(1)
	{
		int rcv = XUartLite_Recv(&UartLite,datarcv,1);          // receives uart data into "datarcv buffer"
                                                                // the function return 1 or 0 depending upon sucessful transaction
        uart.uart_in[0] = datarcv[0];                           // set the first byte of data to our bots uart data
		
        
        xil_printf(" uart rcvd - %d\n ", uart.uart_in[0]);      

		Decode_uart();                                          // function call to decode received uart data

		if(uart.on_off_status == 1)                             // if hundredth place is one, that is bot is off
			reset_bot();                                        // reset bot's parameters and stop.

		else
		{
			if(uart.mode_operation == 1)                        // if mode of operation is 1
			{                                                   // then mode operation is accelerometer mode 
				xil_printf("in accelerometer mode\n\n");
				accelerometer_mode();                           // call to acceleromer function    
			}

			else if(uart.mode_operation == 2)                   // if mode of operation is 2
			{
				xil_printf("in obstacle detection mode\n\n");   // then mode operation is obstacle detection mode
				obstacle_detection_mode();                      // call to obstacle detection function
			}
		}

        XUartLite_ResetFifos(&UartLite);                        // reset uart fifo

	}
}


/*
*   In this function mode of  operation and heaing are given default values
*/

void reset_bot()
{
	uart.mode_operation = 0;                // default value 0 is given, because this number is not assigned to any operation
	uart.headding = 2;                      // default heading is reset to NORTH, hence 2 is assigned
	uart.prev_heading = 2;                  // prev. heading is also set to NORTH

	xil_printf("BOT RESET \n\n");
}


/*
*   This function is used to decode the single byte of data received from UART.
*   The decoded data is then assigned to each parameter of our bot's struct defined above
*/

void Decode_uart()
{
	int input = uart.uart_in[0];                    // load the byte of data in temperory variable

	uart.on_off_status = input / 100;               // extract the 100th place data
                                                    // which if on/off switch data in app
	input = input % 100;                            // remove 100th place digit anf get 2 digit number

	uart.mode_operation = input / 10;               // extract the 10th place data
                                                    // which is mode of operation
	input = input % 10;                             
                                                    // remove 10th place digit anf get single digit numbe
                                                    // which in turn is a direction data from app
	if(uart.mode_operation == 1)                    // if mode of operation is 1, i.e., in accelerometer mode
	{
		uart.direction_from_app = input;            // then only assign unit's place data to direction parameter of the bot
                                                    // otherwise discard
	}
}

/*
*   In this function a switch case is designed which is used to assigned 
*   direction and the speed to motors depending upon the direction given by
*   accelerometers from app.
*   The PWM signal generation module is given a 11 bit count for generating PWM.
*   Hence 2048 would be the 100 % PWM.
*/


void accelerometer_mode()
{
	u8 direction = uart.direction_from_app;
	u16 fwd_speed = 2000;                       // FWD PWM 
	u16 bwd_speed = 2000;                       // reverse PWM
	u16 stop_speed = 0;                         // stop PWM
	u16 turn_speed = 1500;                      // Turn PWM
	u16 ir_input = 0;                           

	ir_input = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);      //reads IR sensor array 

	switch(direction)
	{
	case 1:
		xil_printf("\nstop\n");                                         // if direction is stop
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, stop_speed);         // assign speed 0 to both motors
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, stop_speed);
		break;

	case 2:                                                             // if directiongiven is FWD
		xil_printf("\nforward\n");
		if(ir_input == 255)                                             // if no IR sensor is triggered
		{                                                               // i.e., no obstacle detected
			obstacle[0] = 2;
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 1);               // motor direction is set for 
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, fwd_speed);      // FWD movement and PWM is assigned

			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, fwd_speed);

			/*if(count == 50)
			{
				int a = XUartLite_Send(&UartLite, obstacle, 1);
				count = 0;
			}*/
		}

		else                                                             // if obstacle is detected
		{
			obstacle[0] = 5;
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, stop_speed);     // stop
			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, stop_speed);
			/*if(count == 50)
			{
				int a = XUartLite_Send(&UartLite, obstacle, 1);
				count = 0;
			}*/
			xil_printf("obstacle detected\n\n");
		}

		break;

	case 3:
		xil_printf("\nreverse\n");
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 0);                   // if direction is reverse
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, bwd_speed);          // set motor direction for reverse movement

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 0);                   // assign PWM
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, bwd_speed);
		break;

	case 4:
		xil_printf("\nturn right\n");                                   // if direction is turn right
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 1);
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, turn_speed);         // set motor direction to turn right

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 0);                   // assign pwm to both motors
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, turn_speed);
		break;
	case 5:                                                             // if direction is turn left
		xil_printf("\nturn left\n");
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 0);                   // set motor direction to turn left
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, turn_speed);         // assign pwm to both motors

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, turn_speed);
		break;

	default:
		xil_printf("invalid entry\n\n");
	}

	//count++;
}

/*
*   In this function, depending upon the IR sensor's input, movement is controlled. If the obstacle is detected 
*   then, depending upon which IR sensors are getting triggered, 90 degree turn is performed and new heading is 
*   calculated. Otherwise bot keeps on moving in forward direction keeping the heading same. 
*   
*   The functioning of IR sensor -> when triggered digital output given is 0
*                                   when not triggered gives digital output as 1
*/

void obstacle_detection_mode()
{
	u8 direction = uart.direction_from_app;
	u16 fwd_speed = 2000;
	u16 stop_speed = 0;
	u16 ir_input = 0;
	u8	ir_front = 0;
	u8	ir_right = 0;
	u8	ir_left	 = 0;
	u8	stop_bit = 0;
	int i = 0;


	ir_input = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);       // read IR sensor array input

	ir_front = (ir_input & 0x1);                                             // extract front IR sensor data

	ir_right = (ir_input & 0x0002)>>1;                                       // extract right IR sensor data
    
	ir_left	 = (ir_input & 0x0004)>>2;                                       // extract left IR sensor data

	xil_printf(" %d		%d		%d \n\n", ir_front, ir_right, ir_left);

	if(ir_front & ir_right & ir_left)								// moving forward, i.e., obstacle not detected
	{
		//move forward
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 1);                   // give motors FWD direction
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, fwd_speed);          // Give PWM to motors

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, fwd_speed);
		xil_printf("moving forward***************\n\n");
		uart.prev_heading = uart.headding;                              // set prev heading to current heading
		stop_bit = 0;
	}

	else                                                                // if obstacle detected
	{
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, 0);                  // stop the bot
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, 0);

		for(i = 0; i<20; i++)                                           // delay the stop
		{
			delay_1();
		}


		ir_input = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);  // again read the IR sensor array data

		ir_front = (ir_input & 0x1);                                        // extract front IR sensor data

	    ir_right = (ir_input & 0x0002)>>1;                                  // extract right IR sensor data
    
	    ir_left	 = (ir_input & 0x0004)>>2;                                  // extract left IR sensor data
		
        /* IR sensor data is read again just to ensure we are not missing any obstacle while moving*/

		for(i = 0; i<20; i++)
		{
			delay_1();
		}



		if(ir_right)                                                    // if right sensor is not triggered
		{
			//turn right
			turn_right();                                               // turn right by 90 degrees
			uart.headding = calculate_heading(uart.prev_heading, 1);    // calculate new heading
			xil_printf("turning right//////////////////\n\n");
			stop_bit = 0;                                               // clear the stop bit
		}

		else if(ir_right == 0 && ir_left == 1)                          // if right sensor is triggered and
		{                                                               // left  sensor is not triggered  then
			//turn left
			turn_left();                                                // turn left
			uart.headding = calculate_heading(uart.prev_heading, 2);    // calculate new heading
			xil_printf("turning left\\\\\\\\\\\\\\\\\\\ \n\n");
			stop_bit = 0;                                               // clear heading
		}

		else if(ir_right == 0 && ir_left == 0)                          // if all 3 sensors are triggered
		{
			//stop
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, stop_speed);     // stop the motors
			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, stop_speed);
			xil_printf("**********************************************************\n\n");
			stop_bit = 1;                                               // set stop bit
		}
	}

	xil_printf(" 				heading -	%d\n\n	", uart.headding);
	heading[0] = uart.headding;
	obstacle[0] = 0;
	if(stop_bit == 0)                                               // if stop bit is cleared
	{
		int a = XUartLite_Send(&UartLite, heading, 1);              // send current heading to NodeMCU
		//count = 0;
	}
	else                                                            // if stop bit is set
	{
		int a = XUartLite_Send(&UartLite, obstacle, 1);             // send stop condition to NodeMCU, 
		xil_printf("stop\n");                                       // so that it would stop updating the map
	}

}

/*
*  This function is used to turn right the bot by 90 degrees
*/

void turn_right()
{	int i = 0;
	u16 turn_speed = 1500;                                          // set turn speed

	PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 0);                   // set motor direction to turn right
	PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, turn_speed);         // give pwm to both the motors

	PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
	PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, turn_speed);         
	for(i=0 ; i<20;i++)                                             // delay is given such a way that it would turn right by 90 degrees
		delay_1();


}

void turn_left()
{
	int i = 0;
	u16 turn_speed = 1500;                                          // set turn speed

	PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 0);                   // set motor direction to turn left
	PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, turn_speed);         // give pwm to both the motors

	PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
	PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, turn_speed);         
	for(i=0 ; i<20;i++)                                             // delay is given such a way that it would turn left by 90 degrees
		delay_1();


}

/*
*   This function is used to calculate the new headdings. The previous heading and the turn performed 
*   are parsed to this function and it returns new heading
*
*/

int calculate_heading(int current_heading, int turn)
{
	int new_heading;
	int i;
	if(turn == 1)										// if turn is right
	{
		if(current_heading == 1)						// if current heading is east
			new_heading = 4;							// new heading is south
		else if(current_heading == 2)					// if current heading is north
			new_heading = 1;							// new heading is east
		else if(current_heading == 3)					// if current heading is west
			new_heading = 2;							// new heading is north
		else if(current_heading == 4)					// if current heading is south
			new_heading = 3;							// new heading is west
		else
			xil_printf("incorrect_orientation\n\n");
	}

	else if(turn == 2)									// if turn is left
	{
		if(current_heading == 1)						// if current heading is east
			new_heading = 2;							// new heading is north
		else if(current_heading == 2)					// if current heading is north
			new_heading = 3;							// new heading is west
		else if(current_heading == 3)					// if current heading is west
			new_heading = 4;							// new heading is south
		else if(current_heading == 4)					// if current heading is south
			new_heading = 1;							// new heading is east
		else
			xil_printf("incorrect_orientation\n\n");
	}

	/*for(i = 0; i<10000; i++);
	for(i = 0; i<10000; i++);*/
	
    return new_heading;

}


/* standared delay function */
void delay_1()
{
	int i = 0;
	for(i = 0; i<10000; i++);
	for(i = 0; i<10000; i++);

}


/****************************************************************************/
/**
* initialize the system
*
* This function is executed once at start-up and after resets.  It initializes
* the peripherals and registers the interrupt handler(s)
*****************************************************************************/


int	 do_init(void)
{
	uint32_t status, status_1, status_2, status_3 ;				// status from Xilinx Lib calls

	// initialize the Nexys4 driver and (some of)the devices
	status = (uint32_t) NX4IO_initialize(NX4IO_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}


	// set all of the display digits to blanks and turn off
	// the decimal points using the "raw" set functions.
	// These registers are formatted according to the spec
	// and should remain unchanged when written to Nexys4IO...
	// something else to check w/ the debugger when we bring the
	// drivers up for the first time
	NX4IO_SSEG_setSSEG_DATA(SSEGHI, 0x0058E30E);
	NX4IO_SSEG_setSSEG_DATA(SSEGLO, 0x00144116);

	status = XUartLite_Initialize(&UartLite, UARTLITE_DEVICE_ID);

	if (status != XST_SUCCESS)
	{
		xil_printf("Failed");
		return XST_FAILURE;
	}



	status = XUartLite_SelfTest(&UartLite);

	if (status != XST_SUCCESS)
	{
		xil_printf("Failed");
		return XST_FAILURE;
	}

	// gpio initialisation

	status = XGpio_Initialize(&GPIOInst0, GPIO_0_DEVICE_ID);

	if (status != XST_SUCCESS)
	{
		xil_printf("failed\n");
		return XST_FAILURE;
	}
	//XGpio_SetDataDirection(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL, 0xFF);

	status = AXI_Timer1_initialize();
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	xil_printf("\n\n****** AXI timer 1 initialised\n\n****");

}


/*
 * AXI timer initializes it to generate out a 4Khz signal, Which is given to the Nexys4IO module as clock input.
 * DO NOT MODIFY
 */

int AXI_Timer1_initialize(void){

	uint32_t status;    // status from Xilinx Lib calls
	u32		ctlsts;		// control/status register or mask

	status = XTmrCtr_Initialize(&AXITimerInst,AXI_TIMER1_DEVICE_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	status = XTmrCtr_SelfTest(&AXITimerInst, TmrCtrNumber1);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	ctlsts = XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_EXT_GENERATE_MASK | XTC_CSR_LOAD_MASK |XTC_CSR_DOWN_COUNT_MASK ;
	XTmrCtr_SetControlStatusReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1,ctlsts);

	//Set the value that is loaded into the timer counter and cause it to be loaded into the timer counter
	XTmrCtr_SetLoadReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1, 24998);
	XTmrCtr_LoadTimerCounterReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1);
	ctlsts = XTmrCtr_GetControlStatusReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1);
	ctlsts &= (~XTC_CSR_LOAD_MASK);
	XTmrCtr_SetControlStatusReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1, ctlsts);

	ctlsts = XTmrCtr_GetControlStatusReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1);
	ctlsts |= XTC_CSR_ENABLE_TMR_MASK;
	XTmrCtr_SetControlStatusReg(AXI_TIMER1_BASEADDR, TmrCtrNumber1, ctlsts);

	XTmrCtr_Enable(AXI_TIMER1_BASEADDR, TmrCtrNumber1);
	return XST_SUCCESS;
}




