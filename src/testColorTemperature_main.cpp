#include	<crtdbg.h>
#include	<iostream>
using namespace std;

#include	<AntTweakBar/AntTweakBar.h>


#include	<oreore/SceneGraph.h>	// SceneGraphクラス
#include	<oreore/RenderScene.h>	// RenderSceneクラス
#include	<oreore/SceneLoader.h>
using namespace OreOreLib;

#include	<gl/freeglut.h>


#define KEY_ESCAPE 27

#define	MOUSE_SENSITIVITY	1.0e-2f
#define DELTA 1.0e-1f//1.0f//1.0e-2f//


SceneGraph	*g_SceneGraph	= NULL;	// SceneGraphオブジェクト
RenderScene	*g_RenderScene	= NULL;	// RenderSceneオブジェクト

int			g_ScreenWidth	= 1280;
int			g_ScreenHeight	= 720;


// mouse event
int			g_mouseX		= 0;
int			g_mouseY		= 0;
int			g_mouseDX		= 0;
int			g_mouseDY		= 0;

bool		g_MouseLeftDown	= false;


// keyboard event
int			g_keyDown[256];



bool		g_bEnable	= false;
bool		g_bEnableSSS		= true;
static bool g_UpdateIrradianceVolume	= false;
static bool	g_bEnableIrradianceVolume	= false;


// User Input Material Params
static float	g_SrcTemperature		= 5000.0f;
static float	g_TgtTemperature		= 1000.0f;
static float	g_Strength				= 0.5f;
static bool		g_bPreserveLuminance	= true;


static Vec3f	g_BaseColor3f	= { 0.0f, 0.0f, 0.0f };
static Vec3f	g_Reflection3f	= { 0.0f, 0.0f, 0.0f };

Texture2D	g_Texture;



void mouse( int button, int state, int x, int y )
{

if( !TwEventMouseButtonGLUT( button, state, x, y ) )
{

	switch( button )
	{
	case GLUT_LEFT_BUTTON:

		switch (state)
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

	default:
		break;
	}

}

}



// マウスカーソルが動いたときに呼び出される関数
void motion( int x , int y )
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



void passivemotion( int x , int y )
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


	g_keyDown[ key ] = 1;

	switch ( key ) 
	{
	case KEY_ESCAPE:
		exit ( 0 );   
		break;      

	case 'p':
		g_bEnable = !g_bEnable;
		break;

	default:      
		break;
	}

}
}



void keyup( unsigned char key, int x, int y )
{
	if( g_keyDown['p'] )
	{
		g_UpdateIrradianceVolume	= true;


		CameraObject *pcamObj	= g_SceneGraph->GetCurrentCameraObject();
		LightObject *pLightObj	= g_SceneGraph->GetCurrentLightObject();

		if( !pLightObj )
			return;

		pLightObj->SetViewParams( *pcamObj->GetPosition(), *pcamObj->GetDirection(), *pcamObj->GetVertical(), *pcamObj->GetHorizontal() );
	}

	if( g_keyDown['l'] )
	{
		g_bEnableIrradianceVolume	= !g_bEnableIrradianceVolume;
	}

	g_keyDown[ key ] = 0;
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
	CameraObject *pCamera = g_SceneGraph->GetCurrentCameraObject();

	if( g_keyDown['w'] )	pCamera->MoveForward( DELTA );
	if( g_keyDown['s'] )	pCamera->MoveForward( -DELTA );
	if( g_keyDown['d'] )	pCamera->MoveHorizontal( -DELTA );
	if( g_keyDown['a'] )	pCamera->MoveHorizontal( DELTA );
	if( g_keyDown['r'] )	pCamera->MoveVertical( DELTA );
	if( g_keyDown['f'] )	pCamera->MoveVertical( -DELTA );
	if( g_keyDown['q'] )	pCamera->Roll( -DELTA*0.1f );
	if( g_keyDown['e'] )	pCamera->Roll( +DELTA*0.1f );
	
	if( g_MouseLeftDown && (g_mouseDX != 0 || g_mouseDY != 0) )
		pCamera->SetOrientation( float(g_mouseDY)*MOUSE_SENSITIVITY, float(g_mouseDX)*MOUSE_SENSITIVITY );

	g_mouseDX = 0;
	g_mouseDY = 0;

}


