// pid_embsys_wrapper.v - Top level module for Embedded System
// 
// Created By:	Prasanna Kulkarni 
// Date:	18-April-2019
// Version:	1.0
//
// Description:
// ------------
/*
    The objective for this implementation is to create a SoC system 
    on the Nexys4 DDR FPGA, that can drive a robot wirelessly from a app on an Android phone. 
    Specifically, the idea is to use the ESP8266 NodeMCU to read the Firebase database 
    and drive a bot according to the elements in the database and the mode of operation.
//////////////////////////////////////////////////////////////////////
*/
`timescale 1 ps / 1 ps

module pid_embsys_wrapper
   (
    an_0,
    // Nexys4 Input Push Buttons
    ////////////////////////////
    btnC_0,                     
    btnD_0,
    btnL_0,
    btnR_0,
    btnU_0,
    ////////////////////////////
    // Nexys4 DDR2 Memory 
    ////////////////////////////
    ddr2_sdram_addr,
    ddr2_sdram_ba,
    ddr2_sdram_cas_n,
    ddr2_sdram_ck_n,
    ddr2_sdram_ck_p,
    ddr2_sdram_cke,
    ddr2_sdram_cs_n,
    ddr2_sdram_dm,
    ddr2_sdram_dq,
    ddr2_sdram_dqs_n,
    ddr2_sdram_dqs_p,
    ddr2_sdram_odt,
    ddr2_sdram_ras_n,
    ddr2_sdram_we_n,
    ////////////////////////////
    dp_0,
    led_0,
    reset,
    seg_0,
    sw_0,
    sys_clock,
    // Debug UART Signals 
    ////////////////////////////
    usb_uart_rxd,
    usb_uart_txd,
    ////////////////////////////
    // PMOD Conector Headers
    ////////////////////////////
    JA,
    JB,
    JC,
    JD
    ////////////////////////////
    );
  wire [15:0]GPIO_0_tri_i;
  output [7:0]an_0;
  input btnC_0;
  input btnD_0;
  input btnL_0;
  input btnR_0;
  input btnU_0;
  output [12:0]ddr2_sdram_addr;
  output [2:0]ddr2_sdram_ba;
  output ddr2_sdram_cas_n;
  output [0:0]ddr2_sdram_ck_n;
  output [0:0]ddr2_sdram_ck_p;
  output [0:0]ddr2_sdram_cke;
  output [0:0]ddr2_sdram_cs_n;
  output [1:0]ddr2_sdram_dm;
  inout [15:0]ddr2_sdram_dq;
  inout [1:0]ddr2_sdram_dqs_n;
  inout [1:0]ddr2_sdram_dqs_p;
  output [0:0]ddr2_sdram_odt;
  output ddr2_sdram_ras_n;
  output ddr2_sdram_we_n;
  wire dir_0;
  wire dir_1;
  output dp_0;
  output [15:0]led_0;
  wire [31:0]pwm_0;
  wire [31:0]pwm_1;
  input reset;
  wire sa_input_0;
  wire sa_input_1;
  output [6:0]seg_0;
  input [15:0]sw_0;
  input sys_clock;
  wire usb_uart_1_rxd;
  wire usb_uart_1_txd;
  input usb_uart_rxd;
  output usb_uart_txd;
  inout [7:0] JA,JB,JC,JD;

  wire [7:0]an_0;
  wire btnC_0;
  wire btnD_0;
  wire btnL_0;
  wire btnR_0;
  wire btnU_0;
  wire [12:0]ddr2_sdram_addr;
  wire [2:0]ddr2_sdram_ba;
  wire ddr2_sdram_cas_n;
  wire [0:0]ddr2_sdram_ck_n;
  wire [0:0]ddr2_sdram_ck_p;
  wire [0:0]ddr2_sdram_cke;
  wire [0:0]ddr2_sdram_cs_n;
  wire [1:0]ddr2_sdram_dm;
  wire [15:0]ddr2_sdram_dq;
  wire [1:0]ddr2_sdram_dqs_n;
  wire [1:0]ddr2_sdram_dqs_p;
  wire [0:0]ddr2_sdram_odt;
  wire ddr2_sdram_ras_n;
  wire ddr2_sdram_we_n;
  wire dp_0;
  wire [15:0]led_0;
  wire reset;
  wire [6:0]seg_0;
  wire [15:0]sw_0;
  wire sys_clock;
  wire usb_uart_rxd;
  wire usb_uart_txd;

  ////////// UART ////////////////
  // UART Connections are made here. The JC pin is considered
  // as an inout allowing the same header to be used for receive and
  // transmit pins
  ////////////////////////////
  assign  usb_uart_1_rxd=JC[0]; 
  assign JC[1]=usb_uart_1_txd;
  ////////////////////////////
  
  //////////PMOD HB3_0///////////////
  // These are the signals for the PMOD HB3 drivers
  // The sa input is not used but is still assigned 
  // to prevent floasting errors,
  ////////////////////////////
  assign JB[0]=dir_0;                       
  assign JB[1]=pwm_0;                                
  assign sa_input_0=JB[2];
  //////////////////////////// 
  //////////////////////////// 
  assign JA[0]=dir_1;                       
  assign JA[1]=pwm_1;                                
  assign sa_input_1=JA[2];
  //////////////////////////// 
  //////////////////////////// 
  
  ////////// GPIO //////////////
  //This is the GPIO connection that we have used
  // interface the IR LEDs.  
  //////////////////////////// 
  assign GPIO_0_tri_i = {8'b0, JD};
  //////////////////////////// 
  
  pid_embsys pid_embsys_i
       (.GPIO_0_tri_i(GPIO_0_tri_i),
        .an_0(an_0),
        .btnC_0(btnC_0),
        .btnD_0(btnD_0),
        .btnL_0(btnL_0),
        .btnR_0(btnR_0),
        .btnU_0(btnU_0),
        .ddr2_sdram_addr(ddr2_sdram_addr),
        .ddr2_sdram_ba(ddr2_sdram_ba),
        .ddr2_sdram_cas_n(ddr2_sdram_cas_n),
        .ddr2_sdram_ck_n(ddr2_sdram_ck_n),
        .ddr2_sdram_ck_p(ddr2_sdram_ck_p),
        .ddr2_sdram_cke(ddr2_sdram_cke),
        .ddr2_sdram_cs_n(ddr2_sdram_cs_n),
        .ddr2_sdram_dm(ddr2_sdram_dm),
        .ddr2_sdram_dq(ddr2_sdram_dq),
        .ddr2_sdram_dqs_n(ddr2_sdram_dqs_n),
        .ddr2_sdram_dqs_p(ddr2_sdram_dqs_p),
        .ddr2_sdram_odt(ddr2_sdram_odt),
        .ddr2_sdram_ras_n(ddr2_sdram_ras_n),
        .ddr2_sdram_we_n(ddr2_sdram_we_n),
        .dir_0(dir_0),
        .dir_1(dir_1),
        .dp_0(dp_0),
        .led_0(led_0),
        .pwm_0(pwm_0),
        .pwm_1(pwm_1),
        .reset(reset),
        .sa_input_0(sa_input_0),
        .sa_input_1(sa_input_1),
        .seg_0(seg_0),
        .sw_0(sw_0),
        .sys_clock(sys_clock),
        .usb_uart_1_rxd(usb_uart_1_rxd),
        .usb_uart_1_txd(usb_uart_1_txd),
        .usb_uart_rxd(usb_uart_rxd),
        .usb_uart_txd(usb_uart_txd));
endmodule
