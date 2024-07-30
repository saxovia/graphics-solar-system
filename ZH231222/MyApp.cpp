#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"

#include <imgui.h>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	AssembleProgram( m_programID, "Vert_PosNormTex.vert", "Frag_ZH.frag" );
	InitSkyboxShaders();
	InitSunShaders();
}

void CMyApp::InitSkyboxShaders()
{
	m_programSkyboxID = glCreateProgram();
	AssembleProgram( m_programSkyboxID, "Vert_skybox.vert", "Frag_skybox.frag" );
}
void CMyApp::InitSunShaders()
{
	m_programSunID = glCreateProgram();
	AssembleProgram(m_programSunID, "Vert_PosNormTex.vert", "Frag_sun.frag");
}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
	CleanSkyboxShaders();
	CleanSunShaders();
}

void CMyApp::CleanSkyboxShaders()
{
	glDeleteProgram( m_programSkyboxID );
}
void CMyApp::CleanSunShaders()
{
	glDeleteProgram(m_programSunID);
}

// Nyers parameterek
struct Param
{
	glm::vec3 GetPos( float u, float v ) const noexcept
	{
		return glm::vec3( u, v, 0.0f );
	}
	glm::vec3 GetNorm( float u, float v ) const noexcept
	{
		return glm::vec3( 0.0,0.0,1.0 );
	}
	glm::vec2 GetTex( float u, float v ) const noexcept
	{
		return glm::vec2( u, v );
	}
};

struct Sphere {
	float r;
	Sphere(float _r = 1.0f) : r(_r) {}


	glm::vec3 GetPos(float u, float v) const noexcept
	{
		u *= glm::two_pi<float>();
		v *= glm::pi<float>();


		return glm::vec3(
			r * sinf(v) * cosf(u),
			r * cosf(v),
			r * sinf(v) * sinf(u));
	}

	glm::vec3 GetNorm(float u, float v) const noexcept
	{
		u *= glm::two_pi<float>();
		v *= glm::pi<float>();


		return glm::vec3(
			sinf(v) * cosf(u),
			cosf(v),
			sinf(v) * sinf(u));
	}
	glm::vec2 GetTex(float u, float v) const noexcept
	{
		return glm::vec2(u, -v);
	}
};

void CMyApp::InitGeometry()
{

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( Vertex, position ), 3, GL_FLOAT },
		{ 1, offsetof( Vertex, normal   ), 3, GL_FLOAT },
		{ 2, offsetof( Vertex, texcoord ), 2, GL_FLOAT },
	};

	// Suzanne

	MeshObject<Vertex> suzanneMeshCPU = ObjParser::parse("Assets/Suzanne.obj");
	m_SuzanneGPU = CreateGLObjectFromMesh( suzanneMeshCPU, vertexAttribList );

	// Aszteroida 
	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList2 =
	{
		{ 0, offsetof(Vertex, position), 3, GL_FLOAT },
		{ 1, offsetof(Vertex, normal), 3, GL_FLOAT },
		{ 2, offsetof(Vertex, texcoord), 2, GL_FLOAT },
	};

	MeshObject<Vertex> asteroidMeshCPU = ObjParser::parse("Assets/asteroid.obj");
	m_AsteroidGPU = CreateGLObjectFromMesh(asteroidMeshCPU, vertexAttribList2);


	// quad
	MeshObject<Vertex> quadMeshCPU;
	quadMeshCPU.vertexArray =
	{
		{ glm::vec3(-1.0, -1.0, 0.0), glm::vec3(0.0, 0.0,  1.0), glm::vec2(0.0,0.0) }, // első lap
		{ glm::vec3(1.0, -1.0, 0.0), glm::vec3(0.0, 0.0,  1.0), glm::vec2(1.0,0.0) },
		{ glm::vec3(1.0,  1.0, 0.0), glm::vec3(0.0, 0.0,  1.0), glm::vec2(1.0,1.0) },
		{ glm::vec3(-1.0,  1.0, 0.0), glm::vec3(0.0, 0.0,  1.0), glm::vec2(0.0,1.0) }
	};

	quadMeshCPU.indexArray =
	{
		0, 1, 2, // első lap
		2, 3, 0
	};

	m_quadGPU = CreateGLObjectFromMesh(quadMeshCPU, vertexAttribList);

	// Parametrikus felület
	//MeshObject<Vertex> surfaceMeshCPU = GetParamSurfMesh( Param() );
	MeshObject<Vertex> surfaceMeshCPU = GetParamSurfMesh(Sphere());
	m_surfaceGPU = CreateGLObjectFromMesh( surfaceMeshCPU, vertexAttribList );

	// Skybox
	InitSkyboxGeometry();
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_SuzanneGPU );
	CleanOGLObject( m_AsteroidGPU );
	CleanOGLObject( m_surfaceGPU );
	CleanSkyboxGeometry();
}

