#!/bin/sh
mypath=${0%/*}
export HOME=$mypath
export SDL_QT_INVERT_ROTATION=1
cd $mypath
sleep 1
exec $mypath/quake_r2-tec.dge

