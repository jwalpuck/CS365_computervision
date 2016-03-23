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

using namespace std;

int main(int argc, char *argv[]) {

  //Declare variables
  cv::VideoCapture *capdev;
  cv::Mat frame, thresh, boundingBox;
  ObjectFeature *cur;
  
  char curObjLabel[255] = "UNKNOWN -- press j to classify";
  char displayProcess[255] = "Display Process"; 
  
  char *fileName;
  State state = idle;
  int keyPress;
  int threshValue = 150;
  int centerObj = -9999;

  //Process inputs
  if(argc < 2) {
    printf("Usage: %s <featureDB filename>\n", argv[0]);
    return -1;
  }
  fileName = argv[1];

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
    else if(keyPress == 106) //j to recognize
      state = recog;
    else if(keyPress == 107){ //k to see labels in DB
      int n0;
      char **labels = getLabels(&n0, fileName);
      for(int i = 0; i < n0; i++) {
	printf("Label %d: %s\n", i, labels[i]);
	free(labels[i]);
      }
      free(labels);
    }

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
    
    // JACK: for the last stage of the display to work I need cur->id to update its value,
    //   where in the pipeline should this happen? 
    printf("CUR ID %s\n", curObjLabel );
    //cv::putText( idImg, curObjLabel, center, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar( 0, 0,250), 1, CV_AA);
    
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
    case recog:
      {
      //printf("**Current state = recog**\n");
      
      //Calculate features
      // JACK: I don't think we need to recompute now that we have moved it outside of statemachine
      //cur = (ObjectFeature *)malloc(sizeof(*cur));
      //cur = getFeatures(boundingBox, regionMap, centroid, centerObj);

      //Compare cur feature vector with DB to get the best score
      char *match = findBestFeatureResult(cur, fileName);

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

  // JACK: IS THIS OK TO MOVE HERE? !?!?!?!?!? ***********************************
  //Clean up
  //free(cur);
  
  //Clean up outer vars
  cv::destroyWindow( displayProcess );
  delete capdev;


  return 0;
}
