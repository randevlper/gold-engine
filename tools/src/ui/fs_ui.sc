$input v_texcoord0

#include "../common/common.sh"
SAMPLER2D(u_sprite,  0);
uniform vec4 u_color;

void main()
{
	gl_FragColor = mul(u_color, texture2D(u_sprite, v_texcoord0));
}
