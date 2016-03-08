/*
 * Main loop for 2D object recognition
 * March 8, 2016
 * Project 3
 * Torrie Edwards, Jack Walpuck
*/

#include <cstdio>
#include "opencv2/opencv.hpp"
#include "image_control.h"
#include "feature_code.h"
#include "featureDB_ops.h"


int main(int argc, char *argv[]) {
  // OPEN VIDEO STUFF
  cv::VideoCapture *capdev;
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window"; 

  // open the video device
  printf(" Opening video capture\n");
  capdev = new cv::VideoCapture(0);
  if( !capdev->isOpened() ) {
    printf("Unable to open video device\n");
    return(-1);
  }

  // SET UP VIEWING WINDOWS
  cv::namedWindow(sourceWindowName, 1); // identifies a window?
  cv::namedWindow( thresholdWindowName, 1 ); 

  // MAIN LOOP
  for(;;) {
    cv::Mat frame, thresh;
    
    *capdev >> frame; // get a new frame from the camera, treat as a stream
    cv::imshow( sourceWindowName, frame );
    
    // threshold grow shrink connected components
    cv::imshow(thresholdWindowName, thresh ); 


    // get features

    // 
    
    if(cv::waitKey(10) >= 0)
      break;
    
  }

  // CLEANUP 
  cv::destroyWindow( sourceWindowName );
  cv::destroyWindow( thresholdWindowName );
  
  // terminate the video capture
  printf("Terminating\n");
  delete capdev;
  
  return(0);
}
