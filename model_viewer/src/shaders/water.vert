// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoordinates;

out vec3 v_light;
out vec3 v_view;

out vec2 v_texCoordinates;

uniform mat4 u_mvp;
uniform mat4 u_mv;
uniform vec3 u_light_pos;
uniform vec3 u_view_pos;

void main()
{
	v_light = normalize(u_light_pos - a_position.xyz);
	v_view = normalize(u_view_pos - a_position.xyz);
	v_texCoordinates = a_texCoordinates;
    gl_Position = u_mvp * a_position;
}
