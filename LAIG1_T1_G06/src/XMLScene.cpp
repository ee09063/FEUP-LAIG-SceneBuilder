#include "XMLScene.h"
#include <sys/stat.h>

int exist(char *name)
{
  struct stat   buffer;
  return (stat (name, &buffer) == 0);
}

XMLScene::XMLScene(char *filename, Graph* g, InitVar *iv)
{
	doc=new TiXmlDocument( filename );
	bool loadOkay = doc->LoadFile();

	if ( !loadOkay )
	{
		printf( "COULD NOT LOAD FILE '%s'. ERROR ='%s'. EXITING...\n", filename, doc->ErrorDesc() );
		getchar();
		exit( 1 );
	}
	else {
		printf("LOADED FILE '%s'\n", filename);
	}

	TiXmlElement* anfElement= doc->FirstChildElement( "anf" );

	if (anfElement == NULL)
	{
		printf("ANF BLOCK NOT FOUND. EXITING...\n");
		getchar();
		exit(1);
	}
			
	globalElement = anfElement->FirstChildElement( "globals" );
	appElement = anfElement->FirstChildElement( "appearances" );
	textsElement =  anfElement->FirstChildElement( "textures" );
	graphElement =  anfElement->FirstChildElement( "graph" );
	nodeElement = graphElement->FirstChildElement( "node" );
	primElement =  nodeElement->FirstChildElement( "primitives" );
	cameraElement = anfElement->FirstChildElement("cameras");
	lightElement = anfElement->FirstChildElement("lights");
	
	if (globalElement == NULL){
		printf("GLOBAL BLOCK NOT FOUND. EXITING...\n");
		getchar();
		exit(-1);
	}
	/**
	//GLOBAIS COMECAM AQUI
	*/
	else
	{
		Global* gl = registerGlobalVariables(globalElement);
		iv->setGlobal(gl);
		
		/**CAMERAS HERE*/
		if(cameraElement)
		{
			char* initialCamera;
			if(cameraElement->Attribute("initial")){
				initialCamera = (char*)cameraElement->Attribute("initial");
			}
			else{
				printf("INITIAL CAMERA NOT SET. EXITING...\n");
				getchar();
				exit(-1);
			}
			iv->setInitialCamera(initialCamera);
			printf("REGISTERING CAMERAS...\n");
			registerCameras(cameraElement);
			iv->setCameras(this->cameras);
			if(iv->getCameraById(iv->getInitialCamera()) == -1){
				printf("ERROR: INITIAL CAMERA NOT FOUND. EXITING...\n");
				getchar();
				exit(-1);
			}
		}
		else{
			printf("CAMERA BLOCK NOT FOUND. EXITING...\n");
			getchar();
			exit(-1);
		}
	
		/**LIGHTS HERE**/
		if(lightElement)
		{
			printf("REGISTERING LIGHTS...\n");
			registerLights(lightElement);
			iv->setLights(this->lights);
		}
		else{
			printf("LIGHT BLOCK NOT FOUND. EXITING...\n");
			getchar();
			exit(-1);
		}
		
		/*TEXTURAS*/
		if(textsElement)
		{
			printf("REGISTERING TEXTURES...\n");
			registerTextures(textsElement);
			iv->setTextures(this->textures);
		}
		else{
			printf("TEXTURE BLOCK NOT FOUND. EXITING...\n");
			getchar();
			exit(-1);
		}
		
		/*APPEARANCES*/
		if(appElement)
		{
			printf("REGISTERING APPEARANCES...\n");
			registerApps(appElement);
			iv->setAppearances(this->appearances);
		}
		else{
			printf("APPEARANCE BLOCK NOT FOUND. EXITING...\n");
			getchar();
			exit(-1);
		}
	
		/*GRAFO*/
		if(graphElement)
		{
			printf("PARSING NODES...\n");
			char* rootid=(char*)graphElement->Attribute("rootid");
			g->setRootId(rootid);
			vector<char*> idList;
			for(TiXmlElement* node = graphElement->FirstChildElement();node;node=node->NextSiblingElement())
			{
				char* id;
				if(node->Attribute("id")){
					id = (char*)node->Attribute("id");
					if(checkRepeated(id, idList))
					{
						printf("REPEATED NODE NAME DETECTED -> '%s'", id);
						getchar();
						exit(-1);
					}
					idList.push_back(id);
				}
				else {
					printf("ERROR: NODE DOESN'T HAVE ID. EXITING...\n");
					getchar();
					exit(-1);
				}
				TiXmlElement* appNode = node->FirstChildElement("appearanceref");
				char* appearanceRef;
				if(appNode){
					appearanceRef = (char*)appNode->Attribute("id");
				}
				else{
					appearanceRef = "none";
				}

				Appearance* appearance;

				if(strcmp(appearanceRef, "none")== 0 || strcmp(appearanceRef, "inherit")==0){
					appearance = NULL;
				}
				else{
					appearance = this->getAppearanceById(appearanceRef);
				}
			
				Node* n = new Node(id, appearance, appearanceRef);
				
				TiXmlElement* primitives = node->FirstChildElement("primitives");
				if(primitives)
				{
					TiXmlElement* prim;
					/**READ ALL THE PRIMITIVES***/
					for(prim = primitives->FirstChildElement(); prim; prim = prim->NextSiblingElement())
					{
						Primitive* p = NULL;
						vector<float>f;
						/**REGISTER A RECTANGLE*/
						if(strcmp(prim->Value(),"rectangle")==0)
						{
							p = registerRectangle(prim, id);
						}
						/**REGISTER A TRIANGLE*/
						else if(strcmp(prim->Value(),"triangle")==0)
						{
							p = registerTriangle(prim,id);
						}
						/**REGISTER A CYLINDER*/
						else if(strcmp(prim->Value(),"cylinder")==0)
						{
							p = registerCylinder(prim,id);
						}
						/*REGISTER A SPHERE*/
						else if(strcmp(prim->Value(),"sphere")==0)
						{
							p = registerSphere(prim,id);
						}
						else if(strcmp(prim->Value(),"torus")==0)
						{
							p = registerTorus(prim,id);
						}
						else{
							printf("PRIMITIVE '%s' TYPE NOT FOUND IN NODE '%s'. EXITING...\n", prim->Value(), id);
							getchar();
							exit(-1);
						}
						/**NOW ADD THE PRIMITIVE TO THE NODE**/
						n->addPrim(p);
					}
				}
				/**READ ALL THE DESCENDANTS**/
				TiXmlElement* descendants = node->FirstChildElement("descendants");
				if(descendants)
				{
					TiXmlElement* des;
					for(des = descendants->FirstChildElement(); des; des = des->NextSiblingElement())
					{
						char* valString = (char*)des->Attribute("id");
						if(valString)
							n->addDes(valString);
						else
							printf("PROBLEM WITH noderef IN NODE %s", n->getId());
					}
				}
				/**READ ALL THE TRANSFORMS**/
				TiXmlElement* transforms = node->FirstChildElement("transforms");
				glPushMatrix();
				glLoadIdentity();
				float m[4][4];
				if(transforms)
				{
					TiXmlElement* tr;
					for(tr=transforms->FirstChildElement(); tr; tr=tr->NextSiblingElement())
					{
						if(strcmp(tr->Attribute("type"),"translate")==0)
						{
							char* valString=(char*)tr->Attribute("to");
							float x, y, z;
							if(valString && sscanf_s(valString,"%f %f %f",&x, &y, &z)==3)
							{
								glTranslatef(x,y,z);
							}
						}
						else if(strcmp(tr->Attribute("type"),"scale")==0)
						{
							char* valString=(char*)tr->Attribute("factor");
							float x, y, z;
							if(valString && sscanf_s(valString,"%f %f %f",&x, &y, &z)==3)
							{
								if(x==0.0)x=1.0;
								if(y==0.0)y=1.0;
								if(z==0.0)z=1.0;
								glScalef(x,y,z);
							}
						}
						else if(strcmp(tr->Attribute("type"),"rotate")==0)
						{
							char* valString=(char*)tr->Attribute("axis");
							float angle;
							if(tr->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS)
							{
								if(strcmp(valString,"x")==0){glRotatef(angle,1,0,0);}
								else if(strcmp(valString,"y")==0){glRotatef(angle,0,1,0);}
								else if(strcmp(valString,"z")==0){glRotatef(angle,0,0,1);}
							}
						}
					}
				}
				else{
					printf("TRANSFORMS NOT FOUND IN NODE '%s'. EXITING...\n",id);
					getchar();
					exit(-1);
				}
				/**RETRIEVES MATRIX**/
				glGetFloatv(GL_MODELVIEW_MATRIX, &m[0][0]);
				n->setMatrix(m);
				glPopMatrix();
				/**NOW ADD THE NODE TO THE GRAPH**/
				g->addNode(n);
			}
			printf("Number of nodes -> '%d'\n", g->nodeSize());
			/**PLACE ROOT NODE IN GRAPH**/
			Node* root = g->getNode(g->getRootId());
			if(NULL == root){
				printf("ROOT NODE NOT FOUND. EXITING...\n");
				getchar();
				exit(-1);
			}
			else{
				g->setRootNode(root);
			}
		}
		else{
			printf("GRAPH BLOCK NOT FOUND. EXITING...\n");
			getchar();
			exit(-1);
		}
	}
	postProcess(iv, g);
}

