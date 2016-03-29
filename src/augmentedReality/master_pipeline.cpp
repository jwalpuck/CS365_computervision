/**
 * master_pipeline.cpp
 * 3/29/15
 * Torrie Edwards, Jack Walpuck
 *
 * Contains code to run the main mothafucka
 */

#include <cstdio>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "master_pipeline.h"

using namespace std;

int main(int argc, char *argv[]) {

  //Put code temporarily here to get intial calibration done
  //Jack will go back later and put it all in a video stream loop

  cv::VideoCapture *capdev;
  bool found, calibrated;
  const char static_img[255] = "../../images/checkerboard.png";
  const char stream_label[255] = "Augmented Reality";
  cv::Mat view = cv::imread(static_img);
  cv::Mat gray;
  cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);
  cv::Size boardSize(8, 6);
  vector<cv::Point2f> corners;
  int keyPress, numFramesCaptured;
  State state = idle;
  vector<cv::Point2f> corner_set;
  vector<cv::Vec3f> point_list; 
  vector<cv::Vec2f> corner_list;
    
  //Open image stream
  //capdev = new cv::VideoCapture(0); //Torrie
  capdev = new cv::VideoCapture(1); //Jack
  if(!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return -1;
  }

  //Open the viewing window
  cv::namedWindow(stream_label, 1);

  //Main loop
  numFramesCaptured = 0, calibrated = 0;
  do {
    keyPress = cv::waitKey(10);
    if(keyPress > 0) {
      printf("Key ID = %d\n", keyPress);
    }

    //Set state based on keypress
    //d = 100
    switch(keyPress) {
    case 115: { //User presses s: capture
      state = capture;
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
	state = draw;
      break;
    }
    default: {
      state = idle;
      break;
    }
    }

    //Get image
    cv::Mat frame;
    *capdev >> frame;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    //CALIB_CB_FAST_CHECK quickly sees if there is a checkerboard in the image
    found = cv::findChessboardCorners( frame, boardSize, corners, cv::CALIB_CB_FAST_CHECK);
    if(found) {
      cv::cornerSubPix(gray, corners, cv::Size(17, 13), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 30, 0.1));
  }
    
    //Draw the corners
    drawChessboardCorners(frame, boardSize, cv::Mat(corners), found);
    
    if(view.data == NULL) {
      printf("NOOOOOOOOOOOOO\n");
      return -1;
    }
    
    //Display with pretty lines
    cv::imshow(stream_label, frame);
    
    //Smile for the writeup!
    //cout << "Number of corners: " << corners.size() << endl;
    if(found) {
      //cout << "Coords of first corner: (" << corners[0].x << ", " << corners[0].y << ")" << endl;
    }

    //Save corner locations and corresponding 3d world points
    corner_list.add(corner_set);
    
    //Convert corners to world coordinates these are the same every time
    for(int i = 0; i < boardSize.y; i++) {
      for(int j = 0; j < boardSize.x; j++) {
	corner_list.add(
      }
    }
    

  }while(keyPress != 27);
  return 0;
}