static SceneNode* pcurrMeshNode	= NULL;





void display()
{
	static float radian	= 0.0f;

	keyboardCommand();

	static Quatf quat;
	InitQuat( quat, radian, 0.3f, 0.3f, 0.0f );


	
//	if( radian >= 2.0f * float(M_PI) )	radian = 0.0f;
//	radian += 0.05f;

//pcurrMeshNode->GetAttachedObject()->SetQuaternion( quat );


	// RenderSceneのアップデート
	g_SceneGraph->Update();
	g_RenderScene->Update();
	
	if( g_UpdateIrradianceVolume )
	{
		g_RenderScene->UpdateVCTVoxel( g_UpdateIrradianceVolume );
		//g_RenderScene->UpdateIrradianceVolume( g_UpdateIrradianceVolume );
		//// 光源の位置と向きをカメラに合わせる
		//const CameraObject *pCamObj	= g_SceneGraph->GetCurrentCameraObject();
		//LightObject *pLightObj		= g_SceneGraph->GetCurrentLightObject();

		//if( pLightObj )
		//{
		//	pLightObj->SetViewParams( *pCamObj->GetPosition(), *pCamObj->GetDirection(), *pCamObj->GetVertical(), *pCamObj->GetHorizontal() );
		//	pLightObj->Update();
		//	g_RenderScene->UpdateVCTVoxel( true );
		//	//g_RenderScene->UpdateIrradianceVolume( true );//g_TrackLightCamera ^= 1;//
		//}
		
		g_UpdateIrradianceVolume	= false;
	}

glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	// Render
	//g_RenderScene->Render();
	//g_RenderScene->RenderDeferred( false );
	//g_RenderScene->RenderConstant();
	//g_RenderScene->RenderTransparent();
	//g_RenderScene->Render2Texture( g_bEnable );
	//g_RenderScene->RenderSSS( g_bEnable );
	//g_RenderScene->RenderLocalReflection();
	//g_RenderScene->RenderLightShaft();
	//g_RenderScene->RenderSSDO();
	//g_RenderScene->RenderDOF();
	//g_RenderScene->RenderHDR();
	//g_RenderScene->RenderLightPropagationVolumes();	
	//g_RenderScene->RenderVoxelConeTracing( g_bEnableIrradianceVolume );
	g_RenderScene->RenderColorTemperature( &g_Texture, g_SrcTemperature, g_TgtTemperature, g_Strength, g_bPreserveLuminance );//g_RenderScene->RenderColorTemperature( g_Temperature, g_Strength );


	// Preview Functions for Debug purpose
	//g_RenderScene->ViewReflectiveShadowMaps();
	//g_RenderScene->ViewShadowMaps();
	//g_RenderScene->ViewAtomicCounter();
	//g_RenderScene->ViewTextureBufferObject();


// Draw AntTweakBar GUI
TwDraw();


GLErrorCheck();


#ifdef	ENABLE_SHDR_SILHOUETTE
	//g_RenderScene->RenderPostSilhouette();
	//g_RenderScene->RenderPPIContour();
#endif



#ifdef	ENABLE_DOUBLE_BUFFERING
	glutSwapBuffers();
#endif
	
}



