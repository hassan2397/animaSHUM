#pragma once

#include <Windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>

const float windowScale = 1.5f;

void rotateCamera();

void drawCoordinate();
void draw();

void setupLighting();
void setupInitialCamera();
void setupOpenGLParameters();

bool initOpenGL(int argc, char* argv[]);
