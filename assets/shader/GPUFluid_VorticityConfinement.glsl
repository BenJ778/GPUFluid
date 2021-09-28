// Include Files


// Uniform Locations
uniform float	g_Eps;
uniform float	g_CellSize;
uniform float	g_TimeStep;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_TexVelocity;
layout( binding = 1, r16f )		uniform readonly image2D	g_TexCurlMagnitude;
layout( binding = 2 )			uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_TexSize		= imageSize( g_TexVelocity );
const vec2 g_InverseSize	= 1.0 / vec2( g_TexSize );
const float	g_HalfInverseCellSize	= 0.5 / g_CellSize;


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= g_TexSize.x || texelPos.y >= g_TexSize.y )
		return;

	vec2 velocity	= imageLoad( g_TexVelocity, texelPos ).xy;
	
	
	//====================== Calc curl vector =====================//
	float omega	= imageLoad( g_TexCurlMagnitude, texelPos ).x;
	float curlN	= abs( imageLoad( g_TexCurlMagnitude, texelPos + ivec2(0, +1) ).x );
	float curlS	= abs( imageLoad( g_TexCurlMagnitude, texelPos + ivec2(0, -1) ).x );
	float curlE	= abs( imageLoad( g_TexCurlMagnitude, texelPos + ivec2(+1, 0) ).x );
	float curlW	= abs( imageLoad( g_TexCurlMagnitude, texelPos + ivec2(-1, 0) ).x );
	
	
	// N = ∇|w| / | ∇|w| |
	vec3 n	= vec3( curlE - curlW, curlN - curlS, 0.0 ) * g_HalfInverseCellSize ;
	float len	= length(n);//max( length(n), 1.0e-6 );

	
	// εh(N×ω)
	//vec2 f	= len>0 ? 		
	//	g_Eps * g_CellSize * cross( n/len, vec3(0,0,omega) ).xy:// F = N×ω
	//	vec2(0);

	vec2 f	= mix( vec2( 0 ), g_Eps * g_CellSize * cross( n/len, vec3( 0, 0, omega ) ).xy, (len>0 && abs(omega)>0) );// F = N×ω


	imageStore( g_TexDest, texelPos, vec4( velocity + g_TimeStep * f, 0, 0 ) );
}
