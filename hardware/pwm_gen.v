`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/15/2019 11:40:13 PM
// Design Name: 
// Module Name: pwm_gen
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/15/2019 10:24:00 PM
// Design Name: 
// Module Name: pwm_generator
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module pwm_generator(
    input clk,
    input rst,
    input [10:0] pwm_count,
    output reg pwm
    );
reg [10:0] pwm_counter;
 
always@(posedge clk )
begin
    if (!rst) begin
        pwm_counter <= 10'b0;
    end
    
    else begin
       pwm_counter <= pwm_counter + 1; 
    end
end

always @ (*) begin
  if (!rst) begin
    pwm = 1'b0;
  end
  else begin 
    if (pwm_counter <  pwm_count) begin
        pwm = 1'b1;
    end
    
    else begin
        pwm = 1'b0;
    end
end
end
endmodule
