/**
 * Does manipulation on video feed images
 * Project 3
 * March 8, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include "opencv2/opencv.hpp"
#include "image_control.h"


// Threshold the supplied source image
// This will include:
//   Binary Thresholding, iteratively shrink and grow, connected components analysis
cv::Mat threshold( cv::Mat src, int threshValue ){
  cv::Mat binaryThresh; 
  int i, j;
  float avg = 0;

  // CREATE BINARY IMAGE OF THRESHOLDED REGIONS
  binaryThresh.create( (int)src.size().height, (int)src.size().width, src.type() ); 
  for( i = 0; i < (int)src.size().height; i++){
    for( j = 0; j < (int)src.size().width; j++){
      avg = (src.at<cv::Vec3b>(cv::Point(j, i))[0] +
	     src.at<cv::Vec3b>(cv::Point(j, i))[1] +
	     src.at<cv::Vec3b>(cv::Point(j, i))[2]) / 3;
      if( avg > threshValue ){
	binaryThresh.at<cv::Vec3b>(cv::Point(j, i))[0] = 255; 
	binaryThresh.at<cv::Vec3b>(cv::Point(j, i))[1] = 255; 
	binaryThresh.at<cv::Vec3b>(cv::Point(j, i))[2] = 255; 
      }else{
	binaryThresh.at<cv::Vec3b>(cv::Point(j, i))[0] = 0;
	binaryThresh.at<cv::Vec3b>(cv::Point(j, i))[1] = 0; 
	binaryThresh.at<cv::Vec3b>(cv::Point(j, i))[2] = 0;
      }
    }
  }

  // SHRINK
  cv::Mat shrink, elementS;
  // 3 errosion types: rectangle(1), cross(2), ellipse(3)
  int erosionType = 1;
  int erosionSize = 0;
  
  elementS = cv::getStructuringElement(erosionType, cv::Size( 2 * erosionSize + 1, 2 * erosionSize + 1),
				      cv::Point(erosionSize, erosionSize ) ); 

  cv::erode( binaryThresh, shrink, elementS );
  
  // GROW
  cv::Mat grow, elementG;
  // 3 dilation types: rectangle(1), cross(2), ellipse(3)
  int dilaType = 1;
  int dilaSize = 0;

  elementG = cv::getStructuringElement(dialType, cv::Size( 2 * dilaSize + 1, 2 * dialSize + 1),
				       cv::Point(dialSize, dialSize ) ); 
  
  // CONNECTED COMPONENTS 

  
  return( result );
}
