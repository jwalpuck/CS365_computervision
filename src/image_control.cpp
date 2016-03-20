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

/* Return the index of the object most centered in the frame */
int getCenteredObject(cv::Mat &frame, cv::Mat &boundingBox, cv::Mat &centroid) {
  int centerObj = 0;
  int minDistToCenter = INT_MAX;
  int row = (int)frame.size().height / 2;
  int col = (int)frame.size().width / 2;
  float distance = 0;
 
  for( int i = 0; i < (int)boundingBox.size().height; i++){
    distance = (centroid.at<int>(i,1) - row) * (centroid.at<int>(i,1) - row) + (centroid.at<int>(i,0) - col) *(centroid.at<int>(i,0) - col) ;
    if( distance < minDistToCenter ){
      minDistToCenter = distance;
      centerObj = i;
    }
  }
  return centerObj;
}

void makeRegMapDisplay(cv::Mat &regMapDisplay, cv::Mat &regionMap, cv::Mat &centroid, cv::Mat &boundingBox, int centerObj) {
  int size = 0;
  for( int i = 0; i < (int)regionMap.size().height; i++){
    for( int j = 0; j < (int)regionMap.size().width; j++){
      size += regionMap.at<int>(i, j) > 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) == 2 ? 255 : 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) == 1 ? 255 : 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[2] = (regionMap.at<int>(i, j)) == 0 ? 255 : 0; 
    }
  }

  cv::rectangle( regMapDisplay, cv::Point(  boundingBox.at<int>(centerObj,1), boundingBox.at<int>(centerObj,0)), cv::Point( boundingBox.at<int>(centerObj,3), boundingBox.at<int>(centerObj,2)), cv::Scalar(0, 255, 0));
  cv::circle( regMapDisplay, cv::Point(centroid.at<int>(centerObj,1), centroid.at<int>(centerObj,0)), 2, cv::Scalar( 255, 0, 0), 3);
}
