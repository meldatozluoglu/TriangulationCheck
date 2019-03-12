/*
 * mesh.cpp
 *
 *  Created on: 12 Mar 2019
 *      Author: melda
 */
#include "mesh.h"
#include "triangle.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

mesh::mesh(){
	nNodes = 0;
	nTriangles = 0;
	inputMeshFileName = "NA";
	outputDirectory = "NA";
	thereAreDuplicateNodes = false;
	unusedNodeCounter = 0;
	thresholdCosine = 0.93969262078;//cosine of 20 degrees, I dont want any triangle with smaller angles
	thresholdAreaRatio = 0.1; //no triangle should be smaller than 10% of the rest
	averageArea = 0;
	inputFileSpecified = false;
}

mesh::~mesh(){
	//deleting dynamically allocated pointers
	//TO DO:
	//Raw pointers, legacy code, convert to unq
	while(!triangles.empty()){
		triangle* tmp_pt;
		tmp_pt = triangles.back();
		triangles.pop_back();
		delete tmp_pt;
	}
}

bool mesh::checkFileStatus(std::ifstream& file, std::string fileName){
	if (!file.is_open()) {
		std::cout<<"Cannot open parameter input file, "<<fileName<<std::endl;
		return true; //there is error
	}
	if (!file.good()) {
		std::cout<<"File does not exist, "<<fileName<<std::endl;
		return true; //there is error;
	}
	return false;
}

void	mesh::writeNewMesh(){
	if(outputDirectory =="NA"){
		std::cout<<"no output directory specified, no output generated"<<std::endl;
		std::cout<<"try running with: -outputDirectory ./ "<<std::endl;

		return;
	}
	int nUnhealthy = unhealthyTriangles.size();
	if(nUnhealthy>0){
		std::cout<<"there are problematic triangles that cannot be fixed without re-meshing, generating problematicTriangleList"<<std::endl;
	}
	else{
		std::string saveFileString = outputDirectory +"/newMesh";
		const char* name_saveFileMesh = saveFileString.c_str();
		std::ofstream saveFileMesh;
		saveFileMesh.open(name_saveFileMesh, std::ofstream::out);
		if (!saveFileMesh.good() || !saveFileMesh.is_open()){
			std::cerr<<"could not open file: "<<name_saveFileMesh<<std::endl;
			return;
		}
		saveFileMesh<<nNodes;
		saveFileMesh<<std::endl;
		for (int i=0;i<nNodes; ++i){
			//id
			saveFileMesh.precision(6);
			saveFileMesh.width(10);
			saveFileMesh<<i;
			//pos x
			saveFileMesh.precision(6);
			saveFileMesh.width(10);
			saveFileMesh<<nodeXCoords[i];
			//pos y
			saveFileMesh.precision(6);
			saveFileMesh.width(10);
			saveFileMesh<<nodeYCoords[i];
			// is border
			saveFileMesh.precision(6);
			saveFileMesh.width(10);
			saveFileMesh<<nodeBorderInfo[i];
			saveFileMesh<<std::endl;
		}
		saveFileMesh<<nTriangles;
		saveFileMesh<<std::endl;
		for (triangle* tri_ptr : triangles){
			//id
			saveFileMesh.precision(6);
			saveFileMesh.width(10);
			saveFileMesh<<tri_ptr->index;
			for (auto const &currNodeId : tri_ptr->nodeIds){
				//node ids
				saveFileMesh.precision(6);
				saveFileMesh.width(10);
				saveFileMesh<<currNodeId;
			}
			saveFileMesh<<std::endl;
		}
		std::cout<<"output file generated at: "<<name_saveFileMesh<<std::endl;
	}
}


bool mesh::healthCheckMesh(){
	bool errorInInput = false;
	errorInInput = readInputMesh();
	if (errorInInput){
		return errorInInput;
	}
	markDuplicateNodes();
	if (thereAreDuplicateNodes){
		updateTriangleDuplicateNodes();
	}
	markUnusedNodes();
	checkHealthOfTriangles();
	markTooSmallAreas();
	return errorInInput;
}

void mesh::markTooSmallAreas(){
	for (triangle* tri_ptr : triangles){
		double currArea = tri_ptr->getArea();
		double ratio = currArea/averageArea;
		if (ratio < thresholdAreaRatio){
			unhealthyTriangles.push_back(tri_ptr->index);
			std::cout<<"Triangle: "<<tri_ptr->index<<" too small"<<std::endl;
		}
	}
}

