// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoordinates;
layout(location = 3) in float a_height;

uniform mat4 u_mvp;
uniform float seed;

out vec3 v_color;
out vec2 texCoordinates;

// 2D Random
float random (in vec2 st) {
    //return fract(sin(dot(st.xy,
    //                     vec2(12.9898,78.233)))
    //             * 43758.5453123);
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * seed);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

vec3 mod289(vec3 x){ return x - floor (x * (1.0 / 289.0) ) * 289.0; }
vec2 mod289(vec2 x){ return x - floor (x * (1.0 / 289.0) ) * 289.0; }

vec3 permute(vec3 x){ return mod289 ((( x *34.0) +1.0) *x); }

float snoise(const vec2 v){
    const vec4 C = vec4 (0.211324865405187 , // (3.0 - sqrt (3.0) ) /6.0
    0.366025403784439 , // 0.5*( sqrt (3.0) -1.0)
    -0.577350269189626 , // -1.0 + 2.0 * C.x
    0.024390243902439) ; // 1.0 / 41.0
    // First corner
    vec2 i = floor (v + dot(v, C.yy) );
    vec2 x0 = v - i + dot (i, C.xx);
    // Other corners
    vec2 i1 = (x0.x > x0.y) ? vec2 (1.0 , 0.0) : vec2 (0.0 , 1.0) ;
    vec4 x12 = x0. xyxy + C. xxzz ;
    x12 .xy -= i1;
    // Permutations
    i = mod289 (i); // Avoid truncation effects in permutation
    vec3 p = permute ( permute ( i.y + vec3 (0.0 , i1.y, 1.0 ))
    + i.x + vec3 (0.0 , i1.x, 1.0 ));
    vec3 m = max (0.5 - vec3 (dot (x0 ,x0), dot( x12.xy , x12 .xy),
    dot ( x12 .zw ,x12.zw)), 0.0) ;
    m = m*m; m = m*m;
    // Gradients
    vec3 x = 2.0 * fract (p * C. www ) - 1.0;
    vec3 h = abs (x) - 0.5;
    vec3 a0 = x - floor (x + 0.5) ;
    // Normalise gradients implicitly by scaling m
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    // Compute final noise value at P
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12 .xz + h.yz * x12 .yw;
    return 130.0 * dot (m, g);
}


void main()
{
    vec2 st = vec2(a_position.x*1.0, a_position.z*1.0);
    float y = noise(st);

	float h=snoise(5*st)
+ 0.5*snoise(10.0*st)
+ 0.25*snoise(20.0*st)
+ 0.125*snoise(40.0*st)
+ 0.0625*snoise(80.0*st);
	
	
	gl_Position = u_mvp * vec4(a_position.x, 2.0*y, a_position.z, 1.0);
	v_color = vec3(y,0.0,0.0);
	texCoordinates = a_texCoordinates;
}