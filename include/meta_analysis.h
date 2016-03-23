#ifndef META_ANALYSIS_H

#define META_ANALYSIS_H

#include "opencv2/opencv.hpp"
#include "feature_code.h"
#include "featureDB_ops.h"

/** Meta analysis pipeline (similar to pipeline_control.main) **/
cv::Mat metaAnalysis_pipeline(char *fileName, int argc, char *argv[]);

/** Builds an nxn confusion matrix where n is the number of labels in the filename **/
cv::Mat confusionMatrix_create(char *fileName);

/** We can put confusion matrix analysis functions here **/

#endif
