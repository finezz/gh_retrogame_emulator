// $Id: rgb.vert 11191 2010-01-25 00:49:16Z mthuurne $

uniform vec2 texSize;

varying vec4 scaled;
varying vec2 pos;
varying vec2 videoCoord;

void main()
{
	gl_Position = ftransform();
	scaled.xyz = vec3(gl_MultiTexCoord0.s * texSize.x) + vec3(0.0, 1.0/3.0, 2.0/3.0);
	scaled.w = gl_MultiTexCoord0.t * texSize.y + 0.5;
	pos        = gl_MultiTexCoord0.st;
#if SUPERIMPOSE
	videoCoord = gl_MultiTexCoord1.st;
#endif
}
