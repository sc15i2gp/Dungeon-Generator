#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform world_matrix
{
	mat4 view;
	mat4 projection;
} wm;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_colour;

layout(push_constant) uniform push_constants 
{
	mat4 model;
} pc;

layout(location = 1) out vec3 frag_colour;

void main()
{
	gl_Position = wm.projection * wm.view * pc.model * vec4(in_position, 0.0, 1.0);
	frag_colour = in_colour;
}
