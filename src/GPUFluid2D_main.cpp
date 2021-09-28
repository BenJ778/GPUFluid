#include	<crtdbg.h>
#include	<Windows.h>
#include	<iostream>
using namespace std;

#include	<AntTweakBar/AntTweakBar.h>

#include	<graphics/gl4x/other/GLPrimitives.h>
#include	<graphics/gl4x/common/GLHelperFunctions.h>

#include	<graphics/gl4xext/rendering/QuadShader.h>
#include	"GPUFluid2D.h"
#include	"FlameShader2D.h"
#include	"StreamLineShader.h"

using namespace OreOreLib;



#include	<gl/freeglut.h>


#define KEY_ESCAPE 27

#define	MOUSE_SENSITIVITY	1.0e-2f
#define DELTA 1.0e-1f//1.0f//1.0e-2f//




static int	g_ScreenWidth	= 1280;
static int	g_ScreenHeight	= 720;


// mouse event
int			g_mouseX		= 0;
int			g_mouseY		= 0;
int			g_mouseDX		= 0;
int			g_mouseDY		= 0;

bool		g_MouseLeftDown	= false;
bool		g_MouseRightDown	= false;


// keyboard event
int			g_keyDown[256];



GLVertexArrayObject	g_ScreenSpaceQuad;

FluidVolume2D		g_FluidVoxelData2D;
GPUFluid2D		g_VoxelFluid2D;
Texture2D			g_TexStreamLine;
Texture1D			g_TexRamp;

QuadShader			g_QuadShader;
FlameShader2D		g_RampShader;
StreamLineShader	g_StreamLineShader;




// Fluid Simulation Parameters
static int			g_NumIterations				= GPUFluid2D::DEFAULT_ITERATIONS;
static float		g_TimeStep					= GPUFluid2D::DEFAULT_TIME_STEP;

static float		g_AmbientTemperature		= GPUFluid2D::DEFAULT_AMBIENT_TEMPERATURE;
static float		g_ImpulseTemperature		= GPUFluid2D::DEFAULT_IMPULSE_TEMPERATURE;
static float		g_ImpulseDensity			= GPUFluid2D::DEFAULT_IMPULSE_DENSITY;
static float		g_SmokeBuoyancy				= GPUFluid2D::DEFAULT_SMOKE_BUOYANCY;
static float		g_SmokeWeight				= GPUFluid2D::DEFAULT_SMOKE_WEIGHT;

static float		g_TemperatureDissipation	= GPUFluid2D::DEAFAULT_TEMPERATURE_DISSIPATION;
static float		g_VelocityDissipation		= GPUFluid2D::DEAFAULT_VELOCITY_DISSIPATION;
static float		g_DensityDissipation		= GPUFluid2D::DEAFAULT_DENSITY_DISSIPATION;

static bool			g_bEnableBuoyancy			= true;
static bool			g_bEnableMacCormackMethod	= false;

static float		g_Vorticity					= GPUFluid2D::DEFAULT_VORTICITY;
static float		g_Viscosity					= GPUFluid2D::DEFAULT_VISCOSITY;

static float		g_SmokeRadius				= 20.0f;
static float		g_ObstacleRadius			= 10.0f;

static float		g_StreamLength				= 100.0f;



typedef enum
{
	DENSITY, TEMPERATURE, DIVERGENCE, VELOCITY, OBSTACLES, STREAMLINE, CURL,
} ViewMode;

ViewMode	g_ViewMode	= DENSITY;

TwEnumVal ViewModeEV[] =
{
	{ DENSITY, "Density" },
	{ TEMPERATURE, "Temperature" },
	{ DIVERGENCE, "Divergence" },
	{ VELOCITY, "Velocity" },
	{ OBSTACLES, "Obstacles" },
	{ STREAMLINE, "StreamLine" },
	{ CURL, "Curl" },
};



typedef enum
{
	SMOKE, OBSTACLE,
} DrawMode;

