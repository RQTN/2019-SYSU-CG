#version 330 core

layout (location = 0) in vec3 aPos;

out vec4 vertexColor;

uniform vec4 up_vertex_color;
uniform vec4 left_bottom_vertex_color;
uniform vec4 right_bottom_vertex_color;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	if (aPos == vec3(0.0f, 0.5f, 0.0f))
		vertexColor = up_vertex_color;
	else if (aPos == vec3(-0.5f, -0.5f, 0.0f))
		vertexColor = left_bottom_vertex_color;
	else if (aPos == vec3(0.5f, -0.5f, 0.0f))
		vertexColor = right_bottom_vertex_color;
}