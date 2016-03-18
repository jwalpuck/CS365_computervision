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


// Get the angle that gives the orientation of the axis
float *getCentralAxisAngle( cv::Mat &regMap, cv::Mat &centroids, int idx, int regionSize ){
  float dx, dy;
  float mu20, mu02, mu11 = 0;
  float centralAxis;
  float eigenVal1, eigenVal2, excentricity; 
  cv::Mat centerBasedObjPts; 

  float *result = (float *)malloc(sizeof(float) * 8 ); 
  // --------------------------- Get the central angle --------------------------------------
  centerBasedObjPts.create( regionSize, 2, CV_64FC1 );
  
  int counter = 0;
  for( int i = 0; i < regMap.size().height; i++ ){
    for( int j = 0; j < regMap.size().width; j++ ){
      if( regMap.at<int>(i,j) == idx ){
	// compute the translated position of the points
	dx = j - centroids.at<int>(idx, 1); // x
	dy = centroids.at<int>(idx, 0) - i; // y
	
	// put the translated points into a new list
	centerBasedObjPts.at<float>(counter, 0 ) = dx;
	centerBasedObjPts.at<float>(counter, 1 ) = dy;

	// compute the moments
	mu11 += dx * dy;
	mu20 += dx * dx;
	mu02 += dy * dy;
	counter++; 
      }
    }
  }
  centralAxis = 0.5 * atan2( (2 * mu11),  ( mu20 - mu02 ));
  eigenVal1 = (( mu20 + mu02 ) / 2) + (sqrt( 4 * mu11 * mu11 + ( mu20 - mu02 ))/ 2);
  eigenVal2 =  (( mu20 + mu02 ) / 2) - (sqrt( 4 * mu11 * mu11 + ( mu20 - mu02 ))/ 2);
  excentricity = sqrt( 1 - eigenVal1 / eigenVal2 ); 
  
  //------------------------------ Get oriented bounding box ----------------------------------
  float eCos, eSin;
  float minx, miny, maxx, maxy; 
  int t;

  eCos = cos( centralAxis );
  eSin = sin( centralAxis ); 
 
  minx = miny = FLT_MAX;
  maxx = maxy = FLT_MIN;
  for( t = 0; t < centerBasedObjPts.size().height; t++){
    float x, y;
    float tempX = centerBasedObjPts.at<float>(t, 0);
    float tempY = centerBasedObjPts.at<float>(t, 1); 
    x = tempX * eCos - tempY * eSin; 
    y = tempX * eSin + tempY * eCos;

    // find the min and max X
    /*if( x < minx && y < miny ){
      minx = x;
      miny = y;
    }
    if( x > maxx && y > maxy){
      maxx = x;
      maxy = y;
      }*/
    
    if( x < minx ) {
      minx = x;
      miny = y;
    }else if ( x > maxx ){
      maxx = x;
      maxy = y;
    }

    /*if( y < miny ){
      miny = y;
    }else if ( y > maxy ){
      maxy = y;
      }*/
  }

  float imgMinx, imgMaxx, imgMiny, imgMaxy;
  //rotate the min and max points back
  imgMinx = minx * eCos + miny * eSin;
  imgMiny = miny * eCos -  minx * eSin;

  imgMaxx = maxx *eCos + maxy * eSin;
  imgMaxy = maxy * eCos - maxx * eSin;

  //printf(" ROTATION: %f %f %f %f\n", imgMinx, imgMiny, imgMaxx, imgMaxy );

  // translate back to global coordinates; 
  imgMinx = centroids.at<int>(idx, 1) + imgMinx;
  imgMiny = centroids.at<int>(idx, 0) - imgMiny;

  imgMaxx = centroids.at<int>(idx, 1) + imgMaxx;
  imgMaxy = centroids.at<int>(idx, 0) - imgMaxy;

  //printf("TRANSLATION %f %f %f %f\n", imgMinx, imgMiny, imgMaxx, imgMaxy );

  result[0] = centralAxis;
  result[1] = imgMinx;
  result[2] = imgMiny;
  result[3] = imgMaxx;
  result[4] = imgMaxy;
  result[5] = eigenVal1;
  result[6] = eigenVal2;
  result[7] = excentricity;
  

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
