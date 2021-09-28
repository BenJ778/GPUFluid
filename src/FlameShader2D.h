#ifndef	FLAME_SHADER_2D_H
#define	FLAME_SHADER_2D_H

#include	<graphics/gl4x/shader/IShader.h>
#include	<graphics/gl4x/resource/GLVertexArrayObject.h>



namespace OreOreLib
{


	class FlameShader2D : public IShader
	{
	public:

		FlameShader2D();
		FlameShader2D( const TCHAR *filepath, GLSL_VERSION version );
		~FlameShader2D();

		void InitShader( const TCHAR *filepath, GLSL_VERSION version );
		void Release();


		void Render( Texture1D *texRamp, Texture2D *texScalar, Texture2D *texTemperature, float scale=1.0f );


		// Override Virtual Functions
		inline virtual int NumPasses() const	{ return 1; }
		inline virtual GLuint GetPassID( int shader_idx=0 ) const{ return m_pShader->ID(); }

		inline virtual void BindShader( int shader_idx=0 )	// シェーダーをバインドする
		{
			if( m_pCurrShader == m_pShader )	return;
			m_pCurrShader = m_pShader;
			m_pCurrShader->Bind();
		}

		virtual void BindBufferObject(const IBufferObject* pbufferobj){}
		virtual void UnbindBufferObject(){}

		virtual void Render( const GeometryBuffer *pGeom, const ViewTransformMatrix *cam_mat ){}


	private:

		GLShader	*m_pShader;
		GLint		m_ulTexRamp;		// Ramp Texture
		GLint		m_ulTexDensity;		// Density Texture
		GLint		m_ulTexTemperature;	// Temperature Texture
		GLint		m_ulScale;			// scale;

	};


	class RampShaderFactory : IShaderFactory
	{
	protected:

		IShader* Create( GLSL_VERSION version ){ return new FlameShader2D( _T( "../assets/shader/shader/FlameShader2D.glsl" ), version ); }

	public:

		virtual ~RampShaderFactory(){}
		IShader* CreateInstance( GLSL_VERSION version );

	};


}// end of namespace


#endif	// FLAME_SHADER_2D_H //