#include "main.h"
#include <math.h>
#include <Windows.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>
///////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables

// OpenGL Variables
long depthToRgbMap[width*height * 2];
// We'll be using buffer objects to store the kinect point cloud
GLuint vboId;
GLuint cboId;
// Kinect variables
HANDLE depthStream;
HANDLE rgbStream;
INuiSensor* sensor;
// Stores the coordinates of each joint
Vector4 skeletonPosition[NUI_SKELETON_POSITION_COUNT]; // Current frame position
													   // boolean variables to show what is detected
bool bDetectArmSpread;
bool bDetectFeetClose;
bool bDetectLeftHandSwing;
bool bDetectRightFootSwing;
bool bDetectLeftArmRaised;
bool bDetectRightArmRaised;
bool bDetectFeetSpread;
bool bDetectWave;
bool bDetectSitting;
bool bDetectHandsTogether;
bool bDetectMoving;
bool bDetectJump;
// Past position of the head
Vector4 posHPast;
// Past position of the left hand
Vector4 posLHPast;
// Past position of the left hand
Vector4 posRHPast;
//Past position of the right foot
Vector4 posRFPast;
//Past position of the hip center
Vector4 posHCPast;
// Store the time of the current and the past frames
DWORD timeCurrent, timePast;
//location of floor
const float fFloorY = -1.3;
//TOUCHABLE OBJECT
Vector4 posObject;
float fSize;
bool bTouchObject;
//fRAME NUMBER
int iFrame;
// End of global variables
///////////////////////////////////////////////////////////////////////////////////////////////////
bool initKinect()
{
	// Get a working kinect sensor
	int numSensors;
	if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) return false;
	if (NuiCreateSensorByIndex(0, &sensor) < 0) return false;
	// Initialize sensor
	sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_SKELETON);
	sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, // Depth camera or rgb camera?
		NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
		0,        // Image stream flags, e.g. near mode
		2,        // Number of frames to buffer
		NULL,     // Event handle
		&depthStream);
	sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, // Depth camera or rgb camera?
		NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
		0,      // Image stream flags, e.g. near mode
		2,      // Number of frames to buffer
		NULL,   // Event handle
		&rgbStream);
	sensor->NuiSkeletonTrackingEnable(NULL, 0); // NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT for only upper body
	return sensor;
}
void getDepthData(GLubyte* dest)
{
	float* fdest = (float*)dest;
	long* depth2rgb = (long*)depthToRgbMap;
	NUI_IMAGE_FRAME imageFrame;
	NUI_LOCKED_RECT LockedRect;
	if (sensor->NuiImageStreamGetNextFrame(depthStream, 0, &imageFrame) < 0) return;
	INuiFrameTexture* texture = imageFrame.pFrameTexture;
	texture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0) {
		const USHORT* curr = (const USHORT*)LockedRect.pBits;
		for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; ++i) {
				// Get depth of pixel in millimeters
				USHORT depth = NuiDepthPixelToDepth(*curr++);
				// Store coordinates of the point corresponding to this pixel
				Vector4 pos = NuiTransformDepthImageToSkeleton(i, j, depth << 3, NUI_IMAGE_RESOLUTION_640x480);
				*fdest++ = pos.x / pos.w;
				*fdest++ = pos.y / pos.w;
				*fdest++ = pos.z / pos.w;
				// Store the index into the color array corresponding to this pixel
				NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
					NUI_IMAGE_RESOLUTION_640x480, NUI_IMAGE_RESOLUTION_640x480, NULL,
					i, j, depth << 3, depth2rgb, depth2rgb + 1);
				depth2rgb += 2;
			}
		}
	}
	texture->UnlockRect(0);
	sensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);
}
void getRgbData(GLubyte* dest)
{
	float* fdest = (float*)dest;
	long* depth2rgb = (long*)depthToRgbMap;
	NUI_IMAGE_FRAME imageFrame;
	NUI_LOCKED_RECT LockedRect;
	if (sensor->NuiImageStreamGetNextFrame(rgbStream, 0, &imageFrame) < 0) return;
	INuiFrameTexture* texture = imageFrame.pFrameTexture;
	texture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0) {
		const BYTE* start = (const BYTE*)LockedRect.pBits;
		for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; ++i) {
				// Determine rgb color for each depth pixel
				long x = *depth2rgb++;
				long y = *depth2rgb++;
				// If out of bounds, then don't color it at all
				if (x < 0 || y < 0 || x > width || y > height) {
					for (int n = 0; n < 3; ++n) *(fdest++) = 0.0f;
				}
				else {
					const BYTE* curr = start + (x + width*y) * 4;
					for (int n = 0; n < 3; ++n) *(fdest++) = curr[2 - n] / 255.0f;
				}
			}
		}
	}
	texture->UnlockRect(0);
	sensor->NuiImageStreamReleaseFrame(rgbStream, &imageFrame);
}
void getSkeletalData()
{
	// Backup the left hand position before it is overwritten
	posLHPast.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x;
	posLHPast.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y;
	posLHPast.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z;
	// Backup the left hand position before it is overwritten
	posRHPast.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x;
	posRHPast.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y;
	posRHPast.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z;
	// Backup the right foot position before it is overwritten
	posRFPast.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x;
	posRFPast.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y;
	posRFPast.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z;
	// Backup the right foot position before it is overwritten
	posHPast.x = skeletonPosition[NUI_SKELETON_POSITION_HEAD].x;
	posHPast.y = skeletonPosition[NUI_SKELETON_POSITION_HEAD].y;
	posHPast.z = skeletonPosition[NUI_SKELETON_POSITION_HEAD].z;
	// Backup the right foot position before it is overwritten
	posHCPast.x = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].x;
	posHCPast.y = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].y;
	posHCPast.z = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].z;
	// Get system time and backup previous system time
	timePast = timeCurrent;
	timeCurrent = timeGetTime();
	// Getting current frame information
	NUI_SKELETON_FRAME skeletonFrame = { 0 };
	if (sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame) >= 0) {
		sensor->NuiTransformSmooth(&skeletonFrame, NULL);
		// Loop over all sensed skeletons
		for (int z = 0; z < NUI_SKELETON_COUNT; ++z) {
			const NUI_SKELETON_DATA& skeleton = skeletonFrame.SkeletonData[z];
			// Check the state of the skeleton
			if (skeleton.eTrackingState == NUI_SKELETON_TRACKED) {
				// Copy the joint positions into our array
				for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i) {
					skeletonPosition[i] = skeleton.SkeletonPositions[i];
					if (skeleton.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_NOT_TRACKED) {
						skeletonPosition[i].w = 0;
					}
				}
				break; // Only take the data for one skeleton
			}
		}
	}
	// Check actions
	actionCheckArmSpread();
	actionCheckFeetClose();
	actionCheckLeftHandSwing();
	actionCheckLeftArmRaised();
	actionCheckRightArmRaised();
	actionCheckRightFootSwing();
	actionCheckFeetSpread();
	actionCheckWave();
	actionCheckSitting();
	actionCheckHandsClapping();
	actionCheckMoving();
	// Detect collison using hands
	Vector4 posLH, posRH;
	posLH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x;
	posLH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y;
	posLH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z;
	posRH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x;
	posRH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y;
	posRH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z;
	{
		// Distance calculation left hand
		float fDistanceLH;
		fDistanceLH = (posObject.x - posLH.x)*(posObject.x - posLH.x) + (posObject.y - posLH.y)*(posObject.y - posLH.y) + (posObject.z - posLH.z)*(posObject.z - posLH.z);
		fDistanceLH = sqrt(fDistanceLH);
		// Distance calculation right hand
		float fDistanceRH;
		fDistanceRH = (posObject.x - posRH.x)*(posObject.x - posRH.x) + (posObject.y - posRH.y)*(posObject.y - posRH.y) + (posObject.z - posRH.z)*(posObject.z - posRH.z);
		fDistanceRH = sqrt(fDistanceRH);
		// Detect collision
		if ((fDistanceLH < fSize) || (fDistanceRH < fSize))
			bTouchObject = true;
		else
			bTouchObject = false;
	}
}
void actionCheckMoving()
{
	// Detect hip centre
	Vector4 velHC;
	velHC.x = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].x - posHCPast.x;
	velHC.y = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].y - posHCPast.y;
	velHC.z = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].z - posHCPast.z;
	float timeDiff = (float)(timeCurrent - timePast); // in millisecond
	timeDiff = timeDiff / 1000.0f; // in second now
	velHC.x = velHC.x / timeDiff;
	velHC.y = velHC.y / timeDiff;
	velHC.z = velHC.z / timeDiff;
	// Detect moving around
	float fVelHC = sqrt(velHC.x*velHC.x + velHC.y*velHC.y + velHC.z*velHC.z);
	bDetectMoving = false;
	if (fVelHC > 0.3f) // 0.3 meter per second
		bDetectMoving = true;
}

