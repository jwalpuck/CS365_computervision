/*
  Bruce A. Maxwell

  This program is an implementation of a 2-pass segmentation algorithm
  on a binary OpenCV 8U1 image. 0 is background, not zero is foreground.
*/

#include <stdlib.h>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "segment.h"

// uncomment to generate some debug statements
//#define DEBUG

/*
 */
int locateRegions(cv::Mat &src,
		  cv::Mat &regmap,
		  const long sizeThresh,
		  cv::Mat &centroid,
		  cv::Mat &bbox,
		  cv::Mat &size,
		  const int MaxLocations
		  )
{
  // statics to reduce setup time when calling the function a lot
  static long *boundto = NULL;
  static long *regcard = NULL;

  // local variables
  const long imageSize = src.rows * src.cols;
  long i, j, k;
  unsigned char backpix, uppix, curpix;
  long nRegions=0;
  long *bt, *reg;
  unsigned long upID, backID;
  long *sizeid;
  long numLocations = MaxLocations;

  // static variable allocations
  if(boundto == NULL)
    boundto = (long *)malloc(sizeof(long) * imageSize/4);
  if(regcard == NULL)
    regcard = (long *)malloc(sizeof(long) * imageSize/4);

  // dynamic variable allocations
  sizeid = (long *)malloc(sizeof(long) * MaxLocations );

  // allocate the region map (doesn't allocate more than once if already the right size)
  // initialize the region map to -1
  regmap.create( src.rows, src.cols, CV_32SC1);
  regmap = -1;

  // initialize the region map and boundto variables
#ifdef DEBUG
  printf("Initializing arrays\n");
#endif
  reg = regcard;
  bt = boundto;
  for(i=0;i<imageSize/4;i++, reg++, bt++) {
    *reg = 0;
    *bt = i;
  }


#ifdef DEBUG
  printf("Starting 2-pass algorithm\n");
#endif

  // segment the image using a 2-pass algorithm (4-connected)
  for(i=0;i<src.rows;i++) {
    backpix = 0; // outside the image is 0

    for(j=0;j<src.cols;j++) {

      // get the value of the current pixel
      curpix = src.at<unsigned char>(i, j);

      // if the current pixel is foreground
      if(curpix) {

	// get the up pixel 
	uppix = i > 0 ? src.at<unsigned char>(i-1, j) : 0;

	// test if up-pixel is foreground
	if(uppix) {
	  
	  // test if back-pixel is foreground
	  if(backpix) {

	    // similar to back pixel as well so get the region IDs
	    upID = regmap.at<int>(i-1, j);
	    backID = regmap.at<int>(i, j-1);
						
	    if(backID == upID) { // equal region ids
	      regmap.at<int>(i, j) = upID;
	    }
	    else { // not equal region ids
							
	      if(boundto[upID] < boundto[backID]) {
		regmap.at<int>(i, j) = boundto[upID];
		boundto[backID] = boundto[upID];
	      }
	      else {
		regmap.at<int>(i, j) = boundto[backID];
		boundto[upID] = boundto[backID];
	      }
	    }
	  }
	  else {
	    // similar only to the top pixel
	    regmap.at<int>(i, j) = regmap.at<int>(i-1, j);
	  }
	}
	else if(backpix) {
	  // similar only to back pixel
	  regmap.at<int>(i, j) = regmap.at<int>(i, j-1);
	}
	else {
	  // not similar to either pixel
	  regmap.at<int>(i, j) = nRegions++;
	}
      }

      backpix = curpix;
    }
  }

  // get out if there's nothing else to do
  if(nRegions == 0) {
#ifdef DEBUG
    printf("No regions\n");
#endif
    return(0);
  }

#ifdef DEBUG
  printf("Fixing IDs\n");
#endif
  // second pass, fix the IDs and calculate the region sizes
  for(i=0;i<regmap.rows;i++) {
    for(j=0;j<regmap.cols;j++) {
      if(regmap.at<int>(i, j) >= 0) {
	regmap.at<int>(i, j) = boundto[regmap.at<int>(i, j)];

	// need to follow the tree in some special cases
	while(boundto[regmap.at<int>(i, j)] != regmap.at<int>(i, j))
	  regmap.at<int>(i, j) = boundto[regmap.at<int>(i, j)];

	regcard[regmap.at<int>(i, j)]++;
      }
    }
  }

#ifdef DEBUG
  printf("Calculating the N largest regions\n");
#endif

  size.create(numLocations, 1, CV_32SC1);

  // grab the N largest ones
  for(i=0;i<MaxLocations;i++) {
    size.at<int>(i, 0) = 0;
    sizeid[i] = -1;
    for(j=0;j<nRegions;j++) {

      // don't consider regions already in the list
      for(k=0;k<i;k++) {
	if(j == sizeid[k])
	  break;
      }
      if(k < i)
	continue;

      if((regcard[j] > sizeThresh) && (regcard[j] > size.at<int>(i, 0))) {
	size.at<int>(i, 0) = regcard[j];
	sizeid[i] = j;
      }
    }
    if(size.at<int>(i, 0) == 0) {
      break;
    }
  }

  numLocations = i;

#ifdef DEBUG
  printf("Calculating centroids for %ld regions\n", numLocations);
  for(i=0;i<numLocations;i++) {
    printf("id %ld size %d\n", sizeid[i], size.at<int>(i, 0));
  }
#endif

  // now calculate centroids and bounding boxes
  bbox.create(numLocations, 4, CV_32SC1);
	
  //centroid.create(numLocations, 2, CV_32SC1);
  //centroid = 0;
  
  centroid = cv::Mat::zeros( numLocations, 2, CV_32SC1);

  for(i=0;i<numLocations;i++) {
    bbox.at<int>(i, 0) = bbox.at<int>(i, 1) = 10000;
    bbox.at<int>(i, 2) = bbox.at<int>(i, 3) = 0;
  }
  
  for(j=0;j<src.rows;j++) {
    for(k=0;k<src.cols;k++) {
      for(i=0;i<numLocations;i++) {
	if(regmap.at<int>(j, k) == sizeid[i]) {
	  centroid.at<int>(i, 0) += j; // rows
	  centroid.at<int>(i, 1) += k; // columns

	  bbox.at<int>(i, 0) = j < bbox.at<int>(i, 0) ? j : bbox.at<int>(i, 0);
	  bbox.at<int>(i, 1) = k < bbox.at<int>(i, 1) ? k : bbox.at<int>(i, 1);
	  bbox.at<int>(i, 2) = j > bbox.at<int>(i, 2) ? j : bbox.at<int>(i, 2);
	  bbox.at<int>(i, 3) = k > bbox.at<int>(i, 3) ? k : bbox.at<int>(i, 3);

	  regmap.at<int>(j, k) = i;
	  break;
	}
      }
      if(i == numLocations) {
	regmap.at<int>(j, k) = -1;
      }
    }
  }
  
  // cv::namedWindow("d", 1);
  // if(bbox.size().height > 0) {
  //   printf("%f %f\n", bbox.at<int>(1, 1));
  //   cv::imshow("d", bbox);
  // }
  // else {
  //   printf(":(\n");
  // }

  //cv::waitKey(0);

  for(i=0;i<numLocations;i++) {
    centroid.at<int>(i, 0) /= size.at<int>(i, 0);
    centroid.at<int>(i, 1) /= size.at<int>(i, 0);
  }

#ifdef DEBUG
  printf("Terminating normally\n");
#endif

  // for(int i = 0; i < (int)regmap.size().height; i++) {
  //   for(int j = 0; j < (int)regmap.size().width; j++) {
  //     if() {
  // 	printf(":))^^^*\n");
  //     }
  //   }
  // }

  free(sizeid);

  return(numLocations);
}

