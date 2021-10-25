#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/proba.fs");
}

StandardMaterial::~StandardMaterial()
{
	
}

Light::Light()
{
	Ia = vec3(0.2f, 0.2f, 0.2f);
	Id = vec3(1, 1, 1);
	Is = vec3(1, 1, 1);

	lightpos = vec3(1, 1, 1);

	alpha = 2.0;

	option = 0;
}

Light::~Light()
{


}

void Light::renderInMenu()
{
	/*if (ImGui::TreeNode("Reflection")) {
		Application::instance->node_list[0]->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/mirror.fs");
		ImGui::TreePop();
	}*/
	/*if (ImGui::TreeNode("Phong with reflection")) {
		option = 0;
		Application::instance->node_list[0]->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
		ImGui::DragFloat3("Ia", &this->Ia.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Id", &this->Id.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Is", &this->Is.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Light position", &this->lightpos.x, 1.0f, -100, 100);
		ImGui::DragFloat("alpha", &this->alpha, 1.0f, 1, 100);
		ImGui::TreePop();
	}*/
	/*if (ImGui::TreeNode("Phong")) {
		option = 1;
		Application::instance->node_list[0]->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/justphong.fs");
		ImGui::DragFloat3("Ia", &this->Ia.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Id", &this->Id.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Is", &this->Is.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Light position", &this->lightpos.x, 1.0f, -100, 100);
		ImGui::DragFloat("alpha", &this->alpha, 1.0f, 1, 100);
		ImGui::TreePop();
	}*/
	/*if (ImGui::TreeNode("No Phong")) {
		if (ImGui::Button("Wood")) {
			Application::instance->node_list[0]->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
			Application::instance->node_list[0]->material->texture = Texture::Get("data/textures/natur4.png");
		}
		if (ImGui::Button("Darkness")) {
			Application::instance->node_list[0]->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
			Application::instance->node_list[0]->material->texture = Texture::Get("data/textures/darktexture.png");
		}
		if (ImGui::Button("Watermelon")) {
			Application::instance->node_list[0]->material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
			Application::instance->node_list[0]->material->texture = Texture::Get("data/textures/watermelon.png");
		}
		ImGui::TreePop();
	}*/
}

Skybox_Material::Skybox_Material() 
{

}

Skybox_Material::~Skybox_Material()
{

}


void Skybox_Material::renderInMenu()
{

	if (ImGui::Button("Dragonvale")) {
		texture->cubemapFromImages("data/environments/dragonvale");
		Application::instance->node_list[0]->material->texture->cubemapFromImages("data/environments/dragonvale");
	}
	if (ImGui::Button("City")) {
		texture->cubemapFromImages("data/environments/city");
		Application::instance->node_list[0]->material->texture->cubemapFromImages("data/environments/city");
	}
	if (ImGui::Button("Snow")) {
		texture->cubemapFromImages("data/environments/snow");
		Application::instance->node_list[0]->material->texture->cubemapFromImages("data/environments/snow");
	}
}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_pos", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_exposure", Application::instance->scene_exposure);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void Light::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_pos", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_color", color);
	shader->setUniform("u_exposure", Application::instance->scene_exposure);

	

	// inicialitza variables de la llum
	shader->setUniform("Ia", Ia);
	shader->setUniform("Id", Id);
	shader->setUniform("Is", Is);

	shader->setUniform("lightpos", lightpos);
	shader->setUniform("alpha", alpha);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void Light::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);
		
		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void Skybox_Material::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_pos", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_exposure", Application::instance->scene_exposure);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void Skybox_Material::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}



void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

PBR_Material::PBR_Material()
{	
	//emissive_map = NULL;
	//ao_map = NULL;
	//opacity_map = NULL;
	roughness = 0.01;
	metalness = 0.99;

	has_pbr = 1;
	has_ibl = 1;
	//has_emissive = 1.0;
	//has_opacity = 1.0;
	//has_ao = 1;
}


PBR_Material::~PBR_Material()
{
}

void PBR_Material::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_light_position", Application::instance->node_list[0]->model.getTranslation());
	shader->setUniform("u_light_specular", Application::instance->node_list[0]->light->specular_color);

	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", color);
		
	shader->setUniform("u_normal_texture", normal_map, 1);
	shader->setUniform("u_metalness_texture", metalness_map, 2);
	shader->setUniform("u_roughness_texture", roughness_map, 3);
	shader->setUniform("u_LUT_BRDF", LUT_BRDF, 4);
	shader->setUniform("u_ao_texture", ao_map, 5);
	shader->setUniform("u_albedo_texture", albedo_map, 6);

	shader->setUniform("u_metal_val", metalness);
	shader->setUniform("u_rough_val", roughness);

	if ((texture))
		shader->setUniform("u_texture", texture);
}

void PBR_Material::render(Mesh* mesh, Matrix44 model, Camera* camera)
{

	//set flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}


void PBR_Material::renderInMenu()
{

	ImGui::SliderFloat("Roughness", (float*)&roughness, 0.01, 0.99);
	ImGui::SliderFloat("Metalness", (float*)&metalness, 0.01, 0.99);
	ImGui::Checkbox("PBR On/Off", &has_pbr);
	ImGui::Checkbox("IBL On/Off", &has_ibl);
	//ImGui::Checkbox("Opacity On/Off", &has_opacity);
	//ImGui::Checkbox("Emissive On/Off", &has_emissive);
	ImGui::Checkbox("AO On/Off", &has_ao);

}