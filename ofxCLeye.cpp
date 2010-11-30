//ofxCLeye
//by Elliot Woods, Kimchi and Chips 2010
//MIT license

#include "ofxCLeye.h"

ofxCLeye::ofxCLeye() :
ofVideoGrabber()
{
	attemptFramerate = 60;
	gain = 20;
	exposure = 511;
}

ofxCLeye::~ofxCLeye()
{
	if (bUseThread)
	{
		if (lock())
			close();
	}
}

int ofxCLeye::listDevices()
{
	int numCams = CLEyeGetCameraCount();

	cout << "ofxCLeye found " << numCams << "PlayStation Eye:\n";
	
	GUID guidCam;
	for (int i=0; i<numCams; i++)
	{
		guidCam = CLEyeGetCameraUUID(i);
		printf("Camera %d GUID: [%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x]\n", 
				i, guidCam.Data1, guidCam.Data2, guidCam.Data3,
				guidCam.Data4[0], guidCam.Data4[1], guidCam.Data4[2],
				guidCam.Data4[3], guidCam.Data4[4], guidCam.Data4[5],
				guidCam.Data4[6], guidCam.Data4[7]);
	}

	return numCams;
}

void ofxCLeye::grabFrame()
{
	if (bGrabberInited == true)
	{
		bIsFrameNew = false;

		bool success;

		if (bUseThread)
			//presume we've already got a frame
			success = true;
		else
			//get a frame
			success = CLEyeCameraGetFrame(_cam, (PBYTE) viPixels);

		
		if (success){

			bIsFrameNew = true;

			if (bDoWeNeedToResize == true){

				int inputW, inputH;
				CLEyeCameraGetFrameDimensions(_cam, inputW, inputH);

				float scaleW =	(float)inputW / (float)width;
				float scaleH =	(float)inputH / (float)height;

				for(int i=0;i<width;i++){
					for(int j=0;j<height;j++){

						float posx = i * scaleW;
						float posy = j * scaleH;

						/*

						// start of calculating
						// for linear interpolation

						int xbase = (int)floor(posx);
						int xhigh = (int)ceil(posx);
						float pctx = (posx - xbase);

						int ybase = (int)floor(posy);
						int yhigh = (int)ceil(posy);
						float pcty = (posy - ybase);
						*/

						int posPix = (((int)posy * inputW) + ((int)posx));

						if (colourMode == CLEYE_MONO_PROCESSED)
							pixels[(j*width) + i] = viPixels[posPix];
						else {
							pixels[(j*width)*3 + i*3 + 0] = viPixels[posPix*4 + 0];
							pixels[(j*width)*3 + i*3 + 1] = viPixels[posPix*4 + 1];
							pixels[(j*width)*3 + i*3 + 2] = viPixels[posPix*4 + 2];
						}

					}
				}

			} else {
				
				if (colourMode == CLEYE_MONO_PROCESSED)
					memcpy(pixels, viPixels, width*height);
				else
				{
					for (int i=0; i<width*height; i++)
					{
						pixels[i*3+0] = viPixels[i*4+2];
						pixels[i*3+1] = viPixels[i*4+1];
						pixels[i*3+2] = viPixels[i*4+0];
					}
				}
			}

			if (bUseTexture){
				if (colourMode == CLEYE_MONO_PROCESSED)
					tex.loadData(pixels, width, height, GL_LUMINANCE);
				else
					tex.loadData(pixels, width, height, GL_RGB);
			}

			if (bUseThread)
				unlock();

		}
	}
}

void ofxCLeye::close()
{
	if (bGrabberInited)
	{
		
		if (bUseThread)
			stopThread();

		// Stop camera capture
		CLEyeCameraStop(_cam);
		// Destroy camera object
		CLEyeDestroyCamera(_cam);
		_cam = NULL;

		delete[] viPixels;
	}

	bGrabberInited = false;
}

