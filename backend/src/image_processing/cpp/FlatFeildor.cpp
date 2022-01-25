#include "../header/FlatFeildor.h"

void FlatFeildor::execute(CallBackFunction func, btrgb::ArtObject* images) {
    btrgb::image* art1;
    btrgb::image* art2;
    btrgb::image* white1;
    btrgb::image* white2;
    btrgb::image* dark1;
    btrgb::image* dark2;
    func("Flat Fielding");
    try {
        art1 = images->getImage("art1");
        white1 = images->getImage("white1");;
        dark1 = images->getImage("dark1");;
        art2 = images->getImage("art2");;
        white2 = images->getImage("white2");;
        dark2 = images->getImage("dark2");;
    }
    catch (const btrgb::ArtObj_ImageDoesNotExist& e) {
        func("Error: Flatfielding called out of order. Missing at least 1 image assignment.");
        return;
    }
    //Set up of the size of all the bitmaps for all the different images being looked at
    int height = art1->height();
    int width = art1->width();
    int channels = art1->channels();
    btrgb::pixel* abitmap1 = art1->bitmap();
    btrgb::pixel* abitmap2 = art2->bitmap();
    btrgb::pixel* wbitmap1 = white1->bitmap();
    btrgb::pixel* wbitmap2 = white2->bitmap();
    btrgb::pixel* dbitmap1 = dark1->bitmap();
    btrgb::pixel* dbitmap2 = dark2->bitmap();

    //ONLY NEED CHANNEL 2, aka Green channel, to get averages, y is from calc    
    //Provided from Art Obj
    //Determines how many pixels are compared, 
    //size - 1 = how many rings around the center point to be compared for avg
    //Current default to 3 can be adjusted if needed
    int size = 3;
    //Other data from Art Obj for Target
    int topLeft = images->getTargetInfo("tl");
    int topRight = images->getTargetInfo("tr");
    int botRight = images->getTargetInfo("br");
    int botLeft = images->getTargetInfo("bl");
    int rows = images->getTargetInfo("row");
    int cols = images->getTargetInfo("col");
    //Above will be used with Patch info from singleton to find the pixel that is the center
    //of the white patch.  For now pretend the center pixel is the patchX and patchY

    //Provided from Singleton
    int patchX;
    int patchY;

    float art1Total = 0;
    float white1Total = 0;
    float art2Total = 0;
    float white2Total = 0;
    int x, y, i, ix, iy;
    //May need to be looked at, old implementation of bitmap for loop
    for (y = 0; y < height; y++) {
        iy = y * width * channels;
        for (x = 0; x < width; x++) {
            ix = x * channels;
            //channel will always be index 1, only channel 2 aka green
            i = iy + ix + 1;
            //Need to find out if this is referneced correctly, is ix the x position
            if (x == patchX || x == patchX - (size - 1) || x == patchX + (size - 1)) {
                if (y == patchY || y == patchY - (size - 1) || y == patchY + (size - 1)) {
                    art1Total += abitmap1[i];
                    white1Total += wbitmap1[i];
                    art2Total += abitmap2[i];
                    white2Total += wbitmap2[i];
                }
            }
        }        
    }
    float art1Avg = art1Total / (size * size);
    float white1Avg = white1Total / (size * size);
    float art2Avg = art2Total / (size * size);
    float white2Avg = white2Total / (size * size);
    //Y provided by Singleton
    float yVal;
    float w1 = yVal * (white1Avg / art1Avg);
    float w2 = yVal * (white2Avg / art2Avg);
    //For loop is for every pixel in the image, and gets a corrisponding pixel from white and dark images
    //Every Channel value for each pixel needs to be adjusted
    //Old version of the For Loop, may need to be changed
    int ch;
    float wPix, dPix, aPix;
    for (y = 0; y < height; y++) {
        iy = y * width * channels;
        for (x = 0; x < width; x++) {
            ix = x * channels;
            for (ch = 0; ch < channels; ch++) {
                i = iy + ix + ch;
                // i is the index for the bitmap
                wPix = wbitmap1[i];
                dPix = dbitmap1[i];
                aPix = abitmap1[i];
                //Need to overwrite previous image pixel in the Art Object
                abitmap1[i] = w1 * ((aPix - dPix) / (wPix - dPix));
                //Repeat for image 2
                wPix = wbitmap2[i];
                dPix = dbitmap2[i];
                aPix = abitmap2[i];
                abitmap2[i] = w2 * ((aPix - dPix) / (wPix - dPix));
            }
        }
    }
    sleep_for(seconds(1));
}