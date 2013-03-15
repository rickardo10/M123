/*
Date: 07/01/2013
Creator: Ricardo Antonio Ocampo Vega
Description: Detects the keypoints of an image. After, extracts the
keypoints descriptors. Finally, compares the keypoints of the scene
and the object and find matches.
*/

#include <fstream>
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
#include <math.h>
#include <ctime>

using namespace std;
using namespace cv;

int main( int argc, char *argv[] )
{
   const string obj = "diales.jpg";
   string scn;
   string extension= ".jpg";
   int totalSegmentations =0;
   int badSegmentations = 0;
   int falsePositives = 0;
   int totalDials = 0;

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

  //--Initializes a timer
  clock_t Start = clock();

   //--Concatenates file's names
  for( int i = 1; i <= 157; i++ ){
    stringstream sstm;
    sstm << i << extension;
    scn = sstm.str();

    if( i == 4 || i == 6 || i == 15 || i == 101 || i == 133 || i == 150 || i == 151 || i == 152 || i == 111 || i == 60 ){
      continue;
    }

    //--Counts segmentations
    totalSegmentations++;

    //--Prints image number
    printf("[Image %d] ", i );

    //--Creates and initialize a meter
    meter Meter( scn );

    //--Checks if there is any failure and continues if so
    if( Meter.getFailure() )
    {
      puts("");
//     Meter.showSegmentation();
      badSegmentations++;
      continue;
    }

    //--Creates a vector with the 5 dials
    vector<dial> dials;

    printf("Reading: ");
    for( int j = 4; j >= 0 ; j-- ){
      if( j != 4 ){
         dial Dial( Meter, j, dials[ j + 1 ].getReading() );
         dials.push_back( Dial );
         //--Counts readings
         totalDials++;
      }
      else {
         dial Dial( Meter, j );
         dials.push_back( Dial );
         //--Counts readings
         totalDials++;
      }
    }

    for( int j = 0; j < 5; j++ ){
      cout << dials[ j ].getReading() << " ";
      if( dials[ j ].getReading() == dialR[ j ].at( i - 1 ) ){
        cout << "true" << " ";
      }
      else{
        cout << "false" << " ";
        //--Counts false positive readings
       falsePositives++;
      }
    }


    puts("");
    Meter.showSegmentation();
  }

  printf("\n\n----------------------------------------------------------------\n\n");
  cout << "Total Meters: " << totalSegmentations << endl;
  cout << "Total Dials: " << totalDials << endl;
  cout << "Bad Segmentations: " << badSegmentations << endl;
  cout << "False Positives: " << falsePositives << endl;
  cout <<"% False Positives: " << round( ( double ) falsePositives / totalDials * 100 ) << "%" << endl;
  cout <<"% Bad Segmentations: " << round( ( double ) badSegmentations / totalSegmentations * 100 ) << "%" << endl;
  cout << "Elapsed Time: " << (double)( clock() - Start ) /CLOCKS_PER_SEC << endl;
}
