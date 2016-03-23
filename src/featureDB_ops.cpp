/**
 * Read and write funcitons to parse through the database
 * Project 3
 * March 8, 2016
 * Torrie Edwards, Jack Walpuck
 */

#include <cstdio>
#include <cstring>
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

/** Return the standard deviation of x, which is of length n **/
float n_std(float *x, int n) {
  float mean, total, ssd, std;

  //Calculate the mean of x
  total = 0;
  for(int i = 0; i < n; i++) {
    total += x[i];
  }
  mean = total / n;

  //Use the mean to calculate standard deviation
  ssd = 0;
  for(int i = 0; i < n; i++) {
    ssd += (x[i] - mean) * (x[i] - mean);
  }
  return sqrt(ssd / n);
}

/** Calculate the standard deviation for all vectors across all dimensions and store them
    (by reference) in std_devs **/
void vector_std(float *std_devs, ObjectFeature *vectors, int numVectors) {
  //Declare helper arrays for the features for which we are calculating std_dev
  float a_unOrientedBoundingBox[numVectors];
  float a_width2Height[numVectors];
  float a_fillRatio[numVectors];
  float a_eigenVal1[numVectors];
  float a_eigenVal2[numVectors];

  //Populate the arrays
  for(int i = 0; i < numVectors; i++) {
    a_unOrientedBoundingBox[i] = vectors[i].unOrientedBoundingBox;
    a_width2Height[i] = vectors[i].width2Height;
    a_fillRatio[i] = vectors[i].fillRatio;
    a_eigenVal1[i] = vectors[i].eigenVal1;
    a_eigenVal2[i] = vectors[i].eigenVal2;;
  }

  std_devs[0] = n_std(a_unOrientedBoundingBox, numVectors);
  std_devs[1] = n_std(a_width2Height, numVectors);
  std_devs[2] = n_std(a_fillRatio, numVectors);
  std_devs[3] = n_std(a_eigenVal1, numVectors);
  std_devs[4] = n_std(a_eigenVal2, numVectors);
}

void writeFeatureToFile( ObjectFeature *feature, char *fileOutName ){
  if( debug ){
    printf("Writing Feature to a file\n");
  }
  
  int hold = 0;
  int count = 0;
  FILE *featureFile = fopen( fileOutName, "a");
  
  // Write the struct to the file.
  fwrite( &hold, sizeof(int), 1, featureFile );
  fwrite(feature->id, sizeof(char), 255, featureFile );
  fwrite(&feature->unOrientedBoundingBox, sizeof(float), 1, featureFile );
  fwrite(&feature->width2Height, sizeof(float), 1, featureFile );
  fwrite(&feature->fillRatio, sizeof(float), 1, featureFile );
 
  //NEED TO ADD NEW FEATURES
  fwrite( &feature->size, sizeof(int), 1, featureFile );
  fwrite( &feature->centralAxisAngle, sizeof(float), 1, featureFile );
  fwrite( &feature->eigenVal1, sizeof(float), 1, featureFile );
  fwrite( &feature->eigenVal2, sizeof(float), 1, featureFile ); 
  fwrite( &feature->eccentricity, sizeof(float), 1, featureFile );
  fwrite( &feature->orientedFillRatio, sizeof(float), 1, featureFile );

  if( debug ){
    printf("write 1 %d\n", hold);
    printf("write 2 %s\n", feature->id);
    printf("write 3 %f\n", feature->unOrientedBoundingBox);
    printf("write 4 %f \n", feature->width2Height);
    printf("write 5 %f \n", feature->fillRatio);
    printf("write 6 %d \n", feature->size);
    printf("write 7 %f \n", feature->centralAxisAngle );
    printf("write 8 %f \n", feature->eigenVal1 );
    printf("write 9 %f \n", feature-> eigenVal2 );
    printf("write 10 %f \n", feature->eccentricity );
    printf("write 11 %f \n", feature->orientedFillRatio );
  }

  fclose( featureFile );
  return;
}

