#include "SceneGraph.h"
#include<numeric>

#define PI 3.14159265358979323846

void Node::printNodePrimitives()
{
	for(unsigned int i = 0; i < this->primitives.size(); i++)
	{
		Primitive* p = getPrimitives().at(i);
		printf("%s\n", p->getType());
	}
}

void Graph::printNodePrimitives()
{
	for(int i = 0; i < this->nodeSize(); i++)
	{
		this->nodes.at(i)->printNodePrimitives();
	}
}

void Rectangle::draw(float texS, float texT)
{
	float x1, x2, y1, y2;
	x1 = v.at(0); x2 = v.at(2);
	y1 = v.at(1); y2 = v.at(3);

	float _xDim = abs(x2 - x1);
	float _yDim = abs(y2 - y1);

	glBegin(GL_POLYGON);
		glTexCoord2f(0,0);
		glNormal3f(0,0,1);
		glVertex2f(x1, y1);

		glTexCoord2f(_xDim/texS,0);
		glNormal3f(0,0,1);
		glVertex2f(x2, y1);

		glTexCoord2f(_xDim/texS, _yDim/texT);
		glNormal3f(0,0,1);
		glVertex2f(x2, y2);

		glTexCoord2f(0, _yDim/texT);
		glNormal3f(0,0,1);
		glVertex2f(x1, y2);
	glEnd();
	/*
	if(x1 > x2 && y1 > y2)//switch
	{
		float tempx1, tempy1;
		tempx1 = x1; tempy1 = y1;
		x1 = x2; y1 = y2;
		x2 = tempx1; y2 = tempy1;
	}

	//printf("%f %f %f %f\n\n", x1, y1, x2, y2);

	int _numDivisions = 100;

	float texMultiS, texMultiT;
	texMultiS = _xDim / texS;// / _xDim;
	texMultiT = _yDim / texT;// / _yDim;

	//glRectf(x1, y1, x2, y2);
	
	glPushMatrix();
		glTranslatef(x1, y1, 0);
		glRotatef(-90.0,1,0,0);
		glScalef( _xDim * (1.0/(double) _numDivisions), 1 , _yDim * (1.0/(double) _numDivisions));	
		glNormal3f(0,-1,0);

		for (int bx = 0; bx<_numDivisions; bx++)
		{
			glBegin(GL_TRIANGLE_STRIP);

				glTexCoord2f((bx * 1.0/_numDivisions) * texMultiS, 0.0 * texMultiT);
				glVertex3f(bx, 0, 0);

				for (int bz = 0; bz<_numDivisions; bz++)
				{
					glTexCoord2f(((bx+1) * 1.0/_numDivisions) * texMultiS, (bz * 1.0/_numDivisions) * texMultiT);
					glVertex3f((bx + 1), 0, bz);
					
					glTexCoord2f(((bx+1) * 1.0/_numDivisions) * texMultiS, ((bz+1) * 1.0/_numDivisions) * texMultiT);
					glVertex3f(bx, 0, (bz + 1));

				}
				glTexCoord2f(((bx+1) * 1.0/_numDivisions) * texMultiS, 1.0 * texMultiT);
				glVertex3d((bx+1), 0, _numDivisions);	
	
			glEnd();
		}
	glPopMatrix();
	*/
}

void Cylinder::draw(float texS, float texT)
{
	GLUquadric* quad = gluNewQuadric();
	gluQuadricDrawStyle(quad, GLU_FILL);
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluQuadricTexture(quad, GL_TRUE);
	/*BASE*/
	glPushMatrix();
	glRotatef(180.0f, 1, 0, 0);
	gluDisk(quad, 0, base, this->slices, this->stacks);
	glPopMatrix();
	/*TOP*/
	glPushMatrix();
	glTranslatef(0.0, 0.0, this->height);
	gluDisk(quad, 0, top, this->slices, this->stacks);
	glPopMatrix();
	/*CYLINDER*/
	gluCylinder(quad, this->base, this->top, this->height, this->slices, this->stacks);
}

void Sphere::draw(float texS, float texT)
{
	glPushMatrix();
	//	glRotatef(-90, 1, 0, 0);
		GLUquadric* quad = gluNewQuadric();
		gluQuadricDrawStyle(quad, GLU_FILL);
		gluQuadricNormals(quad, GLU_SMOOTH);
		gluQuadricTexture(quad, GL_TRUE);
		gluSphere(quad, this->radius, this->slices, this->stacks);
	glPopMatrix();
}

void Torus :: draw(float texS, float texT)
{
 	//glutSolidTorus(this->inner, this->outer, this->slices, this->loops);
	int numt = (int)this->loops;
	int numc = (int)this->slices;

	int i, j, k;
	double s, t, x, y, z,x_norm,y_norm,z_norm,norm, twopi;
 
	twopi = 2 * PI;
	for (i = 0; i < numc; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= numt*1; j++)
		{
			x_norm=(outer+inner*cos((i+1)*twopi/numc))*cos((j+0.5)*twopi/numt)-outer*cos((j+0.5)*twopi/numt);
			y_norm=(outer+inner*cos((i+1)*twopi/numc))*sin((j+0.5)*twopi/numt)-outer*sin((j+0.5)*twopi/numt);
			z_norm=inner * sin((i+1)*twopi / numc);
			norm = sqrt(x_norm*x_norm+y_norm*y_norm+z_norm*z_norm);
			x_norm=x_norm/norm;
			y_norm=y_norm/norm;
			z_norm=z_norm/norm;
 
			glNormal3f(x_norm,y_norm,z_norm);
 
			for (k = 1; k >= 0; k--)
			{
				s = (i + k) % numc + 0.5;
				t = j % numt;
 
				x =  (outer+inner*cos(s*twopi/numc))*cos(t*twopi/numt);
				y =  (outer+inner*cos(s*twopi/numc))*sin(t*twopi/numt);
				z =  (inner * sin(s * twopi / numc));

				float texs = (i + k) / (float)numc;
				float text = (j % (numt+1)) / (float)numt;
				
				glTexCoord2f(text,texs);
				glVertex3f(x, y, z);
			}
		}
		glEnd();
	}
}

