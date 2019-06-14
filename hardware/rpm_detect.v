
`timescale 1ns / 1ps
// rpm_detector.v
// 
// Created By:		Prasanna Kulkarni, Atharva Mahindrakar
// Last Modified:	19-May-2019 (PK)
//
//
// Description:
// ------------
// This is a simple edge detection system used for 
// Closed Loop PID control as par of the PMOD_HB3 IP.


module rpm_detector(clock, reset, sa_input , rpm_output);
    input clock;
    input reset;
    input sa_input;		// The input from the HB3 that will tell us the speed
	output reg[31:0] rpm_output; // The calculated output speed
    
    parameter [31:0] SAMPLING_FREQUENCY = 32'd100000000;
    reg       [31:0] SAMPLING_COUNT     = 32'd0; 		
    reg       [31:0] HIGH_COUNT         = 32'd0;

    wire      [31:0] pe;
    reg       sa_input_neg;
    assign pe = sa_input & ~sa_input_neg;  // The edge detection signal the pe stands for pulse edge
    
    always@(posedge clock)
    begin
      if(~reset)
           begin
           SAMPLING_COUNT<=32'd0; // The sampling count which counts til the "known" interval
           HIGH_COUNT <= 32'd0;   // The count that will be used for the rpm output
           end
      else
           begin
           sa_input_neg <=  sa_input;
           if (SAMPLING_COUNT<=SAMPLING_FREQUENCY)
            begin
                if (pe)
                       begin
                       HIGH_COUNT<=HIGH_COUNT+1'd1; // The high count is only incremented when pe is at high
                end
                else begin
                        HIGH_COUNT<=HIGH_COUNT;
                end
                SAMPLING_COUNT<=SAMPLING_COUNT + 1'b1; // The sampling count is incremented every clock cycle
                rpm_output <= rpm_output ;
            end
            else begin
                rpm_output <= HIGH_COUNT * 5; 
                HIGH_COUNT <= 32'b0;
                SAMPLING_COUNT <= 32'b0;
            end
           end
    end 
    
    
endmodule
