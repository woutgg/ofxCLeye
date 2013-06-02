#pragma once
//ofxCLeye
//by Elliot Woods, Kimchi and Chips 2010
//MIT license

//tested with openFrameworks 0.061 / Visual Studio 2008

#include "CLEyeMulticam.h"

#include "ofMain.h"
#include "ofThread.h"

// ofxCLeye driver

class ofxCLeye : public ofBaseVideoGrabber, public ofThread {
public:
	ofxCLeye();
	~ofxCLeye();
	void			listDevices();
	int				getDeviceCount();
	void			close();
	bool			initGrabber(int w, int h, bool useTexture = true, bool isGrey = false, bool useThread = true);
	void			videoSettings();
	void			setDeviceGUID(GUID _deviceGUID);

	void			setExposure(int _exposure);
	void			setGain(int _gain);
	void			setLED(bool _led);
	void			setAutoExposure(bool _autoExposure);
	void			setAutoGain(bool _autoGain);

	int				getExposure() { return exposure; };
	int				getGain() { return gain; };
	bool			getLED() { return led; };
	bool			getAutoExposure() { return autoExposure; };
	bool			getAutoGain() { return autoGain; };

	void			update();
	void			threadedFunction();


	CLEyeCameraColorMode	colourMode;

protected:
	bool					bUseThread;
	bool					bUseTexture;
	bool					bGrabberInited;
	bool					bChooseDevice;
	bool					bIsFrameNew;

	CLEyeCameraInstance		_cam;
	CLEyeCameraResolution	_resolution;

	int						deviceID;
	int						width, height;
	int						attemptFramerate;

	bool					led;
	int						gain, exposure;
	bool					autoGain, autoExposure;

	ofPixels				pixels;
	ofTexture				tex;
	unsigned char			*viPixels;
};
