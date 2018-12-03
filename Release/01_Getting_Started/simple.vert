#version 440

// The model matrix
uniform mat4 M;
// The transformation matrix
uniform mat4 MVP;
// The normal matrix
uniform mat3 N;
// The light transformation matrix
uniform mat4 LightMVP;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming binormal
layout(location = 3) in vec3 binormal;
// Incoming tangent
layout(location = 4) in vec3 tangent;
// Incoming texture coordinates
layout(location = 10) in vec2 tex_coord_in;


// Outgoing position
layout(location = 0) out vec3 vertex_position;
// Outgoing normal
layout(location = 1) out vec2 tex_coord_out;
// Outgoing texture coordinate
layout(location = 2) out vec3 transformed_normal;
// Outgoing binormal
layout(location = 3) out vec3 transformed_binormal;
// Outgoing tangent
layout(location = 4) out vec3 transformed_tangent;
//
//layout(location = 5) out vec4 vertex_light;

void main() {
  // Set position
  gl_Position = MVP * vec4(position, 1);
  // Output other values to fragment shader
  	vertex_position = (M * vec4(position, 1)).xyz;
  	transformed_normal = normalize(N * normal);
  	transformed_tangent = normalize(N * tangent);
  	transformed_binormal = normalize(N * binormal);
  	tex_coord_out = tex_coord_in;
  	//vertex_light = lightMVP * vec4(position, 1.0f);
}