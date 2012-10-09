
#include "Stdafx.h"
#include "KinectStream.h"

//-------------------------------------------------------------------------------------
KinectStream::KinectStream(void)
{
}

//-------------------------------------------------------------------------------------
KinectStream::~KinectStream(void)
{
	NuiShutdown();

	delete[] m_colorCoordinates;
	delete[] m_depthD16;
	delete[] m_colorRGBX;
}

//-------------------------------------------------------------------------------------
void KinectStream::initTexture(Ogre::SceneManager* mSceneMgr)
{
	depthTexPtr = Ogre::TextureManager::getSingleton().createManual("Depth_Texture", 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, 
		640, 
		480, 
		0, 
		Ogre::PF_A8R8G8B8, 
		Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
	
	rgbTexPtr = Ogre::TextureManager::getSingleton().createManual("RGB_Texture", 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, 
		640, 
		480,
		0, 
		Ogre::PF_A8R8G8B8, 
		Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

	indexTexPtr = Ogre::TextureManager::getSingleton().createManual("Index_Texture", 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, 
		640, 
		480, 
		0, 
		Ogre::PF_A8R8G8B8, 
		Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

	alignedRGBTexPtr = Ogre::TextureManager::getSingleton().createManual("Aligned_RGB_Texture", 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, 
		640, 
		480, 
		0, 
		Ogre::PF_A8R8G8B8, 
		Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
}

//-------------------------------------------------------------------------------------
bool KinectStream::initKinectStream()
{
	m_colorCoordinates = new LONG[640 * 480 * 2];
	m_depthD16 = new USHORT[640 * 480];
	m_colorRGBX = new BYTE[640 * 480 * 4];

	HRESULT  hr;

	m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | 
		//NUI_INITIALIZE_FLAG_USES_SKELETON |  
		NUI_INITIALIZE_FLAG_USES_COLOR;

	hr = m_pNuiSensor->NuiInitialize( nuiFlags );
	if ( E_NUI_SKELETAL_ENGINE_BUSY == hr ) { hr = m_pNuiSensor->NuiInitialize( nuiFlags) ; }

	if ( FAILED( hr ) )
	{
		if ( E_NUI_DEVICE_IN_USE == hr ) { /* IDS_ERROR_IN_USE */ }
		else { /* IDS_ERROR_NUIINIT */ }
		return false;
	}

	if ( HasSkeletalEngine( m_pNuiSensor ) )
	{
		hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT );
		if( FAILED( hr ) ) { /*IDS_ERROR_SKELETONTRACKING return false;*/ }
	}
	
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextColorFrameEvent,
		&m_pVideoStreamHandle );
	if ( FAILED( hr ) ) { /*IDS_ERROR_VIDEOSTREAM*/ return false; }

	hr = m_pNuiSensor->NuiImageStreamOpen(
		HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextDepthFrameEvent,
		&m_pDepthStreamHandle );
	if ( FAILED( hr ) ) { /*IDS_ERROR_DEPTHSTREAM*/ return hr; }

	initKinect = true;
	return true;
}

//-------------------------------------------------------------------------------------
void KinectStream::updateKinectStream(Ogre::Real elapsedTime)
{
	if (initKinect)
	{
		updateDepth((float)elapsedTime);
		updateColor((float)elapsedTime);
		alignRGB((float)elapsedTime);
	}
}

//-------------------------------------------------------------------------------------
void KinectStream::alignRGB(float waitTime)
{
	// index buffer
	Ogre::HardwarePixelBufferSharedPtr aRGBPixelBuffer = alignedRGBTexPtr->getBuffer();
	aRGBPixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD); 
	const Ogre::PixelBox& aRGBPixelBox = aRGBPixelBuffer->getCurrentLock();
	Ogre::uint8* pDest = static_cast<Ogre::uint8*>(aRGBPixelBox.data);


	for(LONG x = 0; x < 640; ++x)
	{
		for (LONG y = 0; y < 480; ++y)
		{
			int depthIndex = x + y * 640;		

			//USHORT depth  = m_depthD16[depthIndex];
			//USHORT player = NuiDepthPixelToPlayerIndex(depth);

			LONG colorInDepthX = m_colorCoordinates[depthIndex * 2];
			LONG colorInDepthY = m_colorCoordinates[depthIndex * 2 + 1];

			int idx = x + y * 640;
			//int idx = (x * 480) + y;
			idx *= 4;

			//if ( player > 0 )
			//{
				if ( colorInDepthX >= 0 && colorInDepthX < 640 && colorInDepthY >= 0 && colorInDepthY < 480)
				{
					LONG colorIndex = colorInDepthX + colorInDepthY * 640;
					colorIndex *= 4;

					pDest[idx]		= m_colorRGBX[colorIndex];		// B
					pDest[idx + 1]	= m_colorRGBX[colorIndex + 1];	// G
					pDest[idx + 2]	= m_colorRGBX[colorIndex + 2];	// R
					pDest[idx + 3]	= 254;							// A
				}
			//}
			else
			{
				pDest[idx]		= 0;	// B
				pDest[idx + 1]	= 0;	// G
				pDest[idx + 2]	= 0;	// R
				pDest[idx + 3]	= 254;	// A
			}
		}
	}

	aRGBPixelBuffer->unlock();
}

//-------------------------------------------------------------------------------------
void KinectStream::updateColor(float waitTime)
{
	NUI_IMAGE_FRAME imageFrame;

	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_pVideoStreamHandle, waitTime, &imageFrame );

	if ( FAILED( hr ) )
	{
		return;
	}

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if ( LockedRect.Pitch != 0 )
	{
		memcpy(m_colorRGBX, LockedRect.pBits, LockedRect.size);
		
		// Get the pixel buffer
		Ogre::HardwarePixelBufferSharedPtr pixelBuffer = rgbTexPtr->getBuffer();

		// Lock the pixel buffer and get a pixel box
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD); // for best performance use HBL_DISCARD!
		const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

		Ogre::uint8* pDest = static_cast<Ogre::uint8*>(pixelBox.data);
		Ogre::uint8* pSorc = static_cast<Ogre::uint8*>(LockedRect.pBits);

		// Fill in some pixel data. This will give a semi-transparent blue,
		// but this is of course dependent on the chosen pixel format.
		for(size_t i = 0; i < 640; i++)
		{
			for (size_t j = 0; j < 480; j++)
			{
				int idx = (i * 480) + j;
				idx *= 4;

				pDest[idx]		= pSorc[0]; // B
				pDest[idx + 1]	= pSorc[1]; // G
				pDest[idx + 2]	= pSorc[2]; // R
				pDest[idx + 3]	= 254;		// A

				pSorc += 4;
			}
		}

		// Unlock the pixel buffer
		pixelBuffer->unlock();
	}
	else
	{
		// Buffer length of received texture is bogus
	}

	pTexture->UnlockRect( 0 );

	hr = m_pNuiSensor->NuiImageStreamReleaseFrame( m_pVideoStreamHandle, &imageFrame );
}