void actionCheckLeftHandSwing()
{
	// Detect left hand swing using velocity
	Vector4 velLH;
	velLH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x - posLHPast.x;
	velLH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y - posLHPast.y;
	velLH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z - posLHPast.z;
	float timeDiff = (float)(timeCurrent - timePast); // in millisecond
	timeDiff = timeDiff / 1000.0f; // in second now
	velLH.x = velLH.x / timeDiff;
	velLH.y = velLH.y / timeDiff;
	velLH.z = velLH.z / timeDiff;
	// Detect left hand swing
	float fVelLH = sqrt(velLH.x*velLH.x + velLH.y*velLH.y + velLH.z*velLH.z);
	bDetectLeftHandSwing = false;
	if (fVelLH > 1.0f) // 1.0 meter per second
		bDetectLeftHandSwing = true;
}
void actionCheckWave()
{
	//get positons for the right elbow and right hand
	Vector4 posRH, posE;
	posRH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x;
	posRH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y;
	posRH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z;
	posE.x = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT].x;
	posE.y = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT].y;
	posE.z = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT].z;
	//Detect the right hand with the right elbow (wave)
	bDetectWave = false;
	if (posRH.y > posE.y)
		if (posRH.x > posE.x)
			bDetectWave = true;
}
void actionCheckSitting()
{
	//get positions of both left and right hip and left and right knee
	Vector4 posRH, posRK, posLH, posLK;
	posRH.x = skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT].x;
	posRH.y = skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT].y;
	posRH.z = skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT].z;
	posRK.x = skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT].x;
	posRK.y = skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT].y;
	posRK.z = skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT].z;
	posLH.x = skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT].x;
	posLH.y = skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT].y;
	posLH.z = skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT].z;
	posLK.x = skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT].x;
	posLK.y = skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT].y;
	posLK.z = skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT].z;
	//detect the sitting position 
	bDetectSitting = false;
	if ((posRH.y > posRK.y) && (posLH.y > posLK.y))
		if ((posRH.x < posRK.x) && (posLH.x < posLK.x))
			bDetectSitting = true;
}
void actionCheckRightFootSwing()
{
	// Detect right foot swing using velocity
	Vector4 velRF;
	velRF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x - posRFPast.x;
	velRF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y - posRFPast.y;
	velRF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z - posRFPast.z;
	float timeDiff = (float)(timeCurrent - timePast); // in millisecond
	timeDiff = timeDiff / 1000.0f; // in second now
	velRF.x = velRF.x / timeDiff;
	velRF.y = velRF.y / timeDiff;
	velRF.z = velRF.z / timeDiff;
	// Detect right foot swing
	float fVelRF = sqrt(velRF.x*velRF.x + velRF.y*velRF.y + velRF.z*velRF.z);
	bDetectRightFootSwing = false;
	if (fVelRF > 1.0f) // 1.0 meter per second
		bDetectRightFootSwing = true;
}
void actionCheckFeetClose()
{
	// Get feet positions
	Vector4 posLF, posRF;
	posLF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].x;
	posLF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].y;
	posLF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].z;
	posRF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x;
	posRF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y;
	posRF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z;
	// Calculate distance
	Vector4 vecDistance;
	float fDistance;
	vecDistance.x = posLF.x - posRF.x;
	vecDistance.y = posLF.y - posRF.y;
	vecDistance.z = posLF.z - posRF.z;
	fDistance = sqrt(vecDistance.x*vecDistance.x + vecDistance.y*vecDistance.y + vecDistance.z*vecDistance.z);
	// Check if feet are closed
	bDetectFeetClose = false;
	if (fDistance < 0.3f) // in meter
		bDetectFeetClose = true;
}
void actionCheckArmSpread()
{
	// Get hand positions
	Vector4 posLH, posRH;
	posLH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x;
	posLH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y;
	posLH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z;
	posRH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x;
	posRH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y;
	posRH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z;
	// Calculate distance
	Vector4 vecDistance;
	float fDistance;
	vecDistance.x = posLH.x - posRH.x;
	vecDistance.y = posLH.y - posRH.y;
	vecDistance.z = posLH.z - posRH.z;
	fDistance = sqrt(vecDistance.x*vecDistance.x + vecDistance.y*vecDistance.y + vecDistance.z*vecDistance.z);
	// Check arm spread
	bDetectArmSpread = false;
	if (fDistance > 1.2) // in meter
		bDetectArmSpread = true;
}
void actionCheckFeetSpread()
{
	// Get feet positions
	Vector4 posRF, posLF;
	posLF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].x;
	posLF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].y;
	posLF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].z;
	posRF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x;
	posRF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y;
	posRF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z;
	// Calculate distance
	Vector4 vecDistance;
	float fDistance;
	vecDistance.x = posLF.x - posRF.x;
	vecDistance.y = posLF.y - posRF.y;
	vecDistance.z = posLF.z - posRF.z;
	fDistance = sqrt(vecDistance.x*vecDistance.x + vecDistance.y*vecDistance.y + vecDistance.z*vecDistance.z);
	// Check feet spread
	bDetectFeetSpread = false;
	if (fDistance > 0.5) // in meter
		bDetectFeetSpread = true;
}
void actionCheckLeftArmRaised()
{
	// Get L ELBOW positions
	Vector4 posLE;
	posLE.x = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT].x;
	posLE.y = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT].y;
	posLE.z = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT].z;
	//Get L SHOULDER position
	Vector4 posSL;
	posSL.x = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT].x;
	posSL.y = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT].y;
	posSL.z = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT].z;
	//Get L foot position
	Vector4 posLF;
	posLF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].x;
	posLF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].y;
	posLF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].z;
	// Calculate distance L SHOULDER TO L FOOT
	Vector4 vecDistance;
	float fDistance;
	vecDistance.x = posSL.x - posLF.x;
	vecDistance.y = posSL.y - posLF.y;
	vecDistance.z = posSL.z - posLF.z;
	fDistance = sqrt(vecDistance.x*vecDistance.x + vecDistance.y*vecDistance.y + vecDistance.z*vecDistance.z);
	// Calculate distance L ELBOW TO L FOOT
	Vector4 vecDistance2;
	float fDistance2;
	vecDistance2.x = posLE.x - posLF.x;
	vecDistance2.y = posLE.y - posLF.y;
	vecDistance2.z = posLE.z - posLF.z;
	fDistance2 = sqrt(vecDistance2.x*vecDistance2.x + vecDistance2.y*vecDistance2.y + vecDistance2.z*vecDistance2.z);
	// Check left arm raised
	bDetectLeftArmRaised = false;
	if (fDistance < fDistance2) // in meter
		bDetectLeftArmRaised = true;
}

