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


// NEED TO DECIDE IF WE WANT TO BUILD ONE OBJECT AT A TIME OR MANY OBJECTS AT ONE TIME
void writeFeatureToFile( ObjectFeature *feature, char *fileOutName ){
   FILE *featureFile = fopen( fileOutName, "w+");
   fwrite(feature->id, sizeof(char), 255, featureFile );
   fwrite(&feature->unOrientedBoundingBox, sizeof(float), 1, featureFile );
   fwrite(&feature->width2Height, sizeof(float), 1, featureFile );
   fwrite(&feature->fillRatio, sizeof(float), 1, featureFile );
   return;
}

// Want to create a list of ObjectFeatures? 
ObjectFeature *readFeatureFromFile( ObjectFeature *feature, char *fileInName ){
   FILE *fin = fopen( fileInName, "r+");
   ObjectFeature *results = (ObjectFeature *)malloc(sizeof(ObjectFeature));
   fread( results->id, sizeof(char), 255, fin );
   fread( &results->unOrientedBoundingBox, sizeof(float), 1, fin);
   fread( &results->width2Height, sizeof(float), 1, fin);
   fread( &results->fillRatio, sizeof(float), 1, fin);
   return(results);
}

