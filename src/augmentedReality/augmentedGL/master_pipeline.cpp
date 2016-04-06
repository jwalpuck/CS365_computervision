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

State state;
bool calibrated = 0;
int  numFramesCaptured = 0;
cv::Mat frame;
cv::VideoCapture *capdev = NULL;

void writeIntrinsicParams(char *filename, cv::Mat camera_matrix, cv::Mat distortion) {
  
  FILE *file = fopen(filename, "w+");
  char l1[255] = "CAMERA MATRIX\n";
  char l2[255] = "DISTORTION\n";
  char space = ' ';

  fwrite(l1, sizeof(char), 255,file); 
  for(int i = 0 ; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      fwrite(&camera_matrix.at<double>(i, j), sizeof(double), 1, file);
    }
  }

  fwrite(l2, sizeof(char), 255, file );
  for(int i = 0; i < 5; i++) {
    fwrite(&distortion.at<double>(i, 0), sizeof(double), 1, file );
  }
  
  fclose(file);
  printf("Parameters written to file %s\n", filename);
}


// Do this with opengl things...
void drawAxes( float length){
  glPushAttrib( GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BIT );
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
  cv::Mat view = cv::imread(static_img);
  cv::Mat gray, camera_matrix, distortion;
  vector<cv::Mat> rotations;
  vector<cv::Mat> translations;
  cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);
  cv::Size boardSize(8, 6);
  vector<cv::Point2f> corners;
  int keyPress, numFramesCaptured;
  State state = idle;
  vector<cv::Point2f> corner_set; //We were using Vec2f
  vector<vector<cv::Point2f > > corner_list; //We were using Vec2f
  vector<vector<cv::Point3f > > point_list;
  //Make a static set of points for the world coordinates of the checkerboard
  //We are using units of checkerboard spaces (assuming they are of uniform size)
  vector<cv::Point3f> board_worldCoords;
  for(int i = 0; i > -6; i--) {
    for(int j = 0; j < 8; j++) {
      board_worldCoords.push_back(cv::Point3f(j, i, 0));
    }
  }
    

  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  
  switch(state) {
  case idle: {
    break;
  }

  case capture: {
    //CALIB_CB_FAST_CHECK quickly sees if there is a checkerboard in the image
    found = cv::findChessboardCorners( frame, boardSize, corner_set, cv::CALIB_CB_FAST_CHECK);
      if(found) {
	cv::cornerSubPix(gray, corner_set, cv::Size(17, 13), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 30, 0.1));
      }
    
      //Draw the corners
      drawChessboardCorners(frame, boardSize, cv::Mat(corner_set), found);
    
      if(view.data == NULL) {
	printf("NOOOOOOOOOOOOO\n");
	return -1;
      }
    
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
	
	cv::imshow(stream_label, frame);
	cv::waitKey(500);
      }

      state = idle;
      break;
    }

    case calibrate: {

      //Create the point counts vector
      vector<int> point_counts;
      for(int i = 0; i < numFramesCaptured; i++) {
    	point_counts.push_back(point_list[i].size());
      }

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
      cv::Mat m_rotations = cv::Mat::zeros(3, 1, CV_64FC1);
      cv::Mat m_translations = cv::Mat::zeros(3, 1, CV_64FC1);
      found = cv::findChessboardCorners( frame, boardSize, corner_set, cv::CALIB_CB_FAST_CHECK);
      if(found) {
	cv::cornerSubPix(gray, corner_set, cv::Size(17, 13), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 30, 0.1));
      }
    
      if(view.data == NULL) {
	printf("NOOOOOOOOOOOOO\n");
	return -1;
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
      break;
    }
    }
  // clear the window
  glClear( GL_COLOR_BUFFER_BIT );

  cv::Mat flipped; 
  cv::flip(frame, flipped, 0 );
  glDrawPixels( flipped.size().width, flipped.size().height, GL_BGR, GL_UNSIGNED_BYTE, flipped.ptr() ); 

  glViewport(0, 0, flipped.size().width, flipped.size().height );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // intrinsic camera parameters determine the parameters here:
  // GLdouble fovy,  GLdouble aspect
  GLdouble fovy = cameraMatrix.at<double>(1, 1); 
  GLdouble aspect = cameraMatrix.at<double>(0, 2) / cameraMatrix.at<double>(1, 2);
  
  // GLdouble zNear,  GLdouble zFar: used arbitrary values because we
  //  do not have a clipping plane. 
  gluPerspective( fovy, aspect, 1, 20);

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glLookAt( 0, 0, 5, 0, 0, 0, 0, 1, 0); 

  if( calibrated ){
    glPushMatrix();
    //glTranslatef(translation.at<double>(0,0), translation.at<double>(1, 0), translation.at<double>(2, 0));
    // from our rotation: sqrt( r1^2 + r2^2 + r3^2 );
    //float rTheta = sqrt( rotation.at<double>(0,0) * rotation.at<double>(0,0) + rotation.at<double>(1,0) * rotation.at<double>(1,0) + rotation.at<double>(2,0) *rotation.at<double>(2,0)); 
    //glRotate(rTheta, rotation.at<double>(0, 0), rotation.at<double>(1,0),rotation.at<double>(2,0));
    
    gluSolidTeapot( 0.5 );
    
    glPopMatrix();
  }
  
  glutSwapBuffers( );
  glutPostRedisplay(); 
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
  case 115: { //User presses s: capture
    state = capture;
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
  case 114: { //User presses r: reset -- clear all of the saved vectors
    point_list.clear();
    corner_list.clear();
    calibrated = 0;
    numFramesCaptured = 0;
    state = idle;
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

void idle( ){
 
  *capdev >> frame;
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
   
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA |GLUE_DOUBLE );
  glutInitWindowPosition( 20, 20 );
  glutInitWindowSize( frame.size().width, frame.size().height );
  glutCreateWindow( "OpenGL/ OpenCV Calibration");

  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutMouseFunc( mouse );
  glutKeyboardFunc( keyboard );
  glutIdleFunc( idle );
  
  glutMainLoop( ); 
  
  return( 0 );
}
