#include "TPscene.h"
#include "CGFaxis.h"
#include "CGFapplication.h"
#include "Primitives.h"
#include "CGFappearance.h"

int lightId(int l)
{
	if(l==0) return GL_LIGHT0;
	if(l==1) return GL_LIGHT1;
	if(l==2) return GL_LIGHT2;
	if(l==3) return GL_LIGHT3;
	if(l==4) return GL_LIGHT4;
	if(l==5) return GL_LIGHT5;
	if(l==6) return GL_LIGHT6;
	if(l==7) return GL_LIGHT7;
	else return -1;
}

void TPscene::toggleLights()
{
	//printf("TOGGLING LIGHTS...\n");
	for(unsigned int i = 0; i < lights.size(); i++)
	{
		if(lightCheck[i]==1)
		{
			//printf("ENABLING...\n");
			lights.at(i)->enable();
		}
		else if(lightCheck[i]==0)
		{
			//printf("DISABLING...\n");
			lights.at(i)->disable();
		}
	}
}

void TPscene::init() 
{
	Global *gl = iv->getGlobal();
	// Enables lighting computations

	if(gl->getEnabled() == GL_TRUE)
	{
		printf("LIGHTING ENABLED...\n");
		glEnable(GL_LIGHTING);
	}
	/**SHADE MODEL**/
	glShadeModel(gl->getShadingMode());

	if(gl->getCullFace() != 0)
	{
		printf("CULLFACE ENABLED...\n");
		glEnable(GL_CULL_FACE);
		glCullFace(gl->getCullFace());
		glFrontFace(gl->getOrder());
	}

	glEnable(GL_COLOR_MATERIAL);

	/**LIGHT PARAMETERS FROM GLOBAL**/
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, gl->getSided());
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gl->getAmbient());
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, gl->getLocal());

	/**APPLY BACKGROUND**/
	glClearColor(gl->getBackground()[0], gl->getBackground()[1], gl->getBackground()[2], gl->getBackground()[3]);
	
	/**DECLARING THE LIGHTS**/
	this->letThereBeLight();

	// Defines a default normal
	glNormal3f(0,0,1);
	glEnable(GL_NORMALIZE);

	this->drawingMode = iv->getGlobal()->getDrawingMode();

	/**DEAL WITH INITIAL CAMERA**/
	this->cameraMode = iv->getCameraById(iv->getInitialCamera());
}

void TPscene::display() 
{
	// ---- BEGIN Background, camera and axis setup
	
	// Clear image and depth buffer everytime we update the scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//this->chooseActiveCamera();

	// Apply transformations corresponding to the camera position relative to the origin
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
	CGFcamera* cam = (CGFcamera*)iv->getCameras().at(this->cameraMode);
	CGFscene::activeCamera = cam;
	CGFapplication::activeApp->forceRefresh();
	CGFscene::activeCamera->applyView();

	/**DRAWING THE LIGHTS**/
	/**UPDATE POSITION FOR BOTH, DIRECTION FOR SPOTLIGHT**/
	for(unsigned int i = 0; i < lights.size(); i++)
	{
		if(iv->getLights().at(i)->getMarker() == true)
		{
			lights.at(i)->draw();
		}
		else
			lights.at(i)->update();
		if(strcmp("spot", iv->getLights().at(i)->getType())==0)
		{
			float* direction = iv->getLights().at(i)->getDirection();
			GLfloat spotDir[] = {direction[0], direction[1], direction[2]};
			glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, spotDir); 
		}
	}

	// Draw axis
	axis.draw();

	// ---- END Background, camera and axis setup

	// ---- BEGIN Primitive drawing section

	//materialA->apply();

	glPolygonMode(GL_FRONT_AND_BACK, this->drawingMode);

	glPushMatrix();

		processGraph(g, g->getRootNode(), g->getRootNode()->getNodeApp());

	glPopMatrix();

	// ---- END Primitive drawing section
	glutSwapBuffers();
}

void TPscene::processGraph(Graph* g, Node* n, Appearance* app)
{
	glPushMatrix();
		glMultMatrixf(&n->m[0][0]);
		float s = 1.0, t = 1.0;
		Appearance* nextApp;
		CGFappearance* a;

		if(strcmp(n->getAppMode(), "none") ==0){
			nextApp = NULL;
		}
		else if(strcmp(n->getAppMode(), "inherit")==0){
			s = app->getTexture()->getTexS();
			t = app->getTexture()->getTexT();
			nextApp = app;
			a = app->getApp();
			a->apply();
		}
		else{
			s = n->getNodeApp()->getTexture()->getTexS();
			t = n->getNodeApp()->getTexture()->getTexT();
			nextApp = n->getNodeApp();
			a = n->getNodeApp()->getApp();
			a->apply();
		}

		vector<Primitive*>p = n->getPrimitives();
		for(unsigned int k = 0; k < p.size();k++)
		{
			p.at(k)->draw(s,t);
		}

		vector<char*>des = n->getDescendants();
		for(unsigned int i = 0; i < des.size(); i++)
		{
			processGraph(g, g->getNode(des.at(i)), nextApp);
		}
	glPopMatrix();
}

void TPscene::letThereBeLight()
{
	printf("CREATING LIGHTS...\n");
	for(unsigned int i = 0; i < iv->getLights().size(); i++)
	{
		glPushMatrix();
		Light* l = iv->getLights().at(i);
		float light_pos[4] = {l->getLocation()[0], l->getLocation()[1], l->getLocation()[2], 1.0};
		float* ambient = l->getAmbient();
		float* diffuse = l->getDiffuse();
		float* specular = l->getSpecular();

		CGFlight* light = new CGFlight(lightId(i), light_pos);
		
		light->setAmbient(ambient);
		light->setDiffuse(diffuse);
		light->setSpecular(specular);
		
		if(strcmp(iv->getLights().at(i)->getType(),"spot")==0)
		{
			float angle = iv->getLights().at(i)->getAngle();
			float exp = iv->getLights().at(i)->getExponent();
			float* direction = iv->getLights().at(i)->getDirection();

			glLightf(lightId(i),GL_SPOT_CUTOFF, angle);
			glLightf(lightId(i),GL_SPOT_EXPONENT, exp);
			glLightfv(lightId(i),GL_SPOT_DIRECTION, direction);
		}
		
		if(l->getEnabled()==true)
			light->enable();
		else if(l->getEnabled()==false)
			light->disable();
		lights.push_back(light);

		glPopMatrix();
	}
}

void TPscene::chooseActiveCamera()
{
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	iv->getCameras().at(this->cameraMode)->setView();
	
}

