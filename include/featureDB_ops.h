#ifndef FEATUREDB_OPS_H

#include "feature_code.h"

#define FEATUREDB_OPS_H

const int EUC_DIST = 81;
const int K_NEIGH = 10;

inline float i_mean(float x, float y);
inline float i_stddev(float x, float y);
inline float i_euc(float x, float y);

/** Return the standard deviation of x, which is of length n **/
float n_std(float *x, int n);

void writeFeatureToFile( ObjectFeature *feature, char *fileOutName );

/** Returns an array of featureVectors parsed from the given fileName along with the
    number of vectors (by reference into *n) **/
ObjectFeature *getVectors(int *n, char *fileInName);

char *findBestFeatureResult( ObjectFeature *feature, char *fileInName ); 

/** Returns a list of unique labels in the given file, also passes the size of that array
    by reference **/
char **getLabels(int *n, char *fileName);

float scoreFeatures(ObjectFeature *cur, ObjectFeature *other, int distanceMetric);

float scoreEuclidean(ObjectFeature *cur, ObjectFeature *other);

/** Calculates a class from the given file for the given feature vector cur and stores
    the name of this class at the address match.

    Scoring is done via k_nearestNeighbors using Euclidean distance **/
void *k_nearestNeighbors(ObjectFeature *cur, char *fileName, int k, char *match);

#endif
