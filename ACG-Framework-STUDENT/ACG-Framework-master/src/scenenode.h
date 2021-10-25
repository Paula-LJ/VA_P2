#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"

#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"


class SceneNode {
public:

	static unsigned int lastNameId;

	SceneNode();
	SceneNode(const char* name);
	~SceneNode();

	Shader* shader = NULL;
	Texture* texture = NULL;

	Material * material = NULL;
	Light * light = NULL;
	Skybox_Material * skybox = NULL;

	PBR_Material * pbr = NULL;
	std::string name;

	Mesh* mesh = NULL;
	Mesh* skyboxMesh = NULL;

	Matrix44 model;


	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};



#endif