// Fragment shader
#version 150

out vec4 frag_color;

uniform sampler2D u_texture2;

in vec2 TexCoords;

void main()
{
    vec4 texColor = texture(u_texture2, TexCoords);
    if(texColor.a < 0.01)
        discard;
    frag_color = texColor;
}