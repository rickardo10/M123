/*
Date: 07/01/2013
Creator: Ricardo Antonio Ocampo Vega
Description: Detects the keypoints of an image. After, extracts the
keypoints descriptors. Finally, compares the keypoints of the scene
and the object and find matches.
*/

#include<fstream>
#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "meter.h"
#include "dial.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
   const string obj = "diales.jpg";
   string scn;
   string extension= ".jpg";

   //--Reads real readings from a file
   ifstream file("/home/rocampo/Medidores/M123/readings.txt");
   int a, b, c, d, e;
   vector<int> dialR[5];
   while( !file.eof() )
   {
     file >> a >> b >> c >> d >> e;

     dialR[0].push_back( a );
     dialR[1].push_back( b );
     dialR[2].push_back( c );
     dialR[3].push_back( d );
     dialR[4].push_back( e );
  }

  file.close();

  //--Creates file names
  for( int i = 0; i <= 160; i++ ){
    //--Concatenates file's names
    stringstream sstm;
    sstm << i << extension;
    scn = sstm.str();

    if( i == 150 || i == 151 ){
      continue;
    }

    //--Prints image number
    printf("[Image %d] ", i );

    //--Creates and initialize a meter
    meter Meter( scn );

    //--Checks if there is any failure and continues if so
    if( Meter.getFailure() )
    {
      puts("");
//      Meter.showSegmentation();
      continue;
    }

    //--Creates a vector with the 5 dials
    vector<dial> dials;

    printf("Reading: ");
    for( int j = 0; j < 5; j++ ){
      dial Dial( Meter, j );
      dials.push_back( Dial );

      if( Dial.getReading() == dialR[j].at(i) ){
        cout << "true" << " ";
      }
      else{
        cout << "false" << " ";
      }

    }
    puts("");
//    Meter.showSegmentation();
  }
}

