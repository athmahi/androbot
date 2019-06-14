`timescale 1ns / 1ps
// pwm_gen.v
// 
// Created By:		Prasanna Kulkarni, Atharva Mahindrakar
// Last Modified:	19-May-2019 (PK)
//
//
// Description:
// ------------
// This is a simple pwm generation system used for 
// Closed Loop PID control as part of the PMOD_HB3 IP



module pwm_generator(
    input clk,
    input rst,
    input [10:0] pwm_count,
    output reg pwm
    );
    reg [10:0] pwm_counter; // Counter used to count till a particular value
 
always@(posedge clk )
begin
    if (!rst) begin
        pwm_counter <= 10'b0; 
    end
    
    else begin
       pwm_counter <= pwm_counter + 1;  // Incremented on every clock cycle
    end
end

always @ (*) begin
  if (!rst) begin
    pwm = 1'b0;
  end
  else begin 
    if (pwm_counter <  pwm_count) begin
        pwm = 1'b1; // output pwm is given high when the counter is lower than the pwm count
    end
    
    else begin
        pwm = 1'b0; // ouput pwm zero otherwise
    end
end
end
endmodule
