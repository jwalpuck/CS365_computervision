#ifndef IMAGE_CONTROL_H

#include "opencv2/opencv.hpp"

#define IMAGE_CONTROL_H

int prepImage( cv::Mat src, cv::Mat regionMap, cv::Mat centroid, cv::Mat boundingBox, int threshValue );

#endif
