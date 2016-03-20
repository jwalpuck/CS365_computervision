/**
 * score_test.cpp
 * Torrie Edwards, Jack Walpuck
 *
 * Test the scoring metrics in featureDB_ops
 */

#include <cstdio>
#include "opencv2/opencv.hpp"
#include "feature_code.h"
#include "featureDB_ops.h"
#include "score_test.h"

void test_scoring_euc(void) {
  float score;
  ObjectFeature t1, t2;
  t1.unOrientedBoundingBox = 136206.000000;
  t1.width2Height = 1.258359;
  t1.fillRatio = 0.533912;

  t2.unOrientedBoundingBox = 136207.000000;
  t2.width2Height = 2.258359;
  t2.fillRatio = 1.533912;

  score = scoreFeatures(&t1, &t2, EUC_DIST);
  printf("Score = %f\n", score);
}