DrawMode	g_DrawMode	= SMOKE;

TwEnumVal DrawModeEV[] =
{
	{ SMOKE, "Smoke" },
	{ OBSTACLE, "Obstacles" },
};





void mouse( int button, int state, int x, int y )
{

	if( !TwEventMouseButtonGLUT( button, state, x, y ) )
	{

		switch( button )
		{
		case GLUT_LEFT_BUTTON:

			switch( state )
			{
			case GLUT_DOWN:	//  ボタンを押した直後の一瞬だけ引っかかる
				g_MouseLeftDown	= true;
				g_mouseX = x;
				g_mouseY = y;
				break;

			case GLUT_UP:	//  ボタンを開放した直後の一瞬だけ引っかかる
				g_MouseLeftDown	= false;
				break;

			default:
				break;

			}

			break;


		case GLUT_RIGHT_BUTTON:

			switch( state )
			{
			case GLUT_DOWN:	//  ボタンを押した直後の一瞬だけ引っかかる
				g_MouseRightDown	= true;
				g_mouseX = x;
				g_mouseY = y;
				break;

			case GLUT_UP:	//  ボタンを開放した直後の一瞬だけ引っかかる
				g_MouseRightDown	= false;
				break;

			default:
				break;

			}

			break;


		default:
			break;
		}

	}

}



// マウスカーソルが動いたときに呼び出される関数
void motion( int x, int y )
{
	if( !TwEventMouseMotionGLUT( x, y ) )
	{
		g_mouseDX	= x - g_mouseX;
		g_mouseDY	= y - g_mouseY;

		g_mouseX	= x;
		g_mouseY	= y;

		//printf( "X = %d : Y = %d\n", x, y );
		//printf( "DX = %d : DY = %d\n", g_mouseDX, g_mouseDY );
	}

}



void passivemotion( int x, int y )
{
	if( !TwEventMouseMotionGLUT( x, y ) )
	{
		// Do Nothing at current.(2015.07.15)
	}
}




void keydown( unsigned char key, int x, int y )
{

	if( !TwEventKeyboardGLUT( key, x, y ) )
	{


		g_keyDown[key] = 1;

		switch( key )
		{
		case KEY_ESCAPE:
			exit( 0 );
			break;

		default:
			break;
		}

	}
}



void keyup( unsigned char key, int x, int y )
{
	g_keyDown[key] = 0;
}



void special( int key, int x, int y )
{
	if( !TwEventSpecialGLUT( key, x, y ) )
	{
		// Do Nothing at current.(2015.07.15)
	}

}




void keyboardCommand()
{
	//CameraObject *pCamera = g_SceneGraph->GetCurrentCameraObject();

	//if( g_keyDown['w'] )	pCamera->MoveForward( DELTA );
	//if( g_keyDown['s'] )	pCamera->MoveForward( -DELTA );
	//if( g_keyDown['d'] )	pCamera->MoveHorizontal( -DELTA );
	//if( g_keyDown['a'] )	pCamera->MoveHorizontal( DELTA );
	//if( g_keyDown['r'] )	pCamera->MoveVertical( DELTA );
	//if( g_keyDown['f'] )	pCamera->MoveVertical( -DELTA );
	//if( g_keyDown['q'] )	pCamera->Roll( -DELTA*0.1f );
	//if( g_keyDown['e'] )	pCamera->Roll( +DELTA*0.1f );

	if( g_MouseLeftDown /*&& (g_mouseDX != 0 || g_mouseDY != 0)*/ )
	{
		if( g_DrawMode==OBSTACLE )
			g_VoxelFluid2D.AddObstacles( g_mouseX, ( g_ScreenHeight-g_mouseY ), g_ObstacleRadius );	//tcout << g_mouseX << _T(", ") << g_mouseY << tendl; //
		else
			g_VoxelFluid2D.AddSmoke( g_mouseX, ( g_ScreenHeight-g_mouseY ), float( g_mouseDX )*2.0f, -float( g_mouseDY )*2.0f, g_SmokeRadius );
	}
	if( g_MouseRightDown )
	{
		g_VoxelFluid2D.RemoveObstacles( g_mouseX, ( g_ScreenHeight-g_mouseY ), g_ObstacleRadius );	//tcout << g_mouseX << _T(", ") << g_mouseY << tendl; // 
	}




	g_mouseDX = 0;
	g_mouseDY = 0;

}




