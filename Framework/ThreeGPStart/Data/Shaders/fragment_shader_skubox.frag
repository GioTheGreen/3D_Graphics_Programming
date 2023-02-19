#version 330

/*
	The fragment shader runs once for every fragment in the rasterised primitive
	You can think of a fragment as just the same as a screen pixel (it is a bit different when multisampling)
	Its main job is to output a colour for the fragment

	Note: I set the extension to .frag to avoid a clash with NSight but still allow the highlighting
	plugin to work (GLSL Languaage integration - highly recommended).
*/

// We want to output a 4 element colour (RGBA)
uniform vec4 diffuse_colour;

in float diffuse_intensity;
in vec2 varying_coord;
out vec4 fragment_colour;

uniform sampler2D sampler_tex;

void main(void)
{
	vec3 tex_colour = texture(sampler_tex,varying_coord).rgb;
	

	fragment_colour = vec4(tex_colour * diffuse_intensity, 1.0);
}