void mesh::checkHealthOfTriangles(){
	for (triangle* tri_ptr : triangles){
		double currArea = tri_ptr->calculateAreaAndRotation(nodeXCoords,nodeYCoords);
		averageArea+= currArea;
		bool triagleAnglesTooSmall = tri_ptr->calculateAngles(thresholdCosine, nodeXCoords,nodeYCoords);
		if (triagleAnglesTooSmall){
			std::cout<<"Triangle: "<<tri_ptr->index<<" too skew"<<std::endl;
			unhealthyTriangles.push_back(tri_ptr->index);
		}

	}
	if (nTriangles>0){
		averageArea /= nTriangles;
	}
}

void mesh::updateTriangleDuplicateNodes(){
	for (triangle* tri_ptr : triangles){
		for (auto &currNodeId : tri_ptr->nodeIds){
			std::map<int,int>::iterator duplicateNodeit;
			duplicateNodeit = mapOfDuplicateNodes.find(currNodeId);
			if (  duplicateNodeit != mapOfDuplicateNodes.end() ) {
				// the duplicate is this node, swap it with original:
				currNodeId = duplicateNodeit->second;
			}
		}
	}
}

void mesh::markUnusedNodes(){
	int nodeId = 0;
	while(nodeId<nNodes){
		bool foundNode = false;
		for (triangle* tri_ptr : triangles){
			for (auto const &triangleNodeId : tri_ptr->nodeIds){
				if (triangleNodeId == nodeId){
					foundNode = true;
					break;
				}
			}
			if (foundNode){
				break;
			}
		}
		if (!foundNode){
			unusedNodeCounter++;
			unusedNodes.push_back(nodeId);
			std::cout<<"Detected unused node: "<<nodeId<<std::endl;
		}
		nodeId++;
	}
}

void	mesh::markDuplicateNodes(){
	bool nodeIsDuplicate[nNodes] = { false };
	//classical iterators, I need
	int indexOfXoriginal  = 0;
	for (std::vector<double>::iterator it_xpos = nodeXCoords.begin(); it_xpos<nodeXCoords.end();it_xpos++ ){
		bool thereAreDuplictes = true;
		if(nodeIsDuplicate[indexOfXoriginal]){
			//node is already duplicate, continue
			continue;
		}
		int lowEndDistance = indexOfXoriginal+1;
		while (thereAreDuplictes){
			auto duplicateIterator = find(nodeXCoords.begin()+lowEndDistance, nodeXCoords.end(),(*it_xpos));
			if (duplicateIterator != nodeXCoords.end()){
				//there is a duplicate x coord, is the y coordinate duplicate?
				int indexOfXduplicate = std::distance(nodeXCoords.begin(), duplicateIterator);
				lowEndDistance = indexOfXduplicate+1;
				if (nodeYCoords[indexOfXduplicate] == nodeYCoords[indexOfXoriginal]){
					//node is duplicate
					std::cout<<"there is x and y duplicate at node pair: "<<indexOfXoriginal<<" "<<indexOfXduplicate<<std::endl;
					nodeIsDuplicate[indexOfXduplicate] = true;
					mapOfDuplicateNodes[indexOfXduplicate] = indexOfXoriginal;
					thereAreDuplicateNodes = true;
				}
			}
			else{
				thereAreDuplictes=false;
			}
		}
		indexOfXoriginal++;
	}
}

bool	mesh::readExecutableInputs(int argc, char * argv[]){
	int inputArgumentIndex = 1;
	bool errorInInput = false;
	while(inputArgumentIndex<argc){
		const char *inptype = argv[inputArgumentIndex];
		if (std::string(inptype) == "-inputMeshFile"){
			errorInInput  = readInputFileName(inputArgumentIndex, argc, argv);
		}
		else if (std::string(inptype) == "-outputDirectory"){
			errorInInput  = readOutputDirectory(inputArgumentIndex, argc, argv);
		}
		else {
			std::cerr<<"Please enter a valid option key: {-inputMeshFile, -outputDirectory}, current string: "<<inptype<<std::endl;
			return true;//there is error in executable input
		}
		inputArgumentIndex++;
		if (errorInInput ){
			return errorInInput ;
		}
	}
	if (!inputFileSpecified){
		std::cerr<<"Please enter the input mesh, a test cases can be found under /TriangulationCheck/testCases/ "<<std::endl;
		std::cout<<"   format is: -inputMeshFile $pathToProject/TriangulationCheck/testCases/testInputMesh01"<<std::endl;
		return true;//there is error in executable input
	}
	return errorInInput;
}

bool mesh::readInputFileName(int& inputArgumentIndex, int argc, char **argv){
	inputArgumentIndex++;
	if (inputArgumentIndex >= argc){
		std::cerr<<" input the save directory, contents of which will be displayed"<<std::endl;
		return true;
	}
	inputMeshFileName = argv[inputArgumentIndex];
	inputFileSpecified = true;
	return false;
}


