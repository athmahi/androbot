//@author




/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
/* Std includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/***************************** Include Files *********************************/
/* Project Two includes. */
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

#define mainQUEUE_LENGTH					( 1 )

/* A block time of 0 simply means, "don't block". */
#define mainDONT_BLOCK						( portTickType ) 0


// AXI timer parameters 1 : Nexys4IO
#define AXI_TIMER1_DEVICE_ID	XPAR_AXI_TIMER_1_DEVICE_ID
#define AXI_TIMER1_BASEADDR		XPAR_AXI_TIMER_1_BASEADDR
#define AXI_TIMER1_HIGHADDR		XPAR_AXI_TIMER_1_HIGHADDR
#define TmrCtrNumber1			0

// Definitions for peripheral NEXYS4IO
#define NX4IO_DEVICE_ID		XPAR_NEXYS4IO_0_DEVICE_ID
#define NX4IO_BASEADDR		XPAR_NEXYS4IO_0_S00_AXI_BASEADDR
#define NX4IO_HIGHADDR		XPAR_NEXYS4IO_0_S00_AXI_HIGHADDR

// Definitions for peripheral PMODOLEDRGB
#define RGBDSPLY_DEVICE_ID		XPAR_PMODOLEDRGB_0_DEVICE_ID
#define RGBDSPLY_GPIO_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_BASEADDR
#define RGBDSPLY_GPIO_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_HIGHADD
#define RGBDSPLY_SPI_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_BASEADDR
#define RGBDSPLY_SPI_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_HIGHADDR


//UARTLite

#define UARTLITE_DEVICE_ID	XPAR_AXI_UARTLITE_1_DEVICE_ID

#define TEST_BUFFER_SIZE    	784

#define TEST_BUFFER_SIZE_1    	10

#define TEST_BUFFER_SIZE_2    	1

// Definitions for peripheral PMODHB3
#define PMODHB3_0_BASEADDR		0x44A10000
#define PMODHB3_1_BASEADDR		0x44A20000

#define INTC_DEVICE_ID				XPAR_INTC_0_DEVICE_ID

// Definitions for peripheral GPIO 0
#define GPIO_0_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_0_INPUT_0_CHANNEL		1



//Create Instances
//Function Declarations

//Declare a Sempahore

XTmrCtr		AXITimerInst;
XIntc 		IntrptCtlrInst;				// Interrupt Controller instance
XUartLite   UartLite;
XGpio		GPIOInst0;					// GPIO instance for GPIO 0

typedef struct{
	u32 uart_in[2];
	u8	on_off_status;
	u8	mode_operation;
	u8	direction_from_app;
	u8	headding;
	u8	prev_heading;
}UART;

UART uart;


void reset_bot();
void Decode_uart();
void accelerometer_mode();

u8 obstacle[10];


int main(void)
{

	uint32_t sts;

	u8 data[10];
	u8 datarcv[2];
	u16 gpio_in = 0;


	sts = do_init();
	if (XST_SUCCESS != sts)
	{
		exit(1);
	}

	xil_printf("Hello from FreeRTOS Example\r\n");


	xil_printf(" uart test\n");
	data[0] = 1;

	//datarcv[0] = 1;
	obstacle[0] = 55;
	while(1)
	{
		int rcv = XUartLite_Recv(&UartLite,datarcv,1);
		uart.uart_in[0] = datarcv[0];
		xil_printf(" uart rcvd - %d\n ", uart.uart_in[0]);



		Decode_uart();

		if(uart.on_off_status == 1)
			reset_bot();

		else
		{
			if(uart.mode_operation == 1)
			{
				xil_printf("in accelerometer mode\n\n");
				accelerometer_mode();
			}

			else if(uart.mode_operation == 2)
			{
				xil_printf("in obstacle detection mode\n\n");
				//obstacle_detection_mode();
			}
		}

		//XUartLite_ResetFifos2(&UartLite);


	}

	return 0;
}


void reset_bot()
{
	uart.mode_operation = 0;
	uart.headding = 2;
	uart.prev_heading = 2;

	xil_printf("BOT RESET \n\n");
}


void Decode_uart()
{
	int input = uart.uart_in[0];

	uart.on_off_status = input / 100;

	input = input % 100;

	uart.mode_operation = input / 10;

	input = input % 10;

	if(uart.mode_operation == 1)
	{
		uart.direction_from_app = input;
	}
}


void accelerometer_mode()
{
	u8 direction = uart.direction_from_app;
	u16 fwd_speed = 2000;
	u16 bwd_speed = 2000;
	u16 stop_speed = 0;
	u16 turn_speed = 1500;
	u16 ir_input = 0;

	ir_input = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);

	switch(direction)
	{
	case 1:
		xil_printf("\nstop\n");
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, stop_speed);
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, stop_speed);
		break;

	case 2:
		xil_printf("\nforward\n");
		if(ir_input == 255)
		{
			obstacle[0] = 0;
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 1);
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, fwd_speed);

			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, fwd_speed);
			int a = XUartLite_Send(&UartLite, obstacle, 1);
		}

		else
		{
			obstacle[0] = 55;
			PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, stop_speed);
			PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, stop_speed);
			int a = XUartLite_Send(&UartLite, obstacle, 1);
			xil_printf("obstacle detected\n\n");
		}

		break;

	case 3:
		xil_printf("\nreverse\n");
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 0);
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, bwd_speed);

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 0);
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, bwd_speed);
		break;

	case 4:
		xil_printf("\nturn right\n");
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 1);
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, turn_speed);

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 0);
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, turn_speed);
		break;
	case 5:
		xil_printf("\nturn left\n");
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 4, 0);
		PMOD_HB3_mWriteReg(PMODHB3_0_BASEADDR, 12, turn_speed);

		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 4, 1);
		PMOD_HB3_mWriteReg(PMODHB3_1_BASEADDR, 12, turn_speed);
		break;

	default:
		xil_printf("invalid entry\n\n");
	}
}




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

	/*status = AXI_Timer1_initialize();
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}*/

	//xil_printf("\n\n****** AXI timer 1 initialised\n\n****");

}

/*
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
}*/






