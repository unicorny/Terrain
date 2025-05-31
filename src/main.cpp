#include "main.h"
#include "Camera.h"
#include "Terrain/SimpleTerrain.h"
#include "Terrain/NTerrain.h"
#include "DREngine/DRLogging.h"
#include "DREngine/Engine2Main.h"
#include "DREngine/Const.h"
#include "DRCore2/Threading/DRCPUScheduler.h"
#include "DRCore2/Foundation/DRObject.h"
#include "DRCore2/Foundation/DRFile.h"

#include "SDL.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

Camera m_Camera;
DRCPUScheduler* g_MainScheduler = nullptr;
DRCPUScheduler* g_DiskScheduler = nullptr;
Terrain::SimpleTerrain g_Terrain;
CTerrain g_adaptiveTerrain;
DRProfiler timePerLoop;

DRVideoConfig GetVideoConfig()
{
	DRVideoConfig config;
	cout << "Bitte Breite und dann Hoehe angeben (z.B. 640 480)" << endl;
	int iTemp = 0;
	cin >> iTemp;
	config.setWidth(iTemp);
	cin >> iTemp;
	config.setHeight(iTemp);
	cout << "Fullscreen? (1 oder 0)" << endl;
	cin >> iTemp;
	config.setFullscreen(iTemp);
	
/*	cout << "Pixelformat, Groesse von Rot, Gruen, Blau und Alpha Komponente (z.B. 8 8 8 8 bei 32 Bit)" << endl;
	cin >> pConfig->uiRedSize;
	cin >> pConfig->uiGreenSize;
	cin >> pConfig->uiBlueSize;
	cin >> pConfig->uiAlphaSize;
	*/
	cout << "Pixelformat in Bit angeben (z.B. 16 Bit, 32 Bit)" << endl;
	cin >> iTemp;
	config.setPixelDeep(iTemp);

	cout << "Groesse Z-Buffer und Stencil Buffer (z.B. 24 8)" << endl;
	cin >> iTemp;
	config.setZBuffer(iTemp);
	cin >> iTemp;
	config.setStencilBuffer(iTemp);

	cout << "Multisampling angeben (oder auch Anti-Alising genannt) (z.B. 2, 4)" << endl;
	cin >> iTemp;
	config.setMultiSampling(iTemp);

	DRFile file("VideoConfig.cfg", "wb");
	auto result = file.write(&config, sizeof(DRVideoConfig), 1);
	if(File_OK != result)
	{
		cout << "Schreiben in Datei fehlgeschlagen!" << endl;
		LOG_ERROR("Fehler beim Speichern der VideoConfig", DRVideoConfig());
	}
	file.close();
	return config;
}

