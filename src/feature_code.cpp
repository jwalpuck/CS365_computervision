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

// get the number of points in the region map with the specified index
int getRegionSize( cv::Mat &regMap, int idx ){
  int size = 0; 
  for( int i = 0; i < (int)regMap.size().height; i++){
    for( int j = 0; j < (int)regMap.size().width; j++){
      if( regMap.at<int>(i, j ) == idx ){
	size += 1;
      }
    }
  }
  return( size );
}


// Get the angle that gives the orientation of the axis
float *getCentralAxisAngle( cv::Mat &regMap, cv::Mat &centroids, int idx, int regionSize ){
  float dx, dy;
  float mu20, mu02, mu11 = 0;
  float centralAxis;
  float eigenVal1, eigenVal2, excentricity; 
  cv::Mat centerBasedObjPts; 

  float *result = (float *)malloc(sizeof(float) * 12 ); 
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
  float minXY, minYX, maxXY, maxYX;
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
    if( x < minx ) {
      minx = x;
      minXY = y;
    }else if ( x > maxx ){
      maxx = x;
      maxXY = y;
    }

    if( y < miny ){
      miny = y;
      minYX = x; 
    }else if ( y > maxy ){
      maxy = y;
      maxYX = x;
    }
    //printf("PTS: %f %f %f %f %f %f %f %f \n", minx, minXY, minYX, miny, maxx, maxXY, maxYX, maxy ); 
  }

  float imgMinx, imgMaxx, imgMiny, imgMaxy;
  float imgMinXY, imgMaxXY, imgMinYX, imgMaxYX;
  //rotate the min and max points back
  eSin = sin( centralAxis );
  eCos = cos( centralAxis );
  
  imgMinx = minx * eCos + minXY * eSin;
  imgMinXY = minXY * eCos - minx * eSin; 

  imgMinYX = minYX * eCos +  miny * eSin; 
  imgMiny = miny * eCos -  minYX * eSin;

  imgMaxx = maxx * eCos + maxXY * eSin;
  imgMaxXY = maxXY * eCos - maxx * eSin;

  imgMaxYX = maxYX * eCos + maxy * eSin;
  imgMaxy = maxy * eCos - maxYX * eSin;

  //printf(" ROTATION: %f %f %f %f\n", imgMinx, imgMiny, imgMaxx, imgMaxy );

  // translate back to global coordinates; 
  imgMinx = centroids.at<int>(idx, 1) + imgMinx;
  imgMinXY = centroids.at<int>(idx, 0) - imgMinXY;

  imgMinYX = centroids.at<int>(idx, 1) + imgMinYX; 
  imgMiny = centroids.at<int>(idx, 0) - imgMiny;

  imgMaxx = centroids.at<int>(idx, 1) + imgMaxx;
  imgMaxXY = centroids.at<int>(idx, 0) - imgMaxXY; 

  imgMaxYX = centroids.at<int>(idx, 1) + imgMaxYX; 
  imgMaxy = centroids.at<int>(idx, 0) - imgMaxy;
  

  //printf("TRANSLATION %f %f %f %f\n", imgMinx, imgMiny, imgMaxx, imgMaxy );

  result[0] = centralAxis;
  result[1] = imgMinx;
  result[2] = imgMinXY;
  result[3] = imgMinYX;
  result[4] = imgMiny;
  result[5] = imgMaxx;
  result[6] = imgMaxXY;
  result[7] = imgMaxYX;
  result[8] = imgMaxy;
  result[9] = eigenVal1;
  result[10] = eigenVal2;
  result[11] = excentricity;
  
  //printf("CENTRAL AXIS ANGLE %f \n", centralAxis );
  return( result );
}


// Parent function to call all the possible feature options on each object placed into the database 
ObjectFeature *getFeatures( cv::Mat &boundingBox, cv::Mat &regionMap, cv::Mat &centroids, int closestToCenter, int regionSize){
  ObjectFeature *results = (ObjectFeature *)malloc(sizeof(ObjectFeature));
  results->id[0] = '\0';
  results->unOrientedBoundingBox = bbArea( boundingBox, closestToCenter );
  results->width2Height = width2Height( boundingBox, closestToCenter );
  results->fillRatio = fillRatio( boundingBox, regionMap, closestToCenter );
  results->size = regionSize; 
  float *tempOrientedInfo;
  tempOrientedInfo = getCentralAxisAngle( regionMap, centroids, closestToCenter, regionSize);
  results->centralAxisAngle = tempOrientedInfo[0];
  results->orientedBoundingBox[0] = tempOrientedInfo[1];
  results->orientedBoundingBox[1] = tempOrientedInfo[2];
  results->orientedBoundingBox[2] = tempOrientedInfo[3];
  results->orientedBoundingBox[3] = tempOrientedInfo[4];
  results->orientedBoundingBox[4] = tempOrientedInfo[5];
  results->orientedBoundingBox[5] = tempOrientedInfo[6];
  results->orientedBoundingBox[6] = tempOrientedInfo[7];
  results->orientedBoundingBox[7] = tempOrientedInfo[8];
  results->eigenVal1 = tempOrientedInfo[9];
  results->eigenVal2 = tempOrientedInfo[10];
  results->excentricity = tempOrientedInfo[11];

  free( tempOrientedInfo );
  return(results);
}

