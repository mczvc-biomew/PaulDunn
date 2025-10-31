#version 330 core
attribute vec4 a_data;

attribute int a_id;
uniform float u_angle;
varying vec4 v_color;

vec3 hsv(float h) {
    int i = int(h*6.);
    float f = fract(h*6.);
    float p = 1. - f;
    int v = int(mod(float(i), 6.));
    
    if (v == 0)
        return vec3(1., f, 0.);
    else if (v == 1)
        return vec3(p, 1., 0.);
    else if (v == 2)
        return vec3(0., 1., f);
    else if (v == 3)
        return vec3(0., p, 1.);
    else if (v == 4)
        return vec3(f, 0., 1.);
    else if (v == 5)
        return vec3(1., 0., p);
}


void main()
{
    float dist = distance(a_data.zw, a_data.xy);
    float colorAngle = cos(cos(a_id) - u_angle);
    float distinv = 1./(dist);
    float r =  (1.0 - dist) * normalize(dist) * dist * distinv;
    vec3 chsv = hsv(r);
    v_color = vec4(mix(chsv, hsv(colorAngle), 1.0 - normalize(r*dist)*colorAngle), 1.0);
    gl_Position = vec4(a_data.xy, 1.0, 1.0);
}