float distance(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z)
{
	return sqrtf( powf(p2x-p1x,2) + powf(p2y-p1y,2) + powf(p2z-p1z,2));
}

float* divideByScalar(float* v, float distance)
{
	for(int i = 0; i < 2; i++)
	{
		v[i] = v[i] / distance;
	}

	return v;
}

double scalar_product(float* a, float* b)
{
    double product = 0;
    for (int i = 0; i <= 2; i++)
       product += (a[i])*(b[i]);
    return product;
}

void Triangle::draw(float texS, float texT)
{
	float x1, x2, x3, y1, y2, y3, z1, z2, z3;
	
	x1 = v.at(0); x2 = v.at(1); x3=v.at(2);
	y1 = v.at(3); y2 = v.at(4); y3=v.at(5);
	z1 = v.at(6); z2 = v.at(7); z3=v.at(8);
	/**NORMALS**/
	float Ux, Uy, Uz, Vx, Vy, Vz;
	Ux = x2 - x1; Uy = y2 - y1; Uz = z2 - z1;
	Vx = x3 - x1; Vy = y3 - y1; Vz = z3 - z1;

	float Nx = (Uy*Vz) - (Uz*Vy);
	float Ny = (Uz*Vx) - (Ux*Vz);
	float Nz = (Ux*Vy) - (Uy*Vx);

	/**TEXTURES**/
	float AB = distance(x1, y1, z1, x2, y2, z2);
	float AC = distance(x1, y1, z1, x3, y3, z3);

	float vectorAB[3] = {x2-x1, y2-y1, z2-z1};
	float vectorAC[3] = {x3-x1, y3-y1, z3-z1};

	float* ab = divideByScalar(vectorAB, AB);
	float* ac = divideByScalar(vectorAC, AC);

	float alpha = acos(scalar_product(ab,ac));

	float CD = AC * sin(alpha);
	float AD = AC * cos(alpha);

	glBegin(GL_TRIANGLES);
		
		glTexCoord2f(0,0);
		glNormal3f(Nx, Ny, Nz);
		glVertex3d(x1,y1,z1);

		glTexCoord2f(AB/texS, 0);
		glNormal3f(Nx, Ny, Nz);
		glVertex3d(x2,y2,z2);

		glTexCoord2f(AD/texS, CD/texT);
		glNormal3f(Nx, Ny, Nz);
		glVertex3d(x3,y3,z3);
	
	glEnd();
}

Texture* InitVar::getTexById(char* id)
{
	for(unsigned int i = 0; i < this->textures.size(); i++)
	{
		if(strcmp(id, this->textures.at(i)->getId())==0)
			return this->textures.at(i);
	}
	return NULL;
}

Appearance* InitVar::getAppById(char* id)
{
	for(unsigned int i = 0; i < this->appearances.size(); i++)
	{
		if(strcmp(id, this->appearances.at(i)->getId())==0)
			return this->appearances.at(i);
	}
	return NULL;
}

int InitVar::getCameraById(char* id)
{
	unsigned int i;
	for(i=0; i < this->cameras.size(); i++)
		if(strcmp(id, this->cameras.at(i)->getId()) == 0)
			return i;
	return -1;
}

void CamPerspective::setView()
{
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	float aspect =  (float)width / (float)height;
	gluPerspective(angle, aspect, near, far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(posx, posy, posz,
				targx, targy, targz,
				0.0, 1.0, 0.0);
	/*gluLookAt(10.0, 10.0, 10.0,
				0.0, 0.0, 0.0,
				0.0, 1.0, 0.0);*/
}

void CamOrtho::setView()
{
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	float aspect =  (float)width / (float)height;

	glOrtho(left, right, bottom, top, near, far);

	/*
	if(width <= height)
		glOrtho(left, right, bottom/aspect, top/aspect, near, far);
	else
		glOrtho(left*aspect, right*aspect, bottom, top, near, far);
		*/
	//glOrtho(-12.0f,12,-12.0,12.0f,-10.0f,10.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(direction == 0){
		glRotatef(-90, 0 , 1, 0); //x
	}
	else if(direction == 1){
		glRotatef(90, 1, 0, 0); //y
	}
}

void Graph::DFS(Node* n)
{
	if(n->getVisited()==true)
	{
		printf("ERROR: CYCLE DETECTED IN NODE '%s'\n", n->getId());
		getchar(); exit(-1);
	}
	else
		n->setVisited(true);
	for(unsigned int i = 0; i < n->getDescendants().size(); i++)
	{
		char* d = n->getDescendants().at(i);
		DFS(this->getNode(d));
	}
	n->setVisited(false);
}




