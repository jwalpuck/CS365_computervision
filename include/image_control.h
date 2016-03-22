#ifndef IMAGE_CONTROL_H

#include "feature_code.h"
#include "opencv2/opencv.hpp"

#define IMAGE_CONTROL_H

cv::Mat prepImage( cv::Mat &src, cv::Mat &regionMap, cv::Mat &centroid, cv::Mat &boundingBox, int threshValue );

/* Return the index of the object most centered in the frame */
int getCenteredObject(cv::Mat &frame, cv::Mat &boundingBox, cv::Mat &centroid);

/* Populate the given region map to contain an image with different regions colored differently and a bounding box around the most centered object */
void makeRegMapDisplay(cv::Mat &regMapDisplay, cv::Mat &regionMap, cv::Mat &centroid, cv::Mat &boundingBox, int centerObj);

// Create the features that display the oriented bounding box
void makeOrientedBBDisplay( cv::Mat &orientedBB, cv::Mat &regionMap, ObjectFeature *feature, cv::Mat &centroid, int idx);

#endif
