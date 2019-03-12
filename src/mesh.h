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
	int nNodes;	//the number of triangles forming the surface of lumen
	int nTriangles;

	const char * inputMeshFileName;
	std::string outputDirectory;
	bool thereAreDuplicateNodes;
	int 	unusedNodeCounter;
	double 	averageArea;
	double 	thresholdAreaRatio;
	bool	inputFileSpecified;
	bool 	checkHealthOfTriangles();
	bool	markTooSmallAreas();
	bool 	checkFileStatus(std::ifstream& file, std::string fileName);
	void	markDuplicateNodes();
	void	markUnusedNodes();
	bool	readInputMesh();
	bool 	readNodes(std::ifstream& inputMeshFile);
	bool 	readTriangles(std::ifstream& inputMeshFile);
	void 	updateTriangleDuplicateNodes();
	std::map<int, int> mapOfDuplicateNodes;
	std::vector<int> unusedNodes;
	std::vector <triangle*> triangles;
	std::vector<int>	unhealthyTriangles;

public:
	mesh();
	~mesh();
	double 	thresholdCosine;
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
