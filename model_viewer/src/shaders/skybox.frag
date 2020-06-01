// Fragment shader
#version 150

out vec4 frag_color;

uniform samplerCube skybox;

in vec3 TexCoords;

void main()
{
    frag_color = texture(skybox, TexCoords);
}