void initialize () 
{
	/*
	//===================== シーングラフのセットアップ ======================//
	// Init VertexBuffer
	VertexBuffer vb;
	AllocateVertexBuffer( &vb, 4 );

	InitVec( *vb.Position(0), -1.0f, -1.0f, 0.0f );
	InitVec( *vb.Normal(0), 0.0f, 1.0f, 0.0f );
	InitVec( *vb.TexCoord(0), 0.0f, 0.0f );

	InitVec( *vb.Position(1), +1.0f, -1.0f, 0.0f );
	InitVec( *vb.Normal(1), 0.0f, 1.0f, 0.0f );
	InitVec( *vb.TexCoord(1), 1.0f, 0.0f );
	
	InitVec( *vb.Position(2), +1.0f, +1.0f, 0.0f );
	InitVec( *vb.Normal(2), 0.0f, 1.0f, 0.0f );
	InitVec( *vb.TexCoord(2), 1.0f, 1.0f );
	
	InitVec( *vb.Position(3), -1.0f, +1.0f, 0.0f );
	InitVec( *vb.Normal(3), 0.0f, 1.0f, 0.0f );
	InitVec( *vb.TexCoord(3), 0.0f, 1.0f );

	
	// Init IndexBuffer
	IndexBuffer ib;
	AllocateIndexBuffer( &ib, 2, 3 );

	*ib.Index(0) = 0;
	*ib.Index(1) = 1;
	*ib.Index(2) = 2;
	
	*ib.Index(3) = 3;
	*ib.Index(4) = 0;
	*ib.Index(5) = 2;
	
	// Init MaterialBuffer
	MaterialBuffer	mb;
	AllocateMaterialBuffer( &mb, 1, MTL_OPAQUE );
	InitVec( *mb.Diffuse(0), 0.6f, 0.5f, 0.3f, 1.01f );
	*/

	// Setup Scenegraph
	g_SceneGraph = new SceneGraph();

	// Add Camera
	g_SceneGraph->AddCameraObject( NULL );
	CameraObject *pCam = g_SceneGraph->GetCurrentCameraObject();
	pCam->SetViewParams( 20.0f, 20.0f, 20.0f, -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f );
	pCam->SetProjectionParams( float(M_PI_4), (float)(g_ScreenWidth)/(float)(g_ScreenHeight), /*1.0e-2f*/0.01f, 1.0e+3f );

	// Add Light
	LightAttribute lattrib;
	lattrib.Type					= LIGHT_TYPE_SPOT;//LIGHT_TYPE_DIRECTIONAL;//
	lattrib.Enable					= true;
	lattrib.EmitDiffuse				= true;
	lattrib.EmitSpecular			= true;
	lattrib.CastShadow				= true;
	InitVec( lattrib.Color, 1.0f, 1.0f, 1.0f );
	lattrib.Intensity				= 1.0f;
	lattrib.ConeAngle				= float(M_PI_4);
	lattrib.ConstantAttenuation		= 0.0f;
	lattrib.LinearAttenuation		= 0.0f;
	lattrib.QuadraticAttenuation	= 0.0f;
	lattrib.Radius					= 1000.0f;//0.01f//


	g_SceneGraph->AddLightObject( &lattrib );
	LightObject *pLight = g_SceneGraph->GetCurrentLightObject();
	pLight->SetViewParams( -10.0f, 10.0f, -10.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f );
	pLight->SetColor( 1.0f, 1.0f, 1.0f );//1.0f, 0.9f, 0.78f );
	pLight->SetIntensity( 1.5f );//100000.0f );//1.25f );//30000.0f );//1.0f );// TODO:単位は???
	pLight->SetConeAngle( float(M_PI_2) );
	/*
	// add mesh object
	TransformAttribute	tparam = {0};
	
	InitVec( tparam.Translation, 0.0f, 0.0f, 0.0f );
	InitVec( tparam.Scale, 1.0f, 1.0f, 1.0f );
	MatRotationX( tparam.matRotation, float(0.0f * M_PI) );
	QuatIdentity( tparam.Quat );

	MeshAttribute attrib = { SHDR_SIMPLE_MESH };

	

	MeshObject *pParent, *pChild, *pChild2;
	Vec3f scale;

	// 親ノードを追加
	g_SceneGraph->AddMeshObject( &attrib, &vb, &ib, &mb, NULL, &tparam );
	//g_SceneGraph->GetCurrentMesh()->SetScale( scale );
	//g_SceneGraph->GetCurrentMesh()->SetTranslation( tparam.Translation );
	
	pParent = g_SceneGraph->GetCurrentMesh();

	InitVec( scale, 2.0f, 1.0f, 1.0f );
	pParent->SetScale( scale );
	
	
	// 子ノードを追加
	Vec3f trans = { 0.0f, 0.0f, 1.0f };
	g_SceneGraph->AddChildMesh( &attrib, &vb, &ib, &mb, NULL, &tparam );
	g_SceneGraph->GetCurrentMesh()->SetTranslation( trans );
	//g_SceneGraph->GetCurrentMesh()->SetScale( scale );
	//g_SceneGraph->GetCurrentMesh()->SetQuaternion( quat );

	pChild = g_SceneGraph->GetCurrentMesh();


	// 孫ノードを追加
	InitVec( trans, 0.0f, 0.0f, 1.0f );
	g_SceneGraph->AddChildMesh( &attrib, &vb, &ib, &mb, NULL, &tparam );
	g_SceneGraph->GetCurrentMesh()->SetTranslation( trans );
	//g_SceneGraph->GetCurrentMesh()->SetScale( scale );
	//g_SceneGraph->GetCurrentMesh()->SetRotation( rot );
	
	pChild2 = g_SceneGraph->GetCurrentMesh();
	
	
	// 子ノード追加終わった後で、親をスケーリング
	InitVec( scale, 2.0f, 1.0f, 6.0f );
	pParent->SetScale( scale );
	
	InitVec( trans, 0.0f, 0.0f, 1.0f );
	InitVec( scale, 0.5f, 1.0f, 2.0f );
	pChild->SetScale( scale );
	//pChild->SetTranslation( trans );



	// add camera object
	//CameraAttribute cam_attr = {0};
	Vec3f campos = { 0, 0, -25 };
	///InitQuat( quat, -(float)M_PI*0.15f, 1.0f, 0.0f, 0.0f );

	g_SceneGraph->AddCameraObject( NULL );
	g_SceneGraph->GetCurrentCameraObject()->SetPosition( campos );
	//g_SceneGraph->GetCurrentCameraObject()->SetQuaternion( quat );

//	InitQuaternion( quat, (float)M_PI_2, 0.0f, 1.0f, 0.0f );
//	g_SceneGraph->GetCurrentCameraObject()->SetQuaternion( quat );

	g_SceneGraph->GetCurrentCameraObject()->SetViewParams( 50.0f, 50.0f, 50.0f, -1,-1,-1, 0,0,1 );
	g_SceneGraph->GetCurrentCameraObject()->SetProjectionParams( float(M_PI_4), (float)(g_ScreenWidth)/(float)(g_ScreenHeight), 1.0e-3f, 1.0e+3f );


	// add light object
	LightAttribute lgt_attr = { LGT_DIRECTIONAL, true, true };

	g_SceneGraph->AddLightObject( &lgt_attr, NULL );
	//g_SceneGraph->GetCurrentLight()->SetPosition();


	ReleaseVertexBuffer( &vb );
	ReleaseIndexBuffer( &ib );
	ReleaseMaterialBuffer( &mb );
	
	g_SceneGraph->TraverseMesh();
	*/


	FbxLoader fbxLoader;
	fbxLoader.Init();
	//fbxLoader.Load( "../scene/Sibenik.FBX" );//"../scene/testCornelBox.fbx" );//"../scene/Armadillo.FBX" );//"../scene/metalSphere.FBX" );//"../scene/CrytekSponza.fbx" );//"../scene/carSampleAsset.FBX" );//"../scene/interior_701_oreore.FBX" );//"../scene/testDDDD.fbx" );//
	//
	//"../scene/testMulitiMaterials.FBX" );
	//"../scene/StanfordDragon.fbx" );//"../scene/testShadowmap.fbx" );//"../scene/testTransparent.FBX" );
	//"../scene/recapSample.fbx" );//"../scene/testLPVScene.fbx" );//"../scene/boxes_scaled_parented.fbx" );
	//"../scene/boxes.fbx" );//"../scene/CrytekSponza.fbx");//"../scene/testScene_ObjectIntersection.fbx" );
	//"../scene/testSphere.FBX" );//"../scene/testSphereAndCube.fbx" );//"../scene/dddd.fbx" );
	//"../scene/isd.FBX" );//

	fbxLoader.ScanGeometry( g_SceneGraph );





	//===================== RenderSceneのセットアップ ======================//
	g_RenderScene	= new RenderScene();
	g_RenderScene->BindSceneGraph( g_SceneGraph );
	g_RenderScene->InitViewTransformBuffer();
	g_RenderScene->InitGbuffer( g_ScreenWidth, g_ScreenHeight );
	g_RenderScene->InitShadowBuffer( SHADOWMAP_SIZE, SHADOWMAP_SIZE, MAX_SHADOWS );
	//g_RenderScene->InitIrradianceVolume( 48, 48, 48, 1, 1.0f );
	g_RenderScene->InitVCTVoxelData( 128, 128, 128, 7, 1.0f );
	g_RenderScene->InitRSMBuffer( 1024, 1024 );

pcurrMeshNode	= (SceneNode *)g_SceneGraph->GetCurrentMeshNode()->children;

	//===================== OpenGLステートの初期化 =====================//
	glEnable( GL_DEPTH_TEST );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glClearColor( 0.0, 0.0, 0.0, 0.0 );

	//glPointSize( 1.5 );





g_Texture.Load( "../texture/colorchart/colorchecker_aces_srgb.png" );
g_Texture.GenHardwareBuffer();
}



