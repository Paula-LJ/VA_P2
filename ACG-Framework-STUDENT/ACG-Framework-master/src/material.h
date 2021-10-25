#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;

	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	Material* material = NULL;
	Mesh* mesh = NULL;
	Matrix44 model;

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class Light : public StandardMaterial { 
public:

	Light();
	~Light();

	vec3 Ia;
	vec3 Id;
	vec3 Is;

	vec3 lightpos;

	float alpha;
	float specular_color; //CAL MIRAR SI HI HA UNA MILLOR OPCIÓ
	int option;

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class Skybox_Material :public StandardMaterial {
public:
	Skybox_Material();
	~Skybox_Material();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* skyboxMesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

class PBR_Material : public Material {		
public:

	PBR_Material();
	~PBR_Material();


	Light* light;
	Texture* albedo_map;
	Texture* normal_map;
	Texture* metalness_map;
	Texture* roughness_map;
	//Texture* opacity_map;
	Texture* ao_map;
	//Texture* emissive_map;

	Texture* LUT_BRDF;
	//Texture* HDRE_map;
	/*Texture* HDRE_map_1;
	Texture* HDRE_map_2;
	Texture* HDRE_map_3;
	Texture* HDRE_map_4;
	Texture* HDRE_map_5;*/

	float roughness;
	float metalness;

	bool has_pbr;
	bool has_ibl;
	//bool has_emissive;
	//bool has_opacity;
	bool has_ao;

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};
#endif