#ifndef FEATURE_CODE_H

#include "opencv2/opencv.hpp"

#define FEATURE_CODE_H

typedef struct{
  char id[255];
  float unOrientedBoundingBox;
  float width2Height;
  float fillRatio;
  int size;
  float centralAxisAngle;
  float orientedBoundingBox[8];
  float eigenVal1;
  float eigenVal2;
  float excentricity; 
}ObjectFeature;

//  Size of bounding box
int bbArea( cv::Mat &boundingBox, int idx);

//  width to height ratio
float width2Height( cv::Mat &boundingBox, int idx);

//  fill ratio
float fillRatio( cv::Mat &boundingBox, cv::Mat &regMap, int idx);

int getRegionSize( cv::Mat &regMap, int idx );

float *getCentralAxisAngle( cv::Mat &regMap, cv::Mat &centroids, int idx, int regionSize );

ObjectFeature *getFeatures(cv::Mat &boundingBox, cv::Mat &regionMap, cv::Mat &centroids, int closestToCenter, int regionSize);

void displayProcess( ObjectFeature *feature, cv::Mat &boundingBox, cv::Mat &regionMap,cv::Mat &centroid, cv::Mat &frame, cv::Mat &thresh, int idx);

void destroyDisplay( );

void printFeatures( ObjectFeature *feature);

#endif
