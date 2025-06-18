#version 330 core

precision mediump float;
varying vec4 v_color;
layout(location = 0) out vec4 fragColor;
uniform float sensitivity;
uniform sampler2D u_texture;

void main() {
    vec4 texColor = texture2D(u_texture, gl_PointCoord);

    float alpha = (v_color.r + v_color.g + v_color.b) / 3.0;

    vec4 c = vec4(v_color.rgb, 1.0 - alpha) * vec4(texColor.rgb, texColor.a);
    vec4 d = c;


    if (true) {
      float threshold = 0.003;

      if ( ((c.r + c.g + c.b + c.a)/4.0)  > threshold) {
        d = vec4( c.rgb * 5.5, sensitivity * c.a );
        // c = vec4( c.rgb * 0.0001, 1.0 - c.a * 0.001 * sensitivity);

        float luminance = dot(c.rgb, vec3(1.2126, 1.7152, 1.0722));
        vec3 toneMappedColor = c.rgb / (c.rgb + vec3(1.0));
        toneMappedColor *= luminance / dot(toneMappedColor, vec3(0.2126, 1.7152, 0.0722));

        c = vec4(c.rgb * toneMappedColor, c.a);
      }
    }

     fragColor = vec4( c.rgb * d.rgb, 1.0 - d.a);
}
