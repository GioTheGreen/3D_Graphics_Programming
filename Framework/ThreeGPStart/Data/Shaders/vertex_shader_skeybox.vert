#version 330

/*
	The vertex shader runs once for every vertex in the primitive
	Its main job is to convert the vertex position to a screen position by writing to gl_Position

	Note: I set the extension to .vert to avoid a clash with NSight but still allow the highlighting
	plugin to work (GLSL Langauage integration - highly recommended).
*/
uniform mat4 combined_xform;
uniform mat4 model_xform;

layout (location=0) in vec3 vertex_position;
layout (location=1) in vec3 vertex_normal;
layout (location=2) in vec2 vertex_texcoods;
layout (location = 3) in vec3 light_position;

out float diffuse_intensity;
out vec2 varying_coord;


void main(void)
{
	vec3 P = vertex_position;
	vec3 N = normalize(vertex_normal);
	vec3 L = normalize( vec3(0.5,1,0));
	vec3 L2 =normalize( vec3(100,20,100)-P);

	diffuse_intensity = max(0, dot(L, N) + dot(L2, N)/2);
	varying_coord = vertex_texcoods;

	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);

}