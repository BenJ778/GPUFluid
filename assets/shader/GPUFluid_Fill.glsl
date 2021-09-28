// Include Files


// Uniform Locations
uniform vec2	g_Point;
uniform float	g_Radius;
uniform vec3	g_FillColor;
uniform vec4	g_TexSize;


// Random Accessible Buffers
layout( binding = 0 )		uniform writeonly image2D	g_TexDest;


// Constant Variables


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= int(g_TexSize.x) || texelPos.y >= int(g_TexSize.y) )
		return;

	float d	= distance( g_Point, vec2(texelPos) );
	if( d >= g_Radius )
		return;

	imageStore( g_TexDest, texelPos, vec4( g_FillColor, 1.0 ) );
	
}