void reshape( int width, int height )
{
	g_ScreenWidth	= width;
	g_ScreenHeight	= height;

	g_SceneGraph->GetCurrentCameraObject()->SetAspectRatio( (float)(g_ScreenWidth)/(float)(g_ScreenHeight) );
	g_RenderScene->InitGbuffer( g_ScreenWidth, g_ScreenHeight );
	glViewport( 0, 0, g_ScreenWidth, g_ScreenHeight );


// Send the new window size to AntTweakBar
TwWindowSize( width, height );

}



void Idle()
{
	glutPostRedisplay();
}



int main(int argc, char **argv) 
{
	 _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Ant TweakBar
	TwBar *bar; // Pointer to the tweak bar




	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS );
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
	glutInitWindowSize( g_ScreenWidth, g_ScreenHeight );		// set window size
	glutCreateWindow( "RenderSceneGraph" );						// create Window
	glewInit();
	glutDisplayFunc(display);									// register Display Function
	glutReshapeFunc(reshape);									// register Reshape Function
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
    TwGLUTModifiersFunc(glutGetModifiers);

	initialize();


TwInit(TW_OPENGL, NULL);


bar = TwNewBar("Main Menu");

	TwDefine(" 'Main Menu' size='300 400' color='96 216 224' "); // change default tweak bar size and color

	TwAddVarRW( bar, "SourceTemperature", TW_TYPE_FLOAT, &g_SrcTemperature, " label='SourceTemperature' min=1000.0 max=40000.0 step=1.0 keyIncr=z keyDecr=Z " );
	
	TwAddVarRW( bar, "TargetTemperature", TW_TYPE_FLOAT, &g_TgtTemperature, " label='TargetTemperature' min=1000.0 max=40000.0 step=1.0 keyIncr=z keyDecr=Z " );
	
	TwAddVarRW( bar, "Strength", TW_TYPE_FLOAT, &g_Strength, " label='Strength' min=0.0 max=1.0 step=0.01 keyIncr=z keyDecr=Z " );

	TwAddVarRW( bar, "PreserveLuminance", TW_TYPE_BOOLCPP, &g_bPreserveLuminance, " label='PreserveLuminance' " );


	// Material Params
	//TwAddVarRW( bar, "BaseColor", TW_TYPE_COLOR3F, &g_BaseColor3f, " label='BaseColor' colormode=hls " );
	//TwAddVarRW( bar, "Reflection", TW_TYPE_COLOR3F, &g_Reflection3f, " label='Reflection' colormode=hls " );


	//TwAddVarRW( bar, "EdgeSmoothness", TW_TYPE_FLOAT, g_ProcMat.EdgeSmoothness(),
	//			" label='EdgeSmoothness' min=0.0 max=100.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Amplitude1", TW_TYPE_FLOAT, g_ProcMat.Amplitude(0),
	//			" label='Amplitude1' min=0.0 max=10.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Stretch1", TW_TYPE_FLOAT, g_ProcMat.Stretch(0),
	//			" label='Stretch1' min=0.0 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Roughness1", TW_TYPE_FLOAT, g_ProcMat.Roughness(0),
	//			" label='Roughness1' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Amplitude2", TW_TYPE_FLOAT, g_ProcMat.Amplitude(1),
	//			" label='Amplitude2' min=0.0 max=10.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Stretch2", TW_TYPE_FLOAT, g_ProcMat.Stretch(1),
	//			" label='Stretch2' min=0.0 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Roughness2", TW_TYPE_FLOAT, g_ProcMat.Roughness(1),
	//			" label='Roughness2' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");


	//TwAddVarRW( bar, "ScaleX", TW_TYPE_FLOAT, &g_ProcMat2.Scale.x,
	//			" label='ScaleX' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "ScaleY", TW_TYPE_FLOAT, &g_ProcMat2.Scale.y,
	//			" label='ScaleY' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "ScaleZ", TW_TYPE_FLOAT, &g_ProcMat2.Scale.z,
	//			" label='ScaleZ' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");


	//TwAddVarRW( bar, "Bin", TW_TYPE_FLOAT, &g_ProcMat2.Bin,
	//			" label='Bin' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Gap", TW_TYPE_FLOAT, &g_ProcMat2.Gap,
	//			" label='Gap' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Min", TW_TYPE_FLOAT, &g_ProcMat2.Min,
	//			" label='Min' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");

	//TwAddVarRW( bar, "Max", TW_TYPE_FLOAT, &g_ProcMat2.Max,
	//			" label='Max' min=0.01 max=10000.0 step=0.001 keyIncr=z keyDecr=Z ");


	
