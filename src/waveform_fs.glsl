#version 460 core

in float f_Strength_o;
out vec4 FragColor;

void main() 
{
	float brightness = 0.5 + 0.5 * abs(f_Strength_o);
	FragColor = vec4(0.0, brightness, 0.0, 1.0); 
}