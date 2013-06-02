#include "testApp.h"

static GUID globeCamGuid { 0x28da3561, 0x4334, 0xcebe, { 0x8a, 0x7a, 0x06, 0xf8, 0xe9, 0xf2, 0x79, 0x77 } };
static GUID menuCamGuid { 0xfa3b2701, 0xba88, 0x922b, { 0xa3, 0xf8, 0x99, 0x38, 0xeb, 0xf9, 0xec, 0x35 } };

//--------------------------------------------------------------
void testApp::setup(){
	menuCam.listDevices();

	bool globeCamInited = ofxCLeye::initGrabberWithGUID(globeCam, globeCamGuid, 640, 480, true, false, false);
	bool menuCamInited = ofxCLeye::initGrabberWithGUID(menuCam, menuCamGuid, 640, 480, true, false, false);

	if (!globeCamInited || !menuCamInited) {
		cout << "Error: could not initialize both cameras ("
			<< (globeCamInited ? '+' : '-') << "globeCam, "
			<< (menuCamInited ? '+' : '-') << "menuCam"
			<< "), exiting with code 6." << endl;
		::exit(6);
	}
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
