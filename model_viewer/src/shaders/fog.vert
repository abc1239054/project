// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoordinates;

uniform mat4 u_mvp;
//uniform vec3 u_view_pos;

out vec2 TexCoords;

void main()
{
    gl_Position = u_mvp * a_position;
    TexCoords = a_texCoordinates;
}