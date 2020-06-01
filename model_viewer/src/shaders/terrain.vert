// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoordinates;
layout(location = 3) in float a_height;

out vec3 v_normal;
out vec3 v_light;
out vec3 v_view;

/*out varyingData {
    vec3 normal;
    vec3 light;
    vec3 view;
    vec3 color;
    vec2 texCoordinates;
}v_data;*/


uniform mat4 u_mvp;
uniform mat4 u_mv;
uniform vec3 u_light_pos;
uniform float seed;

out vec3 v_color;
out vec2 v_texCoordinates;

// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * seed);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

void main()
{
	v_normal = a_normal;
	v_light = normalize(u_light_pos - a_position.xyz);
	v_view = normalize(vec3(0,0,3) - a_position.xyz);

    gl_Position = u_mvp * (a_position + vec4(0.0, a_height, 0.0, 0.0));
	v_color = vec3(a_height,0.0,0.0);
	v_texCoordinates = a_texCoordinates;
}


/*void main()
{
    vec2 st = vec2(a_position.x*1.0, a_position.z*1.0);
    float y = noise(st);
    vec3 position = vec3(a_position.x, y*2.0, a_position.z);

    v_data.normal = a_normal;
	v_data.light = normalize(u_light_pos - position.xyz);
	v_data.view = normalize(vec3(0,0,3) - position.xyz);

    gl_Position = u_mvp * vec4(position.xyz, 1.0);
	v_data.color = vec3(y*3, 0, 0);
	v_data.texCoordinates = a_texCoordinates;

}*/