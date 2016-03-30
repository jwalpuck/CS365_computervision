/**
 * master_pipeline.cpp
 * 3/29/15
 * Torrie Edwards, Jack Walpuck
 *
 * Contains code to run the main mothafucka
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "master_pipeline.h"

using namespace std;

void writeIntrinsicParams(char *filename, cv::Mat camera_matrix, cv::Mat distortion) {
  
  FILE *file = fopen(filename, "w");
  char l1[255] = "CAMERA MATRIX\n";
  char l2[255] = "DISTORTION\n";
  char space = ' ';

  fwrite(l1, sizeof(char), 255,file); 
  for(int i = 0 ; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      fwrite(&camera_matrix.at<double>(i, j), sizeof(double), 1, file);
    }
    fwrite(&space, sizeof(char), 1, file);
  }

  fwrite(l2, sizeof(char), 255, file );
  for(int i = 0; i < 5; i++) {
    fwrite(&distortion.at<double>(i, 0), sizeof(double), 1, file );
    fwrite( &space, sizeof(char), 1, file);
  }
  
  fclose(file);

}

int main(int argc, char *argv[]) {

  //Put code temporarily here to get intial calibration done
  //Jack will go back later and put it all in a video stream loop

  cv::VideoCapture *capdev;
  bool found, calibrated;
  const char static_img[255] = "../../images/checkerboard.png";
  const char stream_label[255] = "Augmented Reality";
  cv::Mat view = cv::imread(static_img);
  cv::Mat gray, camera_matrix, distortion;
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

    //Set state based on keypress
    switch(keyPress) {
    case 115: { //User presses s: capture
      state = capture;
      break;
    }
    case 119: { //User presses w: Write the intrinsic parameters (camera mat, dist) to a file
      if(!calibrated) {
	printf("Go fuck yourself :)\n");
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
      //cout << "Number of corners: " << corners.size() << endl;
      if(found) {
	//cout << "Coords of first corner: (" << corners[0].x << ", " << corners[0].y << ")" << endl;
	numFramesCaptured++;
	printf("Valid frame captured! You now have %d out of 5 required images\n", numFramesCaptured);

	//Save corner locations and corresponding 3d world points
	corner_list.push_back(corner_set);
	point_list.push_back(board_worldCoords);
	
	cv::imshow(stream_label, frame);
	cv::waitKey(0);
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
      distortion = cv::Mat::zeros(5, 1, CV_64F);;
      vector<cv::Mat> rotations;
      vector<cv::Mat> translations;

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
      printf("Draw!!!!\n");
    }
    }

    //Display
    cv::imshow(stream_label, frame);
	       
  }while(keyPress != 27);
  return 0;
}
