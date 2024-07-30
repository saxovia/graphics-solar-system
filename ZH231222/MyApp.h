#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include "GLUtils.hpp"
#include "Camera.h"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f; // Program indulása óta eltelt idő
	float DeltaTimeInSec   = 0.0f; // Előző Update óta eltelt idő
};

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update( const SUpdateInfo& );
	void Render();
	void RenderGUI();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	void SetupDebugCallback();

	//
	// Adat változók
	//
	float m_sunSize = 1.0f;
	float m_ElapsedTimeInSec = 0.0f;

	// Kamera
	Camera m_camera;

	//
	// OpenGL-es dolgok
	//
	
	// uniform location lekérdezése
	GLint ul( const char* uniformName ) noexcept;

	// shaderekhez szükséges változók
	GLuint m_programID = 0;		  // shaderek programja
	GLuint m_programSkyboxID = 0; // skybox programja
	GLuint m_programSunID = 0; 
	GLuint m_normalMapID;

	// Fényforrás- ...
	glm::vec4 m_lightPos = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	glm::vec3 m_La = glm::vec3(0.75, 0.5, 0.4);
	glm::vec3 m_Ld = glm::vec3(0.85, 0.8, 0.8);
	glm::vec3 m_Ls = glm::vec3(0.9, 0.9, 0.9);

	float m_lightConstantAttenuation    = 1.0;
	float m_lightLinearAttenuation      = 0.0;
	float m_lightQuadraticAttenuation   = 0.0;

	// ... és anyagjellemzők
	glm::vec3 m_Ka = glm::vec3(0.2);
	glm::vec3 m_Kd = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 m_Ks = glm::vec3( 1.0 );

	float m_Shininess = 80.0f;

	// Shaderek inicializálása, és törtlése
	void InitShaders();
	void CleanShaders();
	void InitSkyboxShaders();
	void CleanSkyboxShaders();
	void InitSunShaders();
	void CleanSunShaders();

	// Geometriával kapcsolatos változók

	OGLObject m_SuzanneGPU = {};
	OGLObject m_surfaceGPU = {};
	OGLObject m_SunGPU = {};
	OGLObject m_SkyboxGPU = {};
	OGLObject m_quadGPU = {};
	OGLObject m_AsteroidGPU = {};

	// Geometria inicializálása, és törtlése
	void InitGeometry();
	void CleanGeometry();
	void InitSkyboxGeometry();
	void CleanSkyboxGeometry();
	void InitSunGeometry();
	void CleanSunGeometry();

	// Textúrázás, és változói

	GLuint m_SuzanneTextureID = 0;
	GLuint m_surfaceTextureID = 0;



	GLuint m_sunTextureID = 0;
	GLuint m_mercuryTextureID = 0;
	GLuint m_venusTextureID = 0;
	GLuint m_earthTextureID = 0;
	GLuint m_moonTextureID = 0;
	GLuint m_marsTextureID = 0;
	GLuint m_jupiterTextureID = 0;
	GLuint m_saturnTextureID = 0;
	GLuint m_uranusTextureID = 0;
	GLuint m_neptuneTextureID = 0;
	GLuint m_plutoTextureID = 0;



	GLuint m_kuiperTextureID = 0;
	GLuint m_saturnRingTextureID = 0;
	GLuint m_uranusRingTextureID = 0;
	GLuint m_neptuneRingTextureID = 0;



	GLuint m_asteroidTextureID = 0;


	GLuint m_skyboxTextureID = 0;

	void InitTextures();
	void CleanTextures();
	void InitSkyboxTextures();
	void CleanSkyboxTextures();
	void InitSunTextures();
	void CleanSunTextures();
};

