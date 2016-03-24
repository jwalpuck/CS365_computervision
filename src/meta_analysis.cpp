/**
 * meta_analysis.cpp
 * Torrie Edwards, Jack Walpuck
 *
 * This file contains a library of functions to evaluate the usefulness of classifiers
 */

#include <cstdio>
//#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "feature_code.h"
#include "image_control.h"
#include "featureDB_ops.h"
#include "meta_analysis.h"

using namespace std;

/** Meta analysis pipeline (similar to pipeline_control.main) **/
cv::Mat metaAnalysis_pipeline(char *fileName, int argc, char *argv[]) {
  //Declare variables
  cv::VideoCapture *capdev;
  cv::Mat frame, thresh, boundingBox;
  ObjectFeature *cur;
  
  char curObjLabel[255] = "UNKNOWN -- press j to classify";
  char displayProcess[255] = "Display Process"; 
  
  int keyPress;
  int threshValue = 150;
  int centerObj = -9999;
  int recog = 0; //State flag -- 0 = idle, 1 = recognize
  int idx_x = -1, idx_y = -1; //To calculate where to increment the confusion matrix

  //Parse inputs
  const int classifier = atoi(argv[3]);
  printf("Classifier = %d\n", classifier);
  int k = 0;
  if(classifier == K_NEIGH) {
    if(argc > 4) {
      k = atoi(argv[4]);
    }
    else {
      k = 5;
    }
  }

  //Get a list of labels
  int numLabels;
  char **labels = getLabels(&numLabels, fileName);

  //Build an empty confusion matrix
  cv::Mat confMat = confusionMatrix_create(fileName);

  //Open image stream
  //capdev = new cv::VideoCapture(0); //Torrie
  capdev = new cv::VideoCapture(1); //Jack
  if(!capdev->isOpened()) {
    printf("Unable to open video device\n");
    exit(-1);
  }

  //Set up viewing windows
  cv::namedWindow( displayProcess, 1 );

  do {
    keyPress = cv::waitKey(10);

    if(keyPress == 13) //return/enter to classify the current object
      recog = 1;
    else if(keyPress == 108) //l to see labels in DB
      printLabels(fileName);

    //Initialize loop variables
    cv::Mat frame, thresh, regionMap, centroid, boundingBox, regMapDisplay, orientedBB, 
    	idImg, processImg;
    string objectLabel = "";

    //Get image
    *capdev >> frame;

    //Preprocess the frame
    int size = 0;
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);

    //Find the object closest to the center of the image
    // we could change this to do many objects pretty easily.
    centerObj = getCenteredObject(frame, boundingBox, centroid);
	
    //Create a region map with a bounding box on the centered object
    regMapDisplay.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    makeRegMapDisplay(regMapDisplay, regionMap, centroid, boundingBox, centerObj); //Populate the image
          
    // get the region size
    cur = getFeatures(boundingBox, regionMap, centroid, centerObj);
    
    // Create Oriented bounding box on centered object
    orientedBB.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    makeOrientedBBDisplay(orientedBB, regionMap, cur, centroid, centerObj); //Populate the image
    
    // create the result image displaying the name of the object
    idImg.create((int)frame.size().height, (int)frame.size().width, frame.type()); 
    idImg = frame.clone();
    cv::Point center; 
    center.x = centroid.at<int>(centerObj, 1);
    center.y = centroid.at<int>(centerObj, 0);
    
    cv::putText( idImg, curObjLabel, center, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar( 0, 0,250), 1, CV_AA);
    
    // Create one image to display all the steps of the pipeline
    processImg.create((int)frame.size().height / 2, (int)frame.size().width / 2, frame.type());
    makeDisplayProcess( processImg, frame, regMapDisplay, orientedBB, idImg);
    cv::imshow( displayProcess, processImg );

    if(recog) {
      char match[255], actual[255];

      //Get the actual name of the object
      printf("Enter the corresponding number of the object centered in the frame::\n");
      for(int i = 0; i < numLabels; i++) {
	printf("%d: %s\n", i, labels[i]);
      }
      getline(cin, objectLabel);
      istringstream(objectLabel) >> idx_y;

      //Compare cur feature vector with DB to get the best score
      if(classifier == EUC_DIST) {
	scaledEuclidean(cur, fileName, match);
      }
      else if(classifier == K_NEIGH) {
	int k = 5;
	k_nearestNeighbors(cur, fileName, k, match);
      }
      else {
	printf("Classifier not recognized, exiting program\n");
	exit(-1);
      }

      //Store the current object lable to be displayed
      strcpy( curObjLabel, (const char *)match);
	  
      //Output label
      printf("Best guess of the centered object in frame: %s\n", match);

      //Determine where in the confusion matrix to increment
      for(int i = 0; i < numLabels; i++) {
	if(strcmp(curObjLabel, labels[i]) == 0) {
	  printf("Guessed match: %s\n", labels[i]);
	  idx_x = i;
	  printf("Idx_x = %d\n", idx_x);
	  break;
	}
	// if(strcmp(cur->id, labels[i]) == 0) {
	//   printf("Actual match: %s\n", labels[i]);
	//   idx_y = i;
	//   printf("Idx_y = %d\n", idx_y);
	// }
      }
      printf("Actual match: %s\n", labels[idx_y]);

      //Increment the confusion matrix at the specified location
      if(idx_x < 0 || idx_y < 0) {
	printf("Invalid label\n");
	exit(-1);
      }
      confMat.at<unsigned char>(idx_y, idx_x) += 1;

      //Move the state back to idle
      recog = 0;
    }
  }while(keyPress != 27);

  return confMat;
}

/** Builds an nxn confusion matrix where n is the number of labels in the filename **/
cv::Mat confusionMatrix_create(char *fileName) {
  cv::Mat confMat;
  char **labels;
  int numLabels;

  //Get the number of classes stored in the current DB
  labels = getLabels(&numLabels, fileName);
  
  //Create a numLabels x numLabels matrix of zeros
  confMat = cv::Mat::zeros(numLabels, numLabels, CV_8UC1);

  return confMat;
}

/** Print the confusion matrix nicely **/
//void printConfMat(cv::Mat confMat) {
  
//}
