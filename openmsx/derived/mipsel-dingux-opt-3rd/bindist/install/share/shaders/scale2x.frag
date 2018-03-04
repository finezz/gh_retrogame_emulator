// $Id: scale2x.frag 10791 2009-11-16 15:39:36Z m9710797 $
// Scale2x scaler.

uniform sampler2D tex;
uniform sampler2D videoTex;

varying vec2 texStep; // could be uniform
varying vec2 coord2pi;
varying vec2 videoCoord;

vec4 scaleNx()
{
	vec4 delta;
	delta.xw = sin(coord2pi) * texStep;
	delta.yz = vec2(0.0);
	
	vec4 posLeftTop  = gl_TexCoord[0].stst - delta;
	vec4 posRightBot = gl_TexCoord[0].stst + delta;

	vec4 left  = texture2D(tex, posLeftTop.xy);
	vec4 top   = texture2D(tex, posLeftTop.zw);
	vec4 right = texture2D(tex, posRightBot.xy);
	vec4 bot   = texture2D(tex, posRightBot.zw);

	if (dot(left.rgb - right.rgb, top.rgb - bot.rgb) == 0.0 || left.rgb != top.rgb) {
		return texture2D(tex, gl_TexCoord[0].st);
	} else {
		return top;
	}
}

void main()
{
#if SUPERIMPOSE
	vec4 col = scaleNx();
	vec4 vid = texture2D(videoTex, videoCoord);
	gl_FragColor = mix(vid, col, col.a);
#else
	gl_FragColor = scaleNx();
#endif
}
