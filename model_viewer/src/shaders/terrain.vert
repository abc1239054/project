// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoordinates;
layout(location = 3) in float a_height;

out vec3 v_normal;
out vec3 v_light;

uniform mat4 u_mvp;
uniform mat4 u_mv;
uniform vec3 u_light_pos;
uniform float seed;
uniform float u_map_height;

out vec2 v_texCoordinates;

void main()
{
	v_normal = a_normal;
	v_light = normalize(u_light_pos - a_position.xyz);

    gl_Position = u_mvp * (a_position + vec4(0.0, a_height * u_map_height, 0.0, 0.0));
	v_texCoordinates = a_texCoordinates;
}
