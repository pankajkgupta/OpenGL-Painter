
// TODO THIS IS RELATIVE TO THE SIZE OF THE WINDOW
//float fc=256;
float fc=100;
// TODO fix the Gq.z issues
//float fs=512;
float fs=1000;

float GLOBAL_VIEWER_POSITION[3];

GLUquadric* nQ;

/* GLOBAL VARIABLES */
CvCapture *videoCapture;
IplImage  *captureImg;
IplImage  *subsampledCaptureImg;
IplImage  *canvas;
CvRect face_rect;



const int ESC = 27;

/* windows size and position constants */
const int GL_WIN_INITIAL_WIDTH = 800;
const int GL_WIN_INITIAL_HEIGHT = 600;
const int GL_WIN_INITIAL_X = 0;
const int GL_WIN_INITIAL_Y = 0;
const int GL_NEAR = 1.0;
const int GL_FAR = 100.0;

int winWidth = GL_WIN_INITIAL_WIDTH;
int winHeight = GL_WIN_INITIAL_HEIGHT;
int SHOW_1=0,SHOW_2=0,SHOW_3=1,SHOW_4=1,SHOW_5=1, SHOW_6=0;

static float float_rand(void) { return rand() / (float) RAND_MAX; }
#define RANDOM_RANGE(lo, hi) ((lo) + (hi - lo) * float_rand())

#ifndef M_PI
#define M_PI 3.14159265
#endif
//bool ball_active = false;
//float ball_position[3] = {0};
//float ball_radius = 0.5;

bool bullet_active = false;
float bullet_position[3] = {0, 0, 0};
float bullet_orientation[3] = {0, 0, -1};
float bullet_radius = 0.4;
float bullet_velocity[3] = {0.05, 0.05, 10.0};
bool flash_red = false;

float cannon_orientation[3] = {0,90,0};
float cannon_position[3] = {0, -1, 0};

#include "texture.h"
GLuint texture;
GLuint textureTarget;
//void *font = GLUT_BITMAP_TIMES_ROMAN_24;
void *font = GLUT_BITMAP_9_BY_15;
//void *font = GLUT_BITMAP_8_BY_13;

void
outputText(float x, float y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}

// print "matrix[]" in standard format
void showMatrix(char * name, GLfloat matrix[]) {
   printf("Matrix : %s\n\n", name);
   for (int i = 0; i < 4; i++) {
      printf("%3.2f   %3.2f   %3.2f   %3.2f \n", matrix[i], matrix[i+4],
         matrix[i+8], matrix[i+12]);
      printf("\n");
      }
   }

void glsceneinit()
{
  /* Enable two OpenGL light. */
GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  /* white diffuse light. */

GLfloat light_position[] = {1.0, -10.0, -10.0, 0.0};  /* Infinite light location. */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);

GLfloat light_diffuse1[] = {1.0, 1.0, 0.8, 1.0}; /* White */ 
GLfloat light_position1[] = {10.0, +10.0, 20.0, 0.0};  /* Infinite light location. */
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glEnable(GL_LIGHT1);

  glEnable(GL_LIGHTING);


/* enable color material*/
glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
glEnable ( GL_COLOR_MATERIAL );


  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);

	//  gluPerspective( /* field of view in degree */ 40.0,
	//    /* aspect ratio */ 1.0,
	//    /* Z near */ 1.0, /* Z far */ 10.0);

  glFrustum( /* left, right */ -1.0,1.0,
    /* botom,top*/ -1.0,1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);



//texture = LoadTextureRAW( "background.bmp", 1725, 600, 1);
//texture = LoadTextureRAW( "toledo.bmp", 4215, 2002, 1);
texture = LoadTextureRAW(captureImg, 1);
//texture = LoadTextureRAW( "pichu.bmp", 1109, 400, 1);
//REMEMBER TO DELETE IT
//    glDeleteTextures( 1, &texture );

	
//textureTarget = LoadTextureRAW( "target.bmp", 180, 180, 1);
textureTarget = LoadTextureRAW_RGBA( "target3.bmp", 180, 180, 1);

}

