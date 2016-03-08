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

  int threshValue = 150;
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window"; 
  char boundingBoxWindowName[255] = "Bounding Box Window";
  
  // open the video device
  printf(" Opening video capture\n");
  // JACK'S COMPUTER!! 
  // capdev = new cv::VideoCapture(1);
  capdev = new cv::VideoCapture(0);
  if( !capdev->isOpened() ) {
    printf("Unable to open video device\n");
    return(-1);
  }

  // SET UP VIEWING WINDOWS
  cv::namedWindow( sourceWindowName, 1); // identifies a window?
  cv::namedWindow( thresholdWindowName, 1 ); 
  cv::namedWindow( boundingBoxWindowName, 1);

  // MAIN LOOP
  for(;;) {
    

    //DEBUG
    int size = 0;

    cv::Mat frame, thresh, regMapDisplay, regionMap, centroid, boundingBox;

    // DISPLAY ORIGINAL IMAGE: 
    *capdev >> frame;
    cv::imshow( sourceWindowName, frame );
    
    // threshold grow shrink connected components
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);
    cv::imshow(thresholdWindowName, thresh);
    
    // GET RESULTS OF THRESHOLDING & CCP ANALYSIS
    regMapDisplay.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    for( int i = 0; i < (int)regionMap.size().height; i++){
      for( int j = 0; j < (int)regionMap.size().width; j++){
    	size += regionMap.at<int>(i, j) > 0;
     	regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) * 0;
     	regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) * 0;
     	regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[2] = (regionMap.at<int>(i, j) + 1) > 0 ? 255 : 0; 
       }
    }

    cv::rectangle( regMapDisplay, cv::Point( boundingBox.at<int>(0,0), boundingBox.at<int>(0,1)), cv::Point( boundingBox.at<int>(1,0), boundingBox.at<int>(1,1)), cv::Scalar(0, 255, 0));
    
    cv::circle( regMapDisplay, cv::Point(centroid.at<int>(0,0), centroid.at<int>(0,1)), 2, cv::Scalar( 255, 0, 0));

    cv::imshow(boundingBoxWindowName, regMapDisplay); 

    printf("BHeight: %d \n", (int)boundingBox.size().height);
    printf("BWidth: %d \n", (int)boundingBox.size().width);
    printf("Centroid: %d %d \n", (int)centroid.size().height, (int)centroid.size().width);
    
    std::cout << boundingBox << "\n";
    // get features

    // 

    if(cv::waitKey(10) >= 0)
      break;

    //return 0;
    frame.release();
    thresh.release();
    regMapDisplay.release();
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
