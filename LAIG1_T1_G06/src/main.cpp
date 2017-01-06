
#include <iostream>
#include <exception>

#include "CGFapplication.h"
#include "TPscene.h"
#include "XMLScene.h"
#include "TPinterface.h"

using std::cout;
using std::exception;


int main(int argc, char* argv[]) {

	CGFapplication app = CGFapplication();
	char* filename;
	if(argc == 2) filename = argv[1];
	else filename = "snowman.anf";
	
	try {
		app.init(&argc, argv);

		Graph* g = new Graph();
		InitVar* iv = new InitVar();
	
		glMatrixMode(GL_MODELVIEW);

		XMLScene *s = new XMLScene(filename, g, iv);

		app.setScene(new TPscene(g,iv));
		app.setInterface(new TPinterface(iv));

		app.run();
	}
	catch(GLexception& ex) {
		cout << "Erro: " << ex.what();
		return -1;
	}
	catch(exception& ex) {
		cout << "Erro inesperado: " << ex.what();
		return -1;
	}
	
	getchar();
	return 0;
}