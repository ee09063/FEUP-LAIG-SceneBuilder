#ifndef TPSCENE_H
#define TPSCENE_H

#include "CGFscene.h"
#include "SceneGraph.h"
#include <vector>

using std::vector;

void processGraph(Graph* g, Node* n);

class TPscene : public CGFscene
{
	Graph *g;
	InitVar *iv;
	vector<CGFlight*> lights;
	GLenum drawingMode;
	
public:
	int drawMode;
	int cameraMode;
	int* lightCheck;
	int lc;
	TPscene(Graph* g, InitVar *iv)
	{
		this->g=g;
		this->iv=iv;
		this->lightCheck = (int*)malloc(sizeof(int)*iv->getLights().size());
		this->cameraMode = 0;
	}
	void init();
	void display();
	void toggleLights();
	void letThereBeLight();
	void chooseActiveCamera();
	void processGraph(Graph* g, Node* n, Appearance* app);
	void setDrawingMode(GLenum drawingMode){this->drawingMode = drawingMode;}
};

#endif