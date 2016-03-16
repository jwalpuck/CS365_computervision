/**
 * Does manipulation and calculation on features
 * Project 3
 * March 8, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include <math.h>
#include "feature_code.h"

#define debug 0

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

// Need to count pixels in the region in the 
int numPixelsInBB( cv::Mat &boundingBox, cv::Mat &regMap, int idx ){
  int count = 0; 
  for( int i = 0; i< regMap.size().height; i++){
    for( int j = 0; j < regMap.size().width; j++){
      if( i > boundingBox.at<int>(idx,1) && i < boundingBox.at<int>(idx, 3)
	  && j > boundingBox.at<int>(idx, 0) && j < boundingBox.at<int>(idx, 2)){
	if( regMap.at<int>(i, j) > 0 ){
	  count ++;
	}
      }
    }
  }
  return( count );
}

// A function to get the pixel locations for items in the region
cv::Mat getRegionLocations( cv::Mat &boundingBox, cv::Mat &regMap, int idx ){
  // This might be better to return a list of points   
  int numPoints = numPixelsInBB( boundingBox, regMap, idx);
  int count = 0; 
  // create a list of points that are in the region
  cv::Mat regionIdxs; 
  regionIdxs.create( numPoints, 2, CV_64FC1);
 

  // Get the index places for points in the bounding box region
  for( int i = 0; i < regMap.size().height; i++){
    for( int j = 0; j < regMap.size().width; j++){
      if( i > boundingBox.at<int>(idx,1) && i < boundingBox.at<int>(idx, 3)
	  && j > boundingBox.at<int>(idx, 0) && j < boundingBox.at<int>(idx, 2)){
	if( regMap.at<int>(i, j) > 0 ){
	  regionIdxs.at<float>(count, 0) = i;
	  regionIdxs.at<float>(count, 1) = j;
	 
	  //if( debug ){
	  printf(" numPoints: %d count: %d x: %f y: %f \n",numPoints, count, regionIdxs.at<float>(count, 0), regionIdxs.at<float>(count,1));
	  //}
	   count ++ ;
	}
      }
    

    }
  }
  return( regionIdxs );
}

// Compute mu11 
float getCentralMoment( cv::Mat &pixelIds, cv::Mat &centroids, int idx ){
  float result = 0;
  float tempX, tempY;
  // the sum of the distances between the centroids and all other pointsin the region
  for( int i = 0; i < pixelIds.size().height; i++){
    tempX = pixelIds.at<float>(i,0) - centroids.at<int>(idx, 1); // x
    tempY = pixelIds.at<float>(i,1) - centroids.at<int>(idx, 0); // y
    result += tempX * tempY; 
  }
  return( result );
}

// compute mu02 if type == 1
// compute mu20 if type == 0
float get2ndOrderMoment(cv::Mat &pixelIds, cv::Mat &boundingBox, cv::Mat &centroids, cv::Mat &regMap, int idx, int type ){
  float area;
  area =  numPixelsInBB( boundingBox, regMap, idx);
  float result = 0;
  float temp; 
  for( int i = 0; i < pixelIds.size().height; i++){
    if( type == 0 ){
      temp = pixelIds.at<float>(i,0) - centroids.at<int>(idx, 1); // x
    }else if( type == 1){
      temp = pixelIds.at<float>(i,1) - centroids.at<int>(idx, 0); // y
    }
    result += (temp * temp); 
  }
  return( result/area );
}

// Get the angle that gives the orientation of the axis
float getCentralAxisAngle( cv::Mat &pixelIds, cv::Mat &boundingBox, cv::Mat &centroids, cv::Mat &regMap, int idx ){
  float result, central11, secondY, secondX, area;
  float mu20, mu02, mu11;
  area =  numPixelsInBB( boundingBox, regMap, idx);
  central11 = getCentralMoment( pixelIds, centroids, idx);
  secondX = get2ndOrderMoment( pixelIds, boundingBox, centroids, regMap, idx, 0 );
  secondY = get2ndOrderMoment( pixelIds, boundingBox, centroids, regMap, idx, 1 );

  mu20 = secondX / area - (float)(centroids.at<int>(idx,1) * centroids.at<int>(idx,1)); // x
  mu02 = secondY / area - (float)(centroids.at<int>(idx,0) * centroids.at<int>(idx,0)); // y

  printf(" %f %f \n", mu20, mu02);
  mu11 = central11 / area - (float)(centroids.at<int>(idx,0) * centroids.at<int>(idx,1));
  
  result = 0.5 * atan2( (2 * mu11),  ( mu02 - mu20 ));
  return( result );
  
}


cv::Mat transformPoints( cv::Mat &pixelIds, cv::Mat &boundingBox, cv::Mat &centroid, cv::Mat &regMap, float centralAxisAngle, int idx ){
  int area; 
  area =  numPixelsInBB( boundingBox, regMap, idx);
  cv::Mat  final, translationMat, transposePoints, result, resultFinal;
  double minVal, maxVal;
  cv::Point minPt, maxPt; 
  
  float eCos = cos( centralAxisAngle );
  float eSin = sin( centralAxisAngle );

  printf(" Sin: %f  Cos: %f\n", eSin, eCos );
  
  cv::transpose( pixelIds, transposePoints );
  if( debug ){
    printf("P: %d %d \n", (int)pixelIds.size().height, (int)pixelIds.size().width);
    printf("PT: %d %d \n", (int)transposePoints.size().height, (int)transposePoints.size().width);
  }

  // Create the 2 needed matrices 
  translationMat = cv::Mat::zeros( transposePoints.size().height, transposePoints.size().width, CV_64FC1);
  final = cv::Mat::zeros( transposePoints.size().height, transposePoints.size().width, transposePoints.type());

  // Translate
  for( int i = 0; i < transposePoints.size().width; i++){
    translationMat.at<float>(0, i) = pixelIds.at<float>(i, 0) - centroid.at<int>(idx, 1 );
    translationMat.at<float>(1, i) = pixelIds.at<float>(i, 1) - centroid.at<int>(idx, 0 );
    
    if(debug)
      printf("T x: %f y: %f \n", translationMat.at<float>(0 ,i), translationMat.at<float>(1, i));
  }
  
  // Rotate     
  for(int i = 0; i < (int)transposePoints.size().width; i++){
    final.at<float>(0, i) = eCos * translationMat.at<float>(0,i) + eSin * translationMat.at<float>(1,i);
    final.at<float>(1, i) = eSin * translationMat.at<float>(0,i) + eCos * translationMat.at<float>(1,i);
    if(debug){
      printf("R2 x: %f y: %f \n", final.at<float>(0 ,i), final.at<float>(1, i));
    }
  }

  // Move back to original coords. 
  result.create( 2, 2, CV_64FC1);
  resultFinal.create( 2, 2, CV_64FC1);
  cv::minMaxLoc( final, &minVal, &maxVal, &minPt, &maxPt );
  //if( debug ){
    printf("%f %f \n", final.at<float>(0, maxPt.x), final.at<float>(1, maxPt.x));
    printf("%f %f \n", final.at<float>(0, minPt.x), final.at<float>(1, minPt.x));
    //}
  
  // Rotate Back
  resultFinal.at<float>(0, 0) = final.at<float>(0, minPt.x) * eSin + final.at<float>(1, minPt.x) * eCos;
  resultFinal.at<float>(1, 0) = final.at<float>(0, minPt.x) * eCos + final.at<float>(1, minPt.x) * eSin;
  resultFinal.at<float>(0, 1) = final.at<float>(0, maxPt.x) * eSin + final.at<float>(1, maxPt.x) * eCos;
  resultFinal.at<float>(1, 1) = final.at<float>(0, maxPt.x) * eCos + final.at<float>(1, maxPt.x) * eSin;

  // Translate Back: put in N x 2 matrix
  result.at<float>(0, 0) = resultFinal.at<float>(0, 0) + centroid.at<int>(idx, 1);
  result.at<float>(0, 1) = resultFinal.at<float>(1, 0) + centroid.at<int>(idx, 0);
  result.at<float>(1, 0) = resultFinal.at<float>(0, 1) + centroid.at<int>(idx, 1);
  result.at<float>(1, 1) = resultFinal.at<float>(1, 1) + centroid.at<int>(idx, 0);
  
  return( result );
}

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
