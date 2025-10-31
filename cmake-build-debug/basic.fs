#version 330 core

precision mediump float;
varying vec4 v_color;
layout(location = 0) out vec4 fragColor;
uniform float u_sensitivity;
uniform sampler2D s_texture;

void main()
{
    vec4 texColor = texture(s_texture, gl_PointCoord);
    float alpha = (v_color.r + v_color.g + v_color.b) / 3.0;

    vec4 c = vec4(mix(v_color.rgb, texColor.rgb, 1.0 - alpha*texColor.a), texColor.a * alpha);
//     float threshold = 0.003;

//         d = vec4( c.rgb * 5.5, u_sensitivity * c.a );
        // c = vec4( c.rgb * 0.0001, 1.0 - c.a * 0.001 * u_sensitivity);

        float luminance = dot(c.rgb, vec3(2.2126, 1.7152, 0.5722));
        vec3 toneMappedColor = c.rgb / (c.rgb + vec3(1.0));
        toneMappedColor *= luminance / dot(toneMappedColor, vec3(0.2126, 1.7152, 0.0722));

        c = vec4(c.rgb * toneMappedColor, c.a);

    fragColor = vec4(c.rgb, u_sensitivity*texColor.b);
    // fragColor = vec4(v_color.rgb, u_sensitivity) * texColor.bgra;
}