/*
 * mesh.h
 *
 *  Created on: 12 Mar 2019
 *      Author: melda
 */

#ifndef MESH_H_
#define MESH_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

class triangle;

class mesh{

protected:
	double 			averageArea;
	const char * 	inputMeshFileName;
	bool			inputFileSpecified;
	std::map<int, int> mapOfDuplicateNodes;
	int 			nNodes;	//the number of triangles forming the surface of lumen
	int 			nTriangles;
	std::string 	outputDirectory;
	bool 			thereAreDuplicateNodes;
	double 			thresholdAreaRatio;
	double 			thresholdCosine;
	std::vector <triangle*> triangles;
	std::vector<int>unhealthyTriangles;
	int 			unusedNodeCounter;
	std::vector<int> unusedNodes;

	bool 	checkFileStatus(std::ifstream& file, std::string fileName);
	bool 	checkHealthOfTriangles();
	void	markDuplicateNodes();
	bool	markTooSmallAreas();
	void	markUnusedNodes();
	bool	readInputMesh();
	bool 	readNodes(std::ifstream& inputMeshFile);
	bool 	readTriangles(std::ifstream& inputMeshFile);
	void 	updateTriangleDuplicateNodes();

public:
	mesh();
	~mesh();

	std::vector <double> nodeXCoords;
	std::vector <double> nodeYCoords;
	std::vector <double> nodeBorderInfo;


	bool	healthCheckMesh();
	bool	readExecutableInputs(int argc, char * argv[]);
	bool	readInputFileName(int& inputArgumentIndex, int argc, char **argv);
	bool 	readOutputDirectory(int& inputArgumentIndex, int argc, char **argv);
	void	writeNewMesh();
};

#endif /* MESH_H_ */
