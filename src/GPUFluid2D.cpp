#include "GPUFluid2D.h"

#include	<graphics/gl4x/common/GLHelperFunctions.h>



namespace OreOreLib
{


	const float GPUFluid2D::SplatRadius					= 32.0f;
	const float GPUFluid2D::CellSize						= 1.0f;//1.25f;//1.25f;
	const float GPUFluid2D::DEFAULT_AMBIENT_TEMPERATURE	= 0.0f;
	const float GPUFluid2D::DEFAULT_IMPULSE_TEMPERATURE	= 10.0f;
	const float GPUFluid2D::DEFAULT_IMPULSE_DENSITY		= 1.0f;
	const int GPUFluid2D::DEFAULT_ITERATIONS				= 40;
	const float GPUFluid2D::DEFAULT_TIME_STEP				= 0.1f;//0.05f;//0.125f;
	const float GPUFluid2D::DEFAULT_SMOKE_BUOYANCY			= 1.0f;
	const float GPUFluid2D::DEFAULT_SMOKE_WEIGHT			= 0.05f;
	const float GPUFluid2D::DEFAULT_GRADIENT_SCALE			= 0.5f / GPUFluid2D::CellSize;//1.125f / CellSize;
	const float GPUFluid2D::DEAFAULT_TEMPERATURE_DISSIPATION	= 1.0f;//0.9999f;//
	const float GPUFluid2D::DEAFAULT_VELOCITY_DISSIPATION		= 1.0f;//0.9999f;//
	const float GPUFluid2D::DEAFAULT_DENSITY_DISSIPATION		= 1.0f;//0.9999f;//
	const float GPUFluid2D::DEFAULT_VORTICITY				= 0.5f;//0.0001f;//
	const float GPUFluid2D::DEFAULT_VISCOSITY				= 0.01f;


	//static const Vec2f ImpulsePosition			= { 512.0f, 32.0f };//{ GridWidth / 2.0f, - (int) SplatRadius / 2.0f};
	//static const float g_Eps	= 0.5f;



	// Default Constructor
	GPUFluid2D::GPUFluid2D()
	{
		m_refFluidVoxelData	= NULL;
		InitZero( m_DispatchGroups );

		m_TimeStep						= DEFAULT_TIME_STEP;
		m_NumIterations					= DEFAULT_ITERATIONS;
		m_AmbientTemperature			= DEFAULT_AMBIENT_TEMPERATURE;
		m_ImpulseTemperature			= DEFAULT_IMPULSE_TEMPERATURE;
		m_ImpulseDensity				= DEFAULT_IMPULSE_DENSITY;
		m_SmokeBuoyancy					= DEFAULT_SMOKE_BUOYANCY;
		m_SmokeWeight					= DEFAULT_SMOKE_WEIGHT;
		m_GradientScale					= DEFAULT_GRADIENT_SCALE;
		m_TemperatureDissipation		= DEAFAULT_TEMPERATURE_DISSIPATION;
		m_VelocityDissipation			= DEAFAULT_VELOCITY_DISSIPATION;
		m_DensityDissipation			= DEAFAULT_DENSITY_DISSIPATION;
		m_bEnableMacCormackAdvection	= false;
		m_bEnableBuoyancy				= false;

		m_Vorticity						= DEFAULT_VORTICITY;
		m_Viscosity						= DEFAULT_VISCOSITY;
	}



	// Destructor
	GPUFluid2D::~GPUFluid2D()
	{
		Release();
	}



