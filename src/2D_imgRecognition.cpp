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

  int state = 0;
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

    // Display the bounding boxes and the centroids
    // Bounding box stored: pt1( x = 1, y = 0 ), pt2( x = 3, y = 2 )
    // Centorid stored: pt( x = 1, y = 0 )
    // pick the one closest to the center
    int centerObj = 0;
    int minDistToCenter = INT_MAX;
    int row = (int)frame.size().height / 2;
    int col = (int)frame.size().width / 2;
    int distance; 
    // Find the object closest to the center of the image
    for( int i = 0; i < (int)boundingBox.size().height; i++){
      distance = (centroid.at<int>(i,1) - row) * (centroid.at<int>(i,1) - row) + (centroid.at<int>(i,0) - col) *(centroid.at<int>(i,0) - col) ;
      if( distance < minDistToCenter ){
	centerObj = i;
      }
    }

    cv::rectangle( regMapDisplay, cv::Point(  boundingBox.at<int>(centerObj,1), boundingBox.at<int>(centerObj,0)), cv::Point( boundingBox.at<int>(centerObj,3), boundingBox.at<int>(centerObj,2)), cv::Scalar(0, 255, 0));
    cv::circle( regMapDisplay, cv::Point(centroid.at<int>(centerObj,1), centroid.at<int>(centerObj,0)), 2, cv::Scalar( 255, 0, 0), 3);
    cv::imshow(boundingBoxWindowName, regMapDisplay);
    
    // get features
    ObjectFeature *features;
    features = getFeatures( boundingBox, regionMap, centerObj );
    if( state == 0 ){
        
      strncpy( features->id, "testFeature", 255); 
      printFeatures( features );
      
      // Write Features to a database.
      writeFeatureToFile( features, "test.txt");
      
      // Read Feature from database
      ObjectFeature *testFeature; 
      testFeature = findBestFeatureResult( testFeature, "test.txt");
      printFeatures( testFeature );
      free(testFeature);
      state = 1; 
    }
    if(state == 1 ){
      // Test that I can add to my file at will!
      writeFeatureToFile( features, "test.txt");
      state = 2;
    }
    free(features);

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