// SHOULD PROBABLY CHANGE THIS TO PULL UP ONE WINDOW
// HCONCAT or VCONCAT the matrices. 
void displayProcess( ObjectFeature *feature, cv::Mat &boundingBox, cv::Mat &regionMap,cv::Mat &centroid, cv::Mat &frame, cv::Mat &thresh, int idx){
  cv::Mat regMapDisplay, orientedBoundingBox;
  // Naming the Windows
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window"; 
  char boundingBoxWindowName[255] = "Bounding Box Window";
  char orientedBoundingBoxWindowName[255] = "Oriented Bounding Box Window";
  
  // SET UP VIEWING WINDOWS
  cv::namedWindow( sourceWindowName, 1); // identifies a window?
  cv::namedWindow( thresholdWindowName, 1 ); 
  cv::namedWindow( boundingBoxWindowName, 1);
  cv::namedWindow( orientedBoundingBoxWindowName, 1);

  //---------------------------------- thresholded image ---------------------------------
  regMapDisplay.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
  for( int i = 0; i < (int)regionMap.size().height; i++){
    for( int j = 0; j < (int)regionMap.size().width; j++){
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[0] = regionMap.at<int>(i, j) == 2 ? 255 : 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[1] = regionMap.at<int>(i, j) == 1 ? 255 : 0;
      regMapDisplay.at<cv::Vec3b>(cv::Point(j,i))[2] = (regionMap.at<int>(i, j)) == 0 ? 255 : 0; 
    }
  }

  cv::rectangle( regMapDisplay, cv::Point(  boundingBox.at<int>(idx,1), boundingBox.at<int>(idx,0)), cv::Point( boundingBox.at<int>(idx,3), boundingBox.at<int>(idx,2)), cv::Scalar(0, 255, 0));
  cv::circle( regMapDisplay, cv::Point(centroid.at<int>(idx,1), centroid.at<int>(idx,0)), 2, cv::Scalar( 255, 0, 0), 3);

  //----------------------------------- oriented bounding box -------------------------
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
  center.x = centroid.at<int>( idx, 1);
  center.y = centroid.at<int>( idx, 0);
      
  distOriented.x = center.x + 200 * cos( feature->centralAxisAngle );
  distOriented.y = center.y - 200 * sin( feature->centralAxisAngle );

  // central axis
  cv::line( orientedBoundingBox, center, distOriented, cv::Scalar( 255, 255, 255), 2);
      
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
  cv::line( orientedBoundingBox, p1, p2, cv::Scalar(0, 255, 255), 3 );
  cv::line( orientedBoundingBox, p2, p3, cv::Scalar(0, 255, 255), 3);
  cv::line( orientedBoundingBox, p3, p4, cv::Scalar(0, 255, 255), 3);
  cv::line( orientedBoundingBox, p4, p1, cv::Scalar(0, 255, 255), 3);

  //---------------------------------------display the images -----------------------
  cv::imshow( sourceWindowName, frame );
  cv::imshow(thresholdWindowName, thresh);
  cv::imshow(boundingBoxWindowName, regMapDisplay);
  cv::imshow( orientedBoundingBoxWindowName, orientedBoundingBox);
}

void destroyDisplay( ) {
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window"; 
  char boundingBoxWindowName[255] = "Bounding Box Window";
  char orientedBoundingBoxWindowName[255] = "Oriented Bounding Box Window";
  
  cv::destroyWindow( sourceWindowName );
  cv::destroyWindow( thresholdWindowName );
  cv::destroyWindow( boundingBoxWindowName );
  cv::destroyWindow( orientedBoundingBoxWindowName ); 
}

// debugging tool to print an ObjectFeature Object
void printFeatures( ObjectFeature *feature ){
  printf("HERE\n");
  printf("Feature Name: %s\n", feature->id );
  printf("Area: %d\n", feature->unOrientedBoundingBox ); 
  printf("Ratio: %f\n", feature->width2Height);
  printf("Fill Ratio: %f\n", feature->fillRatio);
  printf("Central Axis Angle %f \n", feature->centralAxisAngle );
  printf("EgienVal 1 %f \n", feature->eigenVal1 );
  printf("EigenVal 2 %f \n", feature->eigenVal2 );
  printf("Excentricity %f \n", feature->excentricity );
}