	void GPUFluid2D::InitShader()
	{
		GLuint	program_id;


		//=======================	FLUID_PASS_ADVECT Pass	========================//
		// Create shader
		m_Pass[ FLUID_PASS_ADVECT ].Init( _T("../assets/shader/GPUFluid_Advect.glsl"), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_ADVECT ].ID();

		// Init Uniform Location
		m_ulTimeStep		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TimeStep" ) );
		m_ulDissipation		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Dissipation" ) );

		// Link shader
		m_Pass[ FLUID_PASS_ADVECT ].Link();

		// Setup Texture Unit
		m_Pass[ FLUID_PASS_ADVECT ].Bind();
		{
			GL_SAFE_CALL( glUniform1i( glGetUniformLocation( program_id, "g_SourceTexture" ), 0 ) );
		}
		m_Pass[ FLUID_PASS_ADVECT ].Unbind();

		

		//=======================	FLUID_PASS_JACOBI Pass	========================//
		// Create shader
		m_Pass[FLUID_PASS_JACOBI].Init( _T( "../assets/shader/GPUFluid_Jacobi.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_JACOBI ].ID();

		// Init Uniform Location
		m_ulAlpha			= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Alpha" ) );
		m_ulInverseBeta		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_InverseBeta" ) );

		// Link shader
		m_Pass[ FLUID_PASS_JACOBI ].Link();

		// Setup Texture Unit
		m_Pass[ FLUID_PASS_JACOBI ].Bind();
		{
			GL_SAFE_CALL( glUniform1i( glGetUniformLocation( program_id, "g_x" ), 0 ) );
			GL_SAFE_CALL( glUniform1i( glGetUniformLocation( program_id, "g_b" ), 1 ) );
		}
		m_Pass[ FLUID_PASS_JACOBI ].Unbind();



		//======================	FLUID_PASS_SUBTRACT_GRADIENT Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_SUBTRACT_GRADIENT].Init( _T( "../assets/shader/GPUFluid_SubtractGradient.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_SUBTRACT_GRADIENT ].ID();

		// Init Uniform Location
		m_ulGradientScale	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_GradientScale" ) );
		//m_ulHalfCell		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_HalfInverseCellSize" ) );

		// Link shader
		m_Pass[ FLUID_PASS_SUBTRACT_GRADIENT ].Link();

		// Setup Texture Unit
		//m_Pass[ FLUID_PASS_SUBTRACT_GRADIENT ].Bind();
		//{
		//	//glUniform1i( glGetUniformLocation( program_id, "g_Sampler1" ), 0 );
		//}
		//m_Pass[ FLUID_PASS_SUBTRACT_GRADIENT ].Unbind();



		//======================	FLUID_PASS_COMPUTE_DIVERGENCE Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_COMPUTE_DIVERGENCE].Init( _T( "../assets/shader/GPUFluid_ComputeDivergence.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_COMPUTE_DIVERGENCE ].ID();

		// Init Uniform Location
		m_ulHalfCell		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_HalfInverseCellSize" ) );

		// Link shader
		m_Pass[ FLUID_PASS_COMPUTE_DIVERGENCE ].Link();

		// Setup Texture Unit
		//m_Pass[ FLUID_PASS_COMPUTE_DIVERGENCE ].Bind();
		//{
		//	//glUniform1i( glGetUniformLocation( program_id, "g_Sampler1" ), 0 );
		//}
		//m_Pass[ FLUID_PASS_COMPUTE_DIVERGENCE ].Unbind();



		//======================	FLUID_PASS_APPLY_IMPULSE Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_APPLY_IMPULSE].Init( _T( "../assets/shader/GPUFluid_ApplyImpulse.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_APPLY_IMPULSE ].ID();

		// Init Uniform Location
		m_ulPoint			= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Point" ) );
		m_ulRadius			= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Radius" ) );
		m_ulFillColor		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_FillColor" ) );
		m_ulTexSize			= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TexSize" ) );

		// Link shader
		m_Pass[ FLUID_PASS_APPLY_IMPULSE ].Link();

		// Setup Texture Unit
		//m_Pass[ FLUID_PASS_APPLY_IMPULSE ].Bind();
		//{
		//	//glUniform1i( glGetUniformLocation( program_id, "g_Sampler1" ), 0 );
		//}
		//m_Pass[ FLUID_PASS_APPLY_IMPULSE ].Unbind();



		//======================	FLUID_PASS_APPLY_BUOYANCY Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_APPLY_BUOYANCY].Init( _T( "../assets/shader/GPUFluid_ApplyBuoyancy.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_APPLY_BUOYANCY ].ID();

		// Init Uniform Location
		m_ulAmbTemp				= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_AmbientTemperature" ) );
		m_ulTimeStep_BOUYANCY	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TimeStep" ) );
		m_ulSigma				= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Sigma" ) );
		m_ulKappa				= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Kappa" ) );

		// Link shader
		m_Pass[ FLUID_PASS_APPLY_BUOYANCY ].Link();

		// Setup Texture Unit
		//m_Pass[ FLUID_PASS_APPLY_BUOYANCY ].Bind();
		//{
		//	//glUniform1i( glGetUniformLocation( program_id, "g_Sampler1" ), 0 );
		//}
		//m_Pass[ FLUID_PASS_APPLY_BUOYANCY ].Unbind();
		


		//======================	FLUID_PASS_FILL Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_FILL].Init( _T( "../assets/shader/GPUFluid_Fill.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_FILL ].ID();

		// Init Uniform Location
		m_ulPoint_FILL		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Point" ) );
		m_ulRadius_FILL		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Radius" ) );
		m_ulFillColor_FILL	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_FillColor" ) );
		m_ulTexSize_FILL	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TexSize" ) );

		// Link shader
		m_Pass[ FLUID_PASS_FILL ].Link();

		// Setup Texture Unit
		//m_Pass[ FLUID_PASS_FILL ].Bind();
		//{
		//	//glUniform1i( glGetUniformLocation( program_id, "g_Sampler1" ), 0 );
		//}
		//m_Pass[ FLUID_PASS_FILL ].Unbind();



		//======================	FLUID_PASS_ADVECT_MACCORMACK Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_ADVECT_MACCORMACK].Init( _T( "../assets/shader/GPUFluid_AdvectMacCormack.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_ADVECT_MACCORMACK ].ID();

		// Init Uniform Location
		m_ulTimeStep_MACCORMACK		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TimeStep" ) );
		m_ulDissipation_MACCORMACK	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Dissipation" ) );

		// Link shader
		m_Pass[ FLUID_PASS_ADVECT_MACCORMACK ].Link();

		// Setup Texture Unit
		m_Pass[ FLUID_PASS_ADVECT_MACCORMACK ].Bind();
		{
			GL_SAFE_CALL( glUniform1i( glGetUniformLocation( program_id, "g_Phi_n" ), 0 ) );
			GL_SAFE_CALL( glUniform1i( glGetUniformLocation( program_id, "g_Phi_n_hat" ), 1 ) );
		}
		m_Pass[ FLUID_PASS_ADVECT_MACCORMACK ].Unbind();



		//======================	FLUID_PASS_COMPUTE_CURL Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_COMPUTE_CURL].Init( _T( "../assets/shader/GPUFluid_ComputeCurl.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_COMPUTE_CURL ].ID();

		// Init Uniform Location
		m_ulHalfInverseCellSize_CURL	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_HalfInverseCellSize" ) );
		m_ulTimeStep_CURL				= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TimeStep" ) );

		// Link shader
		m_Pass[ FLUID_PASS_COMPUTE_CURL ].Link();



		//======================	FLUID_PASS_VORTICITY_CONFINEMENT Pass	======================//
		// Create shader
		m_Pass[FLUID_PASS_VORTICITY_CONFINEMENT].Init( _T( "../assets/shader/GPUFluid_VorticityConfinement.glsl" ), GLSL_VERSION::GLSL_430 );
		program_id	= m_Pass[ FLUID_PASS_VORTICITY_CONFINEMENT ].ID();

		// Init Uniform Location
		m_ulEps_VORTICITY		= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_Eps" ) );
		m_ulCellSize_VORTICITY	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_CellSize" ) );
		m_ulTimeStep_VORTICITY	= GL_SAFE_CALL( glGetUniformLocation( program_id, "g_TimeStep" ) );

		// Init Output Fragment Location


		// Link shader
		m_Pass[ FLUID_PASS_VORTICITY_CONFINEMENT ].Link();


	}
	


	void GPUFluid2D::Release()
	{
		UnbindVoxelData();

		for( int i=0; i<NUM_FLUID_PASS; ++i )
			m_Pass[i].Release();
		

		m_Phi_n_1_hat.Release();
		m_Phi_n_hat.Release();

		m_Curl.Release();
	}
	


	void GPUFluid2D::BindVoxelData( FluidVolume2D* pVoxelData )
	{
		m_refFluidVoxelData	= pVoxelData;
		InitVec( m_DispatchGroups, (uint32)DivUp( m_refFluidVoxelData->Width(), 8 ), (uint32)DivUp( m_refFluidVoxelData->Height(), 8 ), (uint32)1 );

		// TODO: Experimental buffer for MacCormack Advection
		m_Phi_n_1_hat.Allocate( m_refFluidVoxelData->Width(), m_refFluidVoxelData->Height(), 0, 0, FORMAT_R16G16_FLOAT );
		m_Phi_n_1_hat.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_Phi_n_1_hat.GenHardwareTexture();

		m_Phi_n_hat.Allocate( m_refFluidVoxelData->Width(), m_refFluidVoxelData->Height(), 0, 0, FORMAT_R16G16_FLOAT );
		m_Phi_n_hat.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_Phi_n_hat.GenHardwareTexture();

		m_Curl.Allocate( m_refFluidVoxelData->Width(), m_refFluidVoxelData->Height(), 0, 0, FORMAT_R16_FLOAT );
		m_Curl.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_Curl.GenHardwareTexture();


		uint32 texids[] =
		{
			m_refFluidVoxelData->m_TexVelocity.FrontBuffer()->TexID(),
			m_refFluidVoxelData->m_TexVelocity.BackBuffer()->TexID(),

			m_refFluidVoxelData->m_TexDensity.FrontBuffer()->TexID(),
			m_refFluidVoxelData->m_TexDensity.BackBuffer()->TexID(),

			m_refFluidVoxelData->m_TexPressure.FrontBuffer()->TexID(),
			m_refFluidVoxelData->m_TexPressure.BackBuffer()->TexID(),

			m_refFluidVoxelData->m_TexTemperature.FrontBuffer()->TexID(),
			m_refFluidVoxelData->m_TexTemperature.BackBuffer()->TexID(),

			m_refFluidVoxelData->m_TexDivergence.TexID(),
			m_refFluidVoxelData->m_TexObstacles.TexID(),

			m_Phi_n_1_hat.TexID(),
			m_Phi_n_hat.TexID(),
			m_Curl.TexID(),
		};

		uint32 attachments[] =
		{
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,

			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,

			GL_COLOR_ATTACHMENT4,
			GL_COLOR_ATTACHMENT5,

			GL_COLOR_ATTACHMENT6,
			GL_COLOR_ATTACHMENT7,

			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,

			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4,
		};


		m_Attachment.Init( ArraySize( attachments ), attachments, texids );
		m_Fbo.Init( m_refFluidVoxelData->Width(), m_refFluidVoxelData->Height(), false );

		ClearBuffer( FluidVolume2D::TEX_FLUID_VELOCITY, 0.0f );
		ClearBuffer( FluidVolume2D::TEX_FLUID_VELOCITY_BACK, 0.0f );

		ClearBuffer( FluidVolume2D::TEX_FLUID_DENSITY, 0.0f );
		ClearBuffer( FluidVolume2D::TEX_FLUID_DENDITY_BACK, 0.0f );

		ClearBuffer( FluidVolume2D::TEX_FLUID_PRESSURE, 0.0f );
		ClearBuffer( FluidVolume2D::TEX_FLUID_PRESSURE_BACK, 0.0f );

		ClearBuffer( FluidVolume2D::TEX_FLUID_TEMPERATURE, m_AmbientTemperature );
		ClearBuffer( FluidVolume2D::TEX_FLUID_TEMPERATURE_BACK, m_AmbientTemperature );

		ClearBuffer( FluidVolume2D::TEX_FLUID_DIVERGENCE, 0.0f );
		ClearBuffer( FluidVolume2D::TEX_FLUID_OBSTACLES, 0.0f );

		ClearBuffer( 10, 0.0f );
		ClearBuffer( 11, 0.0f );
		ClearBuffer( 12, 0.0f );

	}



	void GPUFluid2D::UnbindVoxelData()
	{
		m_refFluidVoxelData	= NULL;
		InitZero( m_DispatchGroups );

		m_Attachment.Clear();
		m_Fbo.Release();
	}



	void GPUFluid2D::AddObstacles( int x, int y, float radius )
	{
		const Vec2f pos	= { float(x), float(y) };
		const Vec3f color	= { 1.0f, 0.0f, 0.0f };
		Fill( &m_refFluidVoxelData->m_TexObstacles, pos, color, radius );
	}
	
	
	
	void GPUFluid2D::RemoveObstacles( int x, int y, float radius )
	{
		const Vec2f pos		= { float(x), float(y) };
		const Vec3f color	= { 0.0f, 0.0f, 0.0f };
		Fill( &m_refFluidVoxelData->m_TexObstacles, pos, color, radius );
	}



	void GPUFluid2D::AddSmoke( int x, int y, float dx, float dy, float radius )
	{
		const Vec2f pos	= { float(x), float(y) };
		const Vec3f color	= { dx, dy, 0.0f };

		//tcout << dx << ", " << dy << tendl;
		//if( m_bEnableBuoyancy )
			ApplyImpulse( m_refFluidVoxelData->m_TexTemperature.BackBuffer(), pos, m_ImpulseTemperature, radius );
		ApplyImpulse( m_refFluidVoxelData->m_TexDensity.BackBuffer(), pos, m_ImpulseDensity, radius );
		
		
		Fill( m_refFluidVoxelData->m_TexVelocity.BackBuffer(), pos, color, radius );
	}




	void GPUFluid2D::Update()
	{
		if( !m_refFluidVoxelData )
			return;

		static SwapTexture2D	*pTexVelocity		= &m_refFluidVoxelData->m_TexVelocity,
			*pTexDensity		= &m_refFluidVoxelData->m_TexDensity,
			*pTexPressure		= &m_refFluidVoxelData->m_TexPressure,
			*pTexTemperature	= &m_refFluidVoxelData->m_TexTemperature;

		static Texture2D		*pTexDivergence		= &m_refFluidVoxelData->m_TexDivergence,
			*pTexObstacles		= &m_refFluidVoxelData->m_TexObstacles;


		//ApplyImpulse( pTexTemperature->BackBuffer(), ImpulsePosition, m_ImpulseTemperature, SplatRadius );
		//ApplyImpulse( pTexDensity->BackBuffer(), ImpulsePosition, m_ImpulseDensity, SplatRadius );



		if( m_bEnableBuoyancy )
		{
			ApplyBuoyancy( pTexVelocity->BackBuffer(), pTexTemperature->BackBuffer(), pTexDensity->BackBuffer(), pTexVelocity->FrontBuffer() );
			pTexVelocity->Swap();
		}

		//======================	Vorticity Confinement	===============================//
		ComputeCurl( pTexVelocity->BackBuffer(), &m_Curl, m_TimeStep );
		ApplyVorticityConfinement( pTexVelocity->BackBuffer(), &m_Curl, pTexVelocity->FrontBuffer(), m_TimeStep );
		pTexVelocity->Swap();


		//=================================	Advection	===============================//
		if( m_bEnableMacCormackAdvection )
		{
			AdvectMacCormack( pTexVelocity->BackBuffer(), pTexVelocity->BackBuffer(), pTexObstacles, pTexVelocity->FrontBuffer(), m_VelocityDissipation, m_TimeStep );
			pTexVelocity->Swap();

			//AdvectMacCormack( pTexVelocity->BackBuffer(), pTexTemperature->BackBuffer(), pTexObstacles, pTexTemperature->FrontBuffer(), m_TemperatureDissipation, m_TimeStep );
			//pTexTemperature->Swap();

			//AdvectMacCormack( pTexVelocity->BackBuffer(), pTexDensity->BackBuffer(), pTexObstacles, pTexDensity->FrontBuffer(), m_DensityDissipation, m_TimeStep );
			//pTexDensity->Swap();
		}
		else
		{
			Advect( pTexVelocity->BackBuffer(), pTexVelocity->BackBuffer(), pTexObstacles, pTexVelocity->FrontBuffer(), m_VelocityDissipation, m_TimeStep );
			pTexVelocity->Swap();

			//Advect( pTexVelocity->BackBuffer(), pTexTemperature->BackBuffer(), pTexObstacles, pTexTemperature->FrontBuffer(), m_TemperatureDissipation, m_TimeStep );
			//pTexTemperature->Swap();

			//Advect( pTexVelocity->BackBuffer(), pTexDensity->BackBuffer(), pTexObstacles, pTexDensity->FrontBuffer(), m_DensityDissipation, m_TimeStep );
			//pTexDensity->Swap();
		}


		//================================	Diffusion	===============================//
		if( m_Viscosity * m_TimeStep > 1.0e-6 )
		{
			const float alpha	= CellSize * CellSize / ( m_Viscosity * m_TimeStep );	// DIFFUSION_ALPHA = 1.0 / (VISCOSITY * DT);	// CellSize=1.0の場合を想定?
			const float invbeta	= 1.0f / ( 4.0f + alpha );		// DIFFUSION_BETA_RECIPROCAL = 1.0 / (4.0 + DIFFUSION_ALPHA);	
			//Diffuse( pTexVelocity, pTexObstacles, m_NumIterations, alpha, invbeta );
			Jacobi( pTexVelocity, pTexVelocity->BackBuffer(), pTexObstacles, m_NumIterations, alpha, invbeta );
		}


		//===============================	Projection	================================//
		ComputeDivergence( pTexVelocity->BackBuffer(), pTexObstacles, pTexDivergence );
		ClearBuffer( FluidVolume2D::TEX_FLUID_PRESSURE_BACK, 0.0f );
		
		Jacobi( pTexPressure, pTexDivergence, pTexObstacles, m_NumIterations, -CellSize * CellSize, 0.25f );

		SubtractGradient( pTexVelocity->BackBuffer(), pTexPressure->BackBuffer(), pTexObstacles, pTexVelocity->FrontBuffer() );
		pTexVelocity->Swap();

		
		//if( m_bEnableMacCormackAdvection )
		//{
		//	AdvectMacCormack( pTexVelocity->BackBuffer(), pTexTemperature->BackBuffer(), pTexObstacles, pTexTemperature->FrontBuffer(), m_TemperatureDissipation, m_TimeStep );
		//	pTexTemperature->Swap();

		//	AdvectMacCormack( pTexVelocity->BackBuffer(), pTexDensity->BackBuffer(), pTexObstacles, pTexDensity->FrontBuffer(), m_DensityDissipation, m_TimeStep );
		//	pTexDensity->Swap();
		//}
		//else
		{
			Advect( pTexVelocity->BackBuffer(), pTexTemperature->BackBuffer(), pTexObstacles, pTexTemperature->FrontBuffer(), m_TemperatureDissipation, m_TimeStep );
			pTexTemperature->Swap();

			Advect( pTexVelocity->BackBuffer(), pTexDensity->BackBuffer(), pTexObstacles, pTexDensity->FrontBuffer(), m_DensityDissipation, m_TimeStep );
			pTexDensity->Swap();
		}
		
	}


	
	void GPUFluid2D::Advect( Texture2D* velocity, Texture2D* source, Texture2D* obstacles, Texture2D* dest, float dissipation, float timestep )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );	// g_VelocityTexture
		GL_SAFE_CALL( glBindImageTexture( 1, obstacles->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, obstacles->GLInternalFormat() ) );// g_Obstacles

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 2, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_ADVECT ].Bind();
		{
			// Set Textures
			glActiveTexture( GL_TEXTURE0 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, source->TexID() ) );

			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulTimeStep, timestep ) );
			GL_SAFE_CALL( glUniform1f( m_ulDissipation, dissipation ) );
			
			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_ADVECT ].Unbind();


	}
	
	
	
	void GPUFluid2D::Jacobi( Texture2D* x, Texture2D* b, Texture2D* obstacles, Texture2D* dest, float alpha, float invbeta )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, obstacles->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, obstacles->GLInternalFormat() ) );// g_TexObstacles
		
		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 1, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest

	
		// Execute ComputeShader
		m_Pass[ FLUID_PASS_JACOBI ].Bind();
		{
			// Set Textures
			glActiveTexture( GL_TEXTURE0 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, x->TexID() ) );

			glActiveTexture( GL_TEXTURE1 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, b->TexID() ) );

			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulAlpha, alpha ) );
			GL_SAFE_CALL( glUniform1f( m_ulInverseBeta, invbeta ) );
			
			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_JACOBI ].Unbind();
	}



	void GPUFluid2D::Jacobi( SwapTexture2D* x, Texture2D* b, Texture2D* obstacles, int numIter, float alpha, float invbeta )
	{

		// Execute ComputeShader
		m_Pass[ FLUID_PASS_JACOBI ].Bind();
		{
			// Bind Reference Buffers 
			GL_SAFE_CALL( glBindImageTexture( 0, obstacles->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, obstacles->GLInternalFormat() ) );// g_TexObstacles

			// Set Textures
			glActiveTexture( GL_TEXTURE1 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, b->TexID() ) );

			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulAlpha, alpha ) );
			GL_SAFE_CALL( glUniform1f( m_ulInverseBeta, invbeta ) );


			for( int itr=0; itr<numIter; ++itr )
			{
				// Bind Ping/Pong Buffers
				GL_SAFE_CALL( glBindImageTexture( 1, x->FrontBuffer()->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, x->GLInternalFormat() ) );	// g_TexDest

				// Set Textures
				glActiveTexture( GL_TEXTURE0 );
				GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, x->BackBuffer()->TexID() ) );

				// Dispatch Compute Shader
				GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );


				// Swap Ping/Pong Buffer
				x->Swap();
		
			}// end of itr loop


		}
		m_Pass[ FLUID_PASS_JACOBI ].Unbind();

	}



	void GPUFluid2D::SubtractGradient( Texture2D* velocity, Texture2D* pressure, Texture2D* obstacles, Texture2D* dest )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );	// g_TexVelocity
		GL_SAFE_CALL( glBindImageTexture( 1, pressure->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, pressure->GLInternalFormat() ) );// g_TexPressure
		GL_SAFE_CALL( glBindImageTexture( 2, obstacles->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, obstacles->GLInternalFormat() ) );// g_TexObstacles

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 3, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_SUBTRACT_GRADIENT ].Bind();
		{
			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulGradientScale, m_GradientScale ) );
			//GL_SAFE_CALL( glUniform1f( m_ulHalfCell, 0.5f / CellSize ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_SUBTRACT_GRADIENT ].Unbind();

	}
	
	
	
	void GPUFluid2D::ComputeDivergence( Texture2D* velocity, Texture2D* obstacles, Texture2D* dest )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );	// g_TexVelocity
		GL_SAFE_CALL( glBindImageTexture( 1, obstacles->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, obstacles->GLInternalFormat() ) );// g_TexObstacles

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 2, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_COMPUTE_DIVERGENCE ].Bind();
		{
			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulHalfCell, 0.5f / CellSize ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_COMPUTE_DIVERGENCE ].Unbind();
	
	}
	
	
	
	void GPUFluid2D::ApplyImpulse( Texture2D* dest, const Vec2f& position, float value, float radius )
	{
		// Bind Inputs


		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 0, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_APPLY_IMPULSE ].Bind();
		{
			// Set Unforms
			GL_SAFE_CALL( glUniform2fv( m_ulPoint, 1, position.xy ) );
			GL_SAFE_CALL( glUniform1f( m_ulRadius, radius ) );
			GL_SAFE_CALL( glUniform3f( m_ulFillColor, value, value, value ) );
			GL_SAFE_CALL( glUniform4f( m_ulTexSize, (float)dest->Width(), (float)dest->Height(), 1.0f/(float)dest->Width(), 1.0f/(float)dest->Height() ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_APPLY_IMPULSE ].Unbind();
	
	}
	
	
	
	void GPUFluid2D::ApplyBuoyancy( Texture2D* velocity, Texture2D* temperature, Texture2D* density, Texture2D* dest )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );	// g_TexVelocity
		GL_SAFE_CALL( glBindImageTexture( 1, temperature->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, temperature->GLInternalFormat() ) );// g_TexTemperature
		GL_SAFE_CALL( glBindImageTexture( 2, density->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, density->GLInternalFormat() ) );	// g_TexDensity

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 3, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_APPLY_BUOYANCY ].Bind();
		{
			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulAmbTemp, m_AmbientTemperature ) );
			GL_SAFE_CALL( glUniform1f( m_ulTimeStep_BOUYANCY, m_TimeStep ) );
			GL_SAFE_CALL( glUniform1f( m_ulSigma, m_SmokeBuoyancy ) );
			GL_SAFE_CALL( glUniform1f( m_ulKappa, m_SmokeWeight ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_APPLY_BUOYANCY ].Unbind();
	
	}
	


	void GPUFluid2D::Fill( Texture2D* dest, const Vec2f& position, const Vec3f& fillVal, float radius )
	{
		// Bind Inputs


		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 0, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );	// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_FILL ].Bind();
		{
			// Set Unforms
			GL_SAFE_CALL( glUniform2fv( m_ulPoint_FILL, 1, position.xy ) );
			GL_SAFE_CALL( glUniform1f( m_ulRadius_FILL, radius ) );
			GL_SAFE_CALL( glUniform3fv( m_ulFillColor_FILL, 1, fillVal.xyz ) );
			GL_SAFE_CALL( glUniform4f( m_ulTexSize_FILL, (float)dest->Width(), (float)dest->Height(), 1.0f/(float)dest->Width(), 1.0f/(float)dest->Height() ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_FILL ].Unbind();
	
	}



	void GPUFluid2D::ClearBuffer( int tex_channel, float val )
	{

		m_Fbo.Bind();
		{
//GLErrorCheck();
			m_Attachment.BindAttachment( 1, tex_channel );
			m_Attachment.DrawBuffers( 1, tex_channel );
	
			glClearColor( val, val, val, val );
			glClear(GL_COLOR_BUFFER_BIT);

		}
		m_Fbo.Unbind();
//GLErrorCheck();
	}



	void GPUFluid2D::AdvectMacCormack( Texture2D* velocity, Texture2D* source, Texture2D* obstacles, Texture2D* dest, float dissipation, float timestep )
	{

		//===============	m_Phi_n_1_hatを計算する	===================//
		Advect( velocity, source, obstacles, &m_Phi_n_1_hat, 1.0f, timestep );


		//===============	m_Phi_n_hatを計算する	===================//
		Advect( velocity, &m_Phi_n_1_hat, obstacles, &m_Phi_n_hat, 1.0f, -timestep );


		//===============	補正したベクトル場を計算する	===============//

		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );// g_TexVelocity

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 1, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_ADVECT_MACCORMACK ].Bind();
		{
			// Set Textures
			glActiveTexture( GL_TEXTURE0 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, source->TexID() ) );// g_Phi_n

			glActiveTexture( GL_TEXTURE1 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, m_Phi_n_hat.TexID() ) );// g_Phi_n_hat
			
			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulTimeStep_MACCORMACK, timestep ) );
			GL_SAFE_CALL( glUniform1f( m_ulDissipation_MACCORMACK, dissipation ) );
			
			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		
		}
		m_Pass[ FLUID_PASS_ADVECT_MACCORMACK ].Unbind();
		
	}





	void GPUFluid2D::ComputeCurl( Texture2D* velocity, Texture2D* dest, float timestep )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );// g_TexVelocity

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 1, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );		// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_COMPUTE_CURL ].Bind();
		{
			// Set Unforms
			GL_SAFE_CALL( glUniform1f( m_ulHalfInverseCellSize_CURL, 0.5f / CellSize ) );
			GL_SAFE_CALL( glUniform1f( m_ulTimeStep_CURL, timestep ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
		}
		m_Pass[ FLUID_PASS_COMPUTE_CURL ].Unbind();

	}




	void GPUFluid2D::ApplyVorticityConfinement( Texture2D* velocity, Texture2D* curl, Texture2D* dest, float timestep )
	{
		// Bind Inputs
		GL_SAFE_CALL( glBindImageTexture( 0, velocity->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, velocity->GLInternalFormat() ) );// g_TexVelocity
		GL_SAFE_CALL( glBindImageTexture( 1, curl->TexID(), 0, GL_FALSE, 0, GL_READ_ONLY, curl->GLInternalFormat() ) );		// g_TexCurlMagnitude

		// Bind Outputs
		GL_SAFE_CALL( glBindImageTexture( 2, dest->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, dest->GLInternalFormat() ) );	// g_TexDest


		// Execute ComputeShader
		m_Pass[ FLUID_PASS_VORTICITY_CONFINEMENT ].Bind();
		{
			// Set Uniforms
			GL_SAFE_CALL( glUniform1f( m_ulEps_VORTICITY, m_Vorticity ) );
			GL_SAFE_CALL( glUniform1f( m_ulCellSize_VORTICITY, CellSize ) );
			GL_SAFE_CALL( glUniform1f( m_ulTimeStep_VORTICITY, timestep ) );

			// Dispatch Compute Shader
			GL_SAFE_CALL( glDispatchCompute( m_DispatchGroups.x, m_DispatchGroups.y, m_DispatchGroups.z ) );
			
		}
		m_Pass[ FLUID_PASS_VORTICITY_CONFINEMENT ].Unbind();

	}


}// end of namespace