void actionCheckRightArmRaised()
{
	// Get R ELBOW positions
	Vector4 posRE;
	posRE.x = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT].x;
	posRE.y = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT].y;
	posRE.z = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT].z;
	//Get R SHOULDER position
	Vector4 posSR;
	posSR.x = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x;
	posSR.y = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y;
	posSR.z = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT].z;
	// Get R FEET positions
	Vector4 posRF;
	posRF.x = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x;
	posRF.y = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y;
	posRF.z = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z;

	// Calculate distance R SHOULDER TO R FOOT
	Vector4 vecDistance1;
	float fDistance1;
	vecDistance1.x = posSR.x - posRF.x;
	vecDistance1.y = posSR.y - posRF.y;
	vecDistance1.z = posSR.z - posRF.z;
	fDistance1 = sqrt(vecDistance1.x*vecDistance1.x + vecDistance1.y*vecDistance1.y + vecDistance1.z*vecDistance1.z);

	// Calculate distance R ELBOW TO R FOOT
	Vector4 vecDistance3;
	float fDistance3;
	vecDistance3.x = posRE.x - posRF.x;
	vecDistance3.y = posRE.y - posRF.y;
	vecDistance3.z = posRE.z - posRF.z;
	fDistance3 = sqrt(vecDistance3.x*vecDistance3.x + vecDistance3.y*vecDistance3.y + vecDistance3.z*vecDistance3.z);
	// Check right arm raised
	bDetectRightArmRaised = false;
	if (fDistance1 < fDistance3) // in meter
		bDetectRightArmRaised = true;
}
void actionCheckHandsClapping()
{
	// Get hand positions
	Vector4 posLH, posRH;
	posLH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x;
	posLH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y;
	posLH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z;
	posRH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x;
	posRH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y;
	posRH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z;
	// Calculate distance
	Vector4 vecDistance;
	float fDistance;
	vecDistance.x = posLH.x - posRH.x;
	vecDistance.y = posLH.y - posRH.y;
	vecDistance.z = posLH.z - posRH.z;
	fDistance = sqrt(vecDistance.x*vecDistance.x + vecDistance.y*vecDistance.y + vecDistance.z*vecDistance.z);
	// Detect left hand swing using velocity
	Vector4 velLH;
	velLH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x - posLHPast.x;
	velLH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y - posLHPast.y;
	velLH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z - posLHPast.z;
	float timeDiff = (float)(timeCurrent - timePast); // in millisecond
	timeDiff = timeDiff / 1000.0f; // in second now
	velLH.x = velLH.x / timeDiff;
	velLH.y = velLH.y / timeDiff;
	velLH.z = velLH.z / timeDiff;
	// Detect right hand swing using velocity
	Vector4 velRH;
	velRH.x = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x - posRHPast.x;
	velRH.y = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y - posRHPast.y;
	velRH.z = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z - posRHPast.z;
	float timeDiff1 = (float)(timeCurrent - timePast); // in millisecond
	timeDiff1 = timeDiff1 / 1000.0f; // in second now
	velRH.x = velRH.x / timeDiff1;
	velRH.y = velRH.y / timeDiff1;
	velRH.z = velRH.z / timeDiff1;
	// Detect hand swing and if together
	float fVelLH = sqrt(velLH.x*velLH.x + velLH.y*velLH.y + velLH.z*velLH.z);
	float fVelRH = sqrt(velRH.x*velRH.x + velRH.y*velRH.y + velRH.z*velRH.z);
	bDetectHandsTogether = false;
	if ((fVelRH > 1.0f) && (fVelRH > 1.0f))  // 1.0 meter per second
		if (fDistance < 0.3f) // in meter
			bDetectHandsTogether = true;
}
void getKinectData()
{
#ifdef KINECT_VOXEL_DRAW
	const int dataSize = width*height * 3 * 4;
	GLubyte* ptr;
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	ptr = (GLubyte*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (ptr) {
		getDepthData(ptr);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	ptr = (GLubyte*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (ptr) {
		getRgbData(ptr);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
	getSkeletalData();
}


void drawKinectData()
{
	// Get Kinect data and draw voxels
	getKinectData();

#ifdef KINECT_VOXEL_DRAW
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	glColorPointer(3, GL_FLOAT, 0, NULL);
	// Draw Kinect voxels
	glPointSize(1.f);
	glDrawArrays(GL_POINTS, 0, width*height);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#endif

	// Start drawing here
	drawKinectArms();
	drawAllKinectJoints();
	drawActions();
	iFrame++;
	drawCollision();
	drawKinectFullTorso();
	drawKinectMiddleTorso();
	drawKinectLowerTorso();
	drawKinectShoulderLeft();
	drawKinectShoulderRight();
	drawKinectArmLeft();
	drawKinectArmRight();
	drawKinectUpperArmLeft();
	drawKinectUpperArmRight();
	drawKinectHipLeft();
	drawKinectHipRight();
	drawKinectLegLeft();
	drawKinectLegRight();
	drawKinectLegLeftLower();
	drawKinectLegRightLower();
	drawKinectHead();
}

void drawCollision()
{
	// Draw the touchable object
	glPushMatrix();
	if (bTouchObject)
		glColor3f(1.0, 0.0, 0.0);
	else
		glColor3f(1.0, 1.0, 1.0);
	glTranslatef(posObject.x, posObject.y, posObject.z);
	glutSolidSphere(fSize, 18, 18);
	glPopMatrix();
}

//start of Luke's code
void drawKinectHead()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& head = skeletonPosition[NUI_SKELETON_POSITION_HEAD];
	const Vector4& cshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the head
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (head.w > 0 && cshoulder.w > 0) // If all detected
	{
		glVertex3f(head.x, head.y, head.z);
		glVertex3f(cshoulder.x, cshoulder.y, cshoulder.z);
	}
	glEnd();
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HEAD].x, skeletonPosition[NUI_SKELETON_POSITION_HEAD].y, skeletonPosition[NUI_SKELETON_POSITION_HEAD].z);
	glutSolidSphere(0.10f, 18, 18);
	glPopMatrix();
}
void drawKinectShoulderLeft()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& lshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	const Vector4& cshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the left shoulder
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (lshoulder.w > 0 && cshoulder.w > 0) // If all detected
	{
		glVertex3f(lshoulder.x, lshoulder.y, lshoulder.z);
		glVertex3f(cshoulder.x, cshoulder.y, cshoulder.z);
	}
	glEnd();
}
void drawKinectShoulderRight()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& rshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	const Vector4& cshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the right shoulder
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (rshoulder.w > 0 && cshoulder.w > 0) // If all detected
	{
		glVertex3f(rshoulder.x, rshoulder.y, rshoulder.z);
		glVertex3f(cshoulder.x, cshoulder.y, cshoulder.z);
	}
	glEnd();
}
void drawKinectArmLeft()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& lhand = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT];
	const Vector4& lelbow = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT];
	const Vector4& lshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the left arm
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (lhand.w > 0 && lelbow.w > 0 && lshoulder.w > 0) // If all detected
	{
		glVertex3f(lhand.x, lhand.y, lhand.z);
		glVertex3f(lelbow.x, lelbow.y, lelbow.z);
		glVertex3f(lshoulder.x, lshoulder.y, lshoulder.z);
	}
	glEnd();
}
void drawKinectArmRight()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& rhand = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT];
	const Vector4& relbow = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT];
	const Vector4& rshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the right arm
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (rhand.w > 0 && relbow.w > 0 && rshoulder.w > 0) // If all detected
	{
		glVertex3f(rhand.x, rhand.y, rhand.z);
		glVertex3f(relbow.x, relbow.y, relbow.z);
		glVertex3f(rshoulder.x, rshoulder.y, rshoulder.z);
	}
	glEnd();
}
void drawKinectUpperArmLeft()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& lshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	const Vector4& lelbow = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the upper left arm
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (lshoulder.w > 0 && lelbow.w > 0) // If all detected
	{
		glVertex3f(lshoulder.x, lshoulder.y, lshoulder.z);
		glVertex3f(lelbow.x, lelbow.y, lelbow.z);
	}
	glEnd();
}
void drawKinectUpperArmRight()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& rshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	const Vector4& relbow = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the upper right arm
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (rshoulder.w > 0 && relbow.w > 0) // If all detected
	{
		glVertex3f(rshoulder.x, rshoulder.y, rshoulder.z);
		glVertex3f(relbow.x, relbow.y, relbow.z);
	}
	glEnd();
}
void drawKinectFullTorso()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& lshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	const Vector4& rshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	const Vector4& cshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];
	const Vector4& lhip = skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT];
	const Vector4& rhip = skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT];
	const Vector4& chip = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER];
	const Vector4& spine = skeletonPosition[NUI_SKELETON_POSITION_SPINE];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the upper right arm
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 0.0f);
	if (lshoulder.w > 0 && rshoulder.w > 0 && cshoulder.w > 0 && lhip.w > 0 && rhip.w > 0 &&
		chip.w > 0 && spine.w > 0) // If all detected
	{
		//Triangle 1
		glVertex3f(lshoulder.x, lshoulder.y, lshoulder.z);
		glVertex3f(cshoulder.x, cshoulder.y, cshoulder.z);
		glVertex3f(spine.x, spine.y, spine.z);
		//Triangle 2
		glVertex3f(lshoulder.x, lshoulder.y, lshoulder.z);
		glVertex3f(lhip.x, lhip.y, lhip.z);
		glVertex3f(spine.x, spine.y, spine.z);
		//Triangle 3
		glVertex3f(lhip.x, lhip.y, lhip.z);
		glVertex3f(chip.x, chip.y, chip.z);
		glVertex3f(spine.x, spine.y, spine.z);
		//Triangle 4
		glVertex3f(chip.x, chip.y, chip.z);
		glVertex3f(rhip.x, rhip.y, rhip.z);
		glVertex3f(spine.x, spine.y, spine.z);
		//Triangle 5
		glVertex3f(rhip.x, rhip.y, rhip.z);
		glVertex3f(spine.x, spine.y, spine.z);
		glVertex3f(rshoulder.x, rshoulder.y, rshoulder.z);
		//Triangle 6
		glVertex3f(rshoulder.x, rshoulder.y, rshoulder.z);
		glVertex3f(cshoulder.x, cshoulder.y, cshoulder.z);
		glVertex3f(spine.x, spine.y, spine.z);
	}
	glEnd();
}
void drawKinectMiddleTorso()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& cshoulder = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_CENTER];
	const Vector4& spine = skeletonPosition[NUI_SKELETON_POSITION_SPINE];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the middle torso
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (cshoulder.w > 0 && spine.w > 0) // If all detected
	{
		glVertex3f(cshoulder.x, cshoulder.y, cshoulder.z);
		glVertex3f(spine.x, spine.y, spine.z);
	}
	glEnd();
}

