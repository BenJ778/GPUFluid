// Include Files


// Uniform Locations
uniform sampler2D	g_TexVectorField;
uniform float		g_TraceLength;
uniform int			g_MaxIter;


// Random Accessible Buffers
layout( binding = 0 )		uniform writeonly image2D	g_TexDest;


// Constant Variables
const vec2	g_TexSize		= vec2( textureSize( g_TexVectorField, 0 ) );
const vec2	g_InverseSize	= 1.0 / g_TexSize;

const float g_TimeStep		= 1.0;//0.125;//
int	g_GridStep		= 10;


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy ) * g_GridStep;
	if( texelPos.x >= int( g_TexSize.x ) || texelPos.y >= int( g_TexSize.y ) )
		return;

	int iter		= 0;
	float currlLength = 0.0;

	vec2 currTexelPos	= vec2( texelPos );

	while( currlLength<=g_TraceLength && iter<=g_MaxIter )
	{
		// Get vector of current position
		const vec2 currTexCoord	= ( currTexelPos + vec2( 0.5 ) ) * g_InverseSize;
		vec2 u	= texture( g_TexVectorField, currTexCoord ).xy;

		if( length( u )<1.0e-3 )
			break;

		// Put trail color on currTexelPos
		imageStore( g_TexDest, ivec2( currTexelPos ), vec4( 0.1 ) );


		vec2 advTexelPos	= currTexelPos - g_TimeStep * normalize( u );// * g_InverseSize;

		// Accumulate trail length from currTexelPos
		currlLength	+= distance( currTexelPos, advTexelPos );


		currTexelPos	= advTexelPos;


		iter += 1;

		//break;
	}



}
