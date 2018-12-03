#include <graphics_framework.h>
#include <glm/glm.hpp>

// The namespaces we are using
using namespace std;
using namespace graphics_framework;
using namespace glm;

// Cubemap
cubemap cube_map;

// Shadowmap
// TODO: Finish the shadows
shadow_map shadow;

// Textures and Meshes
mesh skybox;
map<string, mesh> meshes;
map<string, texture> textures;

float overallDiffuse = 0.4f;

// Effects
effect skyboxEffect;
effect mainEffect;
effect shadow_eff;

// Cameras
// TODO: maybe add secound camera?
//chase_camera chaseCamera;
free_camera freeCamera;
bool isFreeCamera = true;
float cameraSpeed = 0.02;

// Lights
// TODO: decide what you gonna do with the lights
directional_light directionalLight;

// cursor vars
double cursor_x = 0.0;
double cursor_y = 0.0;

bool initialise() {
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	return true;
}  
  

// Initial one-time startup code goes here
bool load_content() {
	// create the shadow map
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());

	// Create skybox   
	skybox = mesh(geometry_builder::create_box());
	// Scale it
	skybox.get_transform().scale = vec3(100.0f, 100.0f, 100.0f);

	// Load the pictures for the cubemap
	array<string, 6> cubemapfiles =
	{
		"textures/cloudtop_ft.tga",
		"textures/cloudtop_bk.tga",
		"textures/cloudtop_up.tga",
		"textures/cloudtop_dn.tga",
		"textures/cloudtop_rt.tga",
		"textures/cloudtop_lf.tga"
	};

	cube_map = cubemap(cubemapfiles);

	// Load the meshes
	meshes["plane"] = mesh(geometry_builder::create_box());
	meshes["sniper"] = mesh(geometry("KSR29/KSR29.obj"));
	  
	// Set materials
	// No emission
	meshes["plane"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// Slight diffuse to fake occlusion
	meshes["plane"].get_material().set_diffuse(vec4(overallDiffuse, overallDiffuse, overallDiffuse, 1.0f));
	// Wood is not strongly reflective so 0?
	// TODO: think about fernel
	meshes["plane"].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// Wood is not shiny so 1 (it is a power)
	meshes["plane"].get_material().set_shininess(1.0f);  

	  
	meshes["sniper"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["sniper"].get_material().set_diffuse(vec4(overallDiffuse, overallDiffuse, overallDiffuse, 1.0f));
	meshes["sniper"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["sniper"].get_material().set_shininess(50.0f);

	// Transform meshes
	meshes["plane"].get_transform().scale = vec3(100.0f, 0.1f, 100.0f);
	meshes["plane"].get_transform().rotate(vec3(0.0f, 0.0f, 0.0f));
	meshes["plane"].get_transform().translate(vec3(0.0f, -5.0f, 0.0f));

	meshes["sniper"].get_transform().scale = vec3(1.0f, 1.0f, 1.0f);
	meshes["sniper"].get_transform().rotate(vec3(0.0f, 0.0f, 0.0f));
	meshes["sniper"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));

	// Load textures
	textures["wood"] = texture("textures/wood_plane.jpg");
	textures["sniper_colour"] = texture("KSR29/textures/Colour.jpg");
	textures["sniper_normal"] = texture("KSR29/textures/Normal.jpg");
	textures["sniper_specular"] = texture("KSR29/textures/Specular.jpg");
	textures["sniper_ambientOcclusion"] = texture("KSR29/textures/AmbientOcclusion.jpg");
	// TODO: ambient occlusion
 
	// Load Lights
	// ambient intensity (0.3, 0.3, 0.3), colour white, direction 1,1,1
	directionalLight.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
	directionalLight.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	directionalLight.set_direction(vec3(1.0f, 1.0f, 1.0f));
	  
	// TODO: update / seperate my shaders
	// Load in shaders
	skyboxEffect.add_shader("01_Getting_Started/skybox.vert", GL_VERTEX_SHADER);
	skyboxEffect.add_shader("01_Getting_Started/skybox.frag", GL_FRAGMENT_SHADER);

	mainEffect.add_shader("01_Getting_Started/simple.vert", GL_VERTEX_SHADER);
	mainEffect.add_shader("01_Getting_Started/simple.frag", GL_FRAGMENT_SHADER);

	//TODO: Add the shadow shaders
	//shadow_eff.add_shader("01_Getting_Started/simple.vert", GL_VERTEX_SHADER);
	//shadow_eff.add_shader("01_Getting_Started/simple.frag", GL_FRAGMENT_SHADER);

	// Build effects  
	skyboxEffect.build();
	mainEffect.build();
	//shadow_eff.build();

	// Load free camera
	freeCamera.set_position(vec3(4.0f, 0.0f, 10.0f));
	freeCamera.set_target(vec3(0.0f, 0.0f, 0.0f));
	freeCamera.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	return true;  
}

// Called every frame, do game logic here
bool update(float delta_time) {
	// The ratio of pixels to rotation - remember the fov
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_width());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;

	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	if (isFreeCamera) {
		// Multiply deltas by ratios - gets actual change in orientation
		delta_x = delta_x * ratio_width;
		delta_y = delta_y * ratio_height;
		// Rotate cameras by delta
		freeCamera.rotate(delta_x, -delta_y);
		// Controls
		if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
			freeCamera.move(vec3(delta_x, -delta_y, cameraSpeed));
		}

		if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
			freeCamera.move(vec3(delta_x, -delta_y, -cameraSpeed));
		}

		if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
			freeCamera.move(vec3(delta_x, -delta_y, 0) + vec3(-cameraSpeed, 0, 0));
		}

		if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
			freeCamera.move(vec3(delta_x, -delta_y, 0) + vec3(cameraSpeed, 0, 0));
		}

		if (glfwGetKey(renderer::get_window(), GLFW_KEY_SPACE)) {
			freeCamera.move(vec3(freeCamera.get_up()) * vec3(cameraSpeed));
		}

		if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_SHIFT)) {
			freeCamera.move(vec3(freeCamera.get_up()) * vec3(-cameraSpeed));
		}
		//Update the camera
		freeCamera.update(delta_time);
	}

	// update cursor
	cursor_x = current_x;
	cursor_y = current_y;  

	// TODO: put on screen maybe?
	// FPS counter  
	cout << "FPS: " << 1.0f / delta_time << endl;

	return true;
}

