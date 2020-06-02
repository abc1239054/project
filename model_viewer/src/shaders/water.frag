// Fragment shader
#version 150

in vec3 v_light;
in vec3 v_view;

out vec4 frag_color;

uniform vec3 u_light_clr;
uniform vec3 u_ambient_color;
uniform vec3 u_wtr_diffuse_color;
uniform vec3 u_specular_color;
uniform float u_specular_power;
uniform float u_time;
uniform int u_is_spec;
uniform int u_is_diffuse;
uniform int u_is_amb;
uniform float u_light_int;
uniform float u_ambient_int;
uniform int u_is_light;

float diffuse(vec3 L, vec3 N);
float specular(vec3 N, vec3 H, float specular_power);
vec3 linear_to_gamma(vec3 color);

in vec2 v_texCoordinates;

uniform sampler2D normalMap;

void main()
{
	vec2 coords1 = vec2(v_texCoordinates.x + u_time * 0.05, v_texCoordinates.y);
	vec2 coords2 = vec2(v_texCoordinates.x + u_time * 0.07 + 1.23, v_texCoordinates.y + u_time * 0.09 + 4.56);
    
	vec3 normal = (texture(normalMap, coords1).rgb + texture(normalMap, coords2).rgb) * 0.5;
    normal = normalize(normal * 2.0 - 1.0); 

    vec3 v_normal = vec3(normal.x, normal.z, -normal.y);
	vec3 H = normalize(v_view + v_light);
    
	vec3 clr = vec3(0.0, 0.0, 0.0);
	
	if (u_is_amb == 1)
		clr += u_ambient_color * u_ambient_int;
	
	if (u_is_diffuse == 1 && u_is_light == 1)
		clr += u_wtr_diffuse_color * u_light_clr * u_light_int * diffuse(v_light, v_normal);
			   
	if (u_is_spec == 1 && u_is_light == 1)
		clr += (u_specular_power + 8.0 / 8.0) * u_specular_color * u_light_clr * specular(v_normal, H, u_specular_power);
	
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