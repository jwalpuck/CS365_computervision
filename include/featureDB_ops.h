#ifndef FEATUREDB_OPS_H

#include "feature_code.h"

#define FEATUREDB_OPS_H

void writeFeatureToFile( ObjectFeature *feature, char *fileOutName );

ObjectFeature *findBestFeatureResult( ObjectFeature *feature, char *fileInName ); 

#endif
