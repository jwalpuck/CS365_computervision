#ifndef FEATUREDB_OPS_H

#include "feature_code.h"

#define FEATUREDB_OPS_H

const int EUC_DIST = 81;
const int K_NEIGH = 10;

inline float i_mean(float x, float y);
inline float i_stddev(float x, float y);

void writeFeatureToFile( ObjectFeature *feature, char *fileOutName );

ObjectFeature *findBestFeatureResult( ObjectFeature *feature, char *fileInName ); 

float scoreFeatures(ObjectFeature *cur, ObjectFeature *other, int distanceMetric);

float scoreEuclidean(ObjectFeature *cur, ObjectFeature *other);

#endif
