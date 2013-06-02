//ofxCLeye
//by Elliot Woods, Kimchi and Chips 2010
//MIT license

#include "ofxCLeye.h"

ofxCLeye::ofxCLeye()
: ofBaseVideoGrabber(), bUseThread(true), bUseTexture(true), colourMode(CLEYE_COLOR_PROCESSED),
  bGrabberInited(false), bChooseDevice(false), bIsFrameNew(false) {
	attemptFramerate = 60;
	gain = 20;
	exposure = 511;
}

ofxCLeye::~ofxCLeye() {
	close();
}

void ofxCLeye::listDevices() {
	int numCams = getDeviceCount();

	cout << "ofxCLeye found " << numCams << " PlayStation Eye cameras:\n";

	GUID guidCam;
	for (int i = 0; i < numCams; i++) {
		guidCam = CLEyeGetCameraUUID(i);
		printf("Camera %d GUID: [%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x]\n",
				i, guidCam.Data1, guidCam.Data2, guidCam.Data3,
				guidCam.Data4[0], guidCam.Data4[1], guidCam.Data4[2],
				guidCam.Data4[3], guidCam.Data4[4], guidCam.Data4[5],
				guidCam.Data4[6], guidCam.Data4[7]);
	}
}

void ofxCLeye::close() {
	if (bGrabberInited) {
		if (bUseThread) stopThread();

		CLEyeCameraStop(_cam); // Stop camera capture
		CLEyeDestroyCamera(_cam); // Destroy camera object
		_cam = NULL;

		delete[] viPixels;
		pixels.clear();
		tex.clear();
	}

	bGrabberInited = false;
}

//static
int ofxCLeye::getDeviceCount() {
	return CLEyeGetCameraCount();
}

//static
GUID ofxCLeye::getDeviceGUID(int deviceID) {
	return CLEyeGetCameraUUID(deviceID);
}

//static
int ofxCLeye::getDeviceIdByGUID(const GUID& guid) {
	int id = -1;
	int n = getDeviceCount();
	for (int i = 0; i < n; i++) {
		if (CLEyeGetCameraUUID(i) == guid) id = i;
	}
	return id;
}

//static
bool ofxCLeye::initGrabberWithGUID(ofxCLeye& grabber, const GUID& guid, int w, int h,
						bool useTexture, bool useGrayscale, bool useThread) {
	int id = getDeviceIdByGUID(guid);
	if (id == -1) return false;

	grabber.setDeviceId(id);
	return grabber.initGrabber(w, h);
}

//static
ofxCLeye* ofxCLeye::createGrabberWithGUID(const GUID& guid, int w, int h,
						bool useTexture, bool useGrayscale, bool useThread) {
	int id = getDeviceIdByGUID(guid);
	if (id == -1) return false;

	ofxCLeye* grabber = new ofxCLeye();
	grabber->setDeviceId(id);
	bool rv = grabber->initGrabber(w, h);
	if (!rv) delete grabber;
	return rv ? grabber : NULL;
}

void ofxCLeye::setUseTexture(bool _useTexture) { bUseTexture = _useTexture; }
void ofxCLeye::setGrayscale(bool _useGrayscale) { colourMode = (_useGrayscale) ? CLEYE_MONO_PROCESSED : CLEYE_COLOR_PROCESSED; }
void ofxCLeye::setUseThread(bool _useThread) { bUseThread = _useThread; }

bool ofxCLeye::initGrabber(int w, int h) {
	if (bChooseDevice) ofLog(OF_LOG_NOTICE, "choosing %i", deviceID);
	else deviceID = 0;

	width = w;
	height = h;
	bGrabberInited = false;

	if (width == 640 && height == 480) {
		_resolution = CLEYE_VGA;
	} else if (width == 320 && height == 240) {
		_resolution = CLEYE_QVGA;
	} else {
		ofLog(OF_LOG_WARNING, "Selected resolution " + ofToString(width) + "x" + ofToString(height) + " is not available with ofxCLeye");
		ofLog(OF_LOG_WARNING, "ofxCLeye using 640x480 instead");
		_resolution = CLEYE_VGA;
	}

	if( attemptFramerate < 0) return false;

	GUID guid = CLEyeGetCameraUUID(deviceID);
	_cam = CLEyeCreateCamera(guid, colourMode, _resolution, attemptFramerate);
	bool bOk = (_cam != NULL);
	if (bOk) bOk = bOk && CLEyeCameraStart(_cam);

	int ourRequestedWidth = width;
	int ourRequestedHeight = height;

	int numChannels = (colourMode == CLEYE_MONO_PROCESSED) ? 1 : 4;
	int glFormat = (colourMode == CLEYE_MONO_PROCESSED) ? GL_LUMINANCE : GL_RGBA;

	if (bOk == true) {
		char guidmsg[150];
		sprintf(guidmsg, "ofxCLeye: Camera GUID: [%08x-%04x-%04x-v%02x%02x-%02x%02x%02x%02x%02x%02x]\n",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2],
				guid.Data4[3], guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);
		ofLog(OF_LOG_VERBOSE, guidmsg);

		if (gain <= 0) gain = 20; //default
		if (exposure <= 0) exposure = 511; //default
		CLEyeSetCameraParameter(_cam, CLEYE_GAIN, gain);
		CLEyeSetCameraParameter(_cam, CLEYE_EXPOSURE, exposure);

		CLEyeCameraGetFrameDimensions(_cam, width, height);
		bGrabberInited = true;

		//this line should also accomodate non-colour values
		//here we init an array at the cam's native resolution
		viPixels = new unsigned char[width * height * numChannels];

//		if (width == ourRequestedWidth && height == ourRequestedHeight) {
//			bDoWeNeedToResize = false;
//		} else {
//			bDoWeNeedToResize = true;
//			width = ourRequestedWidth;
//			height = ourRequestedHeight;
//		}

		pixels.allocate(width, height, numChannels);
		pixels.set(0);

		if (bUseTexture) {
			tex.allocate(width, height, glFormat);
			tex.loadData((unsigned char*)pixels.getPixels(), width, height, glFormat);
		}

		if (bUseThread) startThread(true, false);

		ofLog(OF_LOG_NOTICE, "ofxCLeye created camera successfully");
		return true;
	} else {
		ofLog(OF_LOG_ERROR, "error allocating a video device");
		ofLog(OF_LOG_ERROR, "please check your camera with CL-Eye Test");
		bGrabberInited = false;
		return false;
	}
}


