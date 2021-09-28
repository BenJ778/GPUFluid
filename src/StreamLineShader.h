#ifndef	STREAM_LINE_SHADER_H
#define	STREAM_LINE_SHADER_H


#include	<graphics/gl4x/resource/Texture.h>
#include	<graphics/gl4x/resource/GLRenderTarget.h>
#include	<graphics/gl4x/shader/GLComputeShader.h>

//#include	<oreore/GLTextureBufferObject.h>



namespace OreOreLib
{

	// ストリームラインを描画するシェーダー
	class StreamLineShader
	{
	public:

		StreamLineShader();
		~StreamLineShader();

		void InitShader( const TCHAR* filepath );
		void Release();

		void BindVectorField( Texture2D* pTexVectorField, Texture2D* pTexStreamLine );
		void UnbindVectorField();
		
		void SetGridSize( int dimx, int dimy )				{ InitVec( m_GridSize, dimx, dimy ); }
		
		void Draw( float maxTraceLength, int maxIter );
		void ClearBuffer( const Vec4f& val );


	private:

		GLComputeShader	m_Shader;

		GLint			m_ulTexVectorField;
		GLint			m_ulTraceLength;
		GLint			m_ulMaxIter;

		Texture2D*		m_refVectorField;
		Texture2D*		m_refTexStreamLine; 


		Vec2i			m_GridSize;

		GLRenderTarget	m_RT;


	};


}// end of namespace



#endif	// STREAM_LINE_SHADER_H //