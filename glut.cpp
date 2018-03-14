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

extern bool bDetectLeftArmRaised; 
extern bool bDetectRightArmRaised;
extern bool bTouchObject;

//Variables for the environment 
const float fFloorY = -1.3;//Floor
const float fWall1X = -5.0;//Wall 1
const float fWall2X = 5.0;//Wall 2
const float fWall3Z = -5.0;//Wall 3
const float fWall4Z = 5.0;//Wall 4
const float fRoofY = 1.3;//Roof

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
		if (bDetectLeftArmRaised && bDetectRightArmRaised)
		{
			glColor4f(0.3, 0.3, 1.0, 0.2);
		}
		else if (bTouchObject)
		{
			glColor4f((float)rand() / (float)(RAND_MAX + 1), (float)rand() / (float)(RAND_MAX + 1), (float)rand() / (float)(RAND_MAX + 1), 0.5);
		}
		glTranslatef(v4PosRain[i].x, v4PosRain[i].y, v4PosRain[i].z);
		//changing the size of the cube to mimic raindrops
		if (bDetectLeftArmRaised && bDetectRightArmRaised || bTouchObject)
		{
			glScalef(0.2f, 2.5f, 0.2f);
			glutSolidCube(0.05);
		}
		glPopMatrix();
	}
	glDisable(GL_BLEND);
}

