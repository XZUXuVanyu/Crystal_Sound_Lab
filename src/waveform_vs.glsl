#version 460 core

layout (location = 0) in float f_Strength;

// layout control
uniform float uf_x_aspect;
uniform float uf_x_offset;

uniform float uf_y_aspect;
uniform float uf_y_offset;

uniform int ui_point_count_pf;
uniform float uf_gain;

out float f_Strength_o;

void main() 
{
    //calculate x screen coord
    float x = float(gl_VertexID) / float(ui_point_count_pf - 1);
    x = x * 2.0 - 1.0;
    x *= uf_x_aspect; 
    x += uf_x_offset;
    
    //calculate y screen coord
    float y = f_Strength;
    y *= uf_gain * uf_y_aspect;
    y += uf_y_offset;

    gl_Position = vec4(x, y, 0.0, 1.0);
    f_Strength_o = f_Strength;
}