// Fragment shader
#version 150

out vec4 frag_color;
in vec3 v_color;
in vec2 texCoordinates;

uniform sampler2D textureFile;

void main()
{
    frag_color = texture(textureFile, texCoordinates) + vec4(v_color, 1.0);
}
