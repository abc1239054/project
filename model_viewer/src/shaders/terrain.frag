// Fragment shader
#version 150

in vec3 v_normal;
in vec3 v_light;

out vec4 frag_color;

uniform vec3 u_light_clr;
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float specular_power;
uniform int u_is_amb;
uniform float u_light_int;
uniform float u_ambient_int;
uniform int u_is_diffuse;
uniform int u_is_light;
uniform int u_is_texture;

float diffuse(vec3 L, vec3 N);
float specular(vec3 N, vec3 H, float specular_power);
vec3 linear_to_gamma(vec3 color);

in vec2 v_texCoordinates;

uniform sampler2D textureFile;

void main()
{ 
	vec3 clr = vec3(0.0, 0.0, 0.0);
	
	if (u_is_amb == 1)
		clr += ambient_color * u_ambient_int;

	if (u_is_diffuse == 1 && u_is_light == 1)
		clr += diffuse_color * u_light_clr * u_light_int * diffuse(v_light, v_normal);
	
	frag_color = vec4(linear_to_gamma(clr), 1.0);
	
	if (u_is_texture == 1)
		frag_color = texture(textureFile, v_texCoordinates) * frag_color;
}

float diffuse(vec3 L, vec3 N)
{
	return max(0.0, dot(L, N));
}

vec3 linear_to_gamma(vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
}