bool ofxCLeye::initGrabber(int w, int h, bool useTexture, bool isGrey, bool useThread)
{
	bUseTexture = useTexture;
	bUseThread = useThread;

	if (isGrey)
		colourMode = CLEYE_MONO_PROCESSED;
	else
		colourMode = CLEYE_COLOR_PROCESSED;

	if (bChooseDevice){
		ofLog(OF_LOG_NOTICE, "choosing %i", deviceID);
	} else {
		device = 0;
	}

	width = w;
	height = h;
	bGrabberInited = false;

	////////////////////////////////////////
	// RESOLUTION
	////////////////////////////////////////
	if (width == 640 && height == 480)
		_resolution=CLEYE_VGA;
	else if (width == 320 && height == 240)
		_resolution=CLEYE_QVGA;
	else
	{
		ofLog(OF_LOG_WARNING, "Selected resolution " + ofToString(width) + "x" + ofToString(height) + " is not available with ofxCLeye");
		ofLog(OF_LOG_WARNING, "ofxCLeye using 640x480 instead");
		_resolution=CLEYE_VGA;
	}
	////////////////////////////////////////

	////////////////////////////////////////
	// FPS
	////////////////////////////////////////
	if( attemptFramerate < 0)
		return false;
	////////////////////////////////////////

	////////////////////////////////////////
	// INITIALISE
	////////////////////////////////////////
	GUID guid = CLEyeGetCameraUUID(deviceID);
	_cam = CLEyeCreateCamera(guid, colourMode, _resolution, attemptFramerate);
	bool bOk = (_cam != NULL);
	if (bOk) bOk &= CLEyeCameraStart(_cam);
	////////////////////////////////////////

	
	int ourRequestedWidth = width;
	int ourRequestedHeight = height;

	if (bOk == true){
		////////////////////////////////////////
		// REPORT GUID
		////////////////////////////////////////
		char guidmsg[150];
		sprintf(guidmsg, "ofxCLeye: Camera GUID: [%08x-%04x-%04x-v%02x%02x-%02x%02x%02x%02x%02x%02x]\n", 
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2],
				guid.Data4[3], guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);
		ofLog(OF_LOG_VERBOSE, guidmsg);
		////////////////////////////////////////

		////////////////////////////////////////
		// GAIN/EXPOSURE
		////////////////////////////////////////
		if (gain<=0) //default
			gain=20;
		if (exposure<=0) //default
			exposure=511;
		CLEyeSetCameraParameter(_cam, CLEYE_GAIN, gain);
		CLEyeSetCameraParameter(_cam, CLEYE_EXPOSURE, exposure);
		////////////////////////////////////////

		CLEyeCameraGetFrameDimensions(_cam, width, height);
		bGrabberInited = true;

		//this line should also accomodate non-colour values
		//here we init an array at the cam's native resolution
		viPixels= new unsigned char[width * height * (isGrey ? 1 : 4)];

		if (width == ourRequestedWidth && height == ourRequestedHeight){
			bDoWeNeedToResize = false;
		} else {
			bDoWeNeedToResize = true;
			width = ourRequestedWidth;
			height = ourRequestedHeight;
		}

		pixels	= new unsigned char[width * height * (isGrey ? 1 : 3)];

		if (bUseTexture){
			// create the texture, set the pixels to black and
			// upload them to the texture (so at least we see nothing black the callback)
			tex.allocate(width,height,(isGrey ? GL_LUMINANCE : GL_RGB));

			memset(pixels, 0, width*height * (isGrey ? 1 : 3));
			tex.loadData(pixels, width, height, (isGrey ? GL_LUMINANCE : GL_RGB));
		}

		if (bUseThread)
			startThread(true, false);

		ofLog(OF_LOG_NOTICE, "ofxCLeye created camera successfully");
		return true;
	} else {
		ofLog(OF_LOG_ERROR, "error allocating a video device");
		ofLog(OF_LOG_ERROR, "please check your camera with CL-Eye Test");
		bGrabberInited = false;
		return false;
	}
}


void ofxCLeye::videoSettings()
{
	ofLog(OF_LOG_WARNING, "ofxCLeye does not support videoSettings()");
}

void ofxCLeye::setDeviceGUID(GUID _deviceGUID)
{
	int numCams = CLEyeGetCameraCount();

	for (int i=0; i<numCams; i++)
		if (CLEyeGetCameraUUID(i) == _deviceGUID)
		{
			ofVideoGrabber::setDeviceID(i);
			return;
		}
	
	ofLog(OF_LOG_WARNING, "ofxCLeye cannot find camera with selected GUID");
	deviceID		= 0;
	bChooseDevice	= false;
}

void ofxCLeye::update()
{
	grabFrame();
}

void ofxCLeye::setExposure(int _exposure)
{
	exposure = ofClamp(_exposure, 0, 512);

	if (bGrabberInited)
		CLEyeSetCameraParameter(_cam, CLEYE_EXPOSURE, exposure);
	else
		ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");

}

void ofxCLeye::setGain(int _gain)
{
	gain = ofClamp(_gain, 0, 100);

	if (bGrabberInited)
		CLEyeSetCameraParameter(_cam, CLEYE_GAIN, gain);
	else
		ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");


}

void ofxCLeye::setLED(bool _led)
{
	led = _led;

	if (bGrabberInited)
		CLEyeCameraLED(_cam, led);
	else
		ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");
}

void ofxCLeye::setAutoExposure(bool _autoExposure)
{
	autoExposure = _autoExposure;

	if (bGrabberInited)
		CLEyeSetCameraParameter(_cam, CLEYE_AUTO_EXPOSURE, (autoExposure? 1 : 0));
	else
		ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");
}

void ofxCLeye::setAutoGain(bool _autoGain)
{
	autoGain = _autoGain;

	if (bGrabberInited)
		CLEyeSetCameraParameter(_cam, CLEYE_AUTO_GAIN,  (autoGain? 1 : 0));
	else
		ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");
}

void ofxCLeye::threadedFunction()
{
	while (isThreadRunning())
		if (lock())
		{
			CLEyeCameraGetFrame(_cam, (PBYTE) viPixels);
			unlock();
		}
}