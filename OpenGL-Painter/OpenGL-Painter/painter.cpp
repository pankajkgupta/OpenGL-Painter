

#include "painter.h"

#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>

#include <openCV/cv.h>
#include <openCV/highgui.h>

#include "cvfaces.h"
#include "glscene.h"

#include "wii.h"

bool showVideoCapture = false;
bool videoIsWriting = false;

CvPoint3D32f prev;
CvPoint3D32f Gq;

GLint g_hWindow;
CvVideoWriter *writer = 0;

IplImage *screenshoot;

IplImage* processedVideo;

// create a wiimote object
wiimote remote;

void wii_init()
{
	
	// we use a state-change callback to get notified of
	//  extension-related events, and polling for everything else
	// (note you don't have to use both, use whatever suits your app):
	remote.ChangedCallback		= on_state_change;
	//  notify us only when the wiimote connected sucessfully, or something
	//   related to extensions changes
	remote.CallbackTriggerFlags = (state_change_flags)(CONNECTED |
													   EXTENSION_CHANGED |
													   MOTIONPLUS_CHANGED);
}

void cvinit(void)
{
	char      *filename = "haarcascade_frontalface_alt.xml";
	
	cascade = ( CvHaarClassifierCascade* )cvLoad( filename, 0, 0, 0 );
	storage = cvCreateMemStorage( 0 );
	
	videoCapture = cvCaptureFromCAM( -1 );
	//videoCapture = cvCreateCameraCapture(0);
	if(!videoCapture)
	{
		videoCapture = cvCaptureFromAVI( "./Postboxes.avi" );
	}
	
//	cvSetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_WIDTH, VIEWPORT_WIDTH);
//	cvSetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_HEIGHT, VIEWPORT_HEIGHT);
	
//	VIEWPORT_WIDTH  = cvGetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_WIDTH);
//	VIEWPORT_HEIGHT = cvGetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_HEIGHT);

	assert( cascade && storage && videoCapture );
	
	cvNamedWindow( "video", 1 );
	
	captureImg = cvQueryFrame( videoCapture );
	
	/*GF:  subsampled frame  for speed */
	float subsamplefactor = 2;
	subsampledCaptureImg = cvCreateImage( cvSize(captureImg->width/subsamplefactor, captureImg->height/subsamplefactor) , 8, 3 );

}

