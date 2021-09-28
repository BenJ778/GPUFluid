// Include Files


// Uniform Locations
uniform sampler2D	g_x; // x vector (Ax = b)
uniform sampler2D	g_b; // b vector (Ax = b)

uniform float		g_Alpha;
uniform float		g_InverseBeta;


// Random Accessible Buffers
layout( binding = 0, rgba16f )	uniform readonly image2D	g_TexObstacles;
layout( binding = 1 )			uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_TexSize		= textureSize( g_x, 0 );//imageSize( g_TexPressure );


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= g_TexSize.x || texelPos.y >= g_TexSize.y )
		return;


	// Find neighboring variables:
	vec4 xN	= /*imageLoad*/texelFetch( /*g_TexPressure*/g_x, texelPos + ivec2( 0, +1 ), 0 );
	vec4 xS	= /*imageLoad*/texelFetch( /*g_TexPressure*/g_x, texelPos + ivec2( 0, -1 ), 0 );
	vec4 xE	= /*imageLoad*/texelFetch( /*g_TexPressure*/g_x, texelPos + ivec2( +1, 0 ), 0 );
	vec4 xW	= /*imageLoad*/texelFetch( /*g_TexPressure*/g_x, texelPos + ivec2( -1, 0 ), 0 );
	vec4 xC	= /*imageLoad*/texelFetch( /*g_TexPressure*/g_x, texelPos, 0 );

	// Find neighboring obstacles:
	vec3 oN	= imageLoad( g_TexObstacles, texelPos + ivec2(0, +1) ).xyz;
	vec3 oS	= imageLoad( g_TexObstacles, texelPos + ivec2(0, -1) ).xyz;
	vec3 oE	= imageLoad( g_TexObstacles, texelPos + ivec2(+1, 0) ).xyz;
	vec3 oW	= imageLoad( g_TexObstacles, texelPos + ivec2(-1, 0) ).xyz;

	// Use center pressure for solid cells:
	if( oN.x > 0 ) xN = xC;
	if( oS.x > 0 ) xS = xC;
	if( oE.x > 0 ) xE = xC;
	if( oW.x > 0 ) xW = xC;

	vec4 bC	= /*imageLoad*/texelFetch( /*g_TexDivergence*/g_b, texelPos, 0 );

	imageStore( g_TexDest, texelPos, vec4( (xW + xE + xS + xN + g_Alpha * bC) * g_InverseBeta ) );
}
