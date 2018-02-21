#define VIDEO_COLOR15(R,G,B) ((((R)&0xF8)<<8)|(((G)&0xFC)<<3)|(((B)&0xF8)>>3))
#define VIDEO_GETR15(C) (((C)>>8)&0xF8)
#define VIDEO_GETG15(C) (((C)>>3)&0xFC)
#define VIDEO_GETB15(C) (((C)<<3)&0xF8)
inline DEST_PIXEL mix_color16 (SRC_PIXEL color1, SRC_PIXEL color2)
{
        return VIDEO_COLOR15((VIDEO_GETR15(color1)+VIDEO_GETR15(color2))>>1,(VIDEO_GETG15(color1)+VIDEO_GETG15(color2))>>1,(VIDEO_GETB15(color1)+VIDEO_GETB15(color2))>>1);
}

void sdl_update_16_to_16bpp_horzscale(struct osd_bitmap *bitmap)
{
    DEST_PIXEL *buffer_scr = DEST;
    SRC_PIXEL *buffer_mem = (SRC_PIXEL *)bitmap->line[visual.min_y] + visual.min_x;
    int buffer_mem_offset = ((bitmap->line[1] - bitmap->line[0])>>1)-SRC_WIDTH;
    int src_width = (bitmap->line[1] - bitmap->line[0]);
    int xstep,ystep,i,j;
    int x,y=SRC_HEIGHT;

    xstep=DEST_WIDTH/(SRC_WIDTH-DEST_WIDTH);

    if (SRC_HEIGHT>DEST_HEIGHT)
	ystep=DEST_HEIGHT/(SRC_HEIGHT-DEST_HEIGHT)+1;
    else
	ystep=SRC_HEIGHT;

    j=ystep;

    if(INDIRECT)
    {
	do {
	    x=DEST_WIDTH; i=xstep;
	    do {
		*buffer_scr++=INDIRECT[*buffer_mem++];
		x--; i--;
		if (!i) { if (x) { *buffer_scr++=mix_color16(INDIRECT[*buffer_mem++],INDIRECT[*buffer_mem++]); x--; i=xstep-1; } else { buffer_mem++; i=xstep; } }
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
    else
    {
	do {
	    x=DEST_WIDTH; i=xstep;
	    do {
		*buffer_scr++=*buffer_mem++;
		x--; i--;
		if (!i) { if (x) { *buffer_scr++=mix_color16(*buffer_mem++,*buffer_mem++); x--; i=xstep-1; } else { buffer_mem++; i=xstep; } }
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