void drawKinectLowerTorso()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& spine = skeletonPosition[NUI_SKELETON_POSITION_SPINE];
	const Vector4& hipCenter = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the lower torso
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (spine.w > 0 && hipCenter.w > 0) // If all detected
	{
		glVertex3f(spine.x, spine.y, spine.z);
		glVertex3f(hipCenter.x, hipCenter.y, hipCenter.z);
	}
	glEnd();
}
void drawKinectHipLeft()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& hipCenter = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER];
	const Vector4& hipLeft = skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the left hip
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (hipCenter.w > 0 && hipLeft.w > 0) // If all detected
	{
		glVertex3f(hipCenter.x, hipCenter.y, hipCenter.z);
		glVertex3f(hipLeft.x, hipLeft.y, hipLeft.z);
	}
	glEnd();
}
void drawKinectHipRight()
{
	// Get the positions of the hands, wrist, elbows and shoulders
	const Vector4& hipCenter = skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER];
	const Vector4& hipRight = skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT];

	// Thickless of the lines
	glLineWidth(5000);
	// Draw the right hip
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (hipCenter.w > 0 && hipRight.w > 0) // If all detected
	{
		glVertex3f(hipCenter.x, hipCenter.y, hipCenter.z);
		glVertex3f(hipRight.x, hipRight.y, hipRight.z);
	}
	glEnd();
}
void drawKinectLegLeft()
{
	// Get the positions of the foot, ankle, knee and hip
	const Vector4& lhip = skeletonPosition[NUI_SKELETON_POSITION_HIP_LEFT];
	const Vector4& lknee = skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT];
	const Vector4& lfoot = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the left leg
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (lhip.w > 0 && lknee.w > 0 && lfoot.w > 0) // If all detected
	{
		glVertex3f(lhip.x, lhip.y, lhip.z);
		glVertex3f(lknee.x, lknee.y, lknee.z);
		glVertex3f(lfoot.x, lfoot.y, lfoot.z);
	}
	glEnd();
}
void drawKinectLegRight()
{
	// Get the positions of the foot, ankle, knee and hip
	const Vector4& rhip = skeletonPosition[NUI_SKELETON_POSITION_HIP_RIGHT];
	const Vector4& rknee = skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT];
	const Vector4& rfoot = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the right leg
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (rhip.w > 0 && rknee.w > 0 && rfoot.w > 0) // If all detected
	{
		glVertex3f(rhip.x, rhip.y, rhip.z);
		glVertex3f(rknee.x, rknee.y, rknee.z);
		glVertex3f(rfoot.x, rfoot.y, rfoot.z);
	}
	glEnd();
}
void drawKinectLegLeftLower()
{
	// Get the positions of the foot, ankle, knee and hip
	const Vector4& lknee = skeletonPosition[NUI_SKELETON_POSITION_KNEE_LEFT];
	const Vector4& lankle = skeletonPosition[NUI_SKELETON_POSITION_ANKLE_LEFT];
	const Vector4& lfoot = skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the lower left leg
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (lknee.w > 0 && lankle.w > 0 && lfoot.w > 0) // If all detected
	{
		glVertex3f(lknee.x, lknee.y, lknee.z);
		glVertex3f(lankle.x, lankle.y, lankle.z);
		glVertex3f(lfoot.x, lfoot.y, lfoot.z);
	}
	glEnd();
}
void drawKinectLegRightLower()
{
	// Get the positions of the foot, ankle, knee and hip
	const Vector4& rknee = skeletonPosition[NUI_SKELETON_POSITION_KNEE_RIGHT];
	const Vector4& rankle = skeletonPosition[NUI_SKELETON_POSITION_ANKLE_RIGHT];
	const Vector4& rfoot = skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT];
	// Thickless of the lines
	glLineWidth(5000);
	// Draw the lower right leg
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	if (rknee.w > 0 && rankle.w > 0 && rfoot.w > 0) // If all detected
	{
		glVertex3f(rknee.x, rknee.y, rknee.z);
		glVertex3f(rankle.x, rankle.y, rankle.z);
		glVertex3f(rfoot.x, rfoot.y, rfoot.z);
	}
	glEnd();
}

