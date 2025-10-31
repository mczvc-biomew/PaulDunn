#version 330 core
attribute vec4 a_position;
attribute vec4 a_color;
varying vec4 v_color;
void main()
{
    gl_Position = vec4(a_position.xyz, 1.0);
    gl_PointSize = 64.0;
    v_color = a_color;
}