#version 440

// The transformation matrix
uniform mat4 MVP;

// Incoming position
layout (location = 0) in vec3 position;
// Tex coord in
layout(location = 10) in vec2 tex_coord_in;

// Outgoing texture coordinates
layout(location = 1) out vec3 tex_coord_out;
void main()
{
	// Transform the position into screen space
	gl_Position = MVP * vec4(position, 1.0);
	tex_coord_out = position;
}