
#ifndef __DepthRefinement_h_
#define __DepthRefinement_h_

#include "BaseApplication.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "../res/resource.h"
#endif

#include "KinectStream.h"
#include "DisplayMode.h"

class DepthRefinement : public BaseApplication
{
public:
    DepthRefinement(void);
    virtual ~DepthRefinement(void);

	virtual bool frameRenderingQueued(const Ogre::FrameEvent& arg);

protected:
	// OIS::KeyListener
	virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased( const OIS::KeyEvent &arg );

	// OIS::MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent& arg);
	virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
	virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);


    virtual void createScene(void);
	void initDisplays();
	void changeDisplayMode(DisplayMode dispMode);

private:
	
	DisplayMode currentDisplayMode;
	KinectStream* kinectStream;

	

	Ogre::Rectangle2D* depthRect;
	Ogre::SceneNode* depthDisplayNode;
	//Ogre::TexturePtr depthBufferTex;
	//Ogre::Viewport* depthViewport;

	Ogre::Rectangle2D* rgbRect;
	Ogre::SceneNode* rgbDisplayNode;
	//Ogre::TexturePtr rgbBufferTex;
	//Ogre::Viewport* rgbViewport;

	Ogre::Rectangle2D* indexRect;
	Ogre::SceneNode* indexDisplayNode;

	Ogre::Rectangle2D* alignedRGBRect;
	Ogre::SceneNode* alignedRGBDisplayNode;
};

#endif // #ifndef __DepthRefinement_h_
