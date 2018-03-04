// $Id: hqlite.vert 11025 2009-12-26 13:47:09Z mthuurne $

uniform vec2 texSize;

varying vec2 leftTop;
varying vec2 edgePos;
varying vec4 misc;
varying vec2 videoCoord;

void main()
{
	gl_Position = ftransform();

	edgePos = gl_MultiTexCoord0.st * vec2(1.0, 2.0);
	
	vec2 texStep = vec2(1.0 / texSize.x, 0.5 / texSize.y);
	leftTop  = gl_MultiTexCoord0.st - texStep;

	vec2 subPixelPos = edgePos * texSize;
	vec2 texStep2 = 2.0 * texStep;
	misc = vec4(subPixelPos, texStep2);

#if SUPERIMPOSE
	videoCoord = gl_MultiTexCoord1.st;
#endif
}
