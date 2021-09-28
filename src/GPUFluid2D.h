#ifndef	GPU_FLUID_2D_H
#define GPU_FLUID_2D_H

#include	<graphics/gl4x/shader/GLComputeShader.h>
#include	<graphics/gl4x/resource/GLFrameBufferObject.h>

#include	"FluidVolume2D.h"



namespace OreOreLib
{
	class GPUFluid2D
	{

	public:
	
		GPUFluid2D();
		~GPUFluid2D();

		void InitShader();
		void Release();

		void BindVoxelData( FluidVolume2D* pVoxelData );
		void UnbindVoxelData();

		void SetTimeStep( float timestep )				{ m_TimeStep = timestep; }
		void SetIterations( int numiter )				{ m_NumIterations = numiter; }
		void SetAmbientTemperature( float tmp )			{ m_AmbientTemperature = tmp; }
		void SetImpulseTemperature( float tmp )			{ m_ImpulseTemperature = tmp; }
		void SetImpulseDensity( float density )			{ m_ImpulseDensity = density; }
		void SetSmokeBuoyancy( float buoyancy )			{ m_SmokeBuoyancy = buoyancy; }
		void SetSmokeWeight( float weight )				{ m_SmokeWeight = weight; }
		void SetTemperatureDissipation( float diss )	{ m_TemperatureDissipation = diss; }
		void SetVelocityDissipation( float diss )		{ m_VelocityDissipation = diss; }
		void SetDensityDissipation( float diss )		{ m_DensityDissipation = diss; }
		void EnableMackCormackAdvection( bool flag )	{ m_bEnableMacCormackAdvection = flag; }
		void EnableBuoyancy( bool flag )				{ m_bEnableBuoyancy = flag; }
		
		void SetVorticity( float val )					{ m_Vorticity = val; }
		void SetViscosity( float val )					{ m_Viscosity = val; }

		void AddObstacles( int x, int y, float radius );
		void RemoveObstacles( int x, int y, float radius );
		void AddSmoke( int x, int y, float dx, float dy, float radius );
		

		Texture2D* GetCurl() const	{ return (Texture2D *)&m_Curl; }


		void Update();


		// Default simulation parameters
		static const float SplatRadius;
		static const float CellSize;//					= 1.0f;//1.25f;//1.25f;
		static const float DEFAULT_AMBIENT_TEMPERATURE;// = 0.0f;
		static const float DEFAULT_IMPULSE_TEMPERATURE;//			10.0f
		static const float DEFAULT_IMPULSE_DENSITY;//				1.0f
		static const int DEFAULT_ITERATIONS;//					40
		static const float DEFAULT_TIME_STEP;//					0.05f//0.125f;
		static const float DEFAULT_SMOKE_BUOYANCY;//				1.0f
		static const float DEFAULT_SMOKE_WEIGHT;//				0.05f
		static const float DEFAULT_GRADIENT_SCALE;//				0.5f / CellSize;//1.125f / CellSize;
		static const float DEAFAULT_TEMPERATURE_DISSIPATION;//	1.0f//0.9999f;
		static const float DEAFAULT_VELOCITY_DISSIPATION;//		1.0f//0.9999f;
		static const float DEAFAULT_DENSITY_DISSIPATION;//		1.0f//0.9999f;
		static const float DEFAULT_VORTICITY;//				0.5f
		static const float DEFAULT_VISCOSITY;//				0.01f



	private:

		enum FLUID_PASS
		{
			FLUID_PASS_ADVECT,
//FLUID_PASS_DIFFUSE,
			FLUID_PASS_JACOBI,
			FLUID_PASS_SUBTRACT_GRADIENT,
			FLUID_PASS_COMPUTE_DIVERGENCE,
			FLUID_PASS_APPLY_IMPULSE,
			FLUID_PASS_APPLY_BUOYANCY,
			FLUID_PASS_FILL,
			
			FLUID_PASS_ADVECT_MACCORMACK,
FLUID_PASS_COMPUTE_CURL,
FLUID_PASS_VORTICITY_CONFINEMENT,
			NUM_FLUID_PASS,
		};

		GLComputeShader		m_Pass[ NUM_FLUID_PASS ];
		
		FluidVolume2D*		m_refFluidVoxelData;


