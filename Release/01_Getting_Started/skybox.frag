#version 440

// Texture of the object
uniform samplerCube cubemap;

layout(location = 1) in vec3 tex_coord;

layout (location = 0) out vec4 colour;

void main()
{
	vec4 textureSample = texture(cubemap, tex_coord);
	colour = textureSample;
}