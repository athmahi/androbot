

`timescale 1ns / 1ps



module rpm_detector(clock, reset, sa_input , rpm_output);
    input clock;
    input reset;
    input sa_input;
    output reg[31:0] rpm_output;
    
    parameter [31:0] SAMPLING_FREQUENCY = 32'd100000000;// Sampling frequency
    reg       [31:0] SAMPLING_COUNT     = 32'd0; 		// Sampling counter
	reg 	  [31:0] HIGH_COUNT         = 32'd0;
	reg 	  [31:0] RPM                = 32'd0;	
	reg              R_FLAG              = 1'd0;
	
always@(posedge clock)
begin
  if(~reset|| (SAMPLING_FREQUENCY==SAMPLING_COUNT))
	   begin
	   SAMPLING_COUNT<=32'd0;
	   R_FLAG<=1'd1;
	   end
  else
	   begin
	   SAMPLING_COUNT<=SAMPLING_COUNT+1'd1;
	   R_FLAG<=1'd0;
	   end
end 

always@(posedge sa_input or posedge R_FLAG)
begin
	if(R_FLAG)
	   begin
	   HIGH_COUNT<= 32'd0;
	   end
	else
	   begin
	   HIGH_COUNT<=HIGH_COUNT+1'd1;
	   end
    end	
always@(*)
begin
    if((SAMPLING_FREQUENCY-1'd1)==SAMPLING_COUNT)
        begin
        rpm_output<=HIGH_COUNT*5;
    end
end
endmodule