//	static float aaa = 0.0f;
	
//	TwAddVarRW( bar, "test", TW_TYPE_FLOAT, &aaa,
//				" label='test' min=0.01 max=10000.0 step=0.02 ");
	
//	TwAddVarRW( bar, "Param4", TW_TYPE_DIR3F, g_refProcParam->Param4.xyz,
//				" label='Param4' opened=true help='Change the direction.' " );

	TwWindowSize( g_ScreenWidth, g_ScreenHeight);




	// run GLUT mainloop
	glutMainLoop();							


	// terminate AntTweakBar
	TwTerminate();


	// release all allocated data
	SafeDelete( g_SceneGraph );
	SafeDelete( g_RenderScene );


	return 0;
}







//#include	<iostream>
//using namespace std;
//
//
//#include	"SceneGraph.h"	// SceneGraphクラス
//using namespace OreOreLib;
//
//
//SceneGraph	*g_SceneGraph	= NULL;	// SceneGraphオブジェクト
//
//
//
//int main()
//{
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// メモリリークをチェックする
//
//
//	//===================== シーングラフのセットアップ ======================//
//	// Init VertexBuffer
//	VertexBuffer vb;
//	AllocateVertexBuffer( &vb, 4 );
//
//	InitVec( *vb.Position(0), -1.0f, -1.0f, 0.0f );
//	InitVec( *vb.Normal(0), 0.0f, 1.0f, 0.0f );
//	InitVec( *vb.TexCoord(0), 0.0f, 0.0f );
//
//	InitVec( *vb.Position(1), +1.0f, -1.0f, 0.0f );
//	InitVec( *vb.Normal(1), 0.0f, 1.0f, 0.0f );
//	InitVec( *vb.TexCoord(1), 1.0f, 0.0f );
//	
//	InitVec( *vb.Position(2), +1.0f, +1.0f, 0.0f );
//	InitVec( *vb.Normal(2), 0.0f, 1.0f, 0.0f );
//	InitVec( *vb.TexCoord(2), 1.0f, 1.0f );
//	
//	InitVec( *vb.Position(3), -1.0f, +1.0f, 0.0f );
//	InitVec( *vb.Normal(3), 0.0f, 1.0f, 0.0f );
//	InitVec( *vb.TexCoord(3), 0.0f, 1.0f );
//
//	
//	// Init IndexBuffer
//	IndexBuffer ib;
//	AllocateIndexBuffer( &ib, 2, 3 );
//
//	*ib.Index(0) = 0;
//	*ib.Index(1) = 1;
//	*ib.Index(2) = 2;
//	
//	*ib.Index(3) = 3;
//	*ib.Index(4) = 0;
//	*ib.Index(5) = 2;
//	
//	// Init MaterialBuffer
//	MaterialBuffer	mb;
//	AllocateMaterialBuffer( &mb, 1, MTL_OPAQUE );
//	InitVec( *mb.Diffuse(0), 0.6f, 0.5f, 0.3f, 1.01f );
//	
//	
//	// Setup Scenegraph
//	g_SceneGraph = new SceneGraph();
//	
//	// add mesh object
//	TransformAttribute	tparam = {0};
//	
//	InitVec( tparam.Position, 0.0f, 0.0f, 1.0f );
//	InitVec( tparam.Translation, 0.0f, 0.0f, 0.0f );
//	MatScaling( tparam.matScale, 1.0f, 1.0f, 1.0f );
//	MatRotationX( tparam.matRotation, float(0.25f * M_PI) );
//
//	MeshAttribute attrib = { SHDR_SIMPLE_MESH };
//
//	g_SceneGraph->AddMeshObject( attrib, &vb, &ib, &mb, NULL, &tparam );
//	g_SceneGraph->GetCurrentMesh()->SetPosition( tparam.Position );
//
//	g_SceneGraph->AddChildMesh( attrib, &vb, &ib, &mb, NULL, &tparam );
//
//	g_SceneGraph->AddChildMesh( attrib, &vb, &ib, &mb, NULL, &tparam );
//
//	// add camera object
//	//CameraAttribute cam_attr = {0};
//	g_SceneGraph->AddCameraObject( NULL );
//	g_SceneGraph->GetCurrentCameraObject()->SetViewParams( 5,0,4, -5,0,-4, 0,0,1 );
//	g_SceneGraph->GetCurrentCameraObject()->SetProjectionParams( float(M_PI_4), (float)(640)/(float)(480), 1.0e-3f, 1.0e+2f );
//
//	
//	// add light object
//	LightAttribute lgt_attr = { LGT_DIRECTIONAL, true, true };
//
//	g_SceneGraph->AddLightObject( &lgt_attr, NULL );
//	//g_SceneGraph->GetCurrentLight()->SetPosition();
//
//	ReleaseVertexBuffer( &vb );
//	ReleaseIndexBuffer( &ib );
//	ReleaseMaterialBuffer( &mb );
//	
//	g_SceneGraph->TraverseMesh();
//
//	SafeDelete( g_SceneGraph );
//
//	return 0;
//}