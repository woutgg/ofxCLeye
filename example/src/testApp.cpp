#include "testApp.h"

static GUID globeCamGuid { 0x28da3561, 0x4334, 0xcebe, { 0x8a, 0x7a, 0x06, 0xf8, 0xe9, 0xf2, 0x79, 0x77 } };
static GUID menuCamGuid { 0xfa3b2701, 0xba88, 0x922b, { 0xa3, 0xf8, 0x99, 0x38, 0xeb, 0xf9, 0xec, 0x35 } };

//--------------------------------------------------------------
void testApp::setup(){
	int numDevices = ofxCLeye::getDeviceCount();
	bool globeCamFound = false, menuCamFound = false;
	for (int i = 0; i < numDevices; ++i) {
		GUID currGuid = ofxCLeye::getDeviceGUID(i);
		if (currGuid.Data1 == globeCamGuid.Data1) globeCamFound = true;
		if (currGuid.Data1 == menuCamGuid.Data1) menuCamFound = true;
	}

	if (numDevices < 2 || !globeCamFound || !menuCamFound) {
		cout << "Error: less than 2 cameras found (" << numDevices << ") or missing correct GUIDs ("
			<< (globeCamFound ? '+' : '-') << "globeCam, "
			<< (menuCamFound ? '+' : '-') << "menuCam"
			<< "), exiting with code 6." << endl;
		::exit(6);
	}

	menuCam.listDevices();

	//NOTE: threading probably fails because of concurrency issues in the library, in which case we would need a mutex on lib access
	menuCam.setUseThread(false);
	globeCam.setUseThread(false);

	menuCam.setDeviceGUID(menuCamGuid);
	menuCam.initGrabber(640, 480);
	globeCam.setDeviceGUID(globeCamGuid);
	globeCam.initGrabber(640, 480);
}

//--------------------------------------------------------------
void testApp::update(){
	menuCam.update();
	globeCam.update();
}

//--------------------------------------------------------------
void testApp::draw(){
	menuCam.getTexture().draw(0, 0, 320, 240);
	menuCam.getTexture().unbind();
	globeCam.getTexture().draw(320, 0, 320, 240);
	globeCam.getTexture().unbind();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