XMLScene::~XMLScene()
{
	delete(doc);
}

TiXmlElement *XMLScene::findChildByAttribute(TiXmlElement *parent,const char * attr, const char *val)
// Searches within descendants of a parent for a node that has an attribute _attr_ (e.g. an id) with the value _val_
// A more elaborate version of this would rely on XPath expressions
{
	TiXmlElement *child=parent->FirstChildElement();
	int found=0;

	while (child && !found)
		if (child->Attribute(attr) && strcmp(child->Attribute(attr),val)==0)
			found=1;
		else
			child=child->NextSiblingElement();

	return child;
}

Primitive* registerRectangle(TiXmlElement* prim, char* id)
{
	Primitive* p = new Rectangle();
	vector<float>f;
	float x1, x2, y1, y2;
	char* valString=NULL;
	char* valString2=NULL;
	valString = (char*)prim->Attribute("xy1");
	valString2 = (char*)prim->Attribute("xy2");
	if(valString && sscanf_s(valString,"%f %f",&x1, &y1)==2
		&& valString2 && sscanf_s(valString2,"%f %f",&x2, &y2)==2)
	{
		f.push_back(x1); f.push_back(y1); f.push_back(x2); f.push_back(y2);
		p->setValues(f);
		return p;
	}
	else{
		printf("ERROR PARSING RECTANGLE PRIMITIVE IN NODE '%s'. EXITING...\n", id);
		getchar();
		exit(-1);
	}
	return NULL;
}

