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
  grayscaleMat.release();
  binaryThresh.release();
  copy.release();

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

// Build the unoriented bounding box into the display, this also shows the thresholded results
void makeRegMapDisplay(cv::Mat &regMapDisplay, cv::Mat &regionMap, cv::Mat &centroid, cv::Mat &boundingBox, int centerObj) {
  int size = 0;
  for( int i = 0; i < (int)regionMap.size().height; i++){
    for( int j = 0; j < (int)regionMap.size().width; j++){
      size += regionMap.at<int>(i, j) > 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) == 2 ? 255 : 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) == 1 ? 255 : 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[2] = regionMap.at<int>(i, j) == 0 ? 255 : 0; 
    }
  }

  cv::rectangle( regMapDisplay, cv::Point(  boundingBox.at<int>(centerObj,1), boundingBox.at<int>(centerObj,0)), cv::Point( boundingBox.at<int>(centerObj,3), boundingBox.at<int>(centerObj,2)), cv::Scalar(0, 255, 0));
  cv::circle( regMapDisplay, cv::Point(centroid.at<int>(centerObj,1), centroid.at<int>(centerObj,0)), 2, cv::Scalar( 255, 0, 0), 3);
}


// Build the oriented bounding box and central axis into an image
void makeOrientedBBDisplay( cv::Mat &orientedBB, cv::Mat &regionMap, ObjectFeature *feature, cv::Mat &centroid, int idx){
  //----------------------------------- oriented bounding box -------------------------
  for( int i = 0; i < (int)orientedBB.size().height; i++){
    for( int j = 0; j < (int)orientedBB.size().width; j++){
      orientedBB.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) == 2 ? 255 : 0;
      orientedBB.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) == 1 ? 255 : 0;
      orientedBB.at<cv::Vec3b>(cv::Point(j,i))[2] = regionMap.at<int>(i, j) == 0 ? 255 : 0; 
    }
  }

  // Draw
  cv::Point center, distOriented;
  center.x = centroid.at<int>( idx, 1);
  center.y = centroid.at<int>( idx, 0);
      
  distOriented.x = center.x + 200 * cos( feature->centralAxisAngle );
  distOriented.y = center.y - 200 * sin( feature->centralAxisAngle );

  // central axis
  cv::line( orientedBB, center, distOriented, cv::Scalar( 255, 255, 255), 2);
      
  cv::Point p1, p2, p3, p4;
  p1.x = feature->orientedBoundingBox[0]; // p1 is the minx position
  p1.y = feature->orientedBoundingBox[1];

  p2.x = feature->orientedBoundingBox[2]; // p2 is the miny position
  p2.y = feature->orientedBoundingBox[3];

  p3.x = feature->orientedBoundingBox[4]; // p3 is the maxx position
  p3.y = feature->orientedBoundingBox[5];

  p4.x = feature->orientedBoundingBox[6]; // p4 is the maxy position
  p4.y = feature->orientedBoundingBox[7];
      
      
  // bounding box 
  //cv::line( orientedBoundingBox, minMin, maxMax, cv::Scalar( 0, 255, 255), 3);
  cv::line( orientedBB, p1, p2, cv::Scalar(0, 255, 255), 3 );
  cv::line( orientedBB, p2, p3, cv::Scalar(0, 255, 255), 3);
  cv::line( orientedBB, p3, p4, cv::Scalar(0, 255, 255), 3);
  cv::line( orientedBB, p4, p1, cv::Scalar(0, 255, 255), 3);
}

// Display our process in one opencv image
void makeDisplayProcess( cv::Mat &dst, cv::Mat &frame, cv::Mat &regMapDisplay, cv::Mat &orientedBB, cv::Mat &result){
	cv::Mat temp;
	temp.create((int)frame.size().height * 2, (int)frame.size().width * 2, frame.type());
	
	frame.copyTo( temp.rowRange( 0, frame.rows).colRange(0, frame.cols));
	regMapDisplay.copyTo( temp.rowRange( 0, frame.rows).colRange(frame.cols, frame.cols * 2));
	orientedBB.copyTo( temp.rowRange( frame.rows, frame.rows * 2).colRange(0, frame.cols));
	result.copyTo( temp.rowRange( frame.rows, frame.rows * 2).colRange(frame.cols, frame.cols*2 ));

	cv::resize( temp, dst, dst.size(), 0, 0, cv::INTER_AREA);
}

