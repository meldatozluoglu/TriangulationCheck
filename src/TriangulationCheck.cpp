//============================================================================
// Name        : TriangulationCheck.cpp
// Author      : Melda Tozluoglu
// Version     :
// Copyright   : MTozluoglu
// Description : This package reads in a user provided 2D triangulation. This 
//		 triangulated mesh will be used to generate the 3D mesh where
//		 finite element simulations of tissue morhognesis are run.
//		 The mesh should have relatively unifrom triangle size and
//		 internal angles. There should not be dublicate nodes. There
//		 should not be unused stray nodes. The nodes should be in the 
//		 same rotational order in each triangle.
//		 This function will fix rotational and node order problems, but
//		 will not attempt at refining the triangulation, beyond 
//		 reporting problematic nodes. These should be fixed at the 
//		 triangulation stage.  
//
//		 The input file format should be:
//		 # of nodes
//		 index  x  y  is_at_mesh_border
//		 ...
//		 # of triangles
//		 index	node0index node1index node2index
//		 ...
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



