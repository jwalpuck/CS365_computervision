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

  
  // open the video device
  printf(" Opening video capture\n");
  // JACK'S COMPUTER!! 
  // capdev = new cv::VideoCapture(1);
  capdev = new cv::VideoCapture(0);
  if( !capdev->isOpened() ) {
    printf("Unable to open video device\n");
    return(-1);
  }



  int state = 2;
  // MAIN LOOP
  for(;;) {
    // initalize future things you will need. 
    ObjectFeature *features;
    int threshValue = 150; 
    cv::Mat frame, thresh, regionMap, centroid, boundingBox;
    int size;

    // set up for finding the center of a region 
    int centerObj = 0;
    int minDistToCenter = INT_MAX;
    int row = (int)frame.size().height / 2;
    int col = (int)frame.size().width / 2;
    float distance = 0;

    // DISPLAY ORIGINAL IMAGE: 
    *capdev >> frame;
    
    // threshold grow shrink connected components
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);

    // This could go into a separtate function but it is not necessary
    // Find the object closest to the center of the image
    for( int i = 0; i < (int)boundingBox.size().height; i++){
      distance = (centroid.at<int>(i,1) - row) * (centroid.at<int>(i,1) - row) + (centroid.at<int>(i,0) - col) *(centroid.at<int>(i,0) - col) ;
      if( distance < minDistToCenter ){
	minDistToCenter = distance;
	centerObj = i;
      }
    }

    // Get the size of a region
    size = getRegionSize( regionMap, centerObj );

    features = getFeatures( boundingBox, regionMap, centroid, centerObj, size );
    printFeatures( features );

    displayProcess( features, boundingBox, regionMap, centroid, frame, thresh, centerObj );
    // Testing different parts of the pipeline in a mock statemachine
    // State 0: creates a feature file and writes to it and then reads from it
    // State 1: writes to an already existing file at the end
    // State 3: None

    ObjectFeature *testFeature;

    if( state == 0 ){
      // This should come from the command line.
      // if some key stroke prompt the user to label the object
      // write to a file and then go back to normal mode. 
      strncpy( features->id, "testFeature", 255); 
      printFeatures( features );
      
      // Write Features to a database.
      writeFeatureToFile( features, "test.txt");
      
      // Read Feature from database
  
      testFeature = findBestFeatureResult( testFeature, "test.txt");
      printFeatures( testFeature );
      free(testFeature);
      state = 1; 
    }
    if(state == 1 ){
      // Test that I can add to my file at will! --> YES
      writeFeatureToFile( features, "test.txt");
      state = 3;
    }
      
    free(features);
    if(cv::waitKey(10) >= 0)
      break;

    frame.release();
    thresh.release();
    regionMap.release();
    centroid.release();
    boundingBox.release();
  }

  // CLEANUP 
  destroyDisplay();
  
  // terminate the video capture
  printf("Terminating\n");
  delete capdev;
  
  return(0);
}
