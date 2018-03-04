// $Id: scale2x.vert 11025 2009-12-26 13:47:09Z mthuurne $

uniform vec2 texSize;

varying vec2 texStep; // could be uniform
varying vec2 coord2pi;
varying vec2 videoCoord;

float pi = 4.0 * atan(1.0);
float pi2 = 2.0 * pi;

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	coord2pi = gl_MultiTexCoord0.st * texSize * pi2;
	texStep = 1.0 / texSize;

#if SUPERIMPOSE
	videoCoord = gl_MultiTexCoord1.st;
#endif
}
