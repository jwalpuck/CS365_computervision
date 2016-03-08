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
  //cv::Mat regionMap, centroid, boundingBox;
  int threshValue = 150;
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window"; 

  // open the video device
  printf(" Opening video capture\n");
  capdev = new cv::VideoCapture(1);
  if( !capdev->isOpened() ) {
    printf("Unable to open video device\n");
    return(-1);
  }

  // SET UP VIEWING WINDOWS
  cv::namedWindow(sourceWindowName, 1); // identifies a window?
  cv::namedWindow( thresholdWindowName, 1 ); 
  cv::namedWindow("Bounding Box", 1);

  // MAIN LOOP
  for(;;) {
    printf("Top of loop\n");
    

    //DEBUG
    int size = 0;

    cv::Mat frame, thresh, rTest, regionMap, centroid, boundingBox;
    
    *capdev >> frame; // get a new frame from the camera, treat as a stream
    cv::imshow( sourceWindowName, frame );
    
    // threshold grow shrink connected components
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);

    //DEBUG
    printf("Creating test matrix\n");
    rTest.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    printf("Entering loop\n");
    for( int i = 0; i < (int)regionMap.size().height; i++){
      for( int j = 0; j < (int)regionMap.size().width; j++){
    	size += regionMap.at<int>(i, j) > 0;

     	rTest.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) * 0;
     	rTest.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) * 0;
     	rTest.at<cv::Vec3b>(cv::Point(j,i))[2] = (regionMap.at<int>(i, j) + 1) > 0 ? 255 : 0; 

       }
    }
    printf("Exited loop: region with size %d\n", size);
    cv::imshow("Bounding Box", rTest); 

    //END DEBUG
    cv::imshow(thresholdWindowName, thresh);

    // get features

    // 
    
    if(cv::waitKey(10) >= 0)
      break;

    //return 0;
    frame.release();
    thresh.release();
    rTest.release();
    regionMap.release();
    centroid.release();
    boundingBox.release();
     
    
  }

  // CLEANUP 
  cv::destroyWindow( sourceWindowName );
  cv::destroyWindow( thresholdWindowName );
  
  // terminate the video capture
  printf("Terminating\n");
  delete capdev;
  
  return(0);
}