		// Uniform Locations for Advection Pass
		GLint				m_ulTimeStep;
		GLint				m_ulDissipation;

		// Uniform Locations for Jacobi Pass
		GLint				m_ulAlpha;
		GLint				m_ulInverseBeta;

		// Uniform Locations for SubtractGradient Pass
		GLint				m_ulGradientScale;
		//GLint				m_ulHalfCell;

		// Uniform Locations for ComputeDivergence Pass
		GLint				m_ulHalfCell;

		// Uniform Locations for ApplyImpulse Pass
		GLint				m_ulPoint;
		GLint				m_ulRadius;
		GLint				m_ulFillColor;
		GLint				m_ulTexSize;

		// Uniform Locations for ApplyBuoyancy Pass
		GLint				m_ulAmbTemp;
		GLint				m_ulTimeStep_BOUYANCY;
		GLint				m_ulSigma;
		GLint				m_ulKappa;

		// Uniform Locations for Fill Pass
		GLint				m_ulPoint_FILL;
		GLint				m_ulRadius_FILL;
		GLint				m_ulFillColor_FILL;
		GLint				m_ulTexSize_FILL;

		// Uniform Locations for AdvectMacCormack Pass
		GLint				m_ulTimeStep_MACCORMACK;
		GLint				m_ulDissipation_MACCORMACK;

		// Uniform Locations for ComputeCurl Pass
		GLint				m_ulHalfInverseCellSize_CURL;
		GLint				m_ulTimeStep_CURL;

		// Uniform Locations for VorticityConfinement Pass
		GLint				m_ulEps_VORTICITY;
		GLint				m_ulCellSize_VORTICITY;
		GLint				m_ulTimeStep_VORTICITY;


		// Internal Params
		Vec3ui				m_DispatchGroups;//

		// Simulation Params
		float				m_TimeStep;
		int					m_NumIterations;
		float				m_AmbientTemperature;
		float				m_ImpulseTemperature;
		float				m_ImpulseDensity;
		float				m_SmokeBuoyancy;
		float				m_SmokeWeight;
		float				m_GradientScale;
		float				m_TemperatureDissipation;
		float				m_VelocityDissipation;
		float				m_DensityDissipation;
		bool				m_bEnableMacCormackAdvection;
		bool				m_bEnableBuoyancy;

		float				m_Vorticity;
		float				m_Viscosity;

		
// Experimental Buffer for ...MacCormak Advection
Texture2D	m_Phi_n_1_hat;
Texture2D	m_Phi_n_hat;
		
// Experimental Buffer for ...Vorticity Confinement
Texture2D	m_Curl;


		// FBO
		GLFrameBufferObject		m_Fbo;
		FrameBufferAttachment	m_Attachment;


		void Advect( Texture2D* velocity, Texture2D* source, Texture2D* obstacles, Texture2D* dest, float dissipation, float timestep );
		void Jacobi( Texture2D* x, Texture2D* b, Texture2D* obstacles, Texture2D* dest, float alpha, float invbeta );
		void Jacobi( SwapTexture2D* pressure, Texture2D* divergence, Texture2D* obstacles, int numIter, float alpha, float invbeta );
		void SubtractGradient( Texture2D* velocity, Texture2D* pressure, Texture2D* obstacles, Texture2D* dest );
		void ComputeDivergence( Texture2D* velocity, Texture2D* obstacles, Texture2D* dest );
		void ApplyImpulse( Texture2D* dest, const Vec2f& position, float value, float radius );
		void ApplyBuoyancy( Texture2D* velocity, Texture2D* temperature, Texture2D* density, Texture2D* dest );
		void Fill( Texture2D* dest, const Vec2f& position, const Vec3f& fillVal, float radius );
		void ClearBuffer( int tex_channel, float val );
		void AdvectMacCormack( Texture2D* velocity, Texture2D* source, Texture2D* obstacles, Texture2D* dest, float dissipation, float timestep );

// Vorticity Confinement
void ComputeCurl( Texture2D* velocity, Texture2D* dest, float timestep );
void ApplyVorticityConfinement( Texture2D* velocity, Texture2D* curl, Texture2D* dest, float timestep );


	};





}// end of namespace






#endif	// GPU_FLUID_2D_H