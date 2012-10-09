
#include "Stdafx.h"
#include "DepthRefinement.h"

//-------------------------------------------------------------------------------------
DepthRefinement::DepthRefinement(void)
	: kinectStream(0),
	currentDisplayMode(DisplayMode::RAW_DEPTH)
{
}

//-------------------------------------------------------------------------------------
DepthRefinement::~DepthRefinement(void)
{
	if(kinectStream) delete kinectStream;
}

//-------------------------------------------------------------------------------------
void DepthRefinement::createScene(void)
{
	/*
	Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
	Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	headNode->attachObject(ogreHead);
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
	Ogre::Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(20,80,50);
	*/

	kinectStream = new KinectStream();
	kinectStream->initKinectStream();
	kinectStream->initTexture(this->mSceneMgr);
	
	initDisplays();
	changeDisplayMode(DisplayMode::ALIGNED_RGB);

	/*Ogre::CompositorInstance* bilateralCompositor = Ogre::CompositorManager::getSingleton().addCompositor(this->mCamera->getViewport(), 
		"DepthRefinement/BilateralFilter");
	bilateralCompositor->setEnabled(true);*/

	/*Ogre::CompositorInstance* usermapCompositor = Ogre::CompositorManager::getSingleton().addCompositor(this->mCamera->getViewport(), 
		"DepthRefinement/Usermap");
	usermapCompositor->setEnabled(true);*/

	Ogre::CompositorInstance* usermapCompositor = Ogre::CompositorManager::getSingleton().addCompositor(this->mCamera->getViewport(), 
		"DepthRefinement/QuickShift");
	usermapCompositor->setEnabled(true);
}

