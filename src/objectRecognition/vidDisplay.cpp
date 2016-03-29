/*
	Bruce A. Maxwell
	J16 
	Simple example of video capture and manipulation
	Based on OpenCV tutorials

	Compile command

	clang++ -o vid -I /opt/local/include vidDisplay.cpp -L /opt/local/lib -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio

*/
#include <cstdio>
#include "opencv2/opencv.hpp"

int main(int argc, char *argv[]) {
	cv::VideoCapture *capdev;
	char sourceWindowName[255] = "Original Window";
	char thresholdWindowName[255] = "Threshold Window"; 

	// open the video device
	printf(" Opening video capture\n");
	capdev = new cv::VideoCapture(0);
	if( !capdev->isOpened() ) {
		printf("Unable to open video device\n");
		return(-1);
	}

	cv::namedWindow(sourceWindowName, 1); // identifies a window?
	cv::namedWindow( thresholdWindowName, 1 ); 
	for(;;) {
	  cv::Mat frame, thresh;

	  *capdev >> frame; // get a new frame from the camera, treat as a stream
	  cv::imshow( sourceWindowName, frame );

	  // the int parameter controls the threshold value, based on average pixel values. 
	  thresh = threshold( frame, 100 );
	  
	  cv::imshow(thresholdWindowName, thresh ); 
	  
	  if(cv::waitKey(10) >= 0)
	    break;
	  
	}

	cv::destroyWindow( sourceWindowName );
	cv::destroyWindow( thresholdWindowName );
	// terminate the video capture
	printf("Terminating\n");
	delete capdev;

	return(0);
}
