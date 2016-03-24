/**
 * Contains a state machine/pipeline for real-time object recognition
 * Project 3
 * March 17, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include "opencv2/opencv.hpp"
#include "feature_code.h"
#include "image_control.h"
#include "featureDB_ops.h"
#include "pipeline_control.h"
#include "score_test.h"
#include "meta_analysis.h"

using namespace std;

int main(int argc, char *argv[]) {

  char *fileName;

  //Process inputs
  if(argc < 2) {
    printf("Usage: %s <featureDB filename> (<meta-analysis flag = 1> <classifier type> (<additional params>)\n", argv[0]);
    printf("Executing this program with the meta-analysis flag set to 1 will enable meta-analysis mode.\n");
    printf("In meta-analysis mode:\n");
    printf("    New data cannot be written to the given DB\n");
    printf("    The centered object in view will be classified when the user presses 'return'\n");
    printf("    The user can indicate that they are done classifying by pressing 'esc'\n");
    printf("\nClassifier options:\n    0: Scaled pairwise Euclidean distance\n    1: k-Nearest Neighbors -- An additional argument may be used to specify the k value (default 5)\n");
    return -1;
  }
  fileName = argv[1];

  if(atoi(argv[2]) == 1) {
    if(argc < 4) {
      printf("If you want to run in meta-analysis mode, please enter a classifer type:\n");
      printf("\nClassifier options:\n    0: Scaled pairwise Euclidean distance\n    1: k-Nearest Neighbors -- An additional argument may be used to specify the k value (default 5)\n");   
      return -1;
    }
    printf("Running in meta-analysis mode.\n");
    cv::Mat confMat = metaAnalysis_pipeline(fileName, argc, argv);
    
    //Can do analytics here
    //    printConfMat(confMat);
    printf("\n\n");
    printLabels(fileName);
    cout << "ConfMat = " << endl << " " << confMat << endl << endl;

    return 0;
  }
  
  //Declare variables
  cv::VideoCapture *capdev;
  cv::Mat frame, thresh, boundingBox;
  ObjectFeature *cur;
  
  char curObjLabel[255] = "UNKNOWN -- press j to classify";
  char displayProcess[255] = "Display Process"; 

  State state = idle;
  int keyPress;
  int threshValue = 150;
  int centerObj = -9999;

  //Initialize necessary memory

  //Open image stream
  //capdev = new cv::VideoCapture(0); //Torrie
  capdev = new cv::VideoCapture(1); //Jack
  if(!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return -1;
  }

  //Set up viewing windows
  cv::namedWindow( displayProcess, 1 );

  //Main loop
  do {
    keyPress = cv::waitKey(10);

    if(keyPress == 105) //i to train
      state = train;
    else if(keyPress == 106) //j to recognize with scaled euclidean
      state = recog_euc;
    else if(keyPress == 107) //k to recognize with k-nearest neighbors
      state = recog_knn;
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
    processImg.create((int)frame.size().height / 2, (int)frame.size().width * 2, frame.type());
    makeDisplayProcess( processImg, frame, regMapDisplay, orientedBB, idImg);
    cv::imshow( displayProcess, processImg );

    //States: idle, training, recognizing
    switch(state) {

    /** If idle, do nothing **/
    case idle:
      {break;}

    /** If training, calculate features and output with label to given file **/
    case train:
      {
      //printf("**Current state = train**\n");

      //Get object label from the user
      printf("Please enter a label for the centered object in view\n");
      getline(cin, objectLabel);
      size_t ddd = objectLabel.copy(cur->id, objectLabel.size(), 0);
      cur->id[objectLabel.size()] = '\0'; //Add a null terminator
    
      //Write features to DB
      cout << "Writing the features of " << objectLabel << " to " << fileName <<endl; //Need to use cout to print C++ string
      writeFeatureToFile(cur, fileName);
      
      //Move the state back to idle
      state = idle;

      break;
      }

    /** If recognizing, score object in view, compare to DB, output best match **/
    case recog_euc:
    case recog_knn:
      {
      //printf("**Current state = recog**\n");
      
      //Calculate features
      // JACK: I don't think we need to recompute now that we have moved it outside of statemachine
      //cur = (ObjectFeature *)malloc(sizeof(*cur));
      //cur = getFeatures(boundingBox, regionMap, centroid, centerObj);

	char match[255];
	//Compare cur feature vector with DB to get the best score
	if(state == recog_euc) {
	  scaledEuclidean(cur, fileName, match);
	}
	else { //k-nearest neighbors
	  int k = 5;
	  k_nearestNeighbors(cur, fileName, k, match);
	}

      //Store the current object lable to be displayed
      strcpy( curObjLabel, (const char *)match);
	  
      //Output label
      printf("Best guess of the centered object in frame: %s\n", match);

      //Move the state back to idle
      state = idle;

      break;
      }
    }

    //Clean up frame-specific vars
    free(cur);
    frame.release();
    thresh.release();
    regionMap.release();
    centroid.release();
    boundingBox.release();
    orientedBB.release();
    processImg.release();

  }while(keyPress != 27);
  
  //Clean up outer vars
  cv::destroyWindow( displayProcess );
  delete capdev;


  return 0;
}
