/*
 * Reprinted courtesy Dr. Dobb's Journal, (C) 1995. 
 */
/* Test program for reading bitmap files.  It accepts an input file and an
 * output file on the command line.  It will read and process the input file
 * and dump an ASCII representation of the contents to the output file.  The
 * dump will consist of the color image and two masks.  Missing parts will be
 * indicated as such (BMP files have no masks and nonochrome ICO/PTR files
 * have no color data.  In the color image, the dump will be a series of RGB
 * values (in hexadecimal).  In the masks, the dump will be represented by "."
 * symbols representing zeros and "@" symbols representing ones.  
 */

#include <stdio.h>
#include <stdlib.h>
#include "bmptypes.h"
#include "endian.h"
#include "readbmp.h"

int main (int argc, char *argv[])
{
    FILE *fp;
    RGB **argbs;
    char **xorMasks, **andMasks;
    UINT32 *heights, *widths, row, col;
    UINT16 fileType;
    long filePos;
    int numImages, i;
    int rc;
    
    if (argc < 3)
    {
	printf ("usage: test <infile> <outfile>\n");
	return 1;
    }
    
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
	perror ("Error opening source file");
	return 2;
    }

    /*
     * Read the first two bytes as little-endian to determine the file type.
     * Preserve the file position.
     */
    filePos = ftell(fp);
    rc = readUINT16little(fp, &fileType);
    if (rc != 0)
    {
	perror("Error getting file type");
	return 3;
    }
    fseek(fp, filePos, SEEK_SET);

    /*
     * Read the images.
     */
    switch (fileType) {
    case TYPE_ARRAY:
	/*
	 * If this is an array of images, read them.  All the arrays we need
	 * will be allocated by the reader function.
	 */
	rc = readMultipleImage(fp, &argbs, &xorMasks, &andMasks, &heights,
			       &widths, &numImages);
	break;
    case TYPE_BMP:
    case TYPE_ICO:
    case TYPE_ICO_COLOR:
    case TYPE_PTR:
    case TYPE_PTR_COLOR:
	/*
	 * If this is a single-image file, we've a little more work.  In order
	 * to make the output part of this test program easy to write, we're
	 * going to allocate dummy arrays that represent what
	 * readMultipleImage would have allocated.  We'll read the data into
	 * those arrays.
	 */
	argbs = (RGB **)calloc(1, sizeof(RGB *));
	if (argbs == NULL)
	{
	    rc = 1005;
	    break;
	}
	xorMasks = (char **)calloc(1, sizeof(char *));
	if (xorMasks == NULL)
	{
	    free(argbs);
	    rc = 1005;
	    break;
	}
	andMasks = (char **)calloc(1, sizeof(char *));
	if (andMasks == NULL)
	{
	    free(argbs);
	    free(xorMasks);
	    rc = 1005;
	    break;
	}
	heights = (UINT32 *)calloc(1, sizeof(UINT32));
	if (heights == NULL)
	{
	    free(argbs);
	    free(xorMasks);
	    free(andMasks);
	    rc = 1005;
	    break;
	}
	widths = (UINT32 *)calloc(1, sizeof(UINT32));
	if (widths == NULL)
	{
	    free(argbs);
	    free(xorMasks);
	    free(andMasks);
	    free(heights);
	    rc = 1005;
	    break;
	}
	numImages = 1;

	/*
	 * Now that we have our arrays allocted, read the image into them.
	 */
	switch (fileType) {
	case TYPE_BMP:
	    rc = readSingleImageBMP(fp, argbs, widths, heights);
	    break;
	case TYPE_ICO:
	case TYPE_PTR:
	    rc = readSingleImageICOPTR(fp, xorMasks, andMasks, widths,
				       heights);
	    break;
	case TYPE_ICO_COLOR:
	case TYPE_PTR_COLOR:
	    rc = readSingleImageColorICOPTR(fp, argbs, xorMasks, andMasks,
					    widths, heights);
	    break;
	}
	break;
    default:
	rc = 1000;
    }
  
    /*
     * At this point, everything's been read.  Display status messages based
     * on the return values.
     */
    switch (rc) {
    case 1000:
    case 1006:
	printf ("File is not a valid bitmap file\n");
	break;
    case 1001:
	printf ("Illegal information in an image\n");
	break;
    case 1002:
	printf ("Legal information that I can't handle yet in an image\n");
	break;
    case 1003:
    case 1004:
    case 1005:
	printf ("Ran out of memory\n");
	break;
    case 0:
	printf ("Got good data from file, writing results\n");
	break;
    default:
	printf ("Error reading file rc=%d\n", rc);
	perror ("Errno:");
	break;
    }

    /*
     * If the return value wasn't 0, something went wrong.
     */
    if (rc != 0)
    {
	if (rc != 1000 && rc != 1005)
	{
	    for (i=0; i<numImages; i++)
	    {
		if (argbs[i] != NULL)
		    free(argbs[i]);
		if (andMasks[i] != NULL)
		    free(andMasks[i]);
		if (xorMasks[i] != NULL)
		    free(xorMasks[i]);
	    }
	    free(argbs);
	    free(andMasks);
	    free(xorMasks);
	    free(widths);
	    free(heights);
	}
	return rc;
    }
    
    fclose(fp);
    fp = fopen(argv[2], "wt");
    if (fp == NULL)
    {
	perror ("Error opening target file");
	return 3;
    }

    /*
     * Dump the images.
     */
    fprintf (fp, "There are %d images in the file\n", numImages);

    for (i=0; i<numImages; i++)
    {
	/*
	 * Loop through all the images that were returned.
	 */
	fprintf (fp, "Doing image number %d\n\n", i+1);
	fprintf (fp, "Image dimensions: (%ld,%ld)\n", widths[i], heights[i]);
	
	if (argbs[i] != NULL)
	{
	    /*
	     * If the image has colors, dump them (BMP, color ICO and color
	     * PTR files
	     */
	    fprintf(fp, "Colors");
	    for (row = 0; row < heights[i]; row++)
	    {
		fprintf (fp, "\n\nRow %ld pixels (R,G,B), hex values:\n",
			 row);
		for (col = 0; col < widths[i]; col++)
		{
		    fprintf (fp, "(%2.2x,%2.2x,%2.2x)",
			     argbs[i][row * widths[i] + col].red,
			     argbs[i][row * widths[i] + col].green,
			     argbs[i][row * widths[i] + col].blue);
		}
	    }
	}
	else
	{
	    /*
	     * If there image has no colors, say so.  (monochrome ICO and PTR
	     * files) 
	     */
	    fprintf (fp, "No color image\n");
	}

	if (xorMasks[i] != NULL)
	{
	    /*
	     * If the image has an xor mask, dump it.  (ICO and PTR files)
	     */
	    fprintf (fp, "\nXOR mask\n");
	    for (row = 0; row < heights[i]; row++)
	    {
		for (col = 0; col < widths[i]; col++)
		{
		    fprintf (fp, "%c",
			     xorMasks[i][row * widths[i] + col] ? '@' : '.');
		}
		fprintf (fp, "\n");
	    }
	}
	else
	{
	    /*
	     * If the image has no xor mask, say so.  (BMP files).
	     */
	    fprintf (fp, "No xor mask\n");
	}

	if (andMasks[i] != NULL)
	{
	    /*
	     * If the image has an and mask, dump it.  (ICO and PTR files)
	     */
	    fprintf (fp, "\nAND mask\n");
	    for (row = 0; row < heights[i]; row++)
	    {
		for (col = 0; col < widths[i]; col++)
		{
		    fprintf (fp, "%c",
			     andMasks[i][row * widths[i] + col] ? '@' : '.');
		}
		fprintf (fp, "\n");
	    }
	}
	else
	{
	    /*
	     * If the image has noand mask, say so.  (BMP files)
	     */
	    fprintf (fp, "No and mask\n");
	}

	if (i != numImages-1)
	    fprintf (fp, "\n------------------------------------------\n\n");
	
    }
    fclose(fp);
    
    /*
     * Dumping is complete.  Free all the arrays and quit
     */
    for (i=0; i<numImages; i++)
    {
	if (argbs[i] != NULL)
	    free(argbs[i]);
	if (andMasks[i] != NULL)
	    free(andMasks[i]);
	if (xorMasks[i] != NULL)
	    free(xorMasks[i]);
    }
    free(argbs);
    free(andMasks);
    free(xorMasks);
    free(widths);
    free(heights);
    
    return 0;
}

/*
 * Formatting information for emacs in c-mode
 *
 * Local Variables:
 * c-indent-level:4
 * c-continued-statement-offset:4
 * c-brace-offset:-4
 * c-brace-imaginary-offset:0
 * c-argdecl-indent:4
 * c-label-offset:-4
 * End:
 */