/** Returns an array of featureVectors parsed from the given fileName along with the
    number of vectors (by reference into *n) **/
ObjectFeature *getVectors(int *n, char *fileInName) {
  FILE *fin = fopen( fileInName, "r");
  int count = 0, index;

  // Create temporary feature to read from file
  ObjectFeature tempResult;
  ObjectFeature *vectors = (ObjectFeature *)malloc(sizeof(ObjectFeature));

  // Read in entire Database!! 
  while( ( fread(&index, sizeof(int), 1 , fin )) != 0 ){
    fread( tempResult.id, sizeof(char), 255, fin );
    fread( &tempResult.unOrientedBoundingBox, sizeof(float), 1, fin);
    fread( &tempResult.width2Height, sizeof(float), 1, fin);
    fread( &tempResult.fillRatio, sizeof(float), 1, fin);
    // NEED TO ADD NEW FEATURES
    fread( &tempResult.size, sizeof(int), 1, fin );
    fread( &tempResult.centralAxisAngle, sizeof(float), 1, fin );
    fread( &tempResult.eigenVal1, sizeof(float), 1, fin );
    fread( &tempResult.eigenVal2, sizeof(float), 1, fin ); 
    fread( &tempResult.eccentricity, sizeof(float), 1, fin );
    fread( &tempResult.orientedFillRatio, sizeof(float), 1, fin);

    vectors = (ObjectFeature *)realloc(vectors, sizeof(ObjectFeature) * (count + 1));
    if(!vectors) {
      free(vectors);
      exit(-1);
    }
    count++;
  }
  fclose(fin);

  *n = count;
  return vectors;
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
    // NEED TO ADD OTHER FEATURES
    fread( &tempResult.size, sizeof(int), 1, fin );
    
    fread( &tempResult.centralAxisAngle, sizeof(float), 1, fin );
    
    fread( &tempResult.eigenVal1, sizeof(float), 1, fin );
    
    fread( &tempResult.eigenVal2, sizeof(float), 1, fin ); 
    
    fread( &tempResult.eccentricity, sizeof(float), 1, fin );
    
    fread( &tempResult.orientedFillRatio, sizeof(float), 1, fin);
    

   // if( debug ){
      printf("Read 1 %d \n", index);
      printf( "Read 2 %s\n", tempResult.id);
      printf( "Read 3 %f \n", tempResult.unOrientedBoundingBox);
      printf( "Read 4 %f \n", tempResult.width2Height );
      printf( "Read 5 %f\n", tempResult.fillRatio);
      printf( "Read 6 %d\n", tempResult.size);
      printf("Read 7 %f \n", tempResult.centralAxisAngle );
      printf("Read 8 %f \n", tempResult.eigenVal1 );
      printf("Read 9 %f \n", tempResult.eigenVal2 );
      printf("Read 10 %f \n", tempResult.eccentricity );
      printf("Read 11 %f \n", tempResult.orientedFillRatio );
    //}
    // Would want to put some comparison operator here, classifier stuff
    // This would use feature! 
    
    //memcpy((void *)(result), (void *)&tempResult, sizeof(ObjectFeature));
    if( debug ){
      printf( "Copy 1 %s\n", result->id);
      printf( "Copy 2 %f \n", result->unOrientedBoundingBox);
      printf( "Copy 3 %f \n", result->width2Height );
      printf( "Copy 4 %f\n", result->fillRatio);
      printf( "Copy 5 %d \n", result->size);
      printf( "Copy 6 %f \n", result->centralAxisAngle );
      printf( "Copy 7 %f\n", result->eigenVal1);  
      printf( "Copy 8 %f \n", result->eigenVal2);
      printf( "Copy 9 %f\n", result->eccentricity);
      printf( "Copy 10 %f\n", result->orientedFillRatio);
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

  int closed = fclose(fin);
  
  printf("Returning %s\n", result->id);
  return(result->id);
}

/** Returns a list of unique labels in the given file, also passes the size of that array
    by reference **/
char **getLabels(int *n, char *fileName) {
  FILE *fin = fopen( fileName, "r");
  int index = 0, count = 0, stored = 0;
  char **labels = (char **)malloc(sizeof(*labels));

  // Create temporary feature to read from file
  ObjectFeature tempResult;

  // Read in entire Database!! 
  while( ( fread(&index, sizeof(int), 1 , fin )) != 0 ){
    stored = 0; //Assume we have not already stored this label in our array
    fread( tempResult.id, sizeof(char), 255, fin );
    
    //These will not be used, but must be read anyway
    fread( &tempResult.unOrientedBoundingBox, sizeof(float), 1, fin);
    fread( &tempResult.width2Height, sizeof(float), 1, fin);
    fread( &tempResult.fillRatio, sizeof(float), 1, fin);
	// NEED TO ADD OTHER FEATURES 
	fread( &tempResult.size, sizeof(int), 1, fin );
    fread( &tempResult.centralAxisAngle, sizeof(float), 1, fin );
    fread( &tempResult.eigenVal1, sizeof(float), 1, fin );
    fread( &tempResult.eigenVal2, sizeof(float), 1, fin ); 
    fread( &tempResult.eccentricity, sizeof(float), 1, fin );
    fread( &tempResult.orientedFillRatio, sizeof(float), 1, fin);
	
    //Make sure we do not already have this label stored
    for(int i = 0; i < count; i++) {
      if(strcmp(labels[i], tempResult.id) == 0) {
	stored = 1; //If two strings are equal, mark that we have already stored this id
      }
    }
    if(!stored) {
      labels = (char **)realloc(labels, sizeof(*labels) * (count + 1));
      if(!labels) {
	free(labels);
	exit(-1);
      }
      labels[count] = (char *)malloc(strlen(tempResult.id) * sizeof(char));
      strcpy(labels[count], (const char *)tempResult.id);
      count++;
    }
  }

  int closed = fclose(fin);

  *n = count;
  return labels;
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
  score += i_euc( cur->width2Height, other->width2Height );

  //FILL RATIO
  score += i_euc( cur->fillRatio,  other->fillRatio );

  // EIGEN VALUE 1
  score += i_euc( cur->eigenVal1, other->eigenVal1 );
  
  // EIGNE VALUE 2
  score += i_euc( cur->eigenVal2, other->eigenVal2 );
  
  // ECCENTRICITY
  score += i_euc( cur->eccentricity, other->eccentricity );
  
  // ORIENTED BOUNDING BOX compare fill ratio?
  score += i_euc( cur->orientedFillRatio, other->orientedFillRatio );

  score = fabs(score) * -1;
  return score;
}

/** Calculates a class from the given file for the given feature vector cur and stores
    the name of this class at the address match.

    Scoring is done via k_nearestNeighbors using Euclidean distance **/
void *k_nearestNeighbors(ObjectFeature *cur, char *fileName, int k, char *match) {
  int numVectors, numLabels;
  char **labels, nearestLabels[k];
  float nearestDistances[k], *std_devs;
  ObjectFeature *vectors;

  //Get a list of labels (necessary?)
  //labels = getLabels(&numLabels, fileName);

  //Get a list of vectors
  vectors = getVectors(&numVectors, fileName);

  //Initialize the set of best distances to ensure that we populate it
  for(int i = 0; i < k; i++) {
    nearestDistances[i] = FLT_MAX;
  }

  //Calculate the standard deviation in each dimension, store it
  std_devs = (float *)malloc(sizeof(float) * NUM_FEATURES);
  vector_std(std_devs, vectors, numVectors);

  //For each vector in the array:
  for(int i = 0; i < numVectors; i++) {
    //Score the vector vs cur (dist over each dimension summed)
    
    //Check to see if it ranks in top k
  }

  //Clean up
  for(int i = 0; i < numLabels; i++) {
    free(labels[i]);
  }
  free(labels);
  free(vectors);
  free(std_devs);
}
