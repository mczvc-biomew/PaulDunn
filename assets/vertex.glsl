#version 330 core

attribute vec4 position;
attribute vec4 color;
varying vec4 v_color;

void main() {
    gl_Position = vec4(position.xyz, 1.0);
    gl_PointSize = 64.0;
    v_color = color;
}