void getScreenShoot(IplImage *imgO) {
	
	int width = imgO->width;
	int height = imgO->height;
	int hIndex=0;
	int wIndex=0;
	int iout,jout;
	
	unsigned char* imageData = (unsigned char*)malloc(width*height*3);
	IplImage *img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	
	glReadPixels(0, 0, width-1, height-1, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	
	for(int i=0;i<width*height*3;i+=3,wIndex++)
	{
		if(wIndex >= width)
		{
			wIndex=0;
			hIndex++;
		}
		
		((uchar *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 0]=imageData[i+2]; // B
		((uchar *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 1]=imageData[i+1]; // G
		((uchar *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 2]=imageData[i]; // R
		
		// vertically flip the image
		iout = -hIndex+height-1;
		jout = wIndex;
		
		((uchar *)(imgO->imageData + iout*imgO->widthStep))[jout*imgO->nChannels + 0] =
		((uchar *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 0];// B
		((uchar *)(imgO->imageData + iout*imgO->widthStep))[jout*imgO->nChannels + 1] =
		((uchar *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 1];// G
		((uchar *)(imgO->imageData + iout*imgO->widthStep))[jout*imgO->nChannels + 2] =
		((uchar *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 2];// R
	}
	
	free(imageData);
	cvReleaseImage(&img);
	
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluOrtho2D(0, width, 0, height);
    //glMatrixMode(GL_MODELVIEW);
}

void startVideoWrite() {
	std::cout << "START VIDEO CAPTURE\r";
	writer = cvCreateVideoWriter("./out.avi",CV_FOURCC('X','V','I','D'), 30, cvSize(captureImg->width,captureImg->height), 1);
	screenshoot = cvCreateImage(cvSize(captureImg->width,captureImg->height), IPL_DEPTH_8U,3);
	videoIsWriting = true;
}

void stopVideoWrite() {
	std::cout << "STOP VIDEO CAPTURE\r";
	cvReleaseVideoWriter(&writer);
	cvReleaseImage(&screenshoot);
	videoIsWriting = false;
}

void idle(void) {
	
	////////////////////////////////////////////////////
	// IMPORTANT: the wiimote state needs to be refreshed each pass
	remote.RefreshState();

	//hack... somehow opengl orientations are wrong..so we are using Y orientation value in place of X
	cannon_orientation[0] = remote.Acceleration.Orientation.Y;
	cannon_orientation[1] = remote.Acceleration.Orientation.X;
	cannon_orientation[2] = remote.Acceleration.Orientation.Z;

	//detect if B is pressed on wiimote
	if((remote.Button.Bits & (1 << 10)) != 0)
	{
		bullet_active = true;
		//bullet_position[0] = cannon_position[0];
		//bullet_position[1] = cannon_position[1];
		//bullet_position[2] = cannon_position[2]-.3;
		bullet_position[0] = 0;
		bullet_position[1] = 0;
		bullet_position[2] = 0;

		bullet_orientation[0] = remote.Acceleration.Orientation.X;
		bullet_orientation[1] = -remote.Acceleration.Orientation.Y;
		bullet_orientation[2] = remote.Acceleration.Orientation.Z;
	}
	/////////////////////////////////////////////////////////

	int grab = cvGrabFrame(videoCapture);
	captureImg = cvRetrieveFrame( videoCapture );

	//captureImg = cvQueryFrame( videoCapture );
	
	if( !captureImg ) 
	{ 
		fprintf( stderr, "Cannot query frame!\n" ); exit(1); 
	}
	
	//	cvFlip( frame, frame, -1 );
	captureImg->origin = 0;
	
	cvResize(captureImg, subsampledCaptureImg, CV_INTER_NN);
	
	CvPoint3D32f p = detectFaces( subsampledCaptureImg, prev, &face_rect);
	
	
	cvShowImage( "video", subsampledCaptureImg );
	
	
	CvPoint3D32f q = filter(p, prev);
	prev=q;
	//q.z = 100; % freezes distance to the screen
	
	// GF: virtual
	//if(SHOW_WIN_OPENCV) {
	//	cvgenerateview(q,canvas,"win");
	//}
	
	// GF: reduce latency
		int key = cvWaitKey( 1 );
	
	// GF: update OPENGL window with global Gq
	GLOBAL_VIEWER_POSITION[0] = q.x; GLOBAL_VIEWER_POSITION[1] = q.y; GLOBAL_VIEWER_POSITION[2] = q.z;
	//printf("%g %g %g\n", q.x, q.y, q.z);
	glutPostRedisplay();

}

void keyboard (unsigned char key, int x, int y) {

	if(remote.Button.Home())
	{
		if (videoIsWriting) stopVideoWrite() ;
		cvReleaseImage(&processedVideo);
		cvReleaseCapture(&videoCapture);
		//cvReleaseImage(&captureImg);
		glutDestroyWindow(g_hWindow);
		exit(0);
		return;
	}
	IplImage *imgO = cvCreateImage(cvSize(GL_WIN_INITIAL_WIDTH,GL_WIN_INITIAL_HEIGHT), IPL_DEPTH_8U, 3);
	switch (key) {
		case '1':
			SHOW_1=((SHOW_1+1) % 2);
			break;
		case '3':
			SHOW_3=((SHOW_3+1) % 2);
			break;
		case '4':
			SHOW_4=((SHOW_4+1) % 2);
			break;
		case '5':
			SHOW_5=((SHOW_5+1) % 2);
			break;
		case '6':
			SHOW_6=((SHOW_6+1) % 2);
			break;
		case GLUT_KEY_UP:
			fc=fc+1;
			break;
		case GLUT_KEY_DOWN:
			fc=fc-1;
			break;
		case GLUT_KEY_LEFT:
			fs=fs+1;
			break;
		case GLUT_KEY_RIGHT:
			fs=fs-1;
			break;
		case 'p':
			GLfloat projectionMatrix[16];
		   glGetFloatv(GL_PROJECTION_MATRIX,projectionMatrix);
			showMatrix("proj",projectionMatrix);
				  break;
		case 's':
			
			getScreenShoot(imgO);
			cvNamedWindow("Screenshoot",1);
			cvShowImage("Screenshoot", imgO);
			cvWaitKey(0);
			cvReleaseImage(&imgO);
			cvDestroyWindow("Screenshoot");
			break;
			
		case 'w':
			if (!videoIsWriting) {
				startVideoWrite();
			} else {
				stopVideoWrite();
			}
			break;

		case 'c':
			break;
		case 't':

			break;
		case 27:
			if (videoIsWriting) stopVideoWrite() ;
			cvReleaseImage(&processedVideo);
			cvReleaseCapture(&videoCapture);
			//cvReleaseImage(&captureImg);
			glutDestroyWindow(g_hWindow);
			exit(0);
			break;
		default:
			break;
	}
}

int main(int argc, char** argv) 
{
	
	/* 
	 initialize openCV's variables, devices, and debug windows
	 */
	cvinit();

	wii_init();
	
	////////////////////////
	//SetConsoleTitle(_T("- WiiYourself! - Demo: "));
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD pos = { 0, 3 };
	SetConsoleCursorPosition(console, pos);

	// try to connect the first available wiimote in the system
	//  (available means 'installed, and currently Bluetooth-connected'):
	WHITE; _tprintf(_T("  Looking for a Wiimote     "));
	   
	static const TCHAR* wait_str[] = { _T(".  "), _T(".. "), _T("...") };
	unsigned count = 0;
	//while(!remote.Connect(wiimote::FIRST_AVAILABLE)) {
	if(remote.Connect(wiimote::FIRST_AVAILABLE))
	{
		_tprintf(_T("\b\b\b\b%s "), wait_str[count%3]);
		count++;
#ifdef USE_BEEPS_AND_DELAYS
		Beep(500, 30); Sleep(1000);
#endif
		//}

		// connected - light all LEDs
		remote.SetLEDs(0x0f);
		BRIGHT_CYAN; _tprintf(_T("\b\b\b\b... connected!"));
		if(remote.IsBalanceBoard()) 
		{
			BRIGHT_WHITE;_tprintf(_T("  (Balance Board)"));
		}

#ifdef USE_BEEPS_AND_DELAYS
		Beep(1000, 300); Sleep(1000);
#endif
	}
	///////////////////////////
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowPosition( GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y );
	glutInitWindowSize( GL_WIN_INITIAL_WIDTH, GL_WIN_INITIAL_HEIGHT );
    
    g_hWindow = glutCreateWindow("AR-Assignment1");
    glutFullScreen();

    glutDisplayFunc(display);
    //glutReshapeFunc(reshape);
    glutIdleFunc(idle);
	glutKeyboardFunc (keyboard);
	glsceneinit();
	
    glutMainLoop();
	glDeleteTextures( 1, &texture );

	////////////////////////////////////////////
	remote.Disconnect();
	Beep(1000, 200);
	////////////////////////////////////////////
    return EXIT_SUCCESS;
}