void CMyApp::InitSkyboxGeometry()
{
	// skybox geo
	MeshObject<glm::vec3> skyboxCPU =
	{
		std::vector<glm::vec3>
		{
			// hátsó lap
			glm::vec3(-1, -1, -1),
			glm::vec3( 1, -1, -1),
			glm::vec3( 1,  1, -1),
			glm::vec3(-1,  1, -1),
			// elülső lap
			glm::vec3(-1, -1, 1),
			glm::vec3( 1, -1, 1),
			glm::vec3( 1,  1, 1),
			glm::vec3(-1,  1, 1),
		},

		std::vector<GLuint>
		{
			// hátsó lap
			0, 1, 2,
			2, 3, 0,
			// elülső lap
			4, 6, 5,
			6, 4, 7,
			// bal
			0, 3, 4,
			4, 3, 7,
			// jobb
			1, 5, 2,
			5, 6, 2,
			// alsó
			1, 0, 4,
			1, 4, 5,
			// felső
			3, 2, 6,
			3, 6, 7,
		}
	};

	m_SkyboxGPU = CreateGLObjectFromMesh( skyboxCPU, { { 0, offsetof( glm::vec3,x ), 3, GL_FLOAT } } );
}

void CMyApp::CleanSkyboxGeometry()
{
	CleanOGLObject( m_SkyboxGPU );
}

