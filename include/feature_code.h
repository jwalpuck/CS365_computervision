#ifndef FEATURE_CODE_H

#include "opencv2/opencv.hpp"

#define FEATURE_CODE_H


//  Size of bounding box
int bbArea( cv::Mat &boundingBox);

//  width to height ratio
float width2Height( cv::Mat &boundingBox);

//  fill ratio
float fillRatio( cv::Mat &boundingBox, cv::Mat &regMap);

//  moments: central, primary axes**, 
cv::Mat primaryAxis(cv::Mat &boundingBox, cv::Mat &centroid, cv::Mat &regMap);

//  harris features
//  shi-tomasi features

float *getFeatures(cv::Mat boundingBox, cv::Mat regionMap);

#endif
