/**
 * master_pipeline.cpp
 * 3/29/15
 * Torrie Edwards, Jack Walpuck
 *
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "master_pipeline.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

#define debug 0
#define writeup 0

State state = idle;
bool calibrated = 0;
int  numFramesCaptured = 0;
cv::Mat frame,  camera_matrix, distortion;
cv::VideoCapture *capdev = NULL;
vector<vector<cv::Point2f > > corner_list;
vector<vector<cv::Point3f > > point_list;

void writeIntrinsicParams(char *filename, cv::Mat camera_matrix, cv::Mat distortion) {
     
  FILE *file = fopen(filename, "w+");

  for(int i = 0 ; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      fwrite(&camera_matrix.at<double>(i, j), sizeof(double), 1, file);
    }
  }

  for(int i = 0; i < 5; i++) {
    fwrite(&distortion.at<double>(i, 0), sizeof(double), 1, file );
  }
  
  fclose(file);
  printf("Parameters written to file %s\n", filename);
}

void readIntrinsicParameters(char *filename, cv::Mat &camera_matrix, cv::Mat &distortion ){
     
  FILE *file = fopen(filename, "r");

  for(int i = 0 ; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      fread(&camera_matrix.at<double>(i, j), sizeof(double), 1, file);
    }
  }

  for(int i = 0; i < 5; i++) {
    fread(&distortion.at<double>(i, 0), sizeof(double), 1, file );
  }
  
  fclose(file);
  printf("Parameters read to file %s\n", filename);
}
  

// Do this with opengl things...
void drawAxes( float length){
  glPushAttrib( GL_POLYGON_BIT | GL_ENABLE_BIT );//| GL_COLOR_BIT );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

  glBegin(GL_LINES );
  glColor3f( 1, 0, 0);
  // x axis
  glVertex3f( 0, 0, 0);
  glVertex3f( length, 0, 0);

  // y axis
  glColor3f( 0, 1, 0);
  glVertex3f( 0, 0, 0);
  glVertex3f( 0, length, 0);

  // z axis
  glColor3f( 0, 0, 1);
  glVertex3f( 0, 0, 0);
  glVertex3f( 0, 0, length);
  glEnd();

  glPopAttrib();
}  

void display( ){
   bool found;
  const char static_img[255] = "../../images/checkerboard.png";
  const char stream_label[255] = "Augmented Reality";
  // cv::Mat view = cv::imread(static_img);
  cv::Mat gray;
  vector<cv::Mat> rotations;
  vector<cv::Mat> translations;
  //cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);
  cv::Size boardSize(8, 6);
  vector<cv::Point2f> corners;
  int keyPress;
  vector<cv::Point2f> corner_set; //We were using Vec2f
  //Make a static set of points for the world coordinates of the checkerboard
  //We are using units of checkerboard spaces (assuming they are of uniform size)
  vector<cv::Point3f> board_worldCoords;
  cv::Mat m_rotations = cv::Mat::zeros(3, 1, CV_64FC1);
  cv::Mat m_translations = cv::Mat::zeros(3, 1, CV_64FC1);
  
  GLfloat blue[] = {0.0f, 0.0f, 1.0f};
  GLfloat position[] = {0.0, 0.0, 0.0, 1.0};
  
  for(int i = 0; i > -6; i--) {
    for(int j = 0; j < 8; j++) {
      board_worldCoords.push_back(cv::Point3f(j, i, 0));
    }
  }
    

  if(!frame.data) {
    printf("llll\n");
    exit(-1);
  }
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

  // clear the window
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glColor3f(1.0, 1.0, 1.0);
  
  cv::Mat flipped; 
  cv::flip(frame, flipped, 0 );
  glDrawPixels( flipped.size().width, flipped.size().height, GL_BGR, GL_UNSIGNED_BYTE, flipped.ptr() );
  
  switch(state) {
  case idle: {
    break;
  }

  case capture: {
    found = cv::findChessboardCorners( frame, boardSize, corner_set, cv::CALIB_CB_FAST_CHECK);
      if(found) {
	cv::cornerSubPix(gray, corner_set, cv::Size(17, 13), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 30, 0.1));
      }
      if(!found) {
	printf(":(\n");
      }
    
      //Draw the corners
      drawChessboardCorners(frame, boardSize, cv::Mat(corner_set), found);
    
      //Smile for the writeup!
      if( writeup && found ){
	cout << "Number of corners: " << corner_set.size() << endl;
	cout << "Coords of first corner: (" << corner_set[0].x << ", " << corner_set[0].y << ")" << endl;
      }
      if(found) {
	numFramesCaptured++;
	printf("Valid frame captured! You now have %d out of 5 required images\n", numFramesCaptured);

	//Save corner locations and corresponding 3d world points
	corner_list.push_back(corner_set);
	point_list.push_back(board_worldCoords);
	
      }

      state = idle;
      break;
    }

    case calibrate: {

      printf("Attempting to calibrate\n");
      

      //Build the camera matrix
      camera_matrix = cv::Mat::eye(3, 3, CV_64FC1);
      camera_matrix.at<double>(0, 2) = frame.cols/2;
      camera_matrix.at<double>(1, 2) = frame.rows/2;

      cout << "Original camera matrix:" << endl << camera_matrix << endl;
      
      //Make vectors to be filled by the calibrate camera function
      distortion = cv::Mat::zeros(5, 1, CV_64F);
      
      //NOTE: May need to use point_counts vector
      double err = cv::calibrateCamera(point_list, corner_list, frame.size(), camera_matrix, distortion, rotations, translations);

      cout << "New camera matrix: " << endl << camera_matrix << endl;
      printf("Error is: %f\n", err);
      cout << "Distortion " << endl << distortion << endl;

      calibrated = 1;
      state = idle;
      break;
    }

    case draw: {
      found = cv::findChessboardCorners( frame, boardSize, corner_set, cv::CALIB_CB_FAST_CHECK);
      if(found) {
	cv::cornerSubPix(gray, corner_set, cv::Size(17, 13), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 30, 0.1));
      }


      if(found) {
	if( debug ){
	  cout << "Camera Matrix:" << endl << camera_matrix << endl;
	  cout << "Point List" << endl << point_list[0] << endl;
	  cout << "Corner list" << endl << corner_list[0] << endl;
	  cout << "Distortion" << endl << distortion << endl;
	}
	bool success = cv::solvePnP(board_worldCoords, corner_set, camera_matrix, distortion, m_rotations, m_translations);
	if(success) {
	  if( debug ){
	    printf("We did it!\n");
	    printf("It was a success! %lu rotation matrices, %lu translation matrices\n", rotations.size(), translations.size());
	    // print the results
	    cout << "Rotation matrix:" << endl << m_rotations << endl;
	    cout << "Translation matrix:" << endl << m_translations << endl;
	  }
	  
	}
      }

      glViewport(0, 0, flipped.size().width, flipped.size().height );

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      // intrinsic camera parameters determine the parameters here:
      GLdouble fovy = 60;
      GLdouble aspect = camera_matrix.at<double>(0, 0) / camera_matrix.at<double>(1, 1);
  
      // GLdouble zNear,  GLdouble zFar: used arbitrary values because we
      //  do not have a clipping plane. 
      gluPerspective( fovy, aspect, 1, 50);

      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();
      glLightfv(GL_LIGHT0, GL_POSITION, position);
      gluLookAt( 1, 1, 10, 0, 0, 0, 0, 1, 0); 
      
      if( calibrated ){
	glPushMatrix();
	glMaterialfv( GL_FRONT, GL_DIFFUSE, blue); 
	cv::Mat rTheta= cv::Mat::zeros( 3, 3, CV_64FC1);
	cv::Rodrigues( m_rotations, rTheta );  
	GLdouble rt[16] = { rTheta.at<double>(0,0), -rTheta.at<double>(0,2), -rTheta.at<double>(0,1), 0,
			 rTheta.at<double>(1,0), -rTheta.at<double>(1,2), -rTheta.at<double>(1,1), 0,
			 rTheta.at<double>(2,0), -rTheta.at<double>(2,2), -rTheta.at<double>(2,1), 0,
			 m_translations.at<double>(0,0), -m_translations.at<double>(1,0), -m_translations.at<double>(2,0), 1 };
	if( debug ){
	  printf(" RT MATRIX\n");
	  for( int i = 0; i < 16; i+=4){
	    printf("%f %f %f %f \n", rt[0 + i], rt[1+i], rt[2+i], rt[3 + i]);
	  }
	}
	glLoadMatrixd( rt ); 

	//drawAxes( 3 );
	glutSolidTeapot( 2 );
    
	glPopMatrix();
      }
      break;
    }
    }
  glutSwapBuffers( );
  glutPostRedisplay(); 
}

// initialize the lighting and material color values
void initlights(void) {
  //GLfloat ambient[] = {0.1, 0.1, 0.1, 0.0};
  //GLfloat diffuse[] = {1, 1, 1, 1.0};
  //GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
  //GLfloat position[] = {0.0, 0.0, 0.0, 1.0 };
  //GLfloat mat_diffuse[] = {0.0, 0.4, 0.5, 1.0};
  // GLfloat mat_specular[] = {1, 1, 0, 0};
  //GLfloat mat_shininess[] = {50.0};

  // glShadeModel( GL_SMOOTH );
  //glEnable( GL_DEPTH_TEST );
  // glEnable(GL_LIGHTING);
  // glEnable(GL_LIGHT0);

  //material values
  //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

  // generic lighting values
  //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  
  // specific light source
  //glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  //glLightfv(GL_LIGHT0, GL_SPECULAR, specular );
  //glLightfv(GL_LIGHT0, GL_POSITION, position ); 
  // enable lighting, light0 and depth testing
  //glEnable(GL_DEPTH_TEST);

}



void reshape( int w, int h ){
  glViewport( 0, 0, w, h ); 
}

void mouse( int button, int state, int x, int y ){
  if( button == GLUT_LEFT_BUTTON && state == GLUT_UP ){

  }
}

void keyboard( unsigned char key, int x, int y ){
  //Get image

  switch( key ){
  case 27:{
    // quit
    exit( 0 );
    break;
  }
  case 119: { //User presses w: Write the intrinsic parameters (camera mat, dist) to a file
      if(!calibrated) {
	printf("You must calibrate the camera :)\n");
	break;
      }
      string filename;
      char c_filename[255];
      ostream *fout;

      printf("Please enter a filename to print the intrinsic parameters to: \n");
      getline(cin, filename);
      //Copy into a cstring
      size_t ddd = filename.copy(c_filename, filename.size(), 0);
      writeIntrinsicParams(c_filename, camera_matrix, distortion);
      break;
    }
    
  case 114:{ // read from a file
    camera_matrix = cv::Mat::zeros( 3, 3, CV_64FC1);
    distortion = cv::Mat::zeros(5, 1, CV_64F);
    readIntrinsicParameters( "calibration.txt", camera_matrix, distortion );
    cout<< "CALIBRATION: " << endl << camera_matrix << endl;
    cout<< "DISTORITOIN: " << endl << distortion << endl;
    calibrated = 1;
    break;
  }
  case 115: { //User presses s: capture
    state = capture;
    printf("Capturing\n");
    break;
  }
  case 99: { //User presses c: calibrate
    if(numFramesCaptured < 5) {
      printf("You need to capture at least 5 frames before calibrating\n");
      state = idle;
    }
    else {
      state = calibrate;
    }
    break;
  }
  case 100: { //User presses d: draw
    if(calibrated) //We cannot draw anything onto the image before it is calibrated
      if(state == draw)
	state = idle;
      else
	state = draw;
    else
      printf("Must calibrate before attempting to draw\n");
    break;
  }
  default:
    break;
  }
}

void m_idle( ){
 
  *capdev >> frame;
}

// init function
void init(void) {
	// background color
  glClearColor(1.0, 1.0, 1.0, 1.0);

	// whether shading is smooth or flat
  glShadeModel(GL_SMOOTH);
  initlights();

}


int main(int argc, char *argv[]) {

  //Put code temporarily here to get intial calibration done
  //Jack will go back later and put it all in a video stream loop


 
  //Open image stream
  capdev = new cv::VideoCapture(0); //Torrie
  //capdev = new cv::VideoCapture(1); //Jack
  if(!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return -1;
  }

  *capdev >> frame;
   
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
  glutInitWindowPosition( 20, 20 );
  glutInitWindowSize( frame.size().width, frame.size().height );
  glutCreateWindow( "OpenGL/ OpenCV Calibration");
  init();
  
  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutMouseFunc( mouse );
  glutKeyboardFunc( keyboard );
  glutIdleFunc( m_idle );
  
  glutMainLoop( ); 
  
  return( 0 );
}