//-------------------------------------------------------------------------------------
void DepthRefinement::initDisplays()
{
	// kinect stream should already be initialized
	
	// init RGB display
	rgbRect = new Ogre::Rectangle2D(true);
	rgbRect->setCorners(-1.0, 1.0, 1.0, -1.0);	
	rgbRect->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
	Ogre::AxisAlignedBox rgbAAB;
	rgbAAB.setInfinite();
	rgbRect->setBoundingBox(rgbAAB);
	rgbDisplayNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("RGBDisplayNode");
	rgbDisplayNode->attachObject(rgbRect);	

	Ogre::MaterialPtr rgbRenderMat = Ogre::MaterialManager::getSingleton().create("RGBDisplayMat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* rgbMatTechnique = rgbRenderMat->createTechnique();
	rgbMatTechnique->createPass();
	rgbRenderMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	rgbRenderMat->getTechnique(0)->getPass(0)->createTextureUnitState("RGB_Texture");
	rgbRect->setMaterial("RGBDisplayMat");

	// init depth display
	depthRect = new Ogre::Rectangle2D(true);
	depthRect->setCorners(-1.0, 1.0, 1.0, -1.0);	
	depthRect->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
	Ogre::AxisAlignedBox depthAAB;
	depthAAB.setInfinite();
	depthRect->setBoundingBox(depthAAB);
	depthDisplayNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("DepthDisplayNode");
	depthDisplayNode->attachObject(depthRect);	

	Ogre::MaterialPtr depthRenderMat = Ogre::MaterialManager::getSingleton().create("DepthDisplayMat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* depthMatTechnique = depthRenderMat->createTechnique();
	depthMatTechnique->createPass();
	depthRenderMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	depthRenderMat->getTechnique(0)->getPass(0)->createTextureUnitState("Depth_Texture");
	depthRect->setMaterial("DepthDisplayMat");
		
	// init Index display
	indexRect = new Ogre::Rectangle2D(true);
	indexRect->setCorners(-1.0, 1.0, 1.0, -1.0);	
	indexRect->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
	Ogre::AxisAlignedBox indexAAB;
	indexAAB.setInfinite();
	indexRect->setBoundingBox(indexAAB);
	indexDisplayNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("IndexDisplayNode");
	indexDisplayNode->attachObject(indexRect);	
		
	Ogre::MaterialPtr indexRenderMat = Ogre::MaterialManager::getSingleton().create("IndexDisplayMat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* indexMatTechnique = indexRenderMat->createTechnique();
	indexMatTechnique->createPass();
	indexRenderMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	indexRenderMat->getTechnique(0)->getPass(0)->createTextureUnitState("Index_Texture");
	indexRect->setMaterial("IndexDisplayMat");

	// init Aligned RGB display
	alignedRGBRect = new Ogre::Rectangle2D(true);
	alignedRGBRect->setCorners(-1.0, 1.0, 1.0, -1.0);	
	alignedRGBRect->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
	Ogre::AxisAlignedBox aRGBAAB;
	aRGBAAB.setInfinite();
	alignedRGBRect->setBoundingBox(aRGBAAB);
	alignedRGBDisplayNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("AlignedRGBDisplayNode");
	alignedRGBDisplayNode->attachObject(alignedRGBRect);	

	Ogre::MaterialPtr aRGBRenderMat = Ogre::MaterialManager::getSingleton().create("AlignedRGBDisplayMat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* aRGBMatTechnique = aRGBRenderMat->createTechnique();
	aRGBMatTechnique->createPass();
	aRGBRenderMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	aRGBRenderMat->getTechnique(0)->getPass(0)->createTextureUnitState("Aligned_RGB_Texture");
	alignedRGBRect->setMaterial("AlignedRGBDisplayMat");

	
}

//-------------------------------------------------------------------------------------
bool DepthRefinement::frameRenderingQueued(const Ogre::FrameEvent& arg)
{
	bool returnValue = BaseApplication::frameRenderingQueued(arg);
	kinectStream->updateKinectStream(arg.timeSinceLastFrame);
	return returnValue;
}

//-------------------------------------------------------------------------------------
bool DepthRefinement::mouseMoved( const OIS::MouseEvent &arg )
{
	if(!BaseApplication::mouseMoved(arg)) { return false; }
	return true;
}

//-------------------------------------------------------------------------------------
bool DepthRefinement::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if(!BaseApplication::mousePressed(arg, id)) {return false; }
	return true;
}

//-------------------------------------------------------------------------------------
bool DepthRefinement::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if(!BaseApplication::mouseReleased(arg, id)) { return false; }
	return true;
}

//-------------------------------------------------------------------------------------
bool DepthRefinement::keyPressed( const OIS::KeyEvent &arg )
{
	if(!BaseApplication::keyPressed(arg)) { return false; }

	if (arg.key == OIS::KC_1) { changeDisplayMode(DisplayMode::RAW_DEPTH); }
	else if (arg.key == OIS::KC_2) { changeDisplayMode(DisplayMode::RAW_RGB); }
	else if (arg.key == OIS::KC_3) { changeDisplayMode(DisplayMode::INDEX); }
	else if (arg.key == OIS::KC_4) { changeDisplayMode(DisplayMode::ALIGNED_RGB); }
	else if (arg.key == OIS::KC_5) 
	{	
		if(this->mTrayMgr->areFrameStatsVisible())  this->mTrayMgr->hideFrameStats();
		else this->mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	}	

	return true;
}

//-------------------------------------------------------------------------------------
bool DepthRefinement::keyReleased( const OIS::KeyEvent &arg )
{
	if(!BaseApplication::keyReleased(arg)) { return false; }
	return true;
}

//-------------------------------------------------------------------------------------
void DepthRefinement::changeDisplayMode(DisplayMode dispMode)
{
	currentDisplayMode = dispMode;

	bool showDepth = (dispMode == DisplayMode::RAW_DEPTH);
	bool showRGB = (dispMode == DisplayMode::RAW_RGB);
	bool showIndex = (dispMode == DisplayMode::INDEX);
	bool showAlignedRGB = (dispMode == DisplayMode::ALIGNED_RGB);

	depthDisplayNode->setVisible(showDepth);
	rgbDisplayNode->setVisible(showRGB);
	indexDisplayNode->setVisible(showIndex);
	alignedRGBRect->setVisible(showAlignedRGB);
}


