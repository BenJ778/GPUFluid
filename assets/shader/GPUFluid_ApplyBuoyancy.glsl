// Include Files


// Uniform Locations
uniform float	g_AmbientTemperature;
uniform float	g_TimeStep;
uniform float	g_Sigma;
uniform float	g_Kappa;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_TexVelocity;
layout( binding = 1, r16f )		uniform readonly image2D	g_TexTemperature;
layout( binding = 2, r16f )		uniform readonly image2D	g_TexDensity;
layout( binding = 3/*, rg16f*/ )	uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_TexSize		= imageSize( g_TexVelocity );
const vec2 g_InverseSize	= 1.0 / vec2( g_TexSize );


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= g_TexSize.x || texelPos.y >= g_TexSize.y )
		return;

	float T	= imageLoad( g_TexTemperature, texelPos ).r;
	vec2 V	= imageLoad( g_TexVelocity, texelPos ).xy;

	vec2 result	= V;

	//if( T > g_AmbientTemperature )
	{
		float D	= imageLoad( g_TexDensity, texelPos ).x;
		result += ( g_TimeStep * ( T - g_AmbientTemperature ) * g_Sigma - D * g_Kappa ) * vec2( 0, 1 );
	}

	imageStore( g_TexDest, texelPos, vec4( result, 0, 0 ) );
}
