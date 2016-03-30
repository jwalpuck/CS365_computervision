#ifndef MASTERPIPELINE_H

#define MASTERPIPELINE_H

#include "opencv2/opencv.hpp"

enum State {idle, capture, calibrate, draw};

void writeIntrinsicParams(char *filename, cv::Mat camera_matrix, cv::Mat distortion);

#endif
