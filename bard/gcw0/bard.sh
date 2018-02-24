#!/bin/sh
###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                         Copyright (c) 2014                            ##
##                        All Rights Reserved.                           ##
##                                                                       ##
##  Permission is hereby granted, free of charge, to use and distribute  ##
##  this software and its documentation without restriction, including   ##
##  without limitation the rights to use, copy, modify, merge, publish,  ##
##  distribute, sublicense, and/or sell copies of this work, and to      ##
##  permit persons to whom this work is furnished to do so, subject to   ##
##  the following conditions:                                            ##
##   1. The code must retain the above copyright notice, this list of    ##
##      conditions and the following disclaimer.                         ##
##   2. Any modifications must be clearly marked as such.                ##
##   3. Original authors' names are not deleted.                         ##
##   4. The authors' names are not used to endorse or promote products   ##
##      derived from this software without specific prior written        ##
##      permission.                                                      ##
##                                                                       ##
##  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         ##
##  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ##
##  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ##
##  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      ##
##  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ##
##  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ##
##  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ##
##  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ##
##  THIS SOFTWARE.                                                       ##
##                                                                       ##
###########################################################################
##                                                                       ##
##  Startup script for Bard on GCW-Zero, basically makes a sane          ##
##  .bard_config for that platform if there isn't one already            ##
##                                                                       ##
###########################################################################

BARDHOME=/mnt/Bard

BARD_FONT="-font $BARDHOME/LiberationSerif-Regular.ttf"

BARD_ARGS=
if [ ! -f $HOME/.bard_config ]
then
   # No existing bard_config file so start up, setting reasonable
   # starting values for the GCW0, and reading the Bard_Help file
   BARD_VOICES="-voices_dir $BARDHOME/voices"
   # Enough to display basic use commands on first startup
   BARD_FSIZE="-font_size 20"
   BARD_SDELAY="-scroll_delay 45"
   BARD_HTEXT="-text Bard_Help.html"
   BARD_VOICE="-voice $BARDHOME/voices/cmu_us_slt.flitevox"
#   BARD_SBUFFSIZE="-audio_stream_buffer_factor 10"
   BARD_ARGS="$BARD_VOICES $BARD_FSIZE $BARD_SDELAY $BARD_HTEXT $BARD_VOICE $BARD_BLANK $BARD_SBUFFSIZE"
fi

cd $BARDHOME
exec ./bard $BARD_ARGS $BARD_FONT $*


