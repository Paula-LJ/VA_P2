#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "extra/imgui/imgui.h"
#include "extra/imgui/imgui_impl_sdl.h"
#include "extra/imgui/imgui_impl_opengl3.h"

#include <cmath>

bool render_wireframe = false;
Camera* Application::camera = nullptr;
Application* Application::instance = NULL;

Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	scene_exposure = 1;
	output = 0;

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(5.f, 5.f, 5.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	{
		light = new Light();
		StandardMaterial* mat = new StandardMaterial();
		skybox = new Skybox_Material();	
		SceneNode* node = new SceneNode("Visible node");
		
		node->mesh = Mesh::Get("data/models/helmet/helmet.obj.mbin");
		node->material = mat;
		node->light = light;
		
		mat->texture = new Texture();
	
		//mat->texture->cubemapFromImages("data/environments/dragonvale");
		//mat->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/justphong.fs");
		node_list.push_back(node);
		
		skybox->mesh = Mesh::Get("data/meshes/box.ASE");
		skybox->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
		
		skybox->texture = new Texture();
		skybox->texture->cubemapFromImages("data/environments/dragonvale");

		HDRE* hdre = HDRE::Get("data/environments/studio.hdre");
		Texture* hdre_tex = new Texture();

		pbr = new PBR_Material();
		pbr->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/proba.fs");

		node->pbr = pbr;
		
		Texture* albedo_map = new Texture();
		albedo_map = Texture::Get("data/models/helmet/albedo.png");
		Texture* normal_map = new Texture();
		normal_map = Texture::Get("data/models/helmet/normal.png");
		Texture* metalness_map = new Texture();
		metalness_map = Texture::Get("data/models/helmet/metalness.png");
		Texture* roughness_map = new Texture();
		roughness_map = Texture::Get("data/models/helmet/roughness.png");
		Texture* LUT_map = new Texture();
		LUT_map = Texture::Get("data/brdfLUT.png");
		//Texture* opacity_map = new Texture();
		//opacity_map = Texture::Get("data/models/lantern/opacity.png");
		Texture* ao_map = new Texture();
		ao_map = Texture::Get("data/models/helmet/ao.png");
		//Texture* emissive_map = new Texture();
		//emissive_map = Texture::Get("data/models/helmet/emissive.png");

		//HDRE loading (son 6 imatges (6 cares del cubemap))
		/*Texture* HDRE_map = new Texture();
		Texture* HDRE_map_1 = new Texture();
		Texture* HDRE_map_2 = new Texture();
		Texture* HDRE_map_3 = new Texture();
		Texture* HDRE_map_4 = new Texture();
		Texture* HDRE_map_5 = new Texture();

		HDRE_map->cubemapFromHDRE(hdre, 0);
		HDRE_map_1->cubemapFromHDRE(hdre, 1);
		HDRE_map_2->cubemapFromHDRE(hdre, 2);
		HDRE_map_3->cubemapFromHDRE(hdre, 3);
		HDRE_map_4->cubemapFromHDRE(hdre, 4);
		HDRE_map_5->cubemapFromHDRE(hdre, 5);*/

		node->pbr->roughness_map = roughness_map;
		//mat->albedo_map = albedo_map;
		node->pbr->normal_map = normal_map;
		node->pbr->metalness_map = metalness_map;
		node->pbr->LUT_BRDF = LUT_map;
		//mat->opacity_map = opacity_map;
		node->pbr->ao_map = ao_map;
		//material->emissive_map = emissive_map;

		/*material->HDRE_map = HDRE_map;
		material->HDRE_map_1 = HDRE_map_1;
		material->HDRE_map_2 = HDRE_map_2;
		material->HDRE_map_3 = HDRE_map_3;
		material->HDRE_map_4 = HDRE_map_4;
		material->HDRE_map_5 = HDRE_map_5;*/

		pbr->light = node_list[0]->light;
		//node->pbr = pbr;
	}
	
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Application::render(void)
{
	//set the clear color (the background color)
	glClearColor(.1,.1,.1, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();
	
	//Creem l'skybox
	glDisable(GL_DEPTH_TEST);
	Matrix44 skyboxmodel;
	skyboxmodel.translate(camera->eye.x, camera->eye.y, camera->eye.z);
	skybox->render(skybox->mesh, skyboxmodel, camera);
	glEnable(GL_DEPTH_TEST);

	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	for (size_t i = 0; i < node_list.size(); i++) {
		node_list[i]->render(camera);

		if(render_wireframe)
			node_list[i]->renderWireframe(camera);
	}
}

void Application::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * 10; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 1;
	
	//example
	float angle = seconds_elapsed * 10.f * DEG2RAD;
	/*for (int i = 0; i < root.size(); i++) {
		root[i]->model.rotate(angle, Vector3(0,1,0));
	}*/

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT && !ImGui::IsAnyWindowHovered() 
		&& !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())) //is left button pressed?
	{
		camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
	}

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move fast er with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f,  1.0f, 0.0f) * speed);

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

//Keyboard event handler (sync input)
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: render_debug = !render_debug; break;
		case SDLK_F2: render_wireframe = !render_wireframe; break;
		case SDLK_F5: Shader::ReloadAll(); break; 
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event)
{
}

void Application::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Application::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
		case SDL_MOUSEWHEEL:
		{
			if (event.x > 0) io.MouseWheelH += 1;
			if (event.x < 0) io.MouseWheelH -= 1;
			if (event.y > 0) io.MouseWheel += 1;
			if (event.y < 0) io.MouseWheel -= 1;
		}
	}

	if(!ImGui::IsAnyWindowHovered() && event.y)
		camera->changeDistance(event.y * 0.5);
}

void Application::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

void Application::onFileChanged(const char* filename)
{
	Shader::ReloadAll();
}