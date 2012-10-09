
#ifndef __KinectStream_h_
#define __KinectStream_h_

#include "windows.h"
#include "Shlobj.h"
#include "NuiApi.h"
#include "Stdafx.h"

#pragma comment(lib, "Kinect10.lib")

class KinectStream
{
public:
	KinectStream(void);
	virtual ~KinectStream(void);

	bool initKinectStream();
	void initTexture(Ogre::SceneManager* mSceneMgr);
	void updateKinectStream(Ogre::Real elapsedTime);

	Ogre::TexturePtr getDepthTexturePtr() { return depthTexPtr; }
	Ogre::TexturePtr getRGBTexturePtr() { return rgbTexPtr; }
	Ogre::TexturePtr getIndexTexturePtr() { return indexTexPtr; }

protected:

private:
	INuiSensor*	m_pNuiSensor;
	BSTR		m_instanceId;

	HANDLE	m_hNextDepthFrameEvent;
	HANDLE  m_hNextColorFrameEvent;
	HANDLE  m_hNextSkeletonEvent;
	HANDLE  m_pDepthStreamHandle;
	HANDLE  m_pVideoStreamHandle;

	LONG*	m_colorCoordinates;
	USHORT*	m_depthD16;
	BYTE*   m_colorRGBX;

	//DWORD   m_SkeletonIds[NUI_SKELETON_COUNT];
	//RGBQUAD m_rgbWk[640 * 480];

	//int xResPOT = 1024;
	//int yResPOT = 512;

	bool initKinect;

	Ogre::TexturePtr depthTexPtr;
	Ogre::TexturePtr rgbTexPtr;
	Ogre::TexturePtr indexTexPtr;
	Ogre::TexturePtr alignedRGBTexPtr;

private:
	void updateDepth(float waitTime);
	void updateColor(float waitTime);

	void alignRGB(float waitTime);
	//RGBQUAD Nui_ShortToQuad_Depth(USHORT s);
};

#endif 