void drawEnvironment()
{
	//Sky
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.6);
	glTranslatef(0.0, 0.0, 0.0);
	glutSolidSphere(50, 180, 180);
	glPopMatrix();

	//Move everything back and closer to the user
	glPushMatrix();
	glTranslatef(0.0, 0.0, 1.0);

	//Floor
	glPushMatrix();
	glColor3f(0.3, 0.3, 0.3);
	glTranslatef(0.0, fFloorY, 0.0);
	glScalef(10.0, 0.05, 10.0);
	glutSolidCube(1);
	glPopMatrix();

	//Wall 1
	glPushMatrix();
	glColor3f(0.3, 0.3, 0.3);
	glTranslatef(fWall1X, fFloorY, 0.0);
	glRotatef(90, 0, 0, fWall1X);
	glScalef(10.0, 0.05, 10.0);
	glutSolidCube(1);
	glPopMatrix();

	//Wall 2
	glPushMatrix();
	glColor3f(0.3, 0.3, 0.3);
	glTranslatef(fWall2X, fFloorY, 0.0);
	glRotatef(90, 0, 0, fWall2X);
	glScalef(10.0, 0.05, 10.0);
	glutSolidCube(1);
	glPopMatrix();

	//Wall 3
	glPushMatrix();
	glColor3f(0.3, 0.3, 0.3);
	glTranslatef(0.0, fFloorY, fWall3Z);
	glRotatef(90, fWall2X, 0, 0);
	glScalef(10.0, 0.05, 10.0);
	glutSolidCube(1);
	glPopMatrix();

	//Wall 4
	glPushMatrix();
	glColor3f(0.3, 0.3, 0.3);
	glTranslatef(0.0, fFloorY, fWall4Z);
	glRotatef(90, fWall4Z, 0, 0);
	glScalef(10.0, 0.05, 10.0);
	glutSolidCube(1);
	glPopMatrix();

	//Roof
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(0.0, fRoofY, 0.0);
	glScalef(10.0, 0.05, 10.0);
	glutSolidCube(1);
	glPopMatrix();

	/*
	Rotates all the remaining items allowing
	the Kinect to be in the correct orientation
	*/
	glPushMatrix();
	glRotatef(270, 0, 1, 0);

	//Desk Table Top
	glPushMatrix();
	glColor3f(0.5, 0.1, 0.1);
	glTranslatef(0.0, -0.2, 0.0);
	glScalef(1.0, 0.05, 3.0);
	glutSolidCube(1);
	glPopMatrix();

	//Desk Leg 1
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(0.5, -0.92, 1.5);
	glScalef(0.08, 1.5, 0.08);
	glutSolidCube(1);
	glPopMatrix();

	//Desk Leg 2
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(0.5, -0.92, -1.5);
	glScalef(0.08, 1.5, 0.08);
	glutSolidCube(1);
	glPopMatrix();

	//Desk Leg 3
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.92, 1.5);
	glScalef(0.08, 1.5, 0.08);
	glutSolidCube(1);
	glPopMatrix();

	//Desk Leg 4
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.92, -1.5);
	glScalef(0.08, 1.5, 0.08);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor base
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.3, -0.17, 0.0);
	glScalef(0.2, 0.02, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor stand
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.3, 0.1, 0.0);
	glScalef(0.08, 0.7, 0.08);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor back
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.25, 0.3, 0.0);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(0.02, 1.0, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor front Left
	glPushMatrix();
	glColor3f(0.4, 0.4, 0.4);
	glTranslatef(-0.25, 0.3, 0.5);
	glRotatef(90, 1, 0, 0);
	glScalef(0.06, 0.06, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor front Right
	glPushMatrix();
	glColor3f(0.4, 0.4, 0.4);
	glTranslatef(-0.25, 0.3, -0.5);
	glRotatef(90, 1, 0, 0);
	glScalef(0.06, 0.06, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor front Top
	glPushMatrix();
	glColor3f(0.4, 0.4, 0.4);
	glTranslatef(-0.25, 0.57, 0.0);
	glScalef(0.06, 0.06, 1.0);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor front Bottom
	glPushMatrix();
	glColor3f(0.4, 0.4, 0.4);
	glTranslatef(-0.25, 0.03, 0.0);
	glScalef(0.06, 0.06, 1.0);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor screen
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.2);
	glTranslatef(-0.24, 0.3, 0.0);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(0.02, 0.95, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor Button 1
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	glTranslatef(-0.2, 0.03, -0.4);
	glScalef(0.03, 0.03, 0.03);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor Button 2
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.2, 0.03, -0.35);
	glScalef(0.03, 0.03, 0.03);
	glutSolidCube(1);
	glPopMatrix();

	//Monitor Button 3
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-0.2, 0.03, -0.3);
	glScalef(0.03, 0.03, 0.03);
	glutSolidCube(1);
	glPopMatrix();

	//Keyboard
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(0.1, -0.17, 0.0);
	glScalef(0.2, 0.05, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	//Mouse
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(0.1, -0.17, -0.5);
	glutSolidSphere(0.05, 10, 10);
	glPopMatrix();

	//Mouse Mat
	glPushMatrix();
	glColor3f(0.2, 0.2, 0.2);
	glTranslatef(0.1, -0.17, -0.5);
	glScalef(0.2, 0.01, 0.2);
	glutSolidCube(1);
	glPopMatrix();

	//Speaker 1
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(-0.25, -0.15, 0.5);
	glRotatef(20, 0, 1, 0);
	glScalef(0.1, 0.1, 0.25);
	glutSolidCube(1);
	glPopMatrix();

	//Speaker 2
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(-0.25, -0.15, -0.5);
	glRotatef(-20, 0, 1, 0);
	glScalef(0.1, 0.1, 0.25);
	glutSolidCube(1);
	glPopMatrix();

	//Paper On The Desk
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glTranslatef(0.1, -0.17, 1.0);
	glScalef(0.5, 0.001, 0.3);
	glutSolidCube(1);
	glPopMatrix();

	//Pencil
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(0.1, -0.17, 1.0);
	glRotatef(-20, 0, 1, 0);
	glScalef(0.02, 0.02, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	//Computer Tower
	glPushMatrix();
	glColor3f(0.2, 0.2, 0.2);
	glTranslatef(0.0, -0.92, -1.1);
	glScalef(1.0, 1.0, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	//Under Desk Back Panel
	glPushMatrix();
	glColor3f(0.5, 0.1, 0.1);
	glTranslatef(-0.5, -0.45, 0.0);
	glScalef(0.05, 0.5, 3.0);
	glutSolidCube(1);
	glPopMatrix();

	//Under Desk Left Panel
	glPushMatrix();
	glColor3f(0.5, 0.1, 0.1);
	glTranslatef(0.0, -0.45, -1.5);
	glScalef(1.0, 0.5, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	//Under Desk Right Panel
	glPushMatrix();
	glColor3f(0.5, 0.1, 0.1);
	glTranslatef(0.0, -0.45, 1.5);
	glScalef(1.0, 0.5, 0.05);
	glutSolidCube(1);
	glPopMatrix();

	//TV
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(-4.9, 0.6, 0.0);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(0.04, 2.0, 1.2);
	glutSolidCube(1);
	glPopMatrix();

	//TV front Left
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-4.9, 0.6, 1.0);
	glRotatef(90, 1, 0, 0);
	glScalef(0.06, 0.06, 1.25);
	glutSolidCube(1);
	glPopMatrix();

	//TV front Right
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-4.9, 0.6, -1.0);
	glRotatef(90, 1, 0, 0);
	glScalef(0.06, 0.06, 1.25);
	glutSolidCube(1);
	glPopMatrix();

	//TV front Top
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-4.9, 1.2, 0.0);
	glScalef(0.06, 0.06, 2.0);
	glutSolidCube(1);
	glPopMatrix();

	//TV front Bottom
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-4.9, 0.0, 0.0);
	glScalef(0.06, 0.06, 2.0);
	glutSolidCube(1);
	glPopMatrix();

	//Table under TV
	glPushMatrix();
	glColor3f(0.6, 0.1, 0.1);
	glTranslatef(-4.9, -0.6, 0.0);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(1.0, 2.5, 0.8);
	glutSolidCube(1);
	glPopMatrix();

	//Filing Cabinet 1
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(4.8, -0.8, 4.8);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(0.5, 0.7, 2.0);
	glutSolidCube(1);
	glPopMatrix();

	//Filing Cabinet 2
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(3.9, -0.8, 4.8);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(0.5, 0.7, 2.0);
	glutSolidCube(1);
	glPopMatrix();

	//Door
	glPushMatrix();
	glColor3f(0.5, 0.4, 0.1);
	glTranslatef(5.0, -0.6, -3.8);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(0.07, 1.5, 3.0);
	glutSolidCube(1);
	glPopMatrix();

	//Door Handle
	glPushMatrix();
	glColor3f(0.5, 0.4, 0.1);
	glTranslatef(4.9, -0.3, -3.2);
	glutSolidSphere(0.05, 10, 10);
	glPopMatrix();

	//Window
	glPushMatrix();
	glColor3f(0.2, 0.2, 0.8);
	glTranslatef(0.0, 0.0, -4.9);
	glRotatef(90, fWall1X, 0, 0);
	glScalef(3.0, 0.07, 1.5);
	glutSolidCube(1);
	glPopMatrix();

	//Bookcase
	//Left Panel
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(0.0, -0.3, 4.8);
	glScalef(0.07, 2.0, 0.75);
	glutSolidCube(1);
	glPopMatrix();
	//Right Panel
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(-1.5, -0.3, 4.8);
	glScalef(0.07, 2.0, 1.0);
	glutSolidCube(1);
	glPopMatrix();
	//Top Panel
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(-0.75, 0.7, 4.8);
	glRotatef(90, 0, 0, 1);
	glScalef(0.07, 1.5, 1.0);
	glutSolidCube(1);
	glPopMatrix();
	//Bottom Panel
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(0.0, -0.3, 4.8);
	glScalef(0.07, 2.0, 1.0);
	glutSolidCube(1);
	glPopMatrix();
	//Back Panel
	glPushMatrix();
	glColor3f(0.1, 0.1, 0.1);
	glTranslatef(-0.75, -0.3, 4.8);
	glScalef(1.5, 2.0, 0.07);
	glutSolidCube(1);
	glPopMatrix();

	//End of the rotate
	glPopMatrix();

	//End of tranlastion
	glPopMatrix();
}

void rotateCamera() 
{
	//if (g_iFrameCount < 2500)
	{
		double x = 0.5;
		double z = 1.60;
		double y = 0.0;
		gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);
	}

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


	//draws the rain shapes
	drawRain();

	//draw the environment (done by Luke)
	drawEnvironment();

	// Swap buffers to update frame
	glutSwapBuffers();

	// Increment frame count
	g_iFrameCount++;

	if (bDetectLeftArmRaised && bDetectRightArmRaised || bTouchObject)
	{

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
