#include "TPinterface.h"
#include "TPscene.h"
#include <iostream>

TPinterface::TPinterface(InitVar* iv)
{
	this->iv=iv;
}

void TPinterface::initGUI()
{
	printf("INITIATING USER INTERFACE...\n");
	/**LIGHTS**/
	addColumn();
	GLUI_Panel* lightPanel = addPanel("Lights",1);
	
	for(unsigned int i = 0; i < iv->getLights().size(); i++)
	{
		char* id = iv->getLights().at(i)->getId();
		GLUI_Checkbox* lightBox = addCheckboxToPanel(lightPanel, id , &(((TPscene*) scene)->lightCheck[i]), 1);
		if(iv->getLights().at(i)->getEnabled()==false)
			lightBox->set_int_val(0);
		else if(iv->getLights().at(i)->getEnabled()==true)
			lightBox->set_int_val(1);
	}

	addColumn();
	GLUI_Panel* cameras = addPanel("Cameras",1);
	GLUI_RadioGroup *group1 = addRadioGroupToPanel(cameras, &(((TPscene*) scene)->cameraMode), 2);
	for(unsigned int i = 0; i < iv->getCameras().size(); i++)
	{
		char* name = iv->getCameras().at(i)->getId();
		addRadioButtonToGroup(group1, name);
	}

	addColumn();
	GLUI_Panel* wireframe = addPanel("Drawing Mode", 1);
	GLUI_RadioGroup *group2 = addRadioGroupToPanel(wireframe, &(((TPscene*) scene)->drawMode),3);
	addRadioButtonToGroup(group2,"Fill Mode");
	addRadioButtonToGroup(group2,"Line Mode");
	addRadioButtonToGroup(group2,"Point Mode");
}

void TPinterface::processGUI(GLUI_Control *ctrl)
{
	//printf ("GUI control id: %d\n  ",ctrl->user_id);
	switch(ctrl->user_id)
	{
	case 1:
		{
			(((TPscene*) scene)->toggleLights());
			break;
		};
	case 2:
		{
			//printf("%d", (((TPscene*) scene)->cameraMode));
			break;
		};
	case 3:
		{
			int mode = (((TPscene*) scene)->drawMode);
			if(mode == 0)
				(((TPscene*) scene)->setDrawingMode(GL_FILL));
			else if(mode == 1)
				(((TPscene*) scene)->setDrawingMode(GL_LINE));
			else if(mode == 2)
				(((TPscene*) scene)->setDrawingMode(GL_POINT));
			break;
		};
	}
}

