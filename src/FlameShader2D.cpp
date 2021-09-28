#include	"FlameShader2D.h"


namespace OreOreLib
{
	// Default Constructor
	FlameShader2D::FlameShader2D()
	{
#ifdef _DEBUG
		tcout << "FlameShader2D()..."<< tendl;
#endif // _DEBUG
		//const type_info &id	= typeid(*this);
		//tcout << "class name: " << id.name() << endl;// .name()使うと、コンパイラが間違えてメモリリーク報告してくる. 2013.08.05

		m_pShader			= NULL;
		m_ulTexRamp			= -1;
		m_ulTexDensity		= -1;
		m_ulTexTemperature	= -1;
		m_ulScale			= -1;
	}



	// Constructor
	FlameShader2D::FlameShader2D( const TCHAR *filepath, GLSL_VERSION version )
	{
#ifdef _DEBUG
		tcout << "FlameShader2D( const TCHAR *filepath, GLSL_VERSION version )..." << tendl;
#endif // _DEBUG
		//const type_info &id	= typeid(*this);
		//tcout << "class name: " << id.name() << endl;// .name()使うと、コンパイラが間違えてメモリリーク報告してくる. 2013.08.05

		m_pShader			= NULL;
		m_ulTexRamp			= -1;
		m_ulTexDensity		= -1;
		m_ulTexTemperature	= -1;
		m_ulScale			= -1;

		if( filepath )
			InitShader( filepath, version );
	}



	// Destructor
	FlameShader2D::~FlameShader2D()
	{
#ifdef _DEBUG
		tcout << _T("~FlameShader2D()...") << tendl;
#endif
		Release();
	}



	void FlameShader2D::Release()
	{
		SafeDelete( m_pShader );
		m_ulTexRamp			= -1;
		m_ulTexDensity		= -1;
		m_ulTexTemperature	= -1;
		m_ulScale			= -1;
	}



	void FlameShader2D::InitShader( const TCHAR *filepath, GLSL_VERSION version )
	{
		// create shader
		m_pShader	= new GLShader();
		m_pShader->Init( filepath, version );

		// initialize attribute location
		GL_SAFE_CALL( glBindAttribLocation( m_pShader->ID(), ATTRIB_LOCATION_POSITION, "POSITION" ) );
		GL_SAFE_CALL( glBindAttribLocation( m_pShader->ID(), ATTRIB_LOCATION_TEXCOORD, "TEXCOORD" ) );

		// initialize uniform location
		m_ulTexRamp			= GL_SAFE_CALL( glGetUniformLocation( m_pShader->ID(), "g_TexRamp" ) );
		m_ulTexDensity		= GL_SAFE_CALL( glGetUniformLocation( m_pShader->ID(), "g_TexDensity" ) );
		m_ulTexTemperature	= GL_SAFE_CALL( glGetUniformLocation( m_pShader->ID(), "g_TexTemperature" ) );
		m_ulScale			= GL_SAFE_CALL( glGetUniformLocation( m_pShader->ID(), "g_Scale" ) );

		// link shader
		m_pShader->Link();
	}


	void FlameShader2D::Render( Texture1D *texRamp, Texture2D *texDensity, Texture2D *texTemperature, float scale )
	{
		if( !m_refScreenSpaceQuad )	return;

		m_pShader->Bind();
		{
			// UniformLocationの設定
			glActiveTexture( GL_TEXTURE0 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_1D, texRamp->TexID() ) );
			GL_SAFE_CALL( glUniform1i( m_ulTexRamp, 0 ) );

			glActiveTexture( GL_TEXTURE1 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, texDensity->TexID() ) );
			GL_SAFE_CALL( glUniform1i( m_ulTexDensity, 1 ) );

			glActiveTexture( GL_TEXTURE2 );
			GL_SAFE_CALL( glBindTexture( GL_TEXTURE_2D, texTemperature->TexID() ) );
			GL_SAFE_CALL( glUniform1i( m_ulTexTemperature, 2 ) );
			
			GL_SAFE_CALL( glUniform1f( m_ulScale, scale ) );

			// 描画
			m_refScreenSpaceQuad->Draw();
		}
		m_pShader->Unbind();
	}




}// end of namespace