void drawActions()
{
	/*
	// Draw the arm spread action detected
	if (bDetectArmSpread)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x, skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y, skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame *20, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x, skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y, skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame *20, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	}

	// Draw the feet close action detected
	if (bDetectFeetClose)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].x, skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].y, skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame *40, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x, skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y, skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame *40, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	}

	// Draw if both feet close and arm spread are detected
	if (bDetectArmSpread && bDetectFeetClose)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].x, skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].y, skeletonPosition[NUI_SKELETON_POSITION_HIP_CENTER].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame *01, 0, 1, 0);
	glutSolidTeapot(0.2f);
	glPopMatrix();
	}
	// Draw if left hand is swinging
	if (bDetectLeftHandSwing)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x, skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y, skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z);
	glColor3f(1, 0, 0);
	glutSolidTeapot(0.15f);
	glPopMatrix();
	}

	*/
	/*
	// Draw if both ARMS RAISED
	if (bDetectLeftArmRaised && bDetectRightArmRaised)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HEAD].x, skeletonPosition[NUI_SKELETON_POSITION_HEAD].y, skeletonPosition[NUI_SKELETON_POSITION_HEAD].z);
	glColor3f(0, 0, 0);
	glutSolidTeapot(0.15f);
	glPopMatrix();
	}
	*/
	/*
	// Draw if rigth foot is swinging
	if (bDetectRightFootSwing)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x, skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y, skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z);
	glColor3f(1, 0, 0);
	glRotatef((float)iFrame *20, 0, 1, 0);
	glutSolidTeapot(0.15f);
	glPopMatrix();
	}
	*/
	/*
	// Draw if hand is waving
	if (bDetectWave)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x, skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y, skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z);
	glColor3f(0, 0, 0);
	glutSolidTeapot(0.15f);
	glPopMatrix();
	}
	*/
	/*
	// Draw the feet spread action detected
	if (bDetectFeetSpread)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].x, skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].y, skeletonPosition[NUI_SKELETON_POSITION_FOOT_LEFT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame * 20, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].x, skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].y, skeletonPosition[NUI_SKELETON_POSITION_FOOT_RIGHT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame * 20, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	}
	*/
	/*
	// Draw if sititng down
	if (bDetectSitting)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HEAD].x, skeletonPosition[NUI_SKELETON_POSITION_HEAD].y, skeletonPosition[NUI_SKELETON_POSITION_HEAD].z);
	glColor3f(0, 0, 0);
	glutSolidTeapot(0.15f);
	glPopMatrix();
	}
	*/
	/*
	// Draw the hands together action detected
	if (bDetectHandsTogether)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].x, skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].y, skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame * 40, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].x, skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].y, skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT].z);
	glColor3f(0, 1, 0);
	glRotatef((float)iFrame * 40, 0, 1, 0);
	glutSolidTeapot(0.1f);
	glPopMatrix();
	}
	*/
	/*
	if (bDetectMoving)
	{
	glPushMatrix();
	glTranslatef(skeletonPosition[NUI_SKELETON_POSITION_HEAD].x, skeletonPosition[NUI_SKELETON_POSITION_HEAD].y, skeletonPosition[NUI_SKELETON_POSITION_HEAD].z);
	glColor3f(0, 0, 0);
	glutSolidTeapot(0.15f);
	glPopMatrix();
	}
	*/
}

