/*
 *  main.cpp
 *  ImageSaver
 *
 *  Created by Daniel Beard on 6/06/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "tgaImage.h"

int main(int argc, char **argv)
{
	
	//declare image
	short width = 256;
	short height = 256;
	TGAImage *img = new TGAImage(width,height);
	
	//declare a temporary color variable
	Colour c;
	
	//Loop through image and set all pixels to red
	for(int x=0; x<width; x++)
		for(int y=0; y<width; y++)
		{
			c.r = 255;
			c.g = 0;
			c.b = 0;
			c.a = 255;
			img->setPixel(c,x,y);
		}
	
	//write the image to disk
	string filename = "temptest.tga";
	img->WriteImage(filename);
	
	
	return 0;	
}
