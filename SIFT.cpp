/*
Date: 07/01/2013
Creator: Ricardo Antonio Ocampo Vega
Description: Detects the keypoints of an image. After, extracts the
keypoints descriptors. Finally, compares the keypoints of the scene
and the object and find matches.
*/

#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "descriptors.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
  const string obj = "diales.jpg";
  string scn;
  string ftd;
  string desc;
  string mtch;
  int si;
  string extension= ".jpg";

  ftd = "SIFT";
  desc = "SIFT";
  mtch = "BruteForce";

  for( int i = 0; i <= 160; i++ ){
    //--Concatenates file's names
    stringstream sstm;
    sstm << i << extension;
    scn = sstm.str();

    //--Prints image number
    printf("[Image %d] ", i );

    //--Creates and initialize a meter
    descriptors object;

    //--Reads object and scene image's name
    object.setImageObject( obj );
    object.setImageScene( scn );

    //--Reads feature detector tool
    object.setFeatureDetector( ftd );

    //--Reads find descriptor tool
    object.setFindDescriptors( desc );

    //--Reads matcher tool
    object.setMatcher( mtch );

    //--Process available data in order to returns dials
    object.featureDetector();
    puts("");
  }
}

