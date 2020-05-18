// Fragment shader
#version 150

in vec3 v_normal;
in vec3 v_light;
in vec3 v_view;

out vec4 frag_color;

uniform vec3 u_light_clr;
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float specular_power;

float diffuse(vec3 L, vec3 N);
float specular(vec3 N, vec3 H, float specular_power);
vec3 linear_to_gamma(vec3 color);

void main()
{
    vec3 H = normalize(v_view + v_light);
    
	vec3 clr = ambient_color +
			   diffuse_color * u_light_clr * diffuse(v_light, v_normal);// +
			   //(specular_power + 8.0 / 8.0) * specular_color * u_light_clr * specular(v_normal, H, specular_power);
	
	frag_color = vec4(linear_to_gamma(clr), 1.0);
}

float diffuse(vec3 L, vec3 N)
{
	return max(0.0, dot(L, N));
}

float specular(vec3 N, vec3 H, float specular_power)
{
	float normalization = (8.0 + specular_power) / 8.0;
    return normalization *
        pow(max(0.0, dot(N, H)), specular_power);
}

vec3 linear_to_gamma(vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
}