#define DEST_SCALE_X_8(X) ((X)<<3)
#define DEST_SCALE_Y_8(Y) ((Y)<<3)

#ifdef INDIRECT
    {
       if (!use_dirty)
	   {
		  /* normal non dirty */
		  int src_width = (((SRC_PIXEL *)bitmap->line[1]) - ((SRC_PIXEL *)bitmap->line[0]));
		  SRC_PIXEL *line_src = (SRC_PIXEL *)bitmap->line[visual.min_y]   + visual.min_x;
		  SRC_PIXEL *line_end = (SRC_PIXEL *)bitmap->line[visual.max_y+1] + visual.min_x;
		  DEST_PIXEL *line_dest = (DEST_PIXEL *)(DEST);
		  
		  for (;line_src < line_end; line_dest+=DEST_WIDTH, line_src+=src_width)
			 COPY_LINE(line_src, line_src+visual_width, line_dest)
	   }
       else
	   {
		  /* normal dirty */
		  int y, max_y = (visual.max_y+1) >> 3;
		  DEST_PIXEL *line_dest = (DEST_PIXEL *)(DEST) - visual.min_x;
		  for (y=visual.min_y>>3; y<max_y; y++, line_dest+=DEST_SCALE_Y_8(DEST_WIDTH))
		  {
			 if (dirty_lines[y])
			 {
			int x, max_x;
			max_x = (visual.max_x+1) >> 3;
			for(x=visual.min_x>>3; x<max_x; x++)
			{
			   if (dirty_blocks[y][x])
			   {
				  int min_x;
				  int max_x, h, max_h;
				  DEST_PIXEL *block_dest = line_dest + DEST_SCALE_X_8(x);
				  min_x = x << 3;
				  do {
				 dirty_blocks[y][x]=0;
				 x++;
				  } while (dirty_blocks[y][x]);
				  max_x = x << 3;
				  h     = y << 3;
				  max_h = h + 8;
				  for (; h<max_h; h++, block_dest += DEST_WIDTH)
				 COPY_LINE((SRC_PIXEL *)bitmap->line[h]+min_x,
					(SRC_PIXEL *)bitmap->line[h]+max_x, block_dest)
			   }
			}
			dirty_lines[y] = 0;
			 }
		  }
	   }
    }
#else
    {
       if (!use_dirty)
	   {
		  /* normal non dirty */
		  int src_width = (((SRC_PIXEL *)bitmap->line[1]) - ((SRC_PIXEL *)bitmap->line[0]));
		  SRC_PIXEL *line_src = (SRC_PIXEL *)bitmap->line[visual.min_y]   + visual.min_x;
		  SRC_PIXEL *line_end = (SRC_PIXEL *)bitmap->line[visual.max_y+1] + visual.min_x;
		  DEST_PIXEL *line_dest = (DEST_PIXEL *)(DEST);
		  
		  for (;line_src < line_end; line_dest+=DEST_WIDTH, line_src+=src_width)
			 MEMCPY(line_src, line_src+visual_width, line_dest)
	   }
       else
	   {
		  /* normal dirty */
		  int y, max_y = (visual.max_y+1) >> 3;
		  DEST_PIXEL *line_dest = (DEST_PIXEL *)(DEST) - visual.min_x;
		  for (y=visual.min_y>>3; y<max_y; y++, line_dest+=DEST_SCALE_Y_8(DEST_WIDTH))
		  {
			 if (dirty_lines[y])
			 {
			int x, max_x;
			max_x = (visual.max_x+1) >> 3;
			for(x=visual.min_x>>3; x<max_x; x++)
			{
			   if (dirty_blocks[y][x])
			   {
				  int min_x;
				  int max_x, h, max_h;
				  DEST_PIXEL *block_dest = line_dest + DEST_SCALE_X_8(x);
				  min_x = x << 3;
				  do {
				 dirty_blocks[y][x]=0;
				 x++;
				  } while (dirty_blocks[y][x]);
				  max_x = x << 3;
				  h     = y << 3;
				  max_h = h + 8;
				  for (; h<max_h; h++, block_dest += DEST_WIDTH)
					 MEMCPY((SRC_PIXEL *)bitmap->line[h]+min_x, (SRC_PIXEL *)bitmap->line[h]+max_x, block_dest)
			   }
			}
			dirty_lines[y] = 0;
			 }
		  }
	   }
    }
#endif

#undef DEST_SCALE_X_8
#undef DEST_SCALE_Y_8
#undef COPY_LINE
