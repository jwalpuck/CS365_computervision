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
  char orientedBoundingBoxWindowName[255] = "Oriented Bounding Box Window";
  
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
  cv::namedWindow( orientedBoundingBoxWindowName, 1);

  int state = 2;
  // MAIN LOOP
  for(;;) {
    

    //DEBUG
    int size = 0;
    
    cv::Mat frame, thresh, regMapDisplay, regionMap, centroid, boundingBox, orientedBoundingBox;

    // DISPLAY ORIGINAL IMAGE: 
    *capdev >> frame;
    cv::imshow( sourceWindowName, frame );
    
    // threshold grow shrink connected components
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);
    cv::imshow(thresholdWindowName, thresh);


    // Display the unoriented bounding boxes and the centroids
    // Bounding box stored: pt1( x = 1, y = 0 ), pt2( x = 3, y = 2 )
    // Centorid stored: pt( x = 1, y = 0 )
    // pick the one closest to the center
    int centerObj = 0;
    int minDistToCenter = INT_MAX;
    int row = (int)frame.size().height / 2;
    int col = (int)frame.size().width / 2;
    float distance = 0;
    
    // Find the object closest to the center of the image
    for( int i = 0; i < (int)boundingBox.size().height; i++){
      distance = (centroid.at<int>(i,1) - row) * (centroid.at<int>(i,1) - row) + (centroid.at<int>(i,0) - col) *(centroid.at<int>(i,0) - col) ;
      if( distance < minDistToCenter ){
	minDistToCenter = distance;
	centerObj = i;
      }
    }

    
    
    // GET Size of regions 
    regMapDisplay.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    for( int i = 0; i < (int)regionMap.size().height; i++){
      for( int j = 0; j < (int)regionMap.size().width; j++){
    	if( regionMap.at<int>(i, j ) == centerObj ){
	  size += 1;
	}
     	regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) == 2 ? 255 : 0;
     	regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) == 1 ? 255 : 0;
	regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[2] = (regionMap.at<int>(i, j)) == 0 ? 255 : 0; 
       }
    }



    cv::rectangle( regMapDisplay, cv::Point(  boundingBox.at<int>(centerObj,1), boundingBox.at<int>(centerObj,0)), cv::Point( boundingBox.at<int>(centerObj,3), boundingBox.at<int>(centerObj,2)), cv::Scalar(0, 255, 0));
    cv::circle( regMapDisplay, cv::Point(centroid.at<int>(centerObj,1), centroid.at<int>(centerObj,0)), 2, cv::Scalar( 255, 0, 0), 3);
    cv::imshow(boundingBoxWindowName, regMapDisplay);

    // Testing different parts of the pipeline in a mock statemachine
    // State 0: creates a feature file and writes to it and then reads from it
    // State 1: writes to an already existing file at the end
    // State 2: gets the region of pixels defined by the region map, trying to compute oriented bounding box
    // State 3: None
    ObjectFeature *features;
    ObjectFeature *testFeature;
    cv::Mat regionIdxs, translatedPts;

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
    if( state == 2 ){
      // get features
      features = getFeatures( boundingBox, regionMap, centerObj );

      float *test2 = getCentralAxisAngle(regionMap, centroid, centerObj, size); 
      state = 2;

      //printf("Central axis angle %f \n", test2 );

      orientedBoundingBox.create((int)frame.size().height, (int)frame.size().width, frame.type());
      for( int i = 0; i < (int)orientedBoundingBox.size().height; i++){
	for( int j = 0; j < (int)orientedBoundingBox.size().width; j++){
	  orientedBoundingBox.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) == 1 ? 255 : 0;
	  orientedBoundingBox.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) == 0 ? 255 : 0;
	  orientedBoundingBox.at<cv::Vec3b>(cv::Point(j,i))[2] = regionMap.at<int>(i, j) == 2 ? 255 : 0; 
       }
       }

      // Draw
      cv::Point center, distOriented;
      center.x = centroid.at<int>( centerObj, 1);
      center.y = centroid.at<int>( centerObj, 0);
      
      distOriented.x = center.x + 200 * cos( test2[0] );
      distOriented.y = center.y - 200 * sin( test2[0] );

      // central axis
      cv::line( orientedBoundingBox, center, distOriented, cv::Scalar( 255, 255, 255), 2);
      
      cv::Point p1, p2, p3, p4;
      p1.x = test2[1]; // p1 is the minx position
      p1.y = test2[2];

      p2.x = test2[3]; // p2 is the miny position
      p2.y = test2[4];

      p3.x = test2[5]; // p3 is the maxx position
      p3.y = test2[6];

      p4.x = test2[7]; // p4 is the maxy position
      p4.y = test2[8];
      
      
      // bounding box
      //cv::line( orientedBoundingBox, minMin, maxMax, cv::Scalar( 0, 255, 255), 3);
      cv::line( orientedBoundingBox, p1, p2, cv::Scalar(0, 255, 255), 3 );
      cv::line( orientedBoundingBox, p2, p3, cv::Scalar(0, 255, 255), 3);
      cv::line( orientedBoundingBox, p3, p4, cv::Scalar(0, 255, 255), 3);
      cv::line( orientedBoundingBox, p4, p1, cv::Scalar(0, 255, 255), 3);
      imshow( orientedBoundingBoxWindowName, orientedBoundingBox);
      // cleanup the things created. 
      free(features);
      free( test2 );
    }
    
    if(cv::waitKey(10) >= 0)
      break;

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
  cv::destroyWindow( boundingBoxWindowName ); 
  
  // terminate the video capture
  printf("Terminating\n");
  delete capdev;
  
  return(0);
}
