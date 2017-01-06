#ifndef TPinterface_H
#define TPinterface_H

#include "CGFinterface.h"
#include "SceneGraph.h"

class TPinterface: public CGFinterface {
	InitVar* iv;

	public:
		TPinterface(InitVar* iv);

		virtual void initGUI();
		virtual void processGUI(GLUI_Control *ctrl);
};


#endif
