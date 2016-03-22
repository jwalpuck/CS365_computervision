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
  
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window";
  char orientedBBWindowName[255] = "Oriented Bounding Box Window"; 
  
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
  cv::namedWindow( sourceWindowName, 1);
  cv::namedWindow( thresholdWindowName, 1 );
  cv::namedWindow( orientedBBWindowName, 1 );

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
    cv::Mat frame, thresh, regionMap, centroid, boundingBox, regMapDisplay, orientedBB;
    string objectLabel = "";

    //Get image
    *capdev >> frame;
    cv::imshow(sourceWindowName, frame);

    //Preprocess the frame
    int size = 0;
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);

    //Find the object closest to the center of the image
    // we could change this to do many objects pretty easily.
    centerObj = getCenteredObject(frame, boundingBox, centroid);
	
    //Create a region map with a bounding box on the centered object
    regMapDisplay.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    makeRegMapDisplay(regMapDisplay, regionMap, centroid, boundingBox, centerObj); //Populate the image
    cv::imshow(thresholdWindowName, regMapDisplay);

    //States: idle, training, recognizing
    switch(state) {

    /** If idle, do nothing **/
    case idle:
      {break;}

    /** If training, calculate features and output with label to given file **/
    case train:
      {
      //printf("**Current state = train**\n");

      //Calculate features
      cur = (ObjectFeature *)malloc(sizeof(*cur));
      
      // get the region size
      cur = getFeatures(boundingBox, regionMap, centroid, centerObj);

      //Get object label from the user
      printf("Please enter a label for the centered object in view\n");
      getline(cin, objectLabel);
      size_t ddd = objectLabel.copy(cur->id, objectLabel.size(), 0);
      cur->id[objectLabel.size()] = '\0'; //Add a null terminator

      // Display the oriented bounding box here. 
      orientedBB.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
      makeOrientedBBDisplay(orientedBB, regionMap, cur, centroid, centerObj); //Populate the image
      cv::imshow( orientedBBWindowName, orientedBB);

    
      //Write features to DB
      cout << "Writing the features of " << objectLabel << " to " << fileName <<endl; //Need to use cout to print C++ string
      writeFeatureToFile(cur, fileName);

      //Clean up
      free(cur);

      //Move the state back to idle
      state = idle;

      break;
      }

    /** If recognizing, score object in view, compare to DB, output best match **/
    case recog:
      {
      //printf("**Current state = recog**\n");
      
      //Calculate features
      cur = (ObjectFeature *)malloc(sizeof(*cur));
      cur = getFeatures(boundingBox, regionMap, centroid, centerObj);

      //Compare cur feature vector with DB to get the best score
      char *match = findBestFeatureResult(cur, fileName);

      //Output label
      printf("Best guess of the centered object in frame: %s\n", match);

      //Clean up
      free(cur);

      //Move the state back to idle
      state = idle;

      break;
      }
    }

    //Clean up frame-specific vars
    frame.release();
    thresh.release();
    regionMap.release();
    centroid.release();
    boundingBox.release();
    orientedBB.release();

  }while(keyPress != 27);

  //Clean up outer vars
  cv::destroyWindow( sourceWindowName );
  cv::destroyWindow( thresholdWindowName );
  cv::destroyWindow( orientedBBWindowName );
  delete capdev;


  return 0;
}
