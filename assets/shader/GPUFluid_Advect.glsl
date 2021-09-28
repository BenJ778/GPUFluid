// Include Files


// Uniform Locations
uniform float		g_TimeStep;
uniform float		g_Dissipation;
uniform sampler2D	g_SourceTexture;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_VelocityTexture;
layout( binding = 1, rgba16f )	uniform readonly image2D	g_Obstacles;
layout( binding = 2/*, rgba16f*/ )	uniform writeonly image2D	g_TexDest;


// Constant Variables
const ivec2 g_TexSize		= textureSize( g_SourceTexture, 0 );
const vec2 g_InverseSize	= 1.0 / vec2( g_TexSize );


// Workgroup Layout
layout( local_size_x=8, local_size_y=8 ) in;



// Main Kernel
void main()
{
	ivec2 texelPos	= ivec2( gl_GlobalInvocationID.xy );
	if( texelPos.x >= g_TexSize.x || texelPos.y >= g_TexSize.y )
		return;

	const vec2 texCoord	= ( texelPos + vec2(0.5) ) * g_InverseSize;

	float solid	= imageLoad( g_Obstacles, texelPos ).x;

	// Set obstacle texel advection to Zero.
	if( solid > 0 )
	{
		imageStore( g_TexDest, texelPos, vec4(0) );
		return;
	}


	vec2 u			= imageLoad( g_VelocityTexture, texelPos ).xy;
	vec2 advCoord	= texCoord - g_TimeStep * u * g_InverseSize;

	imageStore( g_TexDest, texelPos, g_Dissipation * texture( g_SourceTexture, advCoord ).xyzw );
	

	//imageStore( g_TexDest, texelPos, vec4(texCoord, 0, 0) );
}