void drawKinectArms()
{
	// Get the positions of the hands, elbows and shoulders
	const Vector4& lh = skeletonPosition[NUI_SKELETON_POSITION_HAND_LEFT];
	const Vector4& le = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_LEFT];
	const Vector4& ls = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	const Vector4& rh = skeletonPosition[NUI_SKELETON_POSITION_HAND_RIGHT];
	const Vector4& re = skeletonPosition[NUI_SKELETON_POSITION_ELBOW_RIGHT];
	const Vector4& rs = skeletonPosition[NUI_SKELETON_POSITION_SHOULDER_RIGHT];

	// Thickless of the lines
	glLineWidth(80);
	// Draw the two arms
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.3f, 0.0f);
	if (lh.w > 0 && le.w > 0 && ls.w > 0) // If all detected
	{
		glVertex3f(lh.x, lh.y, lh.z);
		glVertex3f(le.x, le.y, le.z);
		glVertex3f(le.x, le.y, le.z);
		glVertex3f(ls.x, ls.y, ls.z);
	}
	if (rh.w > 0 && re.w > 0 && rs.w > 0) // If all detected
	{
		glVertex3f(rh.x, rh.y, rh.z);
		glVertex3f(re.x, re.y, re.z);
		glVertex3f(re.x, re.y, re.z);
		glVertex3f(rs.x, rs.y, rs.z);
	}
	glEnd();
}
void drawAllKinectJoints()
{
	// Draw the joints
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		glPushMatrix();
		glColor3f(0.0f, 0.2f, 0.7f);
		glTranslatef(skeletonPosition[i].x, skeletonPosition[i].y, skeletonPosition[i].z);
		glutSolidSphere(0.05f, 18, 18);
		glPopMatrix();
	}
}
int main(int argc, char* argv[])
{
	// Initialize variables
	posObject.x = -0.3;
	posObject.y = -0.1;
	posObject.z = 1.0;
	fSize = 0.06;
	bTouchObject = false;
	iFrame = 0;
	// Main loop
	if (!initOpenGL(argc, argv)) return 1;
#ifdef KINECT_SUPPORT
	if (!initKinect()) return 1;
	// Set up array buffers
	const int dataSize = width*height * 3 * 4;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &cboId);
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);
#endif
	glutMainLoop();
	return 0;
}