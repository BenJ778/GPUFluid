// Include Files


// Uniform Locations
uniform float	g_HalfInverseCellSize;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_TexVelocity;
layout( binding = 1, rgba16f )	uniform readonly image2D	g_TexObstacles;

layout( binding = 2/*, rgba16f*/ )	uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_MaxTexelCoord = imageSize( g_TexVelocity ) - ivec2( 1, 1 );

// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos = ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x > g_MaxTexelCoord.x || texelPos.y > g_MaxTexelCoord.y )
		return;


	ivec2 texposN = texelPos + ivec2( 0, +1 );
	ivec2 texposS = texelPos + ivec2( 0, -1 );
	ivec2 texposE = texelPos + ivec2( +1, 0 );
	ivec2 texposW = texelPos + ivec2( -1, 0 );

	// Find neighboring velocities:
	vec2 vN = imageLoad( g_TexVelocity, clamp( texposN, ivec2( 0, 0 ), g_MaxTexelCoord ) ).xy;
	vec2 vS = imageLoad( g_TexVelocity, clamp( texposS, ivec2( 0, 0 ), g_MaxTexelCoord ) ).xy;
	vec2 vE = imageLoad( g_TexVelocity, clamp( texposE, ivec2( 0, 0 ), g_MaxTexelCoord ) ).xy;
	vec2 vW = imageLoad( g_TexVelocity, clamp( texposW, ivec2( 0, 0 ), g_MaxTexelCoord ) ).xy;

	// Find neighboring obstacles:
	vec3 oN = imageLoad( g_TexObstacles, texposN ).xyz;
	vec3 oS = imageLoad( g_TexObstacles, texposS ).xyz;
	vec3 oE = imageLoad( g_TexObstacles, texposE ).xyz;
	vec3 oW = imageLoad( g_TexObstacles, texposW ).xyz;

	// Use obstacle velocities for solid cells:
	if( oN.x > 0 ) vN = oN.yz;
	if( oS.x > 0 ) vS = oS.yz;
	if( oE.x > 0 ) vE = oE.yz;
	if( oW.x > 0 ) vW = oW.yz;

	imageStore( g_TexDest, texelPos, vec4( g_HalfInverseCellSize * ( vE.x - vW.x + vN.y - vS.y ) ) );
}
