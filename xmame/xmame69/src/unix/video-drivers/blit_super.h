// Generic function to perform blits in any rotation
// This blitter does no scaling
void MY_FUNCTION (struct mame_bitmap *bitmap)
{
    SRC_PIXEL *src = SRC;
    DEST_PIXEL *dst = DEST;
    unsigned int src_inc_line = SRC_INC_LINE;
    unsigned int dst_inc_line = DEST_INC_LINE;
    register unsigned int src_inc = SRC_INC;

    unsigned int h;
    register unsigned int w;

    if (INDIRECT)
    {
	for (h = 0; h < SRC_HEIGHT; h++)
	{
	    register SRC_PIXEL *srcX = src;
	    register DEST_PIXEL *dstX = dst;
	    for (w = 0; w < (SRC_WIDTH>>3); w++)
	    {
		dstX[0] = INDIRECT[srcX[0]];
		dstX[1] = INDIRECT[srcX[1]];
		dstX[2] = INDIRECT[srcX[2]];
		dstX[3] = INDIRECT[srcX[3]];
		dstX[4] = INDIRECT[srcX[4]];
		dstX[5] = INDIRECT[srcX[5]];
		dstX[6] = INDIRECT[srcX[6]];
		dstX[7] = INDIRECT[srcX[7]];
		srcX += 8;
		dstX += 8;
	    }
	    src += src_inc_line;
	    dst += dst_inc_line;
	}
    }
    else
    {
	for (h = 0; h < SRC_HEIGHT; h++)
	{
	    register SRC_PIXEL *srcX = src;
	    register DEST_PIXEL *dstX = dst;
	    for (w = 0; w < (SRC_WIDTH>>3); w++)
	    {
		dstX[0] = srcX[0];
		dstX[1] = srcX[1];
		dstX[2] = srcX[2];
		dstX[3] = srcX[3];
		dstX[4] = srcX[4];
		dstX[5] = srcX[5];
		dstX[6] = srcX[6];
		dstX[7] = srcX[7];
		srcX += 8;
		dstX += 8;
	    }
	    src += src_inc_line;
	    dst += dst_inc_line;
	}
    }
}