void position_camera_frustum ( float *Gq )
{

	float ZFAR=150;
	float ZNEAR_base=1;
	float ZNEAR_clip=.5;
	float x=Gq[0];
	float y=Gq[1];
	float z=Gq[2]+100;

	// frustum projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum( /* left, right */ -1.6 +float(x)/fc,1.6 +float(x)/fc,
			/* botom,top*/ -1.0+float(y)/fc,1.0+float(y)/fc,
			/* Z near : this is the projection plane*/  (ZNEAR_base*fs/z), 
			/* Z far */ ZFAR);

	GLfloat projectionMatrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX,projectionMatrix);
	projectionMatrix[10]=(-ZFAR-ZNEAR_clip)/(ZFAR-ZNEAR_clip);
	projectionMatrix[14]=(-2*ZFAR*ZNEAR_clip)/(ZFAR-ZNEAR_clip);
	glLoadMatrixf(projectionMatrix);


	/* This is a hack to move the eye position*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(+float(x)/fc, +float(y)/fc, ZNEAR_base -  (ZNEAR_base*fs/z) );

}



void DrawTarget(float r) {

	glBegin(GL_LINE_STRIP); 
	glColor3f(1.,1.,1.);
	glVertex3f(0., 0, 0);
	glVertex3f(0., 0, -100);
	glEnd(); 

	glEnable( GL_TEXTURE_2D );
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture( GL_TEXTURE_2D, textureTarget );
	
	
	glBegin( GL_QUADS );
	glTexCoord2d(0.0,0.0); glVertex3d(-r,-r,0);
	glTexCoord2d(1.0,0.0); glVertex3d(r ,-r,0);
	glTexCoord2d(1.0,1.0); glVertex3d(r ,r ,0);
	glTexCoord2d(0.0,1.0); glVertex3d(-r,r ,0);
	glEnd();
	glDisable( GL_TEXTURE_2D );
	
	
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(flash_red)
	{
		GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		flash_red = false;
	}
	else
	{
		GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  /* Red diffuse light. */
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	}
	nQ=gluNewQuadric();

	// POSITIONS THE CAMERA FOR THE PERSPECTIVE EFFECT
	position_camera_frustum(GLOBAL_VIEWER_POSITION);

	texture = LoadTextureRAW(captureImg, 1);

	// write text
	if(SHOW_5){
		//  glClear(GL_COLOR_BUFFER_BIT);
		char teststr[1000];
		//glClearColor(0, 0, 0, 1);
		glColor3f(0.0, 1.0, 0.0);

		glPushMatrix();

		glTranslatef(-1.5, -1.5, -1);
		GLfloat matrix[16];
		glGetFloatv(GL_PROJECTION_MATRIX,matrix);
		for (int i = 0; i < 4; i++) {
			sprintf(teststr, "%3.2f  %3.2f  %3.2f  %3.2f", matrix[i], matrix[i+4],
					matrix[i+8], matrix[i+12]);
			outputText(0.,.8-i*.03,teststr);
		}

		sprintf(teststr, "pos: %3.2f  %3.2f  %3.2f ", GLOBAL_VIEWER_POSITION[0]/fc, GLOBAL_VIEWER_POSITION[1]/fc, fs/GLOBAL_VIEWER_POSITION[2]);
		outputText(0.,.8-5*.03,teststr);

		sprintf(teststr, "fc=%3.2f  fs=%3.2f  ", fc, fs);
		outputText(0.,.8-7*.03,teststr);

		
		sprintf(teststr, "Keys 1 to 6 display objects");
		outputText(0.,2.4,teststr);
		
		
		glPopMatrix();

	}

	// MAP A TEXTURE
	if(SHOW_3){
		glEnable( GL_TEXTURE_2D );
		glColor3f(1.0, 1.0, 1.0);
		glBindTexture( GL_TEXTURE_2D, texture );
		glBegin( GL_QUADS );
		glTexCoord2d(0.0,1.0); glVertex3d(-50.0,36.0,-120);
		glTexCoord2d(1.0,1.0); glVertex3d(50.0,36.0,-120);
		glTexCoord2d(1.0,0.0); glVertex3d(50.0,-36.0,-120);
		glTexCoord2d(0.0,0.0); glVertex3d(-50.0,-36.0,-120);
		
		glEnd();
		glDisable( GL_TEXTURE_2D );
	}

	if(SHOW_6){

		// allocate a texture name
		GLuint textureCam;
		glGenTextures( 1, &textureCam);
		// select modulate to mix texture with color for shading
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		// when texture area is small, bilinear filter the closest mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST );
		// when texture area is large, bilinear filter the first mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


		// select our current texture
		glBindTexture( GL_TEXTURE_2D, textureCam );

		char* data2 = subsampledCaptureImg->imageData;
		int width = subsampledCaptureImg->width;
		int height = subsampledCaptureImg->height;
		// build our texture mipmaps
		gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
				GL_BGR_EXT, GL_UNSIGNED_BYTE, data2 );

		glEnable( GL_TEXTURE_2D );
		glBegin( GL_QUADS );
		glColor3f(1.0, 1.0, 1.0);
		glTexCoord2d(0.0,0.0); glVertex3d(-1.5,.9,-1);
		glTexCoord2d(1.0,0.0); glVertex3d(-.9,.9,-1);
		glTexCoord2d(1.0,1.0); glVertex3d(-.9,.4,-1);
		glTexCoord2d(0.0,1.0); glVertex3d(-1.5,.4,-1);
		glEnd();
		glDisable( GL_TEXTURE_2D );
		glDeleteTextures( 1, &textureCam );
	}

	if(SHOW_4){

		// cannon
		glPushMatrix();
		glColor3f(0,0,1);
		glTranslatef(cannon_position[0], cannon_position[1], cannon_position[2]);
		glRotatef(90,cannon_orientation[0] ,cannon_orientation[1] ,cannon_orientation[2] );
		gluCylinder(nQ, .1, .15, .3, 20, 20);//(.1,.5,20,20);
		glPopMatrix();

		if(bullet_active)
		{
			//bullet
			glPushMatrix();
			glColor3f(1,1,0);
			glTranslatef(bullet_position[0], bullet_position[1], bullet_position[2]);
			//glTranslatef(GLOBAL_VIEWER_POSITION[0]/fc, GLOBAL_VIEWER_POSITION[1]/fc, -120);

			bullet_position[0] += bullet_velocity[0] * bullet_orientation[0];
			bullet_position[1] += bullet_velocity[1] * bullet_orientation[1];
			bullet_position[2] -= bullet_velocity[2];
			glutSolidSphere(0.3, 20,20);
			glPopMatrix();

			if(bullet_position[2] < -40)
			{
				//if((face_rect.x-subsampledCaptureImg->width/2)/(fc*bullet_velocity[0]*40) < bullet_position[0] &&
				//	bullet_position[0] < ((face_rect.x+face_rect.width -subsampledCaptureImg->width/2)*bullet_velocity[0]*40)/fc &&
				//	(face_rect.y-subsampledCaptureImg->height/2)/(fc*bullet_velocity[1]*40) < bullet_position[1] &&
				//	bullet_position[1] < ((face_rect.y+face_rect.height-subsampledCaptureImg->height/2)*bullet_velocity[1]*40)/fc)
				if((face_rect.x-subsampledCaptureImg->width/2)/(fc) < bullet_position[0] &&
					bullet_position[0] < ((face_rect.x+face_rect.width -subsampledCaptureImg->width/2))/fc &&
					(face_rect.y-subsampledCaptureImg->height/2)/(fc) < bullet_position[1] &&
					bullet_position[1] < ((face_rect.y+face_rect.height-subsampledCaptureImg->height/2))/fc)
				{
					printf("hit");
					
					flash_red = true;
				}
				bullet_active = false;
			}
		}


		////ball
		//glPushMatrix();
		//glColor3f(.7,.7,.7);
		//glTranslatef(ball_position[0], ball_position[1], ball_position[2]);

		//glutSolidSphere(0.3, 20,20);
		//glPopMatrix();

	}

	if(SHOW_1) {
		float tw=.85;
		float th=.85;
		float front=-3;

		int i;
		int gridDEPTH=10;
		for(i=0;i<=20;i++){
			//tunnel
			glPushMatrix();
			glTranslatef(0, tw*i/10. -1*tw, 0);

			glBegin(GL_LINE_STRIP); 
			glColor3f(1.,1.,1.);
			glVertex3f( tw,0., front);
			glVertex3f( tw,0.,-1.*gridDEPTH); 
			glVertex3f(-tw,0.,-1.*gridDEPTH); 
			glVertex3f(-tw,0., front);
			glEnd(); 

			glPopMatrix();
		}

		for(i=0;i<=20;i++){
			//tunnel
			glPushMatrix();
			glTranslatef( i/10.*th -1*th,0, 0);

			glBegin(GL_LINE_STRIP); 
			glColor3f(1.,1.,1.);
			glVertex3f(0., th, front);
			glVertex3f(0., th,-1.*gridDEPTH); 
			glVertex3f(0.,-th,-1.*gridDEPTH); 
			glVertex3f(0.,-th, front);
			glEnd(); 

			glPopMatrix();
		}

		for(i=0;i<=gridDEPTH+front;i++){
			//tunnel
			glPushMatrix();
			glTranslatef(0, 0, front -1*i);

			glBegin(GL_LINE_LOOP); 
			glColor3f(1.,1.,1.);
			glVertex2f(-tw,-th); 
			glVertex2f(-tw, th); 
			glVertex2f( tw, th); 
			glVertex2f( tw,-th); 
			glEnd(); 

			glPopMatrix();
		}
	}


	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
		
	glPushMatrix();
	glTranslatef(3, 2, -12);
	DrawTarget(.3);		
	glPopMatrix();
		
	glPushMatrix();
	glTranslatef(-2, -1, -7);
	DrawTarget(.3);		
	glPopMatrix();
		
	glPushMatrix();
	glTranslatef(-1.5, .7, -3);
	DrawTarget(.3);		
	glPopMatrix();
		
	glPushMatrix();
	glTranslatef(.5, -.1, .6);
	DrawTarget(.3);		
	glPopMatrix();
	glDisable(GL_BLEND);
	
	
	
	
	

	glutSwapBuffers();
}


