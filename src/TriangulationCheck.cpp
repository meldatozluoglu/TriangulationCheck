//============================================================================
// Name        : TriangulationCheck.cpp
// Author      : Melda Tozluoglu
// Version     :
// Copyright   : MTozluoglu
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "mesh.h"


int	main(int argc, char * argv[]) {
	mesh mesh01{};
	bool errorInInput = false;
	errorInInput = mesh01.readExecutableInputs(argc, argv);
	if (errorInInput){
		return errorInInput;
	}
	errorInInput = mesh01.healthCheckMesh();
	if (errorInInput){
		std::cerr<<" exiting without save"<<std::endl;
		return errorInInput;
	}
	mesh01.writeNewMesh();
	return 0;
}