void CMyApp::InitTextures()
{
	// diffuse textures

	glGenTextures( 1, &m_SuzanneTextureID );
	TextureFromFile( m_SuzanneTextureID, "Assets/wood.jpg" );
	SetupTextureSampling( GL_TEXTURE_2D, m_SuzanneTextureID );

	glGenTextures( 1, &m_sunTextureID );
	TextureFromFile(m_sunTextureID, "Assets/sun.jpg" );
	SetupTextureSampling( GL_TEXTURE_2D, m_sunTextureID);

	glGenTextures(1, &m_mercuryTextureID);
	TextureFromFile(m_mercuryTextureID, "Assets/mercury.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_mercuryTextureID);

	glGenTextures(1, &m_venusTextureID);
	TextureFromFile(m_venusTextureID, "Assets/venus.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_venusTextureID);

	glGenTextures(1, &m_earthTextureID);
	TextureFromFile(m_earthTextureID, "Assets/earth.png");
	SetupTextureSampling(GL_TEXTURE_2D, m_earthTextureID);

	glGenTextures(1, &m_moonTextureID);
	TextureFromFile(m_moonTextureID, "Assets/moon.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_moonTextureID);

	glGenTextures(1, &m_marsTextureID);
	TextureFromFile(m_marsTextureID, "Assets/mars.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_marsTextureID);

	glGenTextures(1, &m_jupiterTextureID);
	TextureFromFile(m_jupiterTextureID, "Assets/jupiter.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_jupiterTextureID);

	glGenTextures(1, &m_saturnTextureID);
	TextureFromFile(m_saturnTextureID, "Assets/saturn.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_saturnTextureID);

	glGenTextures(1, &m_uranusTextureID);
	TextureFromFile(m_uranusTextureID, "Assets/uranus.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_uranusTextureID);

	glGenTextures(1, &m_neptuneTextureID);
	TextureFromFile(m_neptuneTextureID, "Assets/neptune.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_neptuneTextureID);

	glGenTextures(1, &m_plutoTextureID);
	TextureFromFile(m_plutoTextureID, "Assets/pluto.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_plutoTextureID);


	glGenTextures(1, &m_surfaceTextureID);
	TextureFromFile(m_surfaceTextureID, "Assets/sun.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_surfaceTextureID);

	//Gyűrű textúrák
	glGenTextures(1, &m_kuiperTextureID);
	TextureFromFile(m_kuiperTextureID, "Assets/kuiper.png");
	SetupTextureSampling(GL_TEXTURE_2D, m_kuiperTextureID);

	glGenTextures(1, &m_neptuneRingTextureID);
	TextureFromFile(m_neptuneRingTextureID, "Assets/neptune-ring.png");
	SetupTextureSampling(GL_TEXTURE_2D, m_neptuneRingTextureID);

	glGenTextures(1, &m_saturnRingTextureID);
	TextureFromFile(m_saturnRingTextureID, "Assets/saturn-ring.png");
	SetupTextureSampling(GL_TEXTURE_2D, m_saturnRingTextureID);

	glGenTextures(1, &m_uranusRingTextureID);
	TextureFromFile(m_uranusRingTextureID, "Assets/uranus-ring.png");
	SetupTextureSampling(GL_TEXTURE_2D, m_uranusRingTextureID);


	glGenTextures(1, &m_asteroidTextureID);
	TextureFromFile(m_asteroidTextureID, "Assets/rock.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, m_asteroidTextureID);


	// skybox texture

	InitSkyboxTextures();
}

void CMyApp::CleanTextures()
{
	// diffuse textures

	glDeleteTextures( 1, &m_SuzanneTextureID );
	glDeleteTextures( 1, &m_surfaceTextureID );
	glDeleteTextures( 1, &m_sunTextureID );
	glDeleteTextures( 1, &m_mercuryTextureID );
	glDeleteTextures( 1, &m_venusTextureID );
	glDeleteTextures( 1, &m_earthTextureID );
	glDeleteTextures( 1, &m_moonTextureID );
	glDeleteTextures( 1, &m_marsTextureID );
	glDeleteTextures( 1, &m_jupiterTextureID );
	glDeleteTextures( 1, &m_saturnTextureID );
	glDeleteTextures( 1, &m_uranusTextureID );
	glDeleteTextures( 1, &m_neptuneTextureID );
	glDeleteTextures( 1, &m_plutoTextureID );



	glDeleteTextures( 1, &m_kuiperTextureID );
	glDeleteTextures( 1, &m_neptuneRingTextureID );
	glDeleteTextures( 1, &m_saturnRingTextureID );
	glDeleteTextures( 1, &m_uranusRingTextureID );


	glDeleteTextures( 1, &m_asteroidTextureID);



	// skybox texture

	CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures()
{
	// skybox texture

	glGenTextures( 1, &m_skyboxTextureID );
	TextureFromFile( m_skyboxTextureID, "Assets/space_xpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/space_xneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/space_ypos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/space_yneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/space_zpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
	TextureFromFile( m_skyboxTextureID, "Assets/space_zneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
	SetupTextureSampling( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID, false );

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTextures()
{
	glDeleteTextures( 1, &m_skyboxTextureID );
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 7.0, 7.0),// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up


	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;

	m_camera.Update( updateInfo.DeltaTimeInSec );
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Suzanne
	/*
	glBindVertexArray( m_SuzanneGPU.vaoID );

	// - Textúrák beállítása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_SuzanneTextureID );
	*/


	// Load your normal map texture (code for loading texture is not shown)
	glBindTexture(GL_TEXTURE_2D, m_normalMapID);

	// aszteroida
	glBindVertexArray(m_AsteroidGPU.vaoID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_asteroidTextureID);
	
	glUseProgram( m_programID );

	glm::mat4 matWorld = glm::identity<glm::mat4>();

	float spinning_angle = -m_ElapsedTimeInSec / 1.0f * 30.0f;
	glm::mat4 matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(1.0f, 1.0f, 1.0f));
	float rr = 3.5f;
	glm::vec3 spiralPosition11(
		rr * cosf(glm::two_pi<float>() / 110.0f * m_ElapsedTimeInSec),
		0.0f,
		rr * -sinf(glm::two_pi<float>() / 110.0f * m_ElapsedTimeInSec)
	);
	glm::mat4 matWorldSpiral = glm::translate(spiralPosition11);

	matWorld = matWorldSpiral * glm::scale(glm::vec3(0.05)) * matWorldSpinning;


	glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	glUniformMatrix4fv( ul( "viewProj" ), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	// - Fényforrások beállítása
	glUniform3fv( ul( "cameraPos" ), 1, glm::value_ptr( m_camera.GetEye() ) );
	glUniform4fv( ul( "lightPos" ),  1, glm::value_ptr( m_lightPos ) );

	glUniform3fv( ul( "La" ),		 1, glm::value_ptr( m_La ) );
	glUniform3fv( ul( "Ld" ),		 1, glm::value_ptr( m_Ld ) );
	glUniform3fv( ul( "Ls" ),		 1, glm::value_ptr( m_Ls ) );

	glUniform1f( ul( "lightConstantAttenuation"	 ), m_lightConstantAttenuation );
	glUniform1f( ul( "lightLinearAttenuation"	 ), m_lightLinearAttenuation   );
	glUniform1f( ul( "lightQuadraticAttenuation" ), m_lightQuadraticAttenuation);

	// - Anyagjellemzők beállítása
	glUniform3fv( ul( "Ka" ),		 1, glm::value_ptr( m_Ka ) );
	glUniform3fv( ul( "Kd" ),		 1, glm::value_ptr( m_Kd ) );
	glUniform3fv( ul( "Ks" ),		 1, glm::value_ptr( m_Ks ) );

	glUniform1f( ul( "Shininess" ),	m_Shininess );


	// - textúraegységek beállítása
	glUniform1i( ul( "texImage" ), 0 );
	/*
	glDrawElements( GL_TRIANGLES,    
					m_SuzanneGPU.count,			 
					GL_UNSIGNED_INT,
					nullptr );
					*/
	glDrawElements(GL_TRIANGLES,
		m_AsteroidGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	
	// Parametrikus felület
	// Bolygók
	// 
	// 
	// Nap
	//

	glBindVertexArray( m_surfaceGPU.vaoID );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_sunTextureID );
	glUseProgram(m_programSunID);
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::translate(m_camera.GetEye())));
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	glm::mat4 matWorldTranslate = glm::translate( glm::vec3(0.0f,0.0f,0.0f));
	glm::mat4 matWorldRotate = glm::rotate(glm::radians(7.25f), glm::vec3(0.0f, 0.0f, 1.0f));

	float SPINNING_PERIOD = 1.0f;
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 27 * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));

	matWorld = matWorldTranslate * matWorldRotate * matWorldSpinning * glm::scale(glm::vec3(m_sunSize));
	glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	glDrawElements( GL_TRIANGLES,    
					m_surfaceGPU.count,
					GL_UNSIGNED_INT,
					nullptr );



	//Merkúr

	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_mercuryTextureID);
	glUseProgram(m_programID);
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::translate(m_camera.GetEye())));
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	float rotationAroundSun = 89.0f;
	float r = 2.0f;
	glm::vec3 spiralPosition(
		r * cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition);

	matWorldTranslate = glm::translate(glm::vec3(2.0f, 0.0f, 0.0f));
	glm::mat4 matWorldScale = glm::scale(glm::vec3(0.15));
	matWorldRotate = glm::rotate(glm::radians(0.01f), glm::vec3(0.0f, 0.0f, 1.0f));

	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 58.7 * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));

	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 58.7f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));

	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning ;


	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	//Vénusz

	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_venusTextureID);

	matWorldTranslate = glm::translate(glm::vec3(3.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.13));
	matWorldRotate = glm::rotate(glm::radians(177.4f), glm::vec3(0.0f, 0.0f, 1.0f));
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 243 * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationAroundSun = 225.0f;
	r = 3.0f;
	glm::vec3 spiralPosition2(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition2);

	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);

	//Föld
	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_earthTextureID);
	/*
	static constexpr float SPINNING_PERIOD = 4.0f;
	const float spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD * 360.0f;
	glm::mat4 matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	*/
	matWorldTranslate = glm::translate(glm::vec3(4.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.2));
	matWorldRotate = glm::rotate(glm::radians(23.44f), glm::vec3(0.0f, 0.0f, 1.0f));
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));

	rotationAroundSun = 365.0f;
	r = 4.0f;
	glm::vec3 spiralPosition3(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition3);

	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);


	//hold

	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_moonTextureID);
	matWorldTranslate = glm::translate(glm::vec3(0.4, 0.0f, 0.0f));
	float moonsize = 0.12 / 3;
	matWorldScale = glm::scale(glm::vec3(moonsize));
	matWorldRotate = glm::rotate(glm::radians(1.54f), glm::vec3(0.0f, 0.0f, 1.0f));



	matWorld = matWorldSpiral * matWorldTranslate * matWorldScale * matWorldRotate;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	//Mars
	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_marsTextureID);
	matWorldTranslate = glm::translate(glm::vec3(5.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.19));
	matWorldRotate = glm::rotate(glm::radians(25.19f), glm::vec3(0.0f, 0.0f, 1.0f));

	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 1.04f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));

	rotationAroundSun = 687.0f;
	r = 5.0f;
	glm::vec3 spiralPosition4(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition4);

	matWorld = matWorldSpiral *  matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);

	//Jupiter
	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_jupiterTextureID);
	matWorldTranslate = glm::translate(glm::vec3(6.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.4));
	matWorldRotate = glm::rotate(glm::radians(3.13f), glm::vec3(0.0f, 0.0f, 1.0f));

	rotationAroundSun = 4329.0f;
	r = 6.0f;
	glm::vec3 spiralPosition5(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);

	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 0.42f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	matWorldSpiral = glm::translate(spiralPosition5);


	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	//Saturn Szaturnusz
	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_saturnTextureID);
	matWorldTranslate = glm::translate(glm::vec3(7.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.35f));
	matWorldRotate = glm::rotate(glm::radians(26.73f), glm::vec3(0.0f, 0.0f, 1.0f));
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 0.46f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationAroundSun = 10753.0f;
	r = 7.0f;
	glm::vec3 spiralPosition7(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition7);


	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	//Uránusz
	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uranusTextureID);
	matWorldTranslate = glm::translate(glm::vec3(8.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.25f));
	matWorldRotate = glm::rotate(glm::radians(97.77f), glm::vec3(0.0f, 0.0f, 1.0f));
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 0.71f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationAroundSun = 30664.0f;
	r = 8.0f;
	glm::vec3 spiralPosition8(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition8);


	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);

	//Neptunusz

	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_neptuneTextureID);
	matWorldTranslate = glm::translate(glm::vec3(9.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.26f));
	matWorldRotate = glm::rotate(glm::radians(28.32f), glm::vec3(0.0f, 0.0f, 1.0f));
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 0.66f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationAroundSun = 60147.0f;
	r = 9.0f;
	glm::vec3 spiralPosition9(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition9);

	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	//Pluto
	glBindVertexArray(m_surfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_plutoTextureID);
	matWorldTranslate = glm::translate(glm::vec3(10.0f, 0.0f, 0.0f));
	matWorldScale = glm::scale(glm::vec3(0.1f));
	matWorldRotate = glm::rotate(glm::radians(119.61f), glm::vec3(0.0f, 0.0f, 1.0f));
	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 6.37f * 360.0f;
	matWorldSpinning = glm::rotate(glm::radians(spinning_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationAroundSun = 90520.0f;
	r = 10.0f;
	glm::vec3 spiralPosition10(
		r* cosf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec),
		0.0f,
		r * -sinf(glm::two_pi<float>() / rotationAroundSun * m_ElapsedTimeInSec)
	);
	matWorldSpiral = glm::translate(spiralPosition10);
	matWorld = matWorldSpiral * matWorldScale * matWorldRotate * matWorldSpinning;
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_surfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);

	// 
	// 
	//
	// skybox
	//

	// - VAO
	glBindVertexArray( m_SkyboxGPU.vaoID );

	// - Textura
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID );

	// - Program
	glUseProgram( m_programSkyboxID );

	// - uniform parameterek
	glUniformMatrix4fv( ul("world"),    1, GL_FALSE, glm::value_ptr( glm::translate( m_camera.GetEye() ) ) );
	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	// - textúraegységek beállítása
	glUniform1i( ul( "skyboxTexture" ), 1 );

	// mentsük el az előző Z-test eredményt, azaz azt a relációt, ami alapján update-eljük a pixelt.
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	// most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli vágósíkokra
	glDepthFunc(GL_LEQUAL);

	// - Rajzolas
	glDrawElements( GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT, nullptr );

	glDepthFunc(prevDepthFnc);

	//gyűrűk, kuper-öv
	// 
	// Kuiper
	float m_kuiperRotationAngle = -m_ElapsedTimeInSec;
	float m_rotationSpeed = 0.01f;

	m_kuiperRotationAngle += m_rotationSpeed;
	if (m_kuiperRotationAngle > 360.0f) {
		m_kuiperRotationAngle -= 360.0f;
	}
	glBindVertexArray(m_quadGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_kuiperTextureID);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(m_programSunID);
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	//matWorldRotate = glm::rotate(glm::radians(1.54f), glm::vec3(0.0f, 0.0f, 1.0f));
	matWorld = glm::translate(glm::vec3(0.0f, -0.05f, 0.0f)) *
           glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
           glm::rotate(glm::radians(m_kuiperRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f)) *
           glm::scale(glm::vec3(15.0f));
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_quadGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	// Saturn szaturnusz gyűrű
	glBindVertexArray(m_quadGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_saturnRingTextureID);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(m_programSunID);
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	spinning_angle = -m_ElapsedTimeInSec / SPINNING_PERIOD / 0.46f * 360.0f;
	matWorldSpiral = glm::translate(spiralPosition7);

	matWorld = matWorldSpiral * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_quadGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);


	//uránusz gyűrű

	glBindVertexArray(m_quadGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uranusRingTextureID);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(m_programID);
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	matWorldSpiral = glm::translate(spiralPosition8);

	matWorld = matWorldSpiral * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_quadGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	//neptunusz gyűrű

	glBindVertexArray(m_quadGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_neptuneRingTextureID);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(m_programID);
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	matWorldSpiral = glm::translate(spiralPosition9);
	matWorld = matWorldSpiral * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

	glDrawElements(GL_TRIANGLES,
		m_quadGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);






	// shader kikapcsolasa
	glUseProgram( 0 );

	// - Textúrák kikapcsolása, minden egységre külön
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );


	// VAO kikapcsolása
	glBindVertexArray( 0 );
}

void CMyApp::RenderGUI()
{
	if (ImGui::Begin("Lighting settings"))
	{
			//ImGui::SliderFloat("Constant Attenuation", &m_lightConstantAttenuation, 0.001f, 2.0f);
			ImGui::SliderFloat("Sun size", &m_sunSize, 0.1f, 3.0f);
	}
	ImGui::End();
}

GLint CMyApp::ul( const char* uniformName ) noexcept
{
	GLuint programID = 0;

	// Kérdezzük le az aktuális programot!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
	glGetIntegerv( GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>( &programID ) );
	// A program és a uniform név ismeretében kérdezzük le a location-t!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
	return glGetUniformLocation( programID, uniformName );
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 ) // Először lett megnyomva
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack ); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE ); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode ); // Állítsuk be az újat!
		}
	}
	m_camera.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp( key );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove( mouse );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_camera.MouseWheel( wheel );
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.Resize( _w, _h );
}

