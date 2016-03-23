#ifndef FEATURE_CODE_H

#include "opencv2/opencv.hpp"

#define FEATURE_CODE_H
#define NUM_FEATURES 7

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
  float eccentricity; 
  float orientedFillRatio;
}ObjectFeature;

//  Size of bounding box
int bbArea( cv::Mat &boundingBox, int idx);

//  width to height ratio
float width2Height( cv::Mat &boundingBox, int idx);

//  fill ratio
float fillRatio( cv::Mat &boundingBox, cv::Mat &regMap, int idx);

float ptsInOrientedBox( ObjectFeature &feature, cv::Mat &regMap );

int getRegionSize( cv::Mat &regMap, int idx );

float *getCentralAxisAngle( cv::Mat &regMap, cv::Mat &centroids, int idx, int regionSize );

ObjectFeature *getFeatures(cv::Mat &boundingBox, cv::Mat &regionMap, cv::Mat &centroids, int closestToCenter);

void printFeatures( ObjectFeature *feature);

#endif
