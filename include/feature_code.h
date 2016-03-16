#ifndef FEATURE_CODE_H

#include "opencv2/opencv.hpp"

#define FEATURE_CODE_H

typedef struct{
  char id[255];
  float unOrientedBoundingBox;
  float width2Height;
  float fillRatio;
}ObjectFeature;

//  Size of bounding box
int bbArea( cv::Mat &boundingBox, int idx);

//  width to height ratio
float width2Height( cv::Mat &boundingBox, int idx);

//  fill ratio
float fillRatio( cv::Mat &boundingBox, cv::Mat &regMap, int idx);

// Get the number of pixels defined by a region
int numPixelsInBB( cv::Mat &boundingBox, cv::Mat &regMap, int idx );

// Get the pixel locations of all elements in the region
cv::Mat getRegionLocations( cv::Mat &boundingBox, cv::Mat &regMap, int idx );

float getCentralMoment( cv::Mat &pixelIds, cv::Mat &centroids, int idx );

float get2ndOrderMoment( cv::Mat &pixelIds, cv::Mat &boundingBox, cv::Mat &centroids, cv::Mat &regMap, int idx, int type );

float getCentralAxisAngle( cv::Mat &pixelIds, cv::Mat &boundingBox, cv::Mat &centroids, cv::Mat &regMap, int idx );

cv::Mat transformPoints( cv::Mat &pixelIds, cv::Mat &boundingBox, cv::Mat &centroid, cv::Mat &regMap, float centralAxisAngle, int idx );

//  harris features
//  shi-tomasi features

ObjectFeature *getFeatures(cv::Mat boundingBox, cv::Mat regionMap, int closestToCenter);

void printFeatures( ObjectFeature *feature);

#endif