//lookups for color tinting based on player index
static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

//-------------------------------------------------------------------------------------
void KinectStream::updateDepth(float waitTime)
{
	NUI_IMAGE_FRAME imageFrame;

	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_pDepthStreamHandle, waitTime, &imageFrame );
	if ( FAILED( hr ) )
	{
		return;
	}

	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );

	if( LockedRect.Pitch != 0 )
	{
		// heap storage for depth pixel
		memcpy(m_depthD16, LockedRect.pBits, LockedRect.size);

		// depth buffer
		Ogre::HardwarePixelBufferSharedPtr depthPixelBuffer = depthTexPtr->getBuffer();
		depthPixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD); 
		const Ogre::PixelBox& depthPixelBox = depthPixelBuffer->getCurrentLock();
		Ogre::uint8* pDepthDest = static_cast<Ogre::uint8*>(depthPixelBox.data);
		USHORT* pSorc = (USHORT *)(LockedRect.pBits);

		// index buffer
		Ogre::HardwarePixelBufferSharedPtr indexPixelBuffer = indexTexPtr->getBuffer();
		indexPixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD); 
		const Ogre::PixelBox& indexPixelBox = indexPixelBuffer->getCurrentLock();
		Ogre::uint8* pIndexDest = static_cast<Ogre::uint8*>(indexPixelBox.data);

		// Fill in some pixel data. This will give a semi-transparent blue,
		// but this is of course dependent on the chosen pixel format.
		for(size_t i = 0; i < 640; i++)
		{
			for (size_t j = 0; j < 480; j++)
			{
				USHORT RealDepth = NuiDepthPixelToDepth(*pSorc);
				USHORT Player    = NuiDepthPixelToPlayerIndex(*pSorc);

				// transform 13-bit depth information into an 8-bit intensity appropriate
				// for display (we disregard information in most significant bit)
				BYTE intensity = (BYTE)~(RealDepth >> 4);

				// tint the intensity by dividing by per-player values
				RGBQUAD color;
				color.rgbRed   = 0;
				color.rgbGreen = 0;
				color.rgbBlue  = 0;
				BYTE alpha = 0;
				
				if ( Player > 0 )
				{
					color.rgbRed   = intensity >> g_IntensityShiftByPlayerR[Player];
					color.rgbGreen = intensity >> g_IntensityShiftByPlayerG[Player];
					color.rgbBlue  = intensity >> g_IntensityShiftByPlayerB[Player];
					alpha = 255;
				}
				
				int idx = (i * 480) + j;
				idx *= 4;

				pDepthDest[idx]		= intensity;
				pDepthDest[idx + 1]	= intensity;
				pDepthDest[idx + 2]	= intensity;
				pDepthDest[idx + 3]	= 255;

				
				pIndexDest[idx]		= color.rgbRed;
				pIndexDest[idx + 1]	= color.rgbGreen;
				pIndexDest[idx + 2]	= color.rgbBlue;
				pIndexDest[idx + 3]	= alpha;
				
				pSorc++;
			}
		}

		// Unlock the pixel buffer
		depthPixelBuffer->unlock();
		indexPixelBuffer->unlock();
	}

	pTexture->UnlockRect( 0 );

	m_pNuiSensor->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, &imageFrame );

	m_pNuiSensor->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
		NUI_IMAGE_RESOLUTION_640x480,
		NUI_IMAGE_RESOLUTION_640x480,
		640 * 480,
		m_depthD16,
		640 * 480 * 2,
		m_colorCoordinates
		);	
}


