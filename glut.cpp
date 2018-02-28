#include "glut.h"
#include "main.h"
#include "math.h"

#include <Windows.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables

int g_iFrameCount;
double fAngle = 0.0f;
double fCameraTranslate = 0.0f;

Vector4 v4PosRain[10000];

extern Vector4 skeletonPosition[NUI_SKELETON_POSITION_COUNT]; // Current frame position

const float fFloorY = -1.5f;

// End of global variables
///////////////////////////////////////////////////////////////////////////////////////////////////



void drawRain() 
{
	//enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the rain particles
	for (int i = 0; i < 1000; i++)
	{
		glPushMatrix();
		//light blue colour for rain
		//glColor3f(0.3, 0.3, 0.9);
		glColor4f(0.3, 0.3, 1.0, 0.2);
		glTranslatef(v4PosRain[i].x, v4PosRain[i].y, v4PosRain[i].z);
		//changing the size of the cube to mimic raindrops
		glScalef(0.2f, 2.5f, 0.2f);
		glutSolidCube(0.05);
		glPopMatrix();
	}

	glDisable(GL_BLEND);
}

void rotateCamera() 
{
	// Rotate the camera
	static double angle = 0.;
	static double radius = 3.;
	double x = radius*sin(angle);
	double z = radius*(1 - cos(angle)) - radius / 2;
	gluLookAt(x, 0, z, 0, 0, radius / 2, 0, 1, 0);
	angle += 0.01;
}

void drawCoordinate()
{
	// Draw the coordinate system
	float fSize = 0.05f;
	glPushMatrix();
	glScalef(4.0f, 0.5f, 0.5f);
	glTranslatef(fSize / 2.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glutSolidCube(fSize);
	glPopMatrix();
	glPushMatrix();
	glScalef(0.5f, 4.0f, 0.5f);
	glTranslatef(0.0f, fSize / 2.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutSolidCube(fSize);
	glPopMatrix();
	glPushMatrix();
	glScalef(0.5f, 0.5f, 4.0f);
	glTranslatef(0.0f, 0.0f, fSize / 2.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glutSolidCube(fSize);
	glPopMatrix();
}

void draw() 
{
	// Clear screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.5, 0.5, 0.5, 0);
	glClearDepth(1.0f);

	// Rotate camera
	rotateCamera();

#ifdef KINECT_SUPPORT
	drawKinectData();
#endif
	
	// Draw the xyz coordinate system
	drawCoordinate();

	//floor
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(0.0, fFloorY, 0.0);
	glScalef(8.0, 0.05, 8.0);
	glutSolidCube(1.0);
	glPopMatrix();

	//draws the rain shapes
	drawRain();

	// Swap buffers to update frame
	glutSwapBuffers();

	// Increment frame count
	g_iFrameCount++;

	
	// Change the rain position
	for (int i = 0; i < 10000; i++)
	{
		v4PosRain[i].y = v4PosRain[i].y - 0.05;
		if (v4PosRain[i].y < fFloorY)
			v4PosRain[i].y = fFloorY + 3.0;
		//sine function on the rain drops to vary the y values
		v4PosRain[i].y += sin((float)(g_iFrameCount + i) / 20.0) * 0.00;
	}


}

void setupLighting() 
{
	// Lighting parameters for 3 lights
	const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_position0[] = { 5.0f, 5.0f, 5.0f, 0.0f };
	const GLfloat light_position1[] = { -5.0f, 5.0f, 5.0f, 0.0f };
	const GLfloat light_position2[] = { 0.0f, 5.0f, -5.0f, 0.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	// Material paramters
	const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat high_shininess[] = { 100.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
}

void setupInitialCamera()
{
	// Camera setup
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, width / (GLdouble)height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, 1, 0, 1, 0);
}

void setupOpenGLParameters()
{
	// Opengl functions
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

bool initOpenGL(int argc, char* argv[]) 
{
	// Initialize an opengl window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width*windowScale,height*windowScale);
    glutCreateWindow("OpenGL & Kinect");

	// Setup opengl
	setupOpenGLParameters();
	setupLighting();
	setupInitialCamera();

	// OpenGL callback functions
	glutDisplayFunc(draw);
    glutIdleFunc(draw);

	// Initialize glew
	glewInit();

	// Initialize frame count
	g_iFrameCount = 0;

	// Initial position of the snow
	for (int i = 0; i < 10000; i++)
	{
		// (float)rand() is from 0 to RAND_MAX (which is 65535)
		// (float)rand() / (float)(RAND_MAX + 1) is from 0.0 to 1.0
		// (float)rand() / (float)(RAND_MAX + 1) * 4.0 is from 0.0 to 4.0
		// (float)rand() / (float)(RAND_MAX + 1) * 4.0 - 2.0 is from -2.0 to 2.0
		v4PosRain[i].x = (float)rand() / (float)(RAND_MAX + 1) * 4.0 - 2.0; 
		v4PosRain[i].y = (float)rand() / (float)(RAND_MAX + 1) * 3.0;
		v4PosRain[i].z = (float)rand() / (float)(RAND_MAX + 1) * 4.0 - 2.0;
	}

    return true;
}