void display()
{
	static float radian	= 0.0f;

	keyboardCommand();

	//=============== Update Fluid =======================//

	// Set Params
	g_VoxelFluid2D.SetIterations( g_NumIterations );
	g_VoxelFluid2D.SetTimeStep( g_TimeStep );
	g_VoxelFluid2D.SetAmbientTemperature( g_AmbientTemperature );
	g_VoxelFluid2D.SetImpulseTemperature( g_ImpulseTemperature );
	g_VoxelFluid2D.SetImpulseDensity( g_ImpulseDensity );
	g_VoxelFluid2D.SetSmokeBuoyancy( g_SmokeBuoyancy );
	g_VoxelFluid2D.SetSmokeWeight( g_SmokeWeight );
	g_VoxelFluid2D.SetTemperatureDissipation( g_TemperatureDissipation );
	g_VoxelFluid2D.SetVelocityDissipation( g_VelocityDissipation );
	g_VoxelFluid2D.SetDensityDissipation( g_DensityDissipation );
	g_VoxelFluid2D.EnableMackCormackAdvection( g_bEnableMacCormackMethod );
	g_VoxelFluid2D.EnableBuoyancy( g_bEnableBuoyancy );
	g_VoxelFluid2D.SetVorticity( g_Vorticity );
	g_VoxelFluid2D.SetViscosity( g_Viscosity );


	// Update simulation
	g_VoxelFluid2D.Update();


	//================ Update StreamLine ==================//
	g_StreamLineShader.Draw( g_StreamLength, 250 );


	//=============== Draw ===============================//

	static Texture2D *pTex = NULL;
	switch( g_ViewMode )
	{
	case DENSITY:
		pTex	= g_FluidVoxelData2D.m_TexDensity.BackBuffer();
		break;

	case TEMPERATURE:
		pTex	= g_FluidVoxelData2D.m_TexTemperature.BackBuffer();
		break;

	case DIVERGENCE:
		pTex	= &g_FluidVoxelData2D.m_TexDivergence;
		break;

	case VELOCITY:
		pTex	= g_FluidVoxelData2D.m_TexVelocity.BackBuffer();
		break;

	case OBSTACLES:
		pTex	= &g_FluidVoxelData2D.m_TexObstacles;
		break;

	case STREAMLINE:
		pTex	= &g_TexStreamLine;
		break;

	case CURL:
		pTex	= g_VoxelFluid2D.GetCurl();
		break;

	default:
		pTex	= g_FluidVoxelData2D.m_TexDensity.BackBuffer();
		break;
	}



	//glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//g_QuadShader.Render( pTex, 1.0f );


	// Fluid/StreamLine
	glEnable( GL_BLEND );
	glBlendFunc( GL_DST_ALPHA, GL_ONE );

	glClearColor( 0.3f, 0.3f, 0.3f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	g_QuadShader.Render( pTex, 1.0f );
	g_QuadShader.Render( g_FluidVoxelData2D.m_TexObstacles.TexID(), 1.0f );

	//g_RampShader.Render( &g_TexRamp, g_FluidVoxelData2D.m_TexDensity.BackBuffer(), g_FluidVoxelData2D.m_TexTemperature.BackBuffer(), 0.75f );
	//g_RampShader.Render( &g_TexRamp, &g_FluidVoxelData2D.m_TexObstacles, &g_FluidVoxelData2D.m_TexObstacles, 1.0f );
	
	glDisable( GL_BLEND );


	// Draw AntTweakBar GUI
	TwDraw();


	glutSwapBuffers();

}



void initialize()
{
	glutReshapeWindow( g_ScreenWidth, g_ScreenHeight );

	CreateScreenSpaceQuad( g_ScreenSpaceQuad );
	IShader::BindScreenSpaceQuad( &g_ScreenSpaceQuad );

	TCHAR dir[_MAX_PATH];
	GetCurrentDirectory( _MAX_PATH, dir );
	//SetCurrentDirectory( _T( "../" ) );

	g_VoxelFluid2D.InitShader();
	g_QuadShader.InitShader( _T( "../external/graphics/gl4xext/glsl/rendering/QuadShader.glsl" ), GLSL_VERSION::GLSL_430 );
	g_RampShader.InitShader( _T( "../assets/shader/FlameShader2D.glsl" ), GLSL_VERSION::GLSL_430 );
	
	g_StreamLineShader.InitShader( _T( "../assets/shader/StreamLineShader.glsl" ) );


	// back to current directory
	SetCurrentDirectory( dir );



	g_TexRamp = Texture1D();
	g_TexRamp.Load( _T( "../assets/textures/ramp.png" ) );
	g_TexStreamLine.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
	g_TexRamp.GenHardwareTexture();

	g_FluidVoxelData2D.Init( g_ScreenWidth, g_ScreenHeight );

	g_VoxelFluid2D.BindVoxelData( &g_FluidVoxelData2D );
	g_VoxelFluid2D.SetTimeStep( 0.125f );//0.05f );//


	g_TexStreamLine.Allocate( g_ScreenWidth, g_ScreenHeight, 0, 0, DATA_FORMAT::FORMAT_R16G16B16A16_FLOAT );
	g_TexStreamLine.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
	g_TexStreamLine.GenHardwareTexture();

	g_StreamLineShader.BindVectorField( g_FluidVoxelData2D.m_TexVelocity.BackBuffer(), &g_TexStreamLine );
	g_StreamLineShader.SetGridSize( 128, 72 );//320, 240 );

}



void reshape( int width, int height )
{
	g_ScreenWidth	= width;
	g_ScreenHeight	= height;

	glViewport( 0, 0, width, height );


	// Send the new window size to AntTweakBar
	TwWindowSize( width, height );

}



void Idle()
{
	glutPostRedisplay();
}



int main( int argc, char **argv )
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// Ant TweakBar
	TwBar *bar; // Pointer to the tweak bar




	// initialize and run program
	glutInit( &argc, argv );                                      // GLUT initialization
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
	glutInitWindowSize( g_ScreenWidth, g_ScreenHeight );		// set window size
	glutCreateWindow( "GPU Voxel Fluid 2D" );						// create Window
	glewInit();
	glutDisplayFunc( display );									// register Display Function
	glutReshapeFunc( reshape );									// register Reshape Function
	glutIdleFunc( Idle );										// register Idle Function

	memset( g_keyDown, 0, sizeof( g_keyDown ) );
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
	glutPassiveMotionFunc( passivemotion );
	glutKeyboardFunc( keydown );						// register Keyboard Handler
	glutKeyboardUpFunc( keyup );						// register Keyboard Handler
	//glutKeyboardFunc( keyboard );						// register Keyboard Handler
	glutSpecialFunc( special );

	// - Send 'glutGetModifers' function pointer to AntTweakBar;
	//   required because the GLUT key event functions do not report key modifiers states.
	TwGLUTModifiersFunc( glutGetModifiers );

	initialize();


	TwInit( TW_OPENGL, NULL );
	TwWindowSize( g_ScreenWidth, g_ScreenHeight );

	bar = TwNewBar( "Main Menu" );

	TwDefine( " 'Main Menu' size='300 400' color='96 216 224' " ); // change default tweak bar size and color

	TwAddVarRW( bar, "NumIterations", TW_TYPE_INT32, &g_NumIterations, " label='NumIterations' min=1 max=100 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "TimeStep", TW_TYPE_FLOAT, &g_TimeStep, " label='TimeStep' min=0.0001 max=1.0 step=0.001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "AmbientTemperature", TW_TYPE_FLOAT, &g_AmbientTemperature, " label='AmbientTemperature' min=-100.0 max=100.0 step=0.01 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "ImpulseTemperature", TW_TYPE_FLOAT, &g_ImpulseTemperature, " label='ImpulseTemperature' min=-100.0 max=100.0 step=0.01 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "ImpulseDensity", TW_TYPE_FLOAT, &g_ImpulseDensity, " label='ImpulseDensity' min=0.0 max=100.0 step=0.01 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "SmokeBuoyancy", TW_TYPE_FLOAT, &g_SmokeBuoyancy, " label='SmokeBuoyancy' min=-10.0 max=10.0 step=0.001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "SmokeWeight", TW_TYPE_FLOAT, &g_SmokeWeight, " label='SmokeWeight' min=0.0 max=10.0 step=0.001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "TemperatureDissipation", TW_TYPE_FLOAT, &g_TemperatureDissipation, " label='TemperatureDissipation' min=0.0 max=1.0 step=0.0001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "VelocityDissipation", TW_TYPE_FLOAT, &g_VelocityDissipation, " label='VelocityDissipation' min=0.0 max=1.0 step=0.0001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "DensityDissipation", TW_TYPE_FLOAT, &g_DensityDissipation, " label='DensityDissipation' min=0.0 max=1.0 step=0.0001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "EnableMacCormackMethod", TW_TYPE_BOOLCPP, &g_bEnableMacCormackMethod, " label='Use MackCormack' keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "EnableBuoyancy", TW_TYPE_BOOLCPP, &g_bEnableBuoyancy, " label='Enable Buoyancy' keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "Vorticity", TW_TYPE_FLOAT, &g_Vorticity, " label='Vorticity' min=0.0 max=10.0 step=0.0001 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "Viscosity", TW_TYPE_FLOAT, &g_Viscosity, " label='Viscosity' min=0.0 max=100.0 step=0.0001 keyIncr=z keyDecr=Z " );

	TwType viewMode;
	viewMode = TwDefineEnum( "ViewMode", ViewModeEV, ARRAYSIZE( ViewModeEV ) );
	TwAddVarRW( bar, "ViewMode", viewMode, &g_ViewMode, NULL );

	TwType drawMode;
	drawMode = TwDefineEnum( "DrawMode", DrawModeEV, ARRAYSIZE( DrawModeEV ) );
	TwAddVarRW( bar, "DrawMode", drawMode, &g_DrawMode, NULL );

	TwAddVarRW( bar, "SmokeRadius", TW_TYPE_FLOAT, &g_SmokeRadius, " label='SmokeRadius' min=1.0 max=100.0 step=0.01 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "ObstacleRadius", TW_TYPE_FLOAT, &g_ObstacleRadius, " label='ObstacleRadius' min=1.0 max=100.0 step=0.01 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "StreamLength", TW_TYPE_FLOAT, &g_StreamLength, " label='StreamLength' min=0.0 max=5000.0 step=0.1 keyIncr=z keyDecr=Z " );



	//	TwAddVarRW( bar, "PreserveLuminance", TW_TYPE_BOOLCPP, &g_bPreserveLuminance, " label='PreserveLuminance' " );


	//	


	// run GLUT mainloop
	glutMainLoop();


	// terminate AntTweakBar
	TwTerminate();

	// release all allocated data
	g_VoxelFluid2D.Release();
	g_FluidVoxelData2D.Release();

	g_QuadShader.Release();
	g_RampShader.Release();
	g_StreamLineShader.Release();


	g_TexStreamLine.Release();
	g_TexRamp.Release();


	return 0;
}