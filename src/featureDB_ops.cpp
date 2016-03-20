/**
 * Read and write funcitons to parse through the database
 * Project 3
 * March 8, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include "opencv2/opencv.hpp"
#include "feature_code.h"
#include "featureDB_ops.h"

#define debug 0

inline float i_mean(float x, float y) {
  return (x + y) / 2;
}

inline float i_stddev(float x, float y) {
  return sqrt(((x - i_mean(x, y)) * (x - i_mean(x, y))) + ((y - i_mean(x, y)) * (y - i_mean(x, y))) / i_mean(x, y));
}

inline float i_euc(float x, float y) {
  return (x - y) / i_stddev(x, y);
}

// NEED TO DECIDE IF WE WANT TO BUILD ONE OBJECT AT A TIME OR MANY OBJECTS AT ONE TIME
void writeFeatureToFile( ObjectFeature *feature, char *fileOutName ){
  if( debug ){
    printf("Writing Feature to a file\n");
  }
  
  int hold = 0;
  int count = 0;
  FILE *featureFile = fopen( fileOutName, "a");
  
  // Write the struct to the file.
  // Need to decide if we want to do this one at a time. 
  fwrite( &hold, sizeof(int), 1, featureFile );
  fwrite(feature->id, sizeof(char), 255, featureFile );
  fwrite(&feature->unOrientedBoundingBox, sizeof(float), 1, featureFile );
  fwrite(&feature->width2Height, sizeof(float), 1, featureFile );
  fwrite(&feature->fillRatio, sizeof(float), 1, featureFile );

  if( debug ){
    printf("write 1 %d\n", hold);
    printf("write 2 %s\n", feature->id);
    printf("write 3 %f\n", feature->unOrientedBoundingBox);
    printf("write 4 %f \n", feature->width2Height);
    printf("write 5 %f \n", feature->fillRatio);
  }

  fclose( featureFile );
  return;
}

// find the best result from the database file.
// *feature is the feature we are passing in to compare
// *fileInName is the database file
char *findBestFeatureResult( ObjectFeature *feature, char *fileInName ){

  if( debug ){
    printf("reading in a file\n");
  }
  
  FILE *fin = fopen( fileInName, "r+");
  int index = 0;
  float score, top, max;
  max = -1e30;

  // Create temporary feature to read from file
  ObjectFeature tempResult;
  ObjectFeature *result = (ObjectFeature *)malloc(sizeof(ObjectFeature));

  // Read in entire Database!! 
  while( ( fread(&index, sizeof(int), 1 , fin )) != 0 ){
    fread( tempResult.id, sizeof(char), 255, fin );
    

    fread( &tempResult.unOrientedBoundingBox, sizeof(float), 1, fin);
    

    fread( &tempResult.width2Height, sizeof(float), 1, fin);
    

    fread( &tempResult.fillRatio, sizeof(float), 1, fin);
    

    if( debug ){
      printf("Read 1 %d \n", index);
      printf( "Read 2 %s\n", tempResult.id);
      printf( "Read 3 %f \n", tempResult.unOrientedBoundingBox);
      printf( "Read 4 %f \n", tempResult.width2Height );
      printf( "Read 5 %f\n", tempResult.fillRatio);
    }
    // Would want to put some comparison operator here, classifier stuff
    // This would use feature! 
    
    //memcpy((void *)(result), (void *)&tempResult, sizeof(ObjectFeature));
    if( debug ){
      printf( "Copy 1 %s\n", result->id);
      printf( "Copy 2 %f \n", result->unOrientedBoundingBox);
      printf( "Copy 3 %f \n", result->width2Height );
      printf( "Copy 4 %f\n", result->fillRatio);
    }

    score = scoreFeatures(feature, &tempResult, EUC_DIST);
    printf("Score of %s vs max: %f, %f\n", tempResult.id, score, max);
    if(score > max) {
      printf("$$$ New best score! $$$\n");
      max = score;
      //result = &tempResult;
      memcpy((void *)(result), (void *)&tempResult, sizeof(ObjectFeature));
    }
  }
  
  printf("Returning %s\n", result->id);
  return(result->id);
}

float scoreFeatures(ObjectFeature *cur, ObjectFeature *other, int distanceMetric) {
  float score = -9999;

  switch(distanceMetric) {
  case EUC_DIST:
    return scoreEuclidean(cur, other);
    break;
  case K_NEIGH:
    //Call function
    break;
  default:
    //Error
    break;
  }
  
  return score;
}

float scoreEuclidean(ObjectFeature *cur, ObjectFeature *other) {
  float score = 0;

  //Iterate through all features, calculating  Euclidean distance between them
  //UNORIENTED BOUNDING BOX
  score += i_euc(cur->unOrientedBoundingBox, other->unOrientedBoundingBox);

  //WIDTH TO HEIGHT RATIO
  score += i_euc(cur->width2Height, other->width2Height);

  //FILL RATIO
  score += i_euc(cur->fillRatio,  other->fillRatio);

  //Others -----------

  score = fabs(score) * -1;
  return score;
} 

