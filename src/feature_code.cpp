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
int bbArea( cv::Mat &boundingBox ){
  int deltaX = 0;
  int deltaY = 0;
  deltaX = boundingBox.at<int>(0, 3) - boundingBox.at<int>(0, 1);
  deltaY = boundingBox.at<int>(0, 2) - boundingBox.at<int>(0, 0);
  return( deltaX * deltaY ); 
}

// Get the width to the height ratio of the bounding box
float width2Height( cv::Mat &boundingBox){
  int deltaX = 0;
  int deltaY = 0;
  deltaX = boundingBox.at<int>(0, 3) - boundingBox.at<int>(0, 1);
  deltaY = boundingBox.at<int>(0, 2) - boundingBox.at<int>(0, 0);  
  return((float)deltaX / deltaY);
}

// Get the fill ratio of the object in its bounding box
float fillRatio( cv::Mat &boundingBox, cv::Mat &regMap){
  int count = 0;
  float area = 0; 
  for( int i = boundingBox.at<int>(0,1); i < boundingBox.at<int>(0,3); i++){
    for( int j = boundingBox.at<int>(0,0 ); j < boundingBox.at<int>(0,2 ); j++){
      if (regMap.at<int>(j, i) >= 0 ){
	count ++; 
      }
    }
  }
  area = (float)bbArea( boundingBox ); 
  return( count / area );
}

//cv::Mat primaryAxis(cv::Mat &boundingBox, cv::Mat &centroid, cv::Mat &regMap){
//  cv::calcCovarMatrix( ); 

//  return( ); 
//}


float *getFeatures( cv::Mat boundingBox, cv::Mat regionMap){
  float *results = (float *)malloc(sizeof(float) * 3);;
  results[0] = bbArea( boundingBox );
  results[1] = width2Height( boundingBox );
  results[2] = fillRatio( boundingBox, regionMap );
  
  return(results);
}
