void sdl_update_8_to_16bpp_horzscale(struct osd_bitmap *bitmap)
{
    DEST_PIXEL *buffer_scr = DEST;
    SRC_PIXEL *buffer_mem = (SRC_PIXEL *)bitmap->line[visual.min_y] + visual.min_x;
    int buffer_mem_offset = (bitmap->line[1] - bitmap->line[0])-SRC_WIDTH;
    int src_width = (bitmap->line[1] - bitmap->line[0]);
    int xstep,ystep,i,j;
    int x,y=SRC_HEIGHT;

    xstep=DEST_WIDTH/(SRC_WIDTH-DEST_WIDTH);

    if (SRC_HEIGHT>DEST_HEIGHT)
	ystep=DEST_HEIGHT/(SRC_HEIGHT-DEST_HEIGHT)+1;
    else
	ystep=SRC_HEIGHT;

    j=ystep;

    {
	do {
	    x=DEST_WIDTH; i=xstep;
	    do {
		*buffer_scr++=INDIRECT[*buffer_mem++];
		x--; i--;
		if (!i) { buffer_mem++; i=xstep; }
	    } while (x);
	    buffer_mem+=buffer_mem_offset;
	    y--;
	    j--;
	    if (!j && y)
	    {
		/* Skip a source line */
		j=ystep;
		y--;
		buffer_mem+=src_width;
	    }
	} while (y);
    }
}
#undef VIDEO_COLOR15
#undef VIDEO_GETR15
#undef VIDEO_GETG15
#undef VIDEO_GETB15