DRReturn Load()
{
	g_MainScheduler = new DRCPUScheduler(SDL_GetCPUCount(), "mainScheduler");
	g_DiskScheduler = new DRCPUScheduler(2, "diskScheduler");
	// g_MainScheduler = new DRCPUScheduler(4, "mainScheduler");
	// g_DiskScheduler = g_MainScheduler;
	if(EnInit(1.0f))
	{
	//	LOG_ERROR("Fehler beim Engine Init!", DR_ERROR);
		printf("Fehler beim Engine Init!");
		return DR_ERROR;
	}

	DRVideoConfig config;
	bool configLoaded = false;
	DRFile file("VideoConfig.cfg", "rb");
	if (file.isOpen() && file.getSize() == sizeof(DRVideoConfig)) 
	{
		auto result = file.read(&config, sizeof(DRVideoConfig), 1);
		if (File_OK != result) {
			LOG_ERROR("Error on loading Video Config", DR_ERROR);
		}
		configLoaded = true;
	}
	else 
	{
		LOG_INFO("no valid VideoConfig.cfg exist");
	}
	if(!configLoaded)
	{
		config = GetVideoConfig();
	}
	if(EnInit_OpenGL(1.0f, config, "Terrain mit OpenGL", "Terrain.bmp"))
	{
		printf("Fehler beim Init von OpenGL!");
		return DR_ERROR;
	}
	// if (g_adaptiveTerrain.Init("Data/Terrain512")) LOG_ERROR("Fehler bei Terrain Edit", DR_ERROR);
	if (g_Terrain.loadFromTTP("Data/Terrain512.ttp")) LOG_ERROR("Fehler beim Terrain Init", DR_ERROR);
	// if (g_Terrain.loadFromHMP("Data/testMap.hmp", "Data/Terrain512.ttp")) LOG_ERROR("Fehler beim Terrain Init", DR_ERROR);
	//if(g_Terrain.Init("Data/Terrain512.ttp")) LOG_ERROR("Fehler beim terrain2 Init", DR_ERROR);
	//DRLog.writeToLog("Time for DRTerrain: %s", timeUsed.string().data());

	//GL Dinge
	//OpenGL einrichten
	
	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	//Reseten der Matrixen
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	auto resolution = config.getResolution();
	gluPerspective(45.0f, (GLfloat)resolution.x / (GLfloat)resolution.y, 0.1f, 100000.0f);

    glMatrixMode(GL_MODELVIEW);          // Select the modelview matrix
//	glTranslatef(0.0f, 10.0f, 0.0f);
//	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glLoadIdentity();                    // Reset (init) the modelview matrix
    // gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);             // Enable smooth shading
    glClearDepth(1.0f);                  // Clear depth buffer
    glEnable(GL_DEPTH_TEST);             // Enables depth test
    glDepthFunc(GL_LEQUAL);              // Type of depth test to perform

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glLineWidth(1.0f);

    // Enhances image quality
    glColor3f(0.5f, 0.5f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	//Schwarze Clear Farbe festlegen
	glClearColor(0.01f, 0.03f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Camera.Reset();
	m_Camera.SetMovementFriction(0.0f);
	m_Camera.SetRotationFriction(0.0f);
	m_Camera.SetPosition(-402.0f, -190.0f, -344.0f);
	//m_Camera.SetPosition(-10.0f, 0.0f, -25.0f);
	m_Camera.RotateRel({ -0.0f * GRADTORAD, 135.0f * GRADTORAD, 0.0f });
	// DRLog.writeMatrixToLog(m_Camera.GetMatrix(), "Camera");

	return DR_OK;
}

//----------------------------------------------------------------------------------------------------------------------
void Exit()
{
	EnExit();
	delete g_MainScheduler;
	delete g_DiskScheduler;
	g_MainScheduler = nullptr;
	g_DiskScheduler = nullptr;
}

//********************************************************************************************************************++
DRReturn Move(float fTime)
{
	float fRotSpeed = 300.0f;
	float fSpeed = 200.0f;
	//Kamera
	const Uint8 *keystate = SDL_GetKeyboardState(nullptr);

	float xCh = fRotSpeed * -(fTime * (float)keystate[SDL_SCANCODE_UP]     + -fTime * (float)keystate[SDL_SCANCODE_DOWN]);
	float yCh = fRotSpeed * -(fTime * (float)keystate[SDL_SCANCODE_LEFT]   + -fTime * (float)keystate[SDL_SCANCODE_RIGHT]);
	float zCh = fRotSpeed* -(fTime * (float)keystate[SDL_SCANCODE_Q] + -fTime * (float)keystate[SDL_SCANCODE_E]);
	m_Camera.AddRotationRel(DRVector3(xCh, yCh, zCh));

	//Vorwärts Rückwärts
	m_Camera.AddVelocityRel(DRVector3(0.0f, 0.0f, (float)keystate[SDL_SCANCODE_KP_PLUS] * fSpeed));
	m_Camera.AddVelocityRel(DRVector3(0.0f, 0.0f, (float)keystate[SDL_SCANCODE_KP_MINUS] * -fSpeed));

	//Links/Rechts
	m_Camera.AddVelocityRel(DRVector3((float)keystate[SDL_SCANCODE_A] *  fSpeed, 0.0f, 0.0f));
	m_Camera.AddVelocityRel(DRVector3((float)keystate[SDL_SCANCODE_D] * -fSpeed, 0.0f, 0.0f));

	//Hoch/Runter
	m_Camera.AddVelocityRel(DRVector3(0.0f, (float)keystate[SDL_SCANCODE_S] *  fSpeed, 0.0f));
	m_Camera.AddVelocityRel(DRVector3(0.0f, (float)keystate[SDL_SCANCODE_W] * -fSpeed, 0.0f));

	m_Camera.Move(fTime);
	DRVector3 vP = m_Camera.GetPosition();

	// printf("\r%f %f %f", vP.x, vP.y, vP.z);
	return DR_OK;
}
//********************************************************************************************************************++
DRReturn Render(float fTime)
{
	// force reduce framerate
	while (timePerLoop.millis() < 8.0 && g_Terrain.isLoaded()) {
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	}
	timePerLoop.reset();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(m_Camera.GetMatrix());

	glDisable(GL_LIGHTING);
	//Lichter setzen

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.00035f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0f);

	GLfloat	Ambient[] = {1.0f, 0.8f, 0.8f};
	GLfloat	Diffuse[] = {0.4f, 0.4f, 0.4f};
	GLfloat Pos[] = {-402.0f, -190.0f, -344.0f};
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, Diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, Pos);
	glEnable(GL_LIGHT1);

	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	if (SDL_GetKeyboardState(0)[SDL_SCANCODE_R]) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if(g_Terrain.Render() == DR_ERROR) LOG_ERROR("Fehler beim Terrain Render!", DR_ERROR);
	//if (g_adaptiveTerrain.Render(fTime)) { LOG_ERROR("Fehler beim Terrain2 Render!", DR_ERROR); }

	glTranslatef(0.0f, -10.0f, 0.0f);	
	
	glBegin(GL_TRIANGLES);
		glVertex3f(10.0f, 0.0f, 0.0f); glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 10.0f, 0.0f); glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 10.0f); glColor3f(0.0f, 0.0f, 1.0f);
	glEnd();

	return DRGrafikError("Fehler in MainRender");	
}
//----------------------------------------------------------------------------------------------------------------------
// Main Funktion
//main
#ifdef _WIN32
#undef main
#endif
int main()
{
	if(Load())
	{
		printf("Fehler bei Load");
		Exit();
		return 0;
	}
	timePerLoop.reset();
	if(EnGameLoop(Move, Render))
	{
		LOG_WARNING("Fehler in der GameLoop, Exit");
		Exit();
		return -1;
	}
	Exit();

	return 0;
}