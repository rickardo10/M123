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

vector<dial> setDiales( meter Meter );

int main( int argc, char *argv[] )
{
   const string obj = "diales.jpg";
   string scn;
   string extension= ".jpg";
   int totalSegmentations =0;
   int badSegmentations = 0;
   int falsePositives = 0;
   int totalDials = 0;
   long mask;
   long kwhR[ 191 ];
   long kwh[ 191 ];
   long averageError = 0;
   long sum = 0;

   //--Reads real readings from a file
   ifstream file("/home/rocampo/Medidores/M123/readings.txt");
   int a, b, c, d, e;
   vector<int> dialR[5];

   int i = 1;
   while( !file.eof() )
   {
     //-- Assigns each reading of the file to a variable
     file >> a >> b >> c >> d >> e;

     dialR[0].push_back( a );
     dialR[1].push_back( b );
     dialR[2].push_back( c );
     dialR[3].push_back( d );
     dialR[4].push_back( e );

     //-- Converts the individual readings into an integer
     mask = a * 10000 + b * 1000 + c * 100 + d * 10 + e;
     kwhR[ i ] = mask;
     i++;
  }

  file.close();

  //--Initializes a timer
  clock_t Start = clock();

   //--Concatenates file's names
  for( int i = 0; i <= 190; i++ )
  {
    stringstream sstm;
    sstm << i << extension;
    scn = sstm.str();

//    if( i == 4 || i == 6 || i == 15 || i == 19 || i == 42 || i == 97 || i == 101 || i == 133 || i == 150 || i == 151
//       || i == 152 || i == 111 || i == 60 )
//    {
//      continue;
//    }

    if( i == 5 || i == 8 || i == 11 /*|| i == 19*/ || i == 22 || i == 29 || i == 31 || i == 36
           || i == 44 || i == 46 || i == 48 || i == 51 || i == 55 || i == 57 || i == 64
           || i == 66 || i == 69 || i == 80 || i == 86 || i == 90 || i == 92 || i == 100
           || i == 109 || i == 110 || i == 115 || i == 117 || i == 120 || i == 125 || i == 128
           || i == 135 || i == 136 || i == 138 || i == 143 || i == 145 || i == 148 || i == 154 || i == 156
           || i == 159 || i == 161 || i == 162 || i == 163 || i == 164 || i == 165 || i == 166 || i == 169
           || i == 170 || i == 171 || i == 173 || i == 174 || i == 176 || i == 177 || i == 179 || i == 181
           || i == 182 || i == 183 || i == 184 || i == 185 || i == 186 || i == 187 || i == 189 || i == 190 )
    {
      //-- If it is one of the above pictures, process it
    }
    else
    {
        continue;
    }

    //--Counts segmentations
    totalSegmentations++;

    //--Creates and initialize a meter
    meter Meter( scn );

    //--Checks if there is any failure and continues if so
    if( Meter.getFailure() )
    {
      puts("");

//      Meter.showSegmentation();
      badSegmentations++;
      continue;
    }

    //--Initializes a vector with the Meter's dials
    vector<dial> dials = setDiales( Meter );

    //--Prints image number
    printf("[Image %d]: Reading ", i );
    //--Prints and compares readings
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

    //-- Converts the individual readings into an integer
    mask = dials[ 0 ].getReading() * 10000 + dials[ 1 ].getReading() * 1000
           + dials[ 2 ].getReading() * 100 + dials[ 3 ].getReading() * 10 + dials[ 4 ].getReading();
    kwh[ i ] = mask;

    averageError = kwhR[ i ] - kwh[ i ];
    cout << " Kwh Error: " << averageError;
    puts("");

    sum += abs( averageError );
//    Meter.showSegmentation();
  }
  totalDials = ( totalSegmentations - badSegmentations ) * 5;

  printf("\n\n----------------------------------------------------------------\n\n");
  cout << "Total Meters: " << totalSegmentations << endl;
  cout << "Total Dials: " << totalDials << endl;
  cout << "Bad Segmentations: " << badSegmentations << endl;
  cout << "False Positives: " << falsePositives << endl;
  cout <<"% False Positives: " << round( ( double ) falsePositives / totalDials * 100 ) << "%" << endl;
  cout <<"% Bad Segmentations: " << round( ( double ) badSegmentations / totalSegmentations * 100 ) << "%" << endl;
  cout <<"Average Error: " << ( double )sum / ( totalSegmentations - badSegmentations ) << " kwh" << endl;
  cout << "Elapsed Time: " << (double)( clock() - Start ) /CLOCKS_PER_SEC << endl;
}

///--Initializes the 5 dials of a Meter
vector<dial> setDiales( meter Meter )
{
  dial Dial4( Meter, 4 );
  dial Dial3( Meter, 3, Dial4.getReading() );
  dial Dial2( Meter, 2, Dial3.getReading() );
  dial Dial1( Meter, 1, Dial2.getReading() );
  dial Dial0( Meter, 0, Dial1.getReading() );

  //--Creates a vector with the 5 dials
  vector<dial> dials;
  dials.push_back( Dial0 );
  dials.push_back( Dial1 );
  dials.push_back( Dial2 );
  dials.push_back( Dial3 );
  dials.push_back( Dial4 );

  return dials;
}