Primitive* registerTriangle(TiXmlElement* prim, char* id)
{
	Primitive* p=new Triangle();
	float x1, x2 ,x3, y1, y2, y3, z1, z2, z3;
	vector<float>f;
	char* valString=(char*)prim->Attribute("xyz1");
	char* valString2=(char*)prim->Attribute("xyz2");
	char* valString3=(char*)prim->Attribute("xyz3");
	if(valString && sscanf_s(valString,"%f %f %f",&x1, &y1, &z1)==3
		&& valString2 && sscanf_s(valString2,"%f %f %f",&x2, &y2, &z2)==3
		&& valString3 && sscanf_s(valString3, "%f %f %f", &x3, &y3, &z3)==3)
	{
		f.push_back(x1); f.push_back(x2); f.push_back(x3); 
		f.push_back(y1); f.push_back(y2); f.push_back(y3); 
		f.push_back(z1); f.push_back(z2); f.push_back(z3);
		p->setValues(f);
		return p;
	}
	else{
		printf("ERROR PARSING TRIANGLE PRIMITIVE IN NODE '%s'. EXITING...\n", id);
		getchar();
		exit(-1);
	}
	return NULL;
}

Primitive* registerCylinder(TiXmlElement* prim, char* id)
{
	Primitive* p=new Cylinder();
	float base, top, height, slices, stacks;
	vector<float>f;
	if(prim->QueryFloatAttribute("base",&base)==TIXML_SUCCESS &&
		prim->QueryFloatAttribute("top",&top)==TIXML_SUCCESS &&
		prim->QueryFloatAttribute("height",&height)==TIXML_SUCCESS &&
		prim->QueryFloatAttribute("slices",&slices)==TIXML_SUCCESS &&
		prim->QueryFloatAttribute("stacks",&stacks)==TIXML_SUCCESS)
	{
		f.push_back(base); f.push_back(top); f.push_back(height);
		f.push_back(slices); f.push_back(stacks);
		p->setValues(f);
		return p;
	}
	else{
		printf("ERROR PARSING CYLINDER PRIMITIVE IN NODE '%s'. EXITING...\n", id);
		getchar();
		exit(-1);
	}
	return NULL;
}

Primitive* registerSphere(TiXmlElement* prim, char* id)
{
	Primitive* p = new Sphere();
	vector<float>f;
	float radius, stacks, slices;
	if(prim->QueryFloatAttribute("radius",&radius)==TIXML_SUCCESS &&
	   prim->QueryFloatAttribute("slices",&slices)==TIXML_SUCCESS &&
	   prim->QueryFloatAttribute("stacks",&stacks)==TIXML_SUCCESS)
	{
		f.push_back(radius); f.push_back(slices); f.push_back(stacks);
		p->setValues(f);
		return p;
	}
	else{
		printf("ERROR PARSING SPHERE PRIMITIVE IN NODE '%s'. EXITING...\n", id);
		getchar();
		exit(-1);
	}
	return NULL;
}

