// Include Files


// Uniform Locations
uniform float	g_HalfInverseCellSize;
uniform float	g_TimeStep;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_TexVelocity;
layout( binding = 1 )			uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_TexSize		= imageSize( g_TexVelocity );


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= g_TexSize.x || texelPos.y >= g_TexSize.y )
		return;
	
	vec2 vN	= imageLoad( g_TexVelocity, texelPos + ivec2(0, +1) ).xy;
	vec2 vS	= imageLoad( g_TexVelocity, texelPos + ivec2(0, -1) ).xy;
	vec2 vE	= imageLoad( g_TexVelocity, texelPos + ivec2(+1, 0) ).xy;
	vec2 vW	= imageLoad( g_TexVelocity, texelPos + ivec2(-1, 0) ).xy;

	float curlMagnitude	= ( g_HalfInverseCellSize * ( ( vE.y - vW.y ) - ( vN.x - vS.x ) ) );

	imageStore( g_TexDest, texelPos, vec4(curlMagnitude) );

}