void ofxCLeye::videoSettings() {
	ofLog(OF_LOG_WARNING, "ofxCLeye does not support videoSettings()");
}

bool ofxCLeye::setDeviceId(int id) {
	if (getDeviceCount() > id) {
		deviceID = id;
		bChooseDevice = true;
		return true;
	} else {
		ofLog(OF_LOG_WARNING, "ofxCLeye cannot find camera with selected ID");
		bChooseDevice = false;
		deviceID = 0;
		return false;
	}
}

bool ofxCLeye::setDeviceGUID(GUID _deviceGUID) {
	int id = getDeviceIdByGUID(_deviceGUID);

	if (id < 0) {
		ofLog(OF_LOG_WARNING, "ofxCLeye cannot find camera with selected GUID");
		deviceID = 0;
		bChooseDevice = false;
		return false;
	}

	deviceID = id;
	bChooseDevice = true;
	return true;
}

void ofxCLeye::setExposure(int _exposure) {
	exposure = ofClamp(_exposure, 0, 512);

	if (bGrabberInited) CLEyeSetCameraParameter(_cam, CLEYE_EXPOSURE, exposure);
	else ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");

}

void ofxCLeye::setGain(int _gain) {
	gain = ofClamp(_gain, 0, 100);

	if (bGrabberInited) CLEyeSetCameraParameter(_cam, CLEYE_GAIN, gain);
	else ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");


}

void ofxCLeye::setLED(bool _led) {
	led = _led;

	if (bGrabberInited) CLEyeCameraLED(_cam, led);
	else ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");
}

void ofxCLeye::setAutoExposure(bool _autoExposure) {
	autoExposure = _autoExposure;

	if (bGrabberInited) CLEyeSetCameraParameter(_cam, CLEYE_AUTO_EXPOSURE, (autoExposure? 1 : 0));
	else ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");
}

void ofxCLeye::setAutoGain(bool _autoGain) {
	autoGain = _autoGain;

	if (bGrabberInited) CLEyeSetCameraParameter(_cam, CLEYE_AUTO_GAIN,  (autoGain? 1 : 0));
	else ofLog(OF_LOG_WARNING, "ofxCLeye: Attempting to set property failed, grabber not initialed");
}

unsigned char* ofxCLeye::getPixels() { return pixels.getPixels(); }
ofPixelsRef ofxCLeye::getPixelsRef() { return pixels; }
ofTexture& ofxCLeye::getTexture() { return tex; }

bool ofxCLeye::isFrameNew() { return bIsFrameNew; }
float ofxCLeye::getWidth() { return width; }
float ofxCLeye::getHeight() { return height; }

bool ofxCLeye::setPixelFormat(ofPixelFormat fmt) {
	return false;
}

ofPixelFormat ofxCLeye::getPixelFormat() {
	return (colourMode == CLEYE_MONO_PROCESSED) ? OF_PIXELS_MONO : OF_PIXELS_RGBA;
}


void ofxCLeye::update() {
	if (bGrabberInited == true) {
		bIsFrameNew = false;
		bool success;

		if (bUseThread) success = true; //presume we've already got a frame
		else success = CLEyeCameraGetFrame(_cam, (PBYTE) viPixels); //get a frame

		if (success) {
			bIsFrameNew = true;

			pixels.setFromPixels(viPixels, width, height, (colourMode == CLEYE_MONO_PROCESSED) ? 1 : 4);
//			if (colourMode != CLEYE_MONO_PROCESSED) {
//				for (int i=0; i<width*height; i++) {
//					pixels[i*3+0] = viPixels[i*4+2];
//					pixels[i*3+1] = viPixels[i*4+1];
//					pixels[i*3+2] = viPixels[i*4+0];
//				}
//			}

			if (bUseTexture) {
				tex.loadData(pixels.getPixels(), width, height,
								(colourMode == CLEYE_MONO_PROCESSED) ? GL_LUMINANCE : GL_RGBA);
//				if (colourMode == CLEYE_MONO_PROCESSED) tex.loadData(pixels, width, height, GL_LUMINANCE);
//				else tex.loadData(pixels, width, height, GL_RGB);
			}

			if (bUseThread) unlock();
		}
	}
}

void ofxCLeye::threadedFunction() {
	while (isThreadRunning()) {
		if (lock()) {
			CLEyeCameraGetFrame(_cam, (PBYTE) viPixels);
			unlock();
		}
	}
}
