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
ObjectFeature *findBestFeatureResult( ObjectFeature *feature, char *fileInName ){

  if( debug ){
    printf("reading in a file\n");
  }
  
  FILE *fin = fopen( fileInName, "r+");
  int index = 0;

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
    
    memcpy((void *)(result), (void *)&tempResult, sizeof(ObjectFeature));
    if( debug ){
      printf( "Copy 1 %s\n", result->id);
      printf( "Copy 2 %f \n", result->unOrientedBoundingBox);
      printf( "Copy 3 %f \n", result->width2Height );
      printf( "Copy 4 %f\n", result->fillRatio);
    }
  }
  
  return(result);
}

