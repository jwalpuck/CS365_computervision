/**
 * Does manipulation and calculation on features
 * Project 3
 * March 8, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include "opencv2/opencv.hpp"
#include "feature_code.h"

// Get the area of the bounding box!
// Remember to keep track of x and y carefully!! 
int bbArea( cv::Mat &boundingBox, int idx ){
  int deltaX = 0;
  int deltaY = 0;
  deltaX = boundingBox.at<int>(idx, 3) - boundingBox.at<int>(idx, 1);
  deltaY = boundingBox.at<int>(idx, 2) - boundingBox.at<int>(idx, 0);
  return( deltaX * deltaY ); 
}

// Get the width to the height ratio of the bounding box
float width2Height( cv::Mat &boundingBox, int idx){
  int deltaX = 0;
  int deltaY = 0;
  deltaX = boundingBox.at<int>(idx, 3) - boundingBox.at<int>(idx, 1);
  deltaY = boundingBox.at<int>(idx, 2) - boundingBox.at<int>(idx, 0);  
  return((float)deltaX / deltaY);
}

// Get the fill ratio of the object in its bounding box
float fillRatio( cv::Mat &boundingBox, cv::Mat &regMap, int idx){
  int count = 0;
  float area = 0; 
  for( int i = boundingBox.at<int>(idx,1); i < boundingBox.at<int>(idx,3); i++){
    for( int j = boundingBox.at<int>(idx,0 ); j < boundingBox.at<int>(idx,2 ); j++){
      if (regMap.at<int>(j, i) >= 0 ){
	count ++; 
      }
    }
  }
  area = (float)bbArea( boundingBox, idx ); 
  return( count / area );
}

//cv::Mat primaryAxis(cv::Mat &boundingBox, cv::Mat &centroid, cv::Mat &regMap){
//  cv::calcCovarMatrix( ); 

//  return( ); 
//}

// Parent function to call all the possible feature options on each object placed into the database 
ObjectFeature *getFeatures( cv::Mat boundingBox, cv::Mat regionMap, int closestToCenter){
  ObjectFeature *results = (ObjectFeature *)malloc(sizeof(ObjectFeature));
  results->id[0] = '\0';
  results->unOrientedBoundingBox = bbArea( boundingBox, closestToCenter );
  results->width2Height = width2Height( boundingBox, closestToCenter );
  results->fillRatio = fillRatio( boundingBox, regionMap, closestToCenter );
  
  return(results);
}

// debugging tool to print an ObjectFeature Object
void printFeatures( ObjectFeature *feature ){
  printf("HERE\n");
  printf("Feature Name: %s\n", feature->id );
  printf("Area: %d\n", feature->unOrientedBoundingBox ); 
  printf("Ratio: %f\n", feature->width2Height);
  printf("Fill Ratio: %f\n", feature->fillRatio);
}
