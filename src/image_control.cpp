/**
 * Does manipulation on video feed images
 * Project 3
 * March 8, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include "opencv2/opencv.hpp"
#include "segment.h"
#include "image_control.h"


// Threshold the supplied source image
// This will include:
//   Binary Thresholding, iteratively shrink and grow, connected components analysis
cv::Mat prepImage( cv::Mat &src, cv::Mat &regionMap, cv::Mat &centroid, cv::Mat &boundingBox, int threshValue ){
  cv::Mat binaryThresh, copy; 
  int i, j;
  float avg = 0;

  // CREATE BINARY IMAGE OF THRESHOLDED REGIONS
  //Grayscale matrix
  cv::Mat grayscaleMat (src.size(), CV_8U);

  //Convert BGR to Gray
  cv::cvtColor( src, grayscaleMat, cv::COLOR_BGR2GRAY );

  //Binary image
  cv::Mat binaryMat(grayscaleMat.size(), grayscaleMat.type());

  //Apply thresholding
  cv::threshold(grayscaleMat, binaryThresh, 100, 255, 1);

  // SHRINK
  cv::Mat shrink, elementS;
  // 3 errosion types: rectangle(1), cross(2), ellipse(3)
  int erosionType = 1;
  int erosionSize = 2;
  int n_s = 2; 
  elementS = cv::getStructuringElement(erosionType, cv::Size( 2 * erosionSize + 1, 2 * erosionSize + 1),
				       cv::Point(erosionSize, erosionSize ) ); 

  // Shrink several times
  for( int i = 0; i < n_s; i++){
    cv::erode( binaryThresh, shrink, elementS );
    shrink.copyTo(binaryThresh);
  }

  // GROW
  cv::Mat grow, elementG;
  // 3 dilation types: rectangle(1), cross(2), ellipse(3)
  int dilaType = 1;
  int dilaSize = 1;
  int n_g = 3;  
  elementG = cv::getStructuringElement(dilaType, cv::Size( 2 * dilaSize + 1, 2 * dilaSize + 1),
				       cv::Point(dilaSize, dilaSize ) );
  // Grow several times. 
  for( int i = 0; i < n_g; i++){
    cv::dilate( shrink, grow, elementG ); 
    grow.copyTo(shrink);
  }

  // CONNECTED COMPONENTS
  cv::Mat size;
  long sizeThresh = 2000;
  int maxLocations = 2;
  
  locateRegions(grow, regionMap, sizeThresh, centroid, boundingBox, size, maxLocations);

  shrink.release();
  elementS.release();
  elementG.release(); 
  size.release();

  return(grow);
}
