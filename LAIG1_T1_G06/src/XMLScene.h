#pragma once
#ifndef _XMLSCENE_H_
#define _XMLSCENE_H_

#include "tinyxml.h"
#include "string.h"
#include "SceneGraph.h"

using std::vector;

Primitive* registerRectangle(TiXmlElement *prim, char*id);
Primitive* registerTriangle(TiXmlElement *prim, char*id);
Primitive* registerCylinder(TiXmlElement *prim, char*id);
Primitive* registerSphere(TiXmlElement *prim, char*id);
Primitive* registerTorus(TiXmlElement *prim, char*id);

void postProcess(InitVar* iv, Graph* g);

bool checkRepeated(char* id, vector<char*> idList);

char* getParentApp(Node* n, Graph* g);

class XMLScene
{
public:
	XMLScene(char *filename, Graph* g, InitVar *iv);
	~XMLScene();

	static TiXmlElement *findChildByAttribute(TiXmlElement *parent,const char * attr, const char *val);
	
	vector<Camera*> cameras;
	vector<Light*> lights;
	vector<Texture*> textures;
	vector<Appearance*> appearances;

	Texture* getTextureById(char* id);
	Appearance* getAppearanceById(char* id);

	Global* registerGlobalVariables(TiXmlElement* globalElement);
	void registerCameras(TiXmlElement* cameraElement);
	void registerLights(TiXmlElement* lightElement);
	void registerTextures(TiXmlElement* textsElement);
	void registerApps(TiXmlElement* appElement);


protected:

	TiXmlDocument* doc;

	TiXmlElement* globalElement; 
	TiXmlElement* appElement;
	TiXmlElement* textsElement;
	TiXmlElement* primElement;
	TiXmlElement* nodeElement;
	TiXmlElement* graphElement;
	TiXmlElement* cameraElement;
	TiXmlElement* lightElement;
};

#endif