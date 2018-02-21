#define SCALE_X_8(X) ((X)<<3)
#define SCALE_Y_8(Y) ((Y)<<3)

#define COPY_LINE(SRC, END, DST) clut8to16(DST, SRC, INDIRECT, ((END)-(SRC)));
#include "blit_normal_8_to_16.h"
#undef COPY_LINE

#undef SCALE_x_8
#undef SCALE_Y_8
