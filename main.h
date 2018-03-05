#pragma once

#include "glut.h"

//#define KINECT_SUPPORT
//#define KINECT_VOXEL_DRAW

const int width = 640;
const int height = 480;

bool initKinect();

void getDepthData(GLubyte* dest);
void getRgbData(GLubyte* dest);
void getSkeletalData();
void getKinectData();

void drawKinectData();
void drawKinectArms();
void drawAllKinectJoints();

void actionCheckArmSpread();
void actionCheckFeetSpread();
void actionCheckFeetClose();
void actionCheckLeftHandSwing();
void actionCheckLeftArmRaised();
void actionCheckRightArmRaised();
void actionCheckRightFootSwing();
void actionCheckWave();
void actionCheckSitting();
void actionCheckHandsClapping();
void actionCheckMoving();
void drawActions();

void drawCollision();
void drawKinectHead();
void drawKinectFullTorso();
void drawKinectLowerTorso();
void drawKinectMiddleTorso();
void drawKinectShoulderLeft();
void drawKinectShoulderRight();
void drawKinectArmLeft();
void drawKinectArmRight();
void drawKinectUpperArmLeft();
void drawKinectUpperArmRight();
void drawKinectHipLeft();
void drawKinectHipRight();
void drawKinectLegLeft();
void drawKinectLegRight();
void drawKinectLegLeftLower();
void drawKinectLegRightLower();

void drawRain();

int main(int argc, char* argv[]);
