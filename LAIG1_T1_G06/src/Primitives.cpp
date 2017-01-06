
void CylP::draw(vector<float>v)
{
	float base, top, height, slices, stacks;
	base = v.at(0); top = v.at(1), height = v.at(2); slices = v.at(3); stacks = v.at(4);
	float alpha = 2.0 * PI / slices;
	float stackHeight = height/stacks;
	bool smooth=true;
	glPushMatrix();
		glRotated(-90,1,0,0);
		glPushMatrix();
			glRotated(-180,1,0,0);
			glNormal3f(0,0,1);
			glBegin(GL_POLYGON);
					for(int i = 0; i < slices; i++)
					{
						glVertex3f( cos(i * 2.0 * PI / slices), sin(i * PI * 2.0 / slices), 0.0);
					}
			glEnd();
		glPopMatrix();

		glPushMatrix();
			glTranslated(0,0,height);
			glNormal3f(0,0,1);
			glBegin(GL_POLYGON);
					for(int i = 0; i < slices; i++)
					{
						glVertex3f( cos(i * 2.0 * PI / slices), sin(i * PI * 2.0 / slices), 0.0);
					}
			glEnd();
		glPopMatrix();
	
	
		glPushMatrix();
			glTranslated(0,0,0);
			glBegin(GL_QUADS);
					for(int i = 0; i < slices; i++)
					{
						if(smooth)
						{
							for(int j = 0; j < stacks; j++)
							{
								glNormal3f(cos((i+1) * 2.0 * PI / slices), sin((i+1) * PI * 2.0 / slices), 0.0); //0.0
								glVertex3d(cos((i+1) * 2.0 * PI / slices), sin((i+1) * PI * 2.0 / slices), stackHeight*j); //0.0
							
								glNormal3f(cos((i+1) * 2.0 * PI / slices), sin((i+1) * PI * 2.0 / slices), 0.0); //height
								glVertex3d(cos((i+1) * 2.0 * PI / slices), sin((i+1) * PI * 2.0 / slices), stackHeight*(j+1)); //height
							
								glNormal3f(cos(i * 2.0 * PI / slices), sin(i * PI * 2.0 / slices), 0.0); //height
								glVertex3d(cos(i * 2.0 * PI / slices), sin(i * PI * 2.0 / slices), stackHeight*(j+1));// height
							
								glNormal3f(cos(i * 2.0 * PI / slices), sin(i * PI * 2.0 / slices), 0.0); //0.0
								glVertex3d(cos(i * 2.0 * PI / slices), sin(i * PI * 2.0 / slices), stackHeight*j); //0.0
							}
						}
					}
			glEnd();	
		glPopMatrix();
	glPopMatrix();
}

void SphP::draw(vector<float>v)
{
	float radius=v.at(0);
	float slices=v.at(1);
	float stacks=v.at(2);
	bool makeTexCoords=false;
    int i,j;
	for (j = 0; j < stacks; j++)
	{
		double latitude1 = (PI/stacks) * j - PI/2;
		double latitude2 = (PI/stacks) * (j+1) - PI/2;
		double sinLat1 = sin(latitude1);
		double cosLat1 = cos(latitude1);
		double sinLat2 = sin(latitude2);
		double cosLat2 = cos(latitude2);
		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= slices; i++) {
			double longitude = (2*PI/slices) * i;
			double sinLong = sin(longitude);
			double cosLong = cos(longitude);
			double x1 = cosLong * cosLat1;
			double y1 = sinLong * cosLat1;
			double z1 = sinLat1;
			double x2 = cosLong * cosLat2;
			double y2 = sinLong * cosLat2;
			double z2 = sinLat2;
			glNormal3d(x2,y2,z2);
			if (makeTexCoords)
				glTexCoord2d(1.0/slices * i, 1.0/stacks * (j+1));
			glVertex3d(radius*x2,radius*y2,radius*z2);
			glNormal3d(x1,y1,z1);
			if (makeTexCoords)
				glTexCoord2d(1.0/slices * i, 1.0/stacks * j);
			glVertex3d(radius*x1,radius*y1,radius*z1);
		}
		glEnd();
	}
}

void TorP::draw(vector<float>v)
{

}