Primitive* registerTorus(TiXmlElement* prim, char* id)
{
	Primitive* p = new Torus();
	vector<float>f;
	float inner, outer, slices, loops;
	if(prim->QueryFloatAttribute("inner",&inner)==TIXML_SUCCESS &&
	   prim->QueryFloatAttribute("outer",&outer)==TIXML_SUCCESS &&
	   prim->QueryFloatAttribute("slices",&slices)==TIXML_SUCCESS &&
	   prim->QueryFloatAttribute("loops",&loops)==TIXML_SUCCESS)
	{
		f.push_back(inner); f.push_back(outer); f.push_back(slices);
		f.push_back(loops);
		p->setValues(f);
		return p;
	}
	else{
		printf("ERROR PARSING TORUS PRIMITVE IN NODE '%s'. EXITING...\n", id);
		getchar();
		exit(-1);
	}
	return NULL;
}

Global* XMLScene::registerGlobalVariables(TiXmlElement* globalElement)
{
	Global* gl = new Global();
	printf("Processing global variables:\n");
	/*DRAWING ELEMENT*/
	char* drawingMode=NULL;
	char* shading=NULL;
	float* background = (float*)malloc(sizeof(float)*4);
	/**CULLING ELEMENT**/
	/**USUALLY IS CCW**/
	char* face=NULL;
	char* order=NULL;
	/*LIGHTING ELEMENT*/
	bool local;
	bool enabled;
	bool doublesided;
	float* ambientLight = (float*)malloc(sizeof(float)*4);
	
	/*READING DRAWING ELEMENTS*/
	TiXmlElement* drawingElement=globalElement->FirstChildElement("drawing");
	if (drawingElement)
	{
		if(drawingElement->Attribute("mode")){
			drawingMode=(char*)drawingElement->Attribute("mode");
		}
		else{
			printf("ERROR: ATTRIBUTE MODE NOT FOUND IN DRAWING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
		if(drawingElement->Attribute("shading")){
			shading = (char*)drawingElement->Attribute("shading");
		}
		else{
			printf("ERROR: ATTRIBUTE SHADING NOT FOUND IN DRAWING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
		char *valString=NULL;
		float backA, backB, backC, backD;
		
		if(drawingElement->Attribute("background")){
			valString=(char *) drawingElement->Attribute("background");
			
			if(valString && sscanf_s(valString,"%f %f %f %f",&backA, &backB, &backC, &backD)==4)
			{
				background[0]=backA; background[1]=backB; background[2]=backC; background[3]=backD;
			}
			else{
				printf("ERROR PARSING BACKGROUND. EXITING...\n");
				getchar();
				exit(-1);
			}
		}
		else{
			printf("ERROR: ATTRIBUTE BACKGROUND NOT FOUND IN DRAWING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
	}
	else{
		printf("ERROR: DRAWING SECTION NOT FOUND. EXITING...\n");
		getchar();
		exit(-1);
	}

	/**READING CULLING ELEMENTS*/
	TiXmlElement* cullingElement = globalElement->FirstChildElement("culling");
	if(cullingElement)
	{
		printf("Processing culling elements\n");
		if(cullingElement->Attribute("face")){
			face = (char*)cullingElement->Attribute("face");
		}
		else{
			printf("ERROR: ATTRIBUTE FACE NOT FOUND IN CULLING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
		if(cullingElement->Attribute("order")){
			order = (char*)cullingElement->Attribute("order");
		}
		else{
			printf("ERROR: ATTRIBUTE ORDER NOT FOUND IN CULLING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
	}
	else{
		printf("ERROR: CULLING SECTION NOT FOUND. EXITING...\n");
		getchar();
		exit(-1);
	}

	/**READING LIGHT ELEMENTS*/
	TiXmlElement* lightingElement = globalElement->FirstChildElement("lighting");
	if(lightingElement)
	{
		printf("Processing lighting elements\n");
		float ambA, ambB, ambC, ambD;

		char *valString=NULL;

		char* sdoublesided;
		if(lightingElement->Attribute("doublesided")){
			sdoublesided = (char*)lightingElement->Attribute("doublesided");
			if(strcmp(sdoublesided, "true")==0){doublesided=true;}
			else if(strcmp(sdoublesided, "false")==0){doublesided=false;}
			else{
				printf("ERROR: NOT ACCEPTED VALUES IN ATTRIBUTE DOUBLESIDED. EXITING...\n");
				getchar();
				exit(-1);
			}
		}
		else{
			printf("ERROR: ATTRIBUTE DOUBLESIDED NOT FOUND IN LIGHTING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}

		char* slocal;
		if(lightingElement->Attribute("local")){
			slocal= (char*)lightingElement->Attribute("local");
			if(strcmp(slocal, "true")==0){local=true;}
			else if(strcmp(slocal, "false")==0){local=false;}
			else{
				printf("ERROR: NOT ACCEPTED VALUES IN ATTRIBUTE LOCAL. EXITING...\n");
				getchar();
				exit(-1);
			}
		}
		else{
			printf("ERROR: ATTRIBUTE LOCAL NOT FOUND IN LIGHTING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
		
		char* senabled;
		if(lightingElement->Attribute("enabled")){
			senabled = (char*)lightingElement->Attribute("enabled");
			if(strcmp(senabled, "true")==0){enabled=true;}
			else if(strcmp(senabled, "false")==0){enabled=false;}
			else{
				printf("ERROR: NOT ACCEPTED VALUES IN ATTRIBUTE ENABLED. EXITING...\n");
				getchar();
				exit(-1);
			}
		}
			else{
			printf("ERROR: ATTRIBUTE ENABLED NOT FOUND IN LIGHTING SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}

		
		valString = (char*)lightingElement->Attribute("ambient");
		
		if(valString && sscanf_s(valString,"%f %f %f %f",&ambA, &ambB, &ambC, &ambD)==4)
		{
			ambientLight[0]=ambA;
			ambientLight[1]=ambB;
			ambientLight[2]=ambC;
			ambientLight[3]=ambD;
		}
		else{
			printf("ERROR PARSING AMBIENT ATRIBUTTE IN GLOBAL ELEMENTS. EXITING...\n");
			getchar();
			exit(-1);
		}
	}
	else{
		printf("LIGHTING SECTION NOT FOUND. EXITING...\n");
		getchar();
		exit(-1);
	}

	/**NOW TO FINALLY REGISTER THE GLOBAL ELEMENTS**/
	printf("REGISTERING GLOBAL VARIABLES\n");
	gl->setDrawingElements(drawingMode, shading, background);
	gl->setCullingElements(face, order);
	gl->setLigtingElements(doublesided, local, enabled, ambientLight);
	
	return gl;
}

void XMLScene::registerCameras(TiXmlElement* cameraElement)
{
	vector<char*> idList;
	for(TiXmlElement *cam = cameraElement->FirstChildElement(); cam; cam = cam->NextSiblingElement())
	{
		Camera* c = NULL;
		char* cameraId;
		if(cam->Attribute("id")){
			cameraId = (char*)cam->Attribute("id");
		}
		else{
			printf("ERROR: ATRIBUTTE ID MISSING IN CAMERA SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}
		if(checkRepeated(cameraId, idList))
		{
			printf("REPEATED CAMERA NAME DETECTED -> '%s'. EXITING...\n", cameraId);
			getchar();
			exit(-1);
		}
		idList.push_back(cameraId);

		if(strcmp(cam->Value(), "perspective")==0)
		{
			char* valString=(char*)cam->Attribute("pos");
			char* valString2=(char*)cam->Attribute("target");
			float near, far, angle, posx, posy, posz, targx, targy, targz;
			if (cam->QueryFloatAttribute("near",&near)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("far",&far)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS &&
				valString && sscanf_s(valString,"%f %f %f",&posx, &posy, &posz)==3 &&
				valString2 && sscanf_s(valString2,"%f %f %f",&targx, &targy, &targz)==3)
			{
				c = new CamPerspective(cameraId);
				vector<float>values;
				values.push_back(near); values.push_back(far); values.push_back(angle);
				values.push_back(posx); values.push_back(posy); values.push_back(posz);
				values.push_back(targx); values.push_back(targy); values.push_back(targz);
				c->setValues(values);
				cameras.push_back(c);
			}
			else{
				printf("ERROR PARSING ATTRIBUTES OF CAMERA '%s'. EXITING...\n", cam->Attribute("id"));
				getchar();
				exit(-1);
			}
					
		}
		else if(strcmp(cam->Value(), "ortho")==0)
		{
			float near, far, right, left, top, bottom, direction;
			char* dir=(char*)cam->Attribute("direction");
			if (cam->QueryFloatAttribute("near",&near)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("far",&far)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("right",&right)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("left",&left)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("top",&top)==TIXML_SUCCESS &&
				cam->QueryFloatAttribute("bottom",&bottom)==TIXML_SUCCESS &&
				dir)
			{
				c = new CamOrtho(cameraId);
				vector<float>values;
				
				if(strcmp("x", dir)==0) direction=0;
				else if(strcmp("y", dir)==0) direction=1;
				else if(strcmp("z", dir)==0) direction=2;
				else{
					printf("ERROR: WRONG VALUE IN DIRECTION FOR CAMERA '%s'. EXITING...\n", cam->Attribute("id"));
					getchar();
					exit(-1);
				}

				values.push_back(near); values.push_back(far); values.push_back(left);
				values.push_back(right); values.push_back(top); values.push_back(bottom);
				values.push_back(direction);
				c->setValues(values);
				this->cameras.push_back(c);
			}
			else{
				printf("ERROR PARSING ATTRIBUTES OF CAMERA '%s'. EXITING...\n", cam->Attribute("id"));
				getchar();
				exit(-1);
			}
		}
		else{
				printf("ERROR PARSING TYPE OF CAMERA '%s'. EXITING...\n", cam->Attribute("id"));
				getchar();
				exit(-1);
		}
	}
	printf("Number of cameras -> '%d'\n", cameras.size());
	if(cameras.size() == 0){
		printf("ERROR: CAMERA BLOCK IS EMPTY. EXITING...\n");
		getchar();
		exit(-1);
	}
}

void XMLScene::registerLights(TiXmlElement* lightElement)
{
	vector<char*> idList;
	int lightCount = 0;
	for(TiXmlElement *l = lightElement->FirstChildElement(); l; l=l->NextSiblingElement())
	{
		if(lightCount > 7)
		{
			printf("ERROR: ONLY 8 LIGHTS SUPPORTED. EXITING...\n");
			getchar();
			exit(1);
		}
		Light* light;
		char* id;
		if(l->Attribute("id")){
			id=(char*)l->Attribute("id");
		}
		else{
			printf("ERROR: ID NOT FOUND IN LIGHT. EXITING...\n");
			getchar();
			exit(-1);
		}
		if(checkRepeated(id, idList))
		{
			printf("REPEATED LIGHT NAME DETECTED -> '%s'", id);
			getchar();
			exit(-1);
		}
		idList.push_back(id);

		char* type;
		if(l->Attribute("type")){
			type=(char*)l->Attribute("type");
		}
		else{
			printf("ERROR: TYPE NOT FOUND IN LIGHT '%s'. EXITING...\n",id);
			getchar();
			exit(-1);
		}
		bool enabled=NULL;
		bool marker=NULL;
		
		if(l->Attribute("enabled")){
			if(strcmp("true",(char*)l->Attribute("enabled"))==0){enabled=true;}
			else if(strcmp("false",(char*)l->Attribute("enabled"))==0){enabled=false;}
			else{
				printf("ERROR: WRONG VALUE IN ENABLED ATRIBUTTE IN LIGHT '%s'. EXITING...", id);
				getchar();
				exit(-1);
			}
		}
		else{
			printf("ERROR: ATRIBUTTE ENABLED NOT FOUND IN LIGHT '%s'. EXITING...",id);
			getchar(),
			exit(-1);
		}
					
		if(l->Attribute("marker")){
			if(strcmp("true",(char*)l->Attribute("marker"))==0){marker=true;}
			else if(strcmp("false",(char*)l->Attribute("marker"))==0){marker=false;}
			else{
				printf("ERROR: WRONG VALUE IN MARKER ATRIBUTTE IN LIGHT '%s'. EXITING...", id);
				getchar();
				exit(-1);
			}
		}
		else{
			printf("ERROR: ATRIBUTTE MARKER NOT FOUND IN LIGHT '%s'. EXITING...",id);
			getchar(),
			exit(-1);
		}

		char* valString;
		float locx, locy, locz;
		vector<float>location;

		valString=(char*)l->Attribute("pos");
		if (valString && sscanf_s(valString,"%f %f %f",&locx, &locy, &locz)==3)
		{
			location.push_back(locx); location.push_back(locy); location.push_back(locz);
		}
		else{
			printf("ERROR PARSING POS ATTRIBUTE IN LIGHT '%s'. EXITING...", id);
			getchar();
			exit(-1);
		}
		
		float* ambient = (float*)malloc(sizeof(float)*4);
		float* diffuse = (float*)malloc(sizeof(float)*4);
		float* specular = (float*)malloc(sizeof(float)*4);
		/**COMPONENTS**/
		bool a = false, d = false, s = false;
		for(TiXmlElement* c = l->FirstChildElement(); c; c=c->NextSiblingElement())
		{
			float cA, cB, cC, cD;
			if(strcmp(c->Attribute("type"), "ambient")==0)
			{
				valString = (char*)c->Attribute("value");
				if (valString && sscanf_s(valString,"%f %f %f %f",&cA, &cB, &cC, &cD)==4)
				{
					ambient[0]=cA; ambient[1]=cB; ambient[2]=cC; ambient[3]=cD;
					a = true;

				}
				else{
					printf("ERROR PARSING AMBIENT ATTRIBUTE IN LIGHT '%s'. EXITING...", id);
					getchar();
					exit(-1);
				}
			}
			else if(strcmp(c->Attribute("type"), "diffuse")==0)
			{
				valString = (char*)c->Attribute("value");
				if (valString && sscanf_s(valString,"%f %f %f %f",&cA, &cB, &cC, &cD)==4)
				{
					diffuse[0]=cA; diffuse[1]=cB; diffuse[2]=cC; diffuse[3]=cD;
					d = true;
				}
				else{
					printf("ERROR PARSING DIFFUSE ATTRIBUTE IN LIGHT '%s'. EXITING...", id);
					getchar();
					exit(-1);
				}
			}
			else if(strcmp(c->Attribute("type"), "specular")==0)
			{
				valString = (char*)c->Attribute("value");
				if (valString && sscanf_s(valString,"%f %f %f %f",&cA, &cB, &cC, &cD)==4)
				{
					specular[0]=cA; specular[1]=cB; specular[2]=cC; specular[3]=cD;
					s = true;
				}
				else{
					printf("ERROR PARSING SPECULAR ATTRIBUTE IN LIGHT '%s'. EXITING...", id);
					getchar();
					exit(-1);
				}
			}
			else{
				printf("ERROR PARSING COMPONENT IN LIGHT '%s'. NAME NOT FOUND. EXITING...", id);
				getchar();
				exit(-1);
			}
		}
		if(a && s && d){
			}
		else {
			printf("ERROR: COMPONENT MISSING IN LIGHT '%s'. EXITING...\n", id);
			getchar();
			exit(-1);
		}

		if(strcmp(type,"omni")==0)
		{
			float* direction = NULL;
			light = new Light(id, "omni", enabled, marker, 0.0, 0.0, location, direction, ambient, diffuse, specular);
			this->lights.push_back(light);
			
		}
		else if(strcmp(type,"spot")==0)
		{
			float angle;
			float exponent;
			char* valString = (char*)l->Attribute("target");
			float dA, dB, dC;
			float* direction = (float*)malloc(sizeof(float)*3);
			if(l->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS &&
			   l->QueryFloatAttribute("exponent",&exponent)==TIXML_SUCCESS && 
			   valString && sscanf_s(valString,"%f %f %f",&dA, &dB, &dC)==3)
			{
				direction[0]=dA; direction[1]=dB; direction[2]=dC;
			}
			else printf("PROBLEM READING TARGET, ANGLE OR EXPONENT IN LIGHT '%s'\n", id);

			light=new Light(id, "spot", enabled, marker, angle, exponent, location, direction, ambient, diffuse, specular);
			this->lights.push_back(light);
		}
		else{
			printf("ERROR PARSING LIGHT TYPE ATTRIBUTE IN LIGHT '%s'. EXITING...", id);
			getchar();
			exit(-1);
		}
		lightCount++;
	}
	printf("Number of lights -> '%d'\n", lights.size());
}

void XMLScene::registerTextures(TiXmlElement* textsElement)
{
	vector<char*> idList;
	for(TiXmlElement *tex = textsElement->FirstChildElement(); tex; tex = tex->NextSiblingElement())
	{
		char* id;
		if(tex->Attribute("id")){
			id = (char*)tex->Attribute("id");
		}
		else{
			printf("ERROR: ATTRIBUTE ID MISSING IN TEXTURE SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}

		if(checkRepeated(id, idList))
		{
			printf("REPEATED NAME DETECTED IN TEXTURES -> '%s'", id);
			getchar();
			exit(-1);
		}
		idList.push_back(id);
		
		char* file;
		if(tex->Attribute("file")){
			file=(char*)tex->Attribute("file");
			if(!exist(file)){
				printf("ERROR: FILE '%s' DOES NOT EXIST -> TEXTURE '%s'. EXITING...\n", file,id);
				getchar();
				exit(-1);
			}
		}
		else{
			printf("ATRIBUTTE FILE DOES NOT EXIST IN TEXTURE '%s'. EXITING...\n", id);
			getchar();
			exit(-1);
		}

		float lengthT, lengthS;
		if(tex->QueryFloatAttribute("texlength_s",&lengthS)==TIXML_SUCCESS &&
			tex->QueryFloatAttribute("texlength_t",&lengthT)==TIXML_SUCCESS)
		{
			Texture* t = new Texture(id, file, lengthS, lengthT);
			this->textures.push_back(t);
		}
		else{
			printf("ERROR PARSING LENGHT T AND S IN TEXTURE '%s'. EXITING...\n", id);
			getchar();
			exit(-1);
		}
	}
	printf("Number of textures -> '%d'\n", textures.size());
}

void XMLScene::registerApps(TiXmlElement* appElement){
	vector<char*> idList;

	for(TiXmlElement *app = appElement->FirstChildElement(); app; app=app->NextSiblingElement()){
		
		char* id;
		if(app->Attribute("id")){
			id = (char*)app->Attribute("id");
		}
		else{
			printf("ERROR: ATTRIBUTE ID MISSING IN TEXTURE SECTION. EXITING...\n");
			getchar();
			exit(-1);
		}

		if(checkRepeated(id, idList)){
			printf("REPEATED APPEARANCE NAME DETECTED -> '%s'", id);
			getchar();
			exit(-1);
		}

		idList.push_back(id);

		char* textureref="";
		Texture* texture;
		if(app->Attribute("textureref"))
		{
			textureref=(char*)app->Attribute("textureref");
			texture = this->getTextureById(textureref);
		}
		else if(strcmp(textureref, "")==0)
		{
			texture = new Texture("", "", 1.0, 1.0);
		}

		float* ambient = (float*)malloc(sizeof(float)*4);
		float* diffuse = (float*)malloc(sizeof(float)*4);
		float* specular = (float*)malloc(sizeof(float)*4);
		char* valString = NULL;
		float shininess;
		
		if(app->QueryFloatAttribute("shininess",&shininess)==TIXML_SUCCESS){}
		else{
			printf("ERROR PARSING SHININESS IN APPEARANCE '%s'. EXITING...\n", id);
			getchar();
			exit(-1);
		}

		bool a = false, d = false, s = false;
		for(TiXmlElement* c = app->FirstChildElement(); c; c=c->NextSiblingElement())
		{
			float cA, cB, cC, cD;
			if(strcmp(c->Attribute("type"), "ambient")==0){
				valString = (char*)c->Attribute("value");
				if (valString && sscanf_s(valString,"%f %f %f %f",&cA, &cB, &cC, &cD)==4){
					ambient[0]=cA; ambient[1]=cB; ambient[2]=cC; ambient[3]=cD;
					a = true;
				}
				else{
					printf("ERROR PARSING AMBIENT IN APPEARANCE '%s'. EXITING...\n", id);
					getchar();
					exit(-1);
				}
			}
			else if(strcmp(c->Attribute("type"), "diffuse")==0){
				valString = (char*)c->Attribute("value");
				if (valString && sscanf_s(valString,"%f %f %f %f",&cA, &cB, &cC, &cD)==4){
					diffuse[0]=cA; diffuse[1]=cB; diffuse[2]=cC; diffuse[3]=cD;
					d = true;
				}
				else{
					printf("ERROR PARSING DIFFUSE IN APPEARANCE '%s'. EXITING...\n", id);
					getchar();
					exit(-1);
				}
			}
			else if(strcmp(c->Attribute("type"), "specular")==0){
				valString = (char*)c->Attribute("value");
				if (valString && sscanf_s(valString,"%f %f %f %f",&cA, &cB, &cC, &cD)==4){
					specular[0]=cA; specular[1]=cB; specular[2]=cC; specular[3]=cD;
					s = true;
				}
				else{
					printf("ERROR PARSING SPECULAR IN APPEARANCE '%s'. EXITING...\n", id);
					getchar();
					exit(-1);
				}
			}
		}
		if(a && s && d){
			}
		else {
			printf("ERROR: COMPONENT MISSING IN APPEARANCE '%s'. EXITING...\n", id);
			getchar();
			exit(-1);
		}
		Appearance* appearance = new Appearance(id,texture,shininess,ambient,diffuse,specular);
		this->appearances.push_back(appearance);
	}
	printf("Number of appearances -> '%d'\n", this->appearances.size());
}

void postProcess(InitVar* iv, Graph *g)
{
	printf("BEGIN GRAPH POST PROCESSING...\n");
	/**VERIFY CYCLES IN GRAPH*/
	g->DFS(g->getRootNode());
	printf("GRAPH POST PROCESSING ENDED...\n");
}

bool checkRepeated(char* id, vector<char*> idList)
{
	for(unsigned int i = 0; i < idList.size(); i++)
	{
		if(strcmp(id, idList.at(i))==0)
			return true;
	}
	return false;
}

char* getParentApp(Node* n, Graph* g)
{
	int size = g->getNodes().size();
	char* nodeId = n->getId();
	for(int i = 0; i < size; i++)
	{
		Node* parent = g->getNodes().at(i);
		for(unsigned int k = 0; k < parent->getDescendants().size(); k++)
		{
			char* currNode = parent->getDescendants().at(k);
			if(strcmp(nodeId, currNode)==0)
			{
				return parent->getNodeApp()->getId();
			}
		}
	}
	return NULL;
}

Texture* XMLScene::getTextureById(char* id)
{
	for(unsigned int i = 0; i < this->textures.size(); i++){
		if(strcmp(this->textures.at(i)->getId(), id)==0){
			return this->textures.at(i);
		}
	}
	printf("ERROR: TEXTURE '%s' NOT FOUND...\n", id);
	getchar();
	exit(-1);
}

Appearance* XMLScene::getAppearanceById(char* id)
{
	for(unsigned int i = 0; i < this->appearances.size(); i++){
		if(strcmp(this->appearances.at(i)->getId(), id)==0){
			return this->appearances.at(i);
		}
	}
	printf("ERROR: APPEARANCE '%s' NOT FOUND...\n", id);
	getchar();
	exit(-1);
}