bool	mesh::readInputMesh(){
	/**
	 *  This function will read all available model inputs from the file ModelInputObject#parameterFileName. \n
	 *  It will start by opening the model input file, after each attempt to open a file, there will be a health check to ensure the file
	 *  could be opened. In case there are issues with the file (most common one being the file is not opened due to a path error),
	 *  the function will throw an error with corresponding explanatory error message, and quit the simulation.
	 */

	//Open the input file
	bool errorInFile = false;
	std::ifstream inputMeshFile;
	inputMeshFile.open(inputMeshFileName, std::ifstream::in);
	errorInFile = checkFileStatus(inputMeshFile,inputMeshFileName);
	if (errorInFile){
		return errorInFile;
	}
	//read nodes
	errorInFile = readNodes(inputMeshFile);
	if (errorInFile){
		return errorInFile;
	}
	//read triangles
	errorInFile = readTriangles(inputMeshFile);
	if (errorInFile){
		return errorInFile;
	}
	return errorInFile;
}

bool	mesh::readNodes(std::ifstream& inputMeshFile){
	bool errorInFile = false;
	std::string currline;
	std::getline(inputMeshFile,currline);
	while (currline.empty()){
		//skipping empty lines
		getline(inputMeshFile,currline);
		if (inputMeshFile.eof()){
			errorInFile = true;
			std::cerr<<"empty input file"<<std::endl;
			return errorInFile;
		}
	}
	std::istringstream currSStrem(currline);
	currSStrem >> nNodes;      // try to read the node number
	currSStrem >> std::ws;  // eat whitespace after number
	if (currSStrem.fail() || !currSStrem.eof()) {
		errorInFile = true;
		std::cerr<<"Format error in input mesh, Node number is not correct, expected integer, current line: "<<currline<<std::endl;
		return errorInFile;
	}
	int readNodeCounter =0;
	while (!inputMeshFile.eof() && readNodeCounter<nNodes)
	{
		std::getline(inputMeshFile,currline);
		if(currline.empty()){
			continue;
		}
		std::istringstream iss(currline);
		int id;
		double x, y;
		bool isborder;
		if (!(iss >> id >> x >> y >> isborder)) {
			errorInFile = true;
			std::cerr<<"Format error in input mesh, expected coordinates for node number "<<readNodeCounter<<" , current line: "<<currline<<std::endl;
			break;
		}
		nodeXCoords.push_back(x);
		nodeYCoords.push_back(y);
		nodeBorderInfo.push_back(isborder); //This is not useful for us but we keep track in case we need to write a new mesh later on
		readNodeCounter++;
	}
	return errorInFile;
}

bool	mesh::readTriangles(std::ifstream& inputMeshFile){
	bool errorInFile = false;
	std::string currline;
	std::getline(inputMeshFile,currline);
	while (currline.empty()){
		//skipping empty lines
		getline(inputMeshFile,currline);
		if (inputMeshFile.eof()){
			errorInFile = true;
			std::cerr<<"no triangle info on input file"<<std::endl;
			return errorInFile;
		}
	}
	std::istringstream currSStrem(currline);
	currSStrem >> nTriangles;      // try to read the node number
	currSStrem >> std::ws;  // eat whitespace after number
	if (currSStrem.fail() || !currSStrem.eof()) {
		errorInFile = true;
		std::cerr<<"Format error in input mesh, trienagle number is not correct, expected integer, current line: "<<currline<<std::endl;
		return errorInFile;
	}

	int readTriCounter =0;
	while (!inputMeshFile.eof() && readTriCounter<nTriangles)
	{
		std::getline(inputMeshFile,currline);
		if(currline.empty()){
			continue;
		}
		std::istringstream iss(currline);
		int id, node0, node1, node2;
		if (!(iss >> id >> node0 >> node1 >> node2)) {
			errorInFile = true;
			std::cerr<<"Format error in input mesh, expected ids for triangle corners "<<readTriCounter<<" , current line: "<<currline<<std::endl;
			break;
		}
		triangle* tri_p = new triangle(node0, node1, node2,readTriCounter);
		triangles.push_back(tri_p);
		readTriCounter++;
	}
	return errorInFile;
}

bool mesh::readOutputDirectory(int& inputArgumentIndex, int argc, char **argv){
	inputArgumentIndex++;
	if (inputArgumentIndex >= argc){
		std::cerr<<" input the save directory"<<std::endl;
		return false;
	}
	const char* inpstring = argv[inputArgumentIndex];
	//This will set the save directory, but will not change the safe file boolean.
	//If your model input file states no saving, then the error and output files
	//will be directed into this directory, but the frame saving will not be toggled
	outputDirectory= std::string(inpstring);
	return false;//no errors
}
