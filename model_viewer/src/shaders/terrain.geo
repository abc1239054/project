#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in varyingData{
    vec3 normal;
    vec3 light;
    vec3 view;
    vec3 color;
    vec2 texCoordinates;
}v_datas[];

out vec3 v_color;
out vec2 v_texCoordinates;

out vec3 v_normal;
out vec3 v_light;
out vec3 v_view;

void main()
{
    vec3 a = ( gl_in[1].gl_Position - gl_in[0].gl_Position ).xyz;
    vec3 b = ( gl_in[2].gl_Position - gl_in[0].gl_Position ).xyz;
    vec3 N = normalize( cross( a, b ) );

    for( int i=0; i<gl_in.length( ); ++i )
    {
        gl_Position = gl_in[i].gl_Position;
        v_normal = N;
        v_color = v_datas[i].color;
        v_light = v_datas[i].light;
        v_texCoordinates = v_datas[i].texCoordinates;
        v_view = v_datas[i].view;

        EmitVertex( );
    }
    
    EndPrimitive( );

}