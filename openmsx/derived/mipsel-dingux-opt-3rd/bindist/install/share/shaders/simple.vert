// $Id: simple.vert 11025 2009-12-26 13:47:09Z mthuurne $

uniform vec2 texSize;
uniform vec3 texStepX; // = vec3(vec2(1.0 / texSize.x), 0.0);
uniform vec4 cnst;     // = vec4(scan_a, scan_b, scan_c, alpha);

varying vec2 scaled;
varying vec3 misc;
varying vec2 videoCoord;

void main()
{
	float alpha  = cnst.w;

	gl_Position = ftransform();
	misc = vec3((vec2(0.5) - vec2(1.0, 0.0) * alpha) * texStepX.x, gl_MultiTexCoord0.t);
	scaled.x = gl_MultiTexCoord0.s * texSize.x;
	scaled.y = gl_MultiTexCoord0.t * texSize.y + 0.5;

#if SUPERIMPOSE
	videoCoord = gl_MultiTexCoord1.st;
#endif
}
