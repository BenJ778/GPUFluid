#include	"StreamLineShader.h"

#include	<oreore/common/Utility.h>
#include	<graphics/gl4x/common/GLHelperFunctions.h>


#define	GRID_SIZE	64



namespace OreOreLib
{


	StreamLineShader::StreamLineShader()
	{
		m_refVectorField	= NULL;
		m_refTexStreamLine	= NULL;

		InitZero( m_GridSize );
	}
	


	StreamLineShader::~StreamLineShader()
	{
		Release();
	}

	

	void StreamLineShader::InitShader( const TCHAR* filepath )
	{
		GLuint	program_id;

		// Init shader
		m_Shader.Init( filepath, GLSL_VERSION::GLSL_430 );
		program_id	= m_Shader.ID();


		// Bindnit Attribute Location
		glBindAttribLocation( program_id, ATTRIB_LOCATION_POSITION, "POSITION" );
		glBindAttribLocation( program_id, ATTRIB_LOCATION_TEXCOORD, "TEXCOORD" );

		// Init Uniform Locations
		m_ulTexVectorField	= glGetUniformLocation( program_id, "g_TexVectorField" );
		m_ulTraceLength		= glGetUniformLocation( program_id, "g_TraceLength" );
		m_ulMaxIter			= glGetUniformLocation( program_id, "g_MaxIter" );

	
		// Bind FragData Locations
		// glBindFragDataLocation( program_id, channel, "FragColor0" );

		// Link Shader
		m_Shader.Link();

		// Setup Texture Unit
		m_Shader.Bind();
		{
			glUniform1i( m_ulTexVectorField, 0 );
		}
		m_Shader.Unbind();

	}



	void StreamLineShader::Release()
	{
		UnbindVectorField();
		m_Shader.Release();
	}



	void StreamLineShader::BindVectorField( Texture2D* pTexVectorField, Texture2D* pTexStreamLine )
	{
		m_refVectorField = pTexVectorField; 
		m_refTexStreamLine = pTexStreamLine;


		uint32 texids[] = { m_refTexStreamLine->TexID() };
		m_RT.Init( m_refTexStreamLine->Width(), m_refTexStreamLine->Height(), false );
		m_RT.BindTextures( 1, g_DefaultColorAttachments, texids );

	}
	
	
	
	void StreamLineShader::UnbindVectorField()	
	{ 
		m_refVectorField = NULL;
		m_refTexStreamLine = NULL;
		
		m_RT.Release();
	}



	void StreamLineShader::Draw( float maxTraceLength, int maxIter )
	{
		if( !m_refVectorField || !m_refTexStreamLine )
			return;

		const Vec4f clearColor	= { 0.0f, 0.0f, 0.0f, 0.0f };
		ClearBuffer( clearColor );


		// Bind Input


		// Bind Output
		glBindImageTexture( 0, m_refTexStreamLine->TexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, m_refTexStreamLine->GLInternalFormat() );


		// Execute ComputeShader
		m_Shader.Bind();
		{
			// Set Textures
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, m_refVectorField->TexID() );

			// Set Uniforms
			glUniform1f( m_ulTraceLength, maxTraceLength );
			glUniform1i( m_ulMaxIter, maxIter );

			// Dispatch Compute Shader
			glDispatchCompute( DivUp( m_GridSize.x, 8 ), DivUp( m_GridSize.y, 8 ), 1 );
		}
		m_Shader.Unbind();
	
	
	}



	void StreamLineShader::ClearBuffer( const Vec4f& val )
	{
		m_RT.Bind();
		{
//GLErrorCheck();
			glClearColor( val.x, val.y, val.z, val.w );
			glClear( GL_COLOR_BUFFER_BIT );
		}
		m_RT.Unbind();
//GLErrorCheck();
	}



}// end of namespace