#include	"FluidVolume2D.h"




namespace OreOreLib
{


	FluidVolume2D::FluidVolume2D()
	{
		
	}
	


	FluidVolume2D::FluidVolume2D( int width, int height )
	{
		Init( width, height );	
	}



	FluidVolume2D::~FluidVolume2D()
	{
		Release();
	}



	void FluidVolume2D::Init( int width, int height )
	{
		m_TexVelocity.Allocate( width, height, 0, 0, DATA_FORMAT::FORMAT_R16G16_FLOAT );
		m_TexVelocity.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_TexVelocity.GenHardwareTexture();

		m_TexDensity.Allocate( width, height, 0, 0, DATA_FORMAT::FORMAT_R16_FLOAT );
		m_TexDensity.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_TexDensity.GenHardwareTexture();

		m_TexPressure.Allocate( width, height, 0, 0, DATA_FORMAT::FORMAT_R16_FLOAT );
		m_TexPressure.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_TexPressure.GenHardwareTexture();

		m_TexTemperature.Allocate( width, height, 0, 0, DATA_FORMAT::FORMAT_R16_FLOAT );
		m_TexTemperature.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_TexTemperature.GenHardwareTexture();
		
		m_TexDivergence.Allocate( width, height, 0, 0, DATA_FORMAT::FORMAT_R16G16B16A16_FLOAT );
		m_TexDivergence.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_TexDivergence.GenHardwareTexture();
		
		m_TexObstacles.Allocate( width, height, 0, 0, DATA_FORMAT::FORMAT_R16G16B16A16_FLOAT );
		m_TexObstacles.SetFilterMode( FILTER_MODE::FILTER_MAG_MIN_LINEAR );
		m_TexObstacles.GenHardwareTexture();
	}
	


	void FluidVolume2D::Release()
	{
		m_TexVelocity.Release();
		m_TexDensity.Release();
		m_TexPressure.Release();
		m_TexTemperature.Release();
		m_TexDivergence.Release();
		m_TexObstacles.Release();
	}


}// end of namespace