// Called every frame, do rendering here
bool render() { 
	//Shadow
	/*{
		// Set render target to shadow map
		renderer::set_render_target(shadow);
		// Clear depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		// Set face cull mode to front
		glCullFace(GL_FRONT);

		// Light projection Matrix with fov 90
		mat4 lightP = perspective<float>(90.0f, renderer::get_screen_aspect(), 0.1f, 1000.0f);
		// bind shader
		renderer::bind(shadow_eff);
		// Render meshes for shadows
		for (auto &e : meshes) {
			auto m = e.second;
			// Create MVP matrix
			auto M = m.get_transform().get_transform_matrix();
			// View matrix taken from shadow map
			auto V = shadow.get_view();
			auto MVP = lightP * V * M;
			// Set MVP matrix uniform
			glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			renderer::render(m);
		}

		// Set render target back to the screen
		renderer::set_render_target();
		// Set face cull mode to back
		glCullFace(GL_BACK);
	} */

	// Skybox
	{
		// Disable depth test,depth mask,face culling
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);  
		glCullFace(GL_FRONT);

		// Bind effect
		renderer::bind(skyboxEffect);

		// Create MVP matrix
		auto M = skybox.get_transform().get_transform_matrix();
		auto V = freeCamera.get_view();
		auto P = freeCamera.get_projection();
		auto MVP = P * V * M;

		// Set MVP matrix uniform
		glUniformMatrix4fv(skyboxEffect.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

		// Set cubemap uniform  
		renderer::bind(cube_map, 0);    
		glUniform1i(skyboxEffect.get_uniform_location("cubemap"), 0); 
		    
		// Render skybox
		renderer::render(skybox);  

		// Enable depth test,depth mask,face culling
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
	}   
	 
	
	// Bind effect
	renderer::bind(mainEffect);

	// Render meshes
	for (auto &e : meshes) {
		auto m = e.second;

		// Textures
		if (e.first == "plane") {
			glUniform1i(mainEffect.get_uniform_location("index"), 0);
			renderer::bind(textures["wood"], 0);
		}

		if (e.first == "sniper") {
			glUniform1i(mainEffect.get_uniform_location("index"), 1);
			renderer::bind(textures["sniper_colour"], 0);

			renderer::bind(textures["sniper_normal"], 2);
			glUniform1i(mainEffect.get_uniform_location("norm"), 2);
			renderer::bind(textures["sniper_specular"], 3);
			glUniform1i(mainEffect.get_uniform_location("spec"), 3);
			renderer::bind(textures["sniper_ambientOcclusion"], 4);
			glUniform1i(mainEffect.get_uniform_location("occlu"), 4);
		}

		glUniform1i(mainEffect.get_uniform_location("tex"), 0);

		// Bind material
		renderer::bind(m.get_material(), "mat"); 
		
		// Get the normal of the mesh
		auto N = m.get_transform().get_normal_matrix();
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = freeCamera.get_view();
		auto P = freeCamera.get_projection();
		auto MVP = P * V * M;

		// Set MVP matrix uniform
		glUniformMatrix4fv(mainEffect.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Set M matrix uniform
		glUniformMatrix4fv(mainEffect.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// N = transpose(inverse(N));
		glUniformMatrix3fv(mainEffect.get_uniform_location("N"), 1, GL_FALSE, value_ptr(N));

		// Bind lights
		renderer::bind(directionalLight, "directionalLight");
		glUniform3fv(mainEffect.get_uniform_location("eye_pos"), 1, value_ptr(freeCamera.get_position()));

		// Bind the cube_map
		renderer::bind(cube_map, 0);

		// Render mesh
		renderer::render(m);
	}

	return true; 
}

void main() {
  // Create application
  app application("Sniper");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}