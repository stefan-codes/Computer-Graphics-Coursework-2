#version 440

// A directional light structure
struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};

// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Directional light 
uniform directional_light directionalLight;

// Index of the item
uniform int index;

// Material of the object
uniform material mat;

// Texture of the object
uniform sampler2D texBase, tex, norm, spec, occlu;

// Position of the camera
uniform vec3 eye_pos;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec2 tex_coord;
// Incoming texture coordinate
layout(location = 2) in vec3 normal;
// Incoming binormal
layout(location = 3) in vec3 binormal;
// Incoming tangent
layout(location = 4) in vec3 tangent;
// Incoming light
//layout(location = 5) in vec4 light_position;

// Outgoing colour
layout(location = 0) out vec4 colour;

// Calculate a normal with normal map if present
vec3 calcNormal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D norm, in vec2 tex_coord){
	vec3 sampleNormal = texture(norm, tex_coord).xyz;
	sampleNormal = 2.0f * sampleNormal - vec3(1.0f, 1.0f, 1.0f);
	mat3 TBN = mat3(tangent, binormal, normal);
	vec3 newNormal = normalize(TBN * sampleNormal);
	return newNormal;
}

// Calculate the colour from the directional light
vec4 directional_light_colour(in directional_light light, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_sample, in vec4 spec_sample, in vec4 occlu_sample){
	vec4 result;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	// Calculate ambient
	ambient = mat.diffuse_reflection * light.light_colour;
	ambient = ambient * tex_sample;
	ambient = ambient * occlu_sample;

	// Calculate diffuse
	float diffuseK = max(dot(normal, light.light_dir), 0.0f);
	vec4 primary = diffuseK * (mat.diffuse_reflection * light.light_colour);
	diffuse = mat.emissive + primary;
	diffuse = diffuse * tex_sample;

	// Calculate specular
	vec3 half_vector = normalize(light.light_dir + view_dir);
	float specularK = pow(max(dot(normal, half_vector), 0.0f), mat.shininess);
	specular = specularK * (mat.specular_reflection * light.light_colour);
	specular = specular * spec_sample;
	
	// Phong = ambient + diffuse + specular
	result = ambient + diffuse + specular;
	result.a = 1.0f;

	return result;	
}

void main() {
	// Calculate view direction
	vec3 view_direction = normalize(eye_pos - position);
	// Pass the normal
	vec3 updatedNormal = normal;
	if ( index == 1) {
		updatedNormal = calcNormal(normal, tangent, binormal, norm, tex_coord);
	}
	
	// Sample the texture
	vec4 textureSample = texture(tex, tex_coord);

	// Sample the occlusion
	vec4 occlusionSample = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if (index == 1) {
		occlusionSample = texture(occlu, tex_coord);
	}

	// Sample the specular
	vec4 specularSample = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if (index == 1) {
		specularSample = texture(spec, tex_coord);
	}

	// Calculate the actual colour
	colour = directional_light_colour(directionalLight, mat, position, updatedNormal, view_direction, textureSample, specularSample, occlusionSample);
}