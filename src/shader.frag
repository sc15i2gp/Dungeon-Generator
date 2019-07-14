#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;
layout(location = 1) in vec3 frag_colour;

void main()
{
	out_colour = vec4(frag_colour, 1.0);
}
