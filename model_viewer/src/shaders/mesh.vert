// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;
out vec3 v_light;
out vec3 v_view;

uniform mat4 u_mvp;
uniform mat4 u_mv;
uniform vec3 u_light_pos;

void main()
{
    v_normal = a_normal;
	v_light = normalize(u_light_pos - a_position.xyz);
	v_view = normalize(vec3(0,0,3) - a_position.xyz);
    gl_Position = u_mvp * a_position;
}
