// Include Files



// Uniform Locations
uniform float		g_TimeStep;
uniform float		g_Dissipation;
uniform sampler2D	g_Phi_n;
uniform sampler2D	g_Phi_n_hat;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_TexVelocity;
layout( binding = 1 )			uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_TexSize		= textureSize( g_Phi_n, 0 );
const vec2 g_InverseSize	= 1.0 / vec2( g_TexSize );


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()  
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= g_TexSize.x || texelPos.y >= g_TexSize.y )
		return;

	vec2 texCoord	= ( texelPos + vec2(0.5) ) * g_InverseSize;
	vec2 texCoordFloor	= texelPos * g_InverseSize;

	// Trace back along the initial characteristic - we'll use  
	// values near this semi-Lagrangian "particle" to clamp our  
	// final advected value.  
	vec2 cellVelocity	= imageLoad( g_TexVelocity, texelPos ).xy;
	vec2 phi_n			= texture( g_Phi_n/*point*/, texCoord/*texCoordFloor*/ ).xy;

	if( length(cellVelocity) < 1.0e-6 )
	{
		imageStore( g_TexDest, texelPos, vec4( phi_n, 0, 0 ) );
		return;
	}


	vec2 npos	= vec2(texelPos) - g_TimeStep * cellVelocity;
	// Find the cell corner closest to the "particle" and compute the  
	// texture coordinate corresponding to that location.  
	//npos = floor( npos + vec2(0.5) );	// 必要???
	npos	= ( npos + vec2(0.5) ) * g_InverseSize;


	// Get the values of nodes that contribute to the interpolated value.  
	// Texel centers will be a half-texel away from the cell corner.  
	vec2 ht	= 0.5 * g_InverseSize;

	vec2 nodeValues[4];
	nodeValues[0]	= texture( g_Phi_n, npos + vec2(-ht.x, -ht.y) ).xy;
	nodeValues[1]	= texture( g_Phi_n, npos + vec2(-ht.x, +ht.y) ).xy;
	nodeValues[2]	= texture( g_Phi_n, npos + vec2(+ht.x, -ht.y) ).xy;
	nodeValues[3]	= texture( g_Phi_n, npos + vec2(+ht.x, +ht.y) ).xy;


	// Determine a valid range for the result.  
	vec2 phiMin = min( min( min( nodeValues[0], nodeValues[1] ), nodeValues[2] ), nodeValues[3] );
	vec2 phiMax = max( max( max( nodeValues[0], nodeValues[1] ), nodeValues[2] ), nodeValues[3] );


	// Perform final advection, combining values from intermediate advection steps.  
	vec2 r	= texture( g_Phi_n/*linear*/, npos ).xy + 0.5 * ( phi_n - texture( g_Phi_n_hat/*point*/, texCoord/*texCoordFloor*/ ).xy );

	// Clamp result to the desired range.  
	r = max( min( r * g_Dissipation, phiMax ), phiMin );


	imageStore( g_TexDest, texelPos, vec4( r, 0, 0 ) );
	//return r;
}
