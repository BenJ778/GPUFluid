// Include Files


// Uniform Locations
uniform float	g_GradientScale;


// Random Accessible Buffers
layout( binding = 0, rg16f )	uniform readonly image2D	g_TexVelocity;
layout( binding = 1, r16f )		uniform readonly image2D	g_TexPressure;
layout( binding = 2, rgba16f )	uniform readonly image2D	g_TexObstacles;

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


    vec3 oC = imageLoad( g_TexObstacles, texelPos ).xyz;
    if( oC.x>0 )
	{
		imageStore( g_TexDest, texelPos, vec4( oC.yz, 0, 0 ) );
        return;
    }

    // Find neighboring pressure:
    float pN = imageLoad( g_TexPressure, texelPos + ivec2(0, +1) ).r;
    float pS = imageLoad( g_TexPressure, texelPos + ivec2(0, -1) ).r;
    float pE = imageLoad( g_TexPressure, texelPos + ivec2(+1, 0) ).r;
    float pW = imageLoad( g_TexPressure, texelPos + ivec2(-1, 0) ).r;
    float pC = imageLoad( g_TexPressure, texelPos ).r;

    // Find neighboring obstacles:
    vec3 oN = imageLoad( g_TexObstacles, texelPos + ivec2(0, +1) ).xyz;
    vec3 oS = imageLoad( g_TexObstacles, texelPos + ivec2(0, -1) ).xyz;
    vec3 oE = imageLoad( g_TexObstacles, texelPos + ivec2(+1, 0) ).xyz;
    vec3 oW = imageLoad( g_TexObstacles, texelPos + ivec2(-1, 0) ).xyz;

    // Use center pressure for solid cells:
    vec2 obstV = vec2(0);
    vec2 vMask = vec2(1);

	if (oN.x > 0) { pN = pC; obstV.y = oN.z; vMask.y = 0; }
	if (oS.x > 0) { pS = pC; obstV.y = oS.z; vMask.y = 0; }
	if (oE.x > 0) { pE = pC; obstV.x = oE.y; vMask.x = 0; }
	if (oW.x > 0) { pW = pC; obstV.x = oW.y; vMask.x = 0; }

    // Enforce the free-slip boundary condition:
    vec2 oldV = imageLoad( g_TexVelocity, texelPos ).xy;
    vec2 grad = vec2(pE - pW, pN - pS) * g_GradientScale;
    vec2 newV = oldV - grad;
    
	imageStore( g_TexDest, texelPos, vec4( (vMask * newV) + obstV, 0, 0 ) );


	//imageStore( g_TexDest, texelPos, vec4( newV, 0, 0 ) );
}
