/*
   AlphaMask Maker 

   Ryan Geary


   This program implements a limited color palette on an image.  An image
   will be taken in as an input and, using a text file, each pixel's color
   in the image will be carefully selected and replaced in order for the image
   to fit within the desired limited color palette.
*/

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <OpenImageIO/imageio.h>
#include <fstream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

OIIO_NAMESPACE_USING

static int icolor = 0;
int w,h,c;
unsigned char* pixels;
unsigned char* colormask;

char* f;
ImageInput *in;
ImageOutput *out;
ImageSpec spec;

//A linked list to represent the list of colors in the palette
struct palette{
   int r;
   int b;
   int g;
   struct palette *next;
};
struct palette *curr;
struct palette *head;

//Read the image information with the given input and image specs.
void fileRead(ImageInput *in, ImageSpec spec){
  w = spec.width;
  h = spec.height;
  c = spec.nchannels;
  int stride = 4;
  int scanlinesize = w * c * sizeof(pixels[0]);

  pixels = new unsigned char[w*h*c];
  in->read_image (TypeDesc::UINT8,(char *)pixels+(h-1)*scanlinesize, c,-scanlinesize,c);
  in->close();
  delete in;
}

//Write the image out to a designated file.  The new colors are calculated
//and the changes will be taken into account for the written image.
void fileWrite(ImageOutput *out, ImageSpec spec, char *f, int gscaleMap){
   int currvalue;
   float diffRed, diffGreen, diffBlue;
   float cdistance, tdistance;
   int currentselection = 256;
   int red, green, blue;
   colormask = new unsigned char[w*h*4];

    for(int i = 0; i < w*h*c; i+=c){ 
    cdistance = 100000.0;
       //Traverse through each color and determine which one should be 
       //selected.
       while(curr) { 
         diffRed = (curr->r)-pixels[i];
         diffGreen = (curr->g)-pixels[i+1];
         diffBlue = (curr->b)-pixels[i+2];
         tdistance = sqrt(diffRed*diffRed+diffGreen*diffGreen+diffBlue*diffBlue);

         //Compare current color being reviewed in the palette and the color
         //in the selected pixel.
         if(tdistance < cdistance){
         cdistance = tdistance;
         colormask[i] = curr->r;
         colormask[i+1] = curr->g;
         colormask[i+2] = curr->b;
         }
         
         curr = curr->next ;
       }
       curr = head;
    }

    int scanlinesize = w * 3 * sizeof(colormask[0]);
    out->open(f, spec);
    out->write_image(TypeDesc::UINT8, (char *)colormask+(h-1)*scanlinesize, AutoStride, -scanlinesize, AutoStride);
    out->close();
    delete out;
}

//Read in the limited color palette from the file.
void paletteRead(FILE *f){
   int red, green, blue;
   char d;
   while(fscanf(f, "%d", &red) != EOF){
      fscanf(f, "%d", &green);
      fscanf(f, "%d", &blue);
      curr = (palette *)malloc(sizeof(palette));
      curr->r = red;
      curr->g = green;
      curr->b = blue;
    
      curr->next  = head;
      head = curr;      
   }
   curr = head;
  printf("\nColor palette selected: \n");

  //Print & traverse the list of colors read in.
  while(curr) {
      printf("%d %d %d\n", curr->r, curr->g, curr->b);
      curr = curr->next ;
   }
}

int main(int argc, char* argv[]){
  int gscaleMap = 255;
  if(argv[3] == NULL){
     std::cerr<<"Please enter valid palette file. \n"<<geterror();
     exit(1);
  }
  FILE *lpalette;
  lpalette = fopen(argv[3],"r");
  paletteRead(lpalette);

  
  in = ImageInput::open(argv[1]);
   if(!in){
     std::cerr<<"File not valid. "<<geterror();
     exit(1);
   }
  
  if(argv[2] != NULL){
    f = argv[2];
  
    out = ImageOutput::create(f);
    if (!out) {
      std::cerr << "Could not create: " << geterror();
      exit(-1);
    }
  }

  spec = in->spec();
  fileRead(in, spec);
  fileWrite(out, spec, f, gscaleMap);
}
