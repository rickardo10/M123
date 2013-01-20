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
#include "matcher.h"

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

//  //--Iniciaci�n de valores iniciales
//  printf("Escriba el nombre de la escena:\n? ");
//  cin >> scn;
//  printf("\nEscriba el m�todo de Feature Detector:\nHINT: Algunos m�todos son FAST, SIFT, SURF, ORB, MSER, GFTT,\nHARRIS, Dense, SimpleBlob\n? ");
//  cin >> ftd;
//  printf("\nEscriba el m�todo de Descritor Extractor:\nHINT: Algunos m�todos son SIFT, SURF, ORB, BRIEF, FREAK\n? ");
//  cin >> desc;
//  printf("\nEscriba el m�todo de Match\nHINT:Algunos m�todos son BruteForce, BruteForce-L1, BruteForce-Hamming,\nBruteForce-Hamming(2), FlannBased\n? ");
//  cin >> mtch;
  ftd = "SIFT";
  desc = "SIFT";
  mtch = "BruteForce";

  float desv;
  cout << endl << "Escriba un valor para la desviaci�n:\n? ";
  cin >> desv;

  for( int i = 0; i <= 158; i++ ){
    //--Obtenci�n de descriptores del objeto
    stringstream sstm;
    sstm << i << extension;
    scn = sstm.str();

    if( imread(scn).empty() ){
        puts("Can't read the image");
        continue;
    }

    descriptors object;
    object.setImage(obj);
    object.featureDetector(ftd);

  //  cout << "Desea ver la imagen con los puntos clave?\n1. S�\n2. No\n? ";
  //  cin >> si;
  //
  //  if( si == 1 ){
  //    object.writeKeypoints();
  //  }

    object.findDescriptors(desc);

    //--Obtenci�n de descriptores de la escena
    descriptors scene;
    scene.setImage(scn);
    scene.featureDetector(ftd);

  //  cout << "Desea ver la imagen con los puntos clave?\n1. S�\n2. No\n? ";
  //  cin >> si;
  //
  //  if( si == 1 ){
  //    scene.writeKeypoints();
  //  }

    scene.findDescriptors(desc);

    //--Detecci�n de similitudes
    matcher match1;
    match1.setInitialData( object.getKeypoints(), scene.getKeypoints(), object.getDescriptors(), scene.getDescriptors());
    match1.match(mtch, obj, scn, desv);
  }
}
