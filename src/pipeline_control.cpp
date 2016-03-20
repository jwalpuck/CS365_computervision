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

  //DEBUG -- test scoring
  // test_score_euc();
  // return;



  //Declare variables
  cv::VideoCapture *capdev;
  cv::Mat frame, thresh, boundingBox;
  ObjectFeature *cur;
  char sourceWindowName[255] = "Original Window";
  char thresholdWindowName[255] = "Threshold Window";
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

  //Main loop
  do {
  //for(;;) {

    //TEST
    keyPress = cv::waitKey(10);
    //printf("Key pressed: %d\n", keyPress);
    if(keyPress == 105) //i
      state = train;
    else if(keyPress == 106) //j
      state = recog;

    //Initialize loop variables
    cv::Mat frame, thresh, regionMap, centroid, boundingBox, regMapDisplay;
    string objectLabel = "";

    //Get image
    *capdev >> frame;
    cv::imshow(sourceWindowName, frame);

    //Preprocess the frame
    int size = 0;
    thresh = prepImage(frame, regionMap, centroid, boundingBox, threshValue);

    //Find the object closest to the center of the image
    centerObj = getCenteredObject(frame, boundingBox, centroid);

    //Create a region map with a bounding box on the centered object
    regMapDisplay.create((int)regionMap.size().height, (int)regionMap.size().width, frame.type());
    makeRegMapDisplay(regMapDisplay, regionMap, centroid, boundingBox, centerObj); //Populate the image
    

    cv::imshow(thresholdWindowName, regMapDisplay);

    //States: idle, training, recognizing
    switch(state) {

    /** If idle, do nothing **/
    case idle:
      break;

    /** If training, calculate features and output with label to given file **/
    case train:
      //printf("**Current state = train**\n");

      //Calculate features
      cur = (ObjectFeature *)malloc(sizeof(*cur));
      cur = getFeatures(boundingBox, regionMap, centerObj);

      //Get object label from the user
      printf("Please enter a label for the centered object in view\n");
      getline(cin, objectLabel);
      //const char *cstr = objectLabel.c_str();
      //char cstr[255] = {};
      std::copy(objectLabel.begin(), objectLabel.end(), cur->id);
      //cur->id = cstr;

      /****/
      //NOTE TO TORRIE: We could display features here if we wanted
      /****/

      //Write features to DB
      cout << "Writing the features of " << objectLabel << " to " << fileName <<endl; //Need to use cout to print C++ string
      writeFeatureToFile(cur, fileName);

      //Clean up
      free(cur);
      //delete [] cstr;

      //Move the state back to idle
      state = idle;

      break;

    /** If recognizing, score object in view, compare to DB, output best match **/
    case recog:
      //printf("**Current state = recog**\n");
      
      //Calculate features
      cur = (ObjectFeature *)malloc(sizeof(*cur));
      cur = getFeatures(boundingBox, regionMap, centerObj);

      //Compare cur feature vector with DB to get the best score
      char *match = findBestFeatureResult(cur, fileName);

      //Output label
      printf("Best guess of the centered object in frame: %s\n", match);

      //Clean up
      free(cur);

      //Move the state back to idle
      state = idle;

      break;

      //(Have running thread listening for keystrokes to change state?)
    }

    //Clean up frame-specific vars
    frame.release();
    thresh.release();
    regionMap.release();
    centroid.release();
    boundingBox.release();
    //  }
  }while(keyPress != 27);

  //Clean up outer vars
  cv::destroyWindow( sourceWindowName );
  cv::destroyWindow( thresholdWindowName );
  delete capdev;


  return 0;
}
