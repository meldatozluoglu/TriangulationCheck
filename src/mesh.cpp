/*
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

bool mesh::checkHealthOfTriangles(){
	/**
	 *  This function calcuates average triangle area of the mesh, 
	 *  corrects for node rotation in the process of calculating each
	 *  triangle area. Then the angles are checked for skewness.
	 */	
	bool thereAreProblematicTriangles = false;
	for (triangle* tri_ptr : triangles){
		double currArea = tri_ptr->calculateAreaAndRotation(nodeXCoords,nodeYCoords);
		averageArea+= currArea;
		bool triagleAnglesTooSmall = tri_ptr->calculateAngles(thresholdCosine, nodeXCoords,nodeYCoords);
		if (triagleAnglesTooSmall){
			std::cout<<"Triangle: "<<tri_ptr->index<<" too skew"<<std::endl;
			unhealthyTriangles.push_back(tri_ptr->index);
			thereAreProblematicTriangles = true;
		}

	}
	if (nTriangles>0){
		averageArea /= nTriangles;
	}
	return thereAreProblematicTriangles;
}

bool mesh::healthCheckMesh(){
	/**
	 *  This function reads in the mesh from input file. If the mesh is 
	 *  successfully generated, node list is flagged for duplicates and 
	 *  unused nodes, cleaning these from triangles at the same time. 
	 *  These corrections are things this tool can clean and still 
	 *  produce a new mesh. /n
	 *
	 *  Next, the triangles are checked for their skewness and area. 
	 *  Thses issues are flagged, each triangle is reported, yet a new 
	 *  mesh is not generated.
	 */
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
	bool thereAreProblematicTriangles = checkHealthOfTriangles();
	if (!thereAreProblematicTriangles){
		std::cout<<"All triangles are healthy for skewness"<<std::endl;
	}
	else{
		errorInInput=true;
	}
	thereAreProblematicTriangles = markTooSmallAreas();
	if (!thereAreProblematicTriangles){
		std::cout<<"All triangles are sufficiently large for area, current threshold: "<<thresholdAreaRatio*100<<" percent of average"<<std::endl;
	}
	else{
		errorInInput=true;
	}
	thereAreProblematicTriangles = updateTriangleNodeIds();
	if (thereAreProblematicTriangles){
		errorInInput=true;
	}
	return errorInInput;
}

void	mesh::markDuplicateNodes(){
	/**
	 *  This function records the duplicate nodes in a map, the first occurance
	 *  of a node is presered, while the remaining repetitions are replaced by
	 *  the initial id in triangles./n
	 */
	bool nodeIsDuplicate[nNodes] = { false };
	//classical iterators, I need to loop over two arrays at once, clearer code in search functions this way.
	int indexOfXoriginal  = 0;
	for (std::vector<double>::iterator it_xpos = nodeXCoords.begin(); it_xpos<nodeXCoords.end();it_xpos++ ){
		bool thereAreDuplictes = true;
		if(nodeIsDuplicate[indexOfXoriginal]){
			//this node is already duplicate, continue
			continue;
		}
		//start the search from the position after the current node
		int lowEndDistance = indexOfXoriginal+1;
		while (thereAreDuplictes){
			auto duplicateIterator = find(nodeXCoords.begin()+lowEndDistance, nodeXCoords.end(),(*it_xpos));
			if (duplicateIterator != nodeXCoords.end()){
				//there is a duplicate x coord, is the y coordinate duplicate?
				int indexOfXduplicate = std::distance(nodeXCoords.begin(), duplicateIterator);
				//move the bottom of search range to above current duplicate.
				lowEndDistance = indexOfXduplicate+1;
				if (nodeYCoords[indexOfXduplicate] == nodeYCoords[indexOfXoriginal]){
					//node is duplicate
					std::cout<<"there is x and y duplicate at node pair: "<<indexOfXoriginal<<" "<<indexOfXduplicate<<std::endl;
					nodeIsDuplicate[indexOfXduplicate] = true;
					//add the pair to the map. 
					/** The map is structured as [duplicae][original], to allow for 
	 				 *  searching the duplicates as keys and replacing them.
	 				 */
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
	/** The executable takes in two arguments, the input file is 
	 *  necessary, the output path is optional.
	 */
	 
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
		std::cout<<"Please enter the input mesh, a test cases can be found under /TriangulationCheck/testCases/ "<<std::endl;
		std::cout<<"   format is: -inputMeshFile $pathToProject/TriangulationCheck/testCases/testInputMesh01"<<std::endl;
		return true;//there is error in executable input
	}
	return errorInInput;
}

bool mesh::markTooSmallAreas(){
	bool thereAreProblematicTriangles = false;
	for (triangle* tri_ptr : triangles){
		double currArea = tri_ptr->getArea();
		double ratio = currArea/averageArea;
		if (ratio < thresholdAreaRatio){
			unhealthyTriangles.push_back(tri_ptr->index);
			std::cout<<"Triangle: "<<tri_ptr->index<<" too small"<<std::endl;
			thereAreProblematicTriangles = true;
		}
	}
	return thereAreProblematicTriangles;
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

bool	mesh::readInputMesh(){
	/**
	 *  This function will read the mesh from the file opened with name read into 
	 *  mesh#inputMeshFileName. \n
	 *  It will start by opening the model input file, after the attempt to open a file, 
	 *  there will be a health check to ensure the file could be opened. In case there
	 *  are issues with the file (most common one being the file is not opened due to a path error),
	 *  the function will throw an error with corresponding explanatory error message,
	 *  and quit the simulation.
	 */
	//Open the input file
	bool errorInFile = false;
	std::ifstream inputMeshFile;
	inputMeshFile.open(inputMeshFileName, std::ifstream::in);
	errorInFile = checkFileStatus(inputMeshFile,inputMeshFileName);
	if (errorInFile){
		return errorInFile;
	}
	/** Next the nodes and triangles will be read in via functions
	 * mesh#readNodes and mesh#readTriangles.
	 */
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

bool	mesh::readNodes(std::ifstream& inputMeshFile){
	/** Possible emty lines at the beginning of the file will be skipped, but
	 *  further gaps will not be accepted.
	 *
	 */
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
	/** The node number will be checked, and any trailing characters will result 
	 *  in throwing an error.
	 */
	std::istringstream currSStrem(currline);
	currSStrem >> nNodes;      // try to read the node number
	currSStrem >> std::ws;  // eat whitespace after number
	if (currSStrem.fail() || !currSStrem.eof()) {
		errorInFile = true;
		std::cerr<<"Format error in input mesh, Node number is not correct, expected integer, current line: "<<currline<<std::endl;
		return errorInFile;
	}
	/** For each node, an indez, x and y positions and a boolean giving topology 
	 * information (at border?) will be read in, any mismatch will throw an error. 
	 * Any characters trainling the line are ignored. This is prefered, as more
	 * topological/material identifiers may be added to the nodes later on, without
	 * breaking the pipeline for mesh check and warnings./n
	 */
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
	/** If the available node information is less than the given node number, there will be an error.
	 *  At this stage, the triangles have not been read, and end of file is reached, the file 
	 *  is corrupt/n
	 */
	if (readNodeCounter<nNodes){
		std::cerr<<" current read node count: "<<readNodeCounter<<" desired node count"<<nNodes<<std::endl;
		errorInFile  = true;
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
	outputDirectory= std::string(inpstring);
	return false;//no errors
}

bool	mesh::readTriangles(std::ifstream& inputMeshFile){
	/** Possible emty lines between nodes and triangles will be skipped, but
	 *  further gaps will not be accepted.
	 */
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
	/** The node number will be checked, and any trailing characters will result 
	 *  in throwing an error.
	 */
	std::istringstream currSStrem(currline);
	currSStrem >> nTriangles;      // try to read the node number
	currSStrem >> std::ws;         // eat whitespace after number
	if (currSStrem.fail() || !currSStrem.eof()) {
		errorInFile = true;
		std::cerr<<"Format error in input mesh, trienagle number is not correct, expected integer, current line: "<<currline<<std::endl;
		return errorInFile;
	}
	/** For each triangle, an indez, node0,1,2 ids will be read in, any mismatch will
	 *  throw an error. Any characters trainling the line are ignored. This is prefered
	 *  as more topological/material identifiers may be added to the triangles later on,
	 *  without breaking the pipeline for mesh check and warnings./n
	 */
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
		if (node0>nNodes || node1>nNodes || node2>nNodes  ){
			errorInFile = true;
			std::cerr<<"Format error in input mesh, triangle corners refer to nodes above given count "<<node0<<" "<<node1<<" "<<node2<<std::endl;
			break;
		}
		triangle* tri_p = new triangle(node0, node1, node2,readTriCounter);
		triangles.push_back(tri_p);
		readTriCounter++;
	}
	/** If the available trinagle information is less than the given number, there will be an error.s
	 */
	if (readTriCounter<nTriangles){
		std::cerr<<" current read triangle count: "<<readTriCounter<<" desired triangle count"<<nTriangles<<std::endl;
		errorInFile  = true;
	}
	return errorInFile;
}



void mesh::updateTriangleDuplicateNodes(){
	/** Looping over all tirangles and then their nodes, chacing id any nide is in the dupicate map.
	 *  The duplicate is then updated with the original (hence using the address of auto).
	 */
	for (triangle* tri_ptr : triangles){
		//I want to update the node id, I need the address not the copy.
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

bool mesh::updateTriangleNodeIds(){
	/** If I have deleted any unused nodes from the system, I would leave
	 *  a gap in node indexes in most cases. Then I can have node id in the 
	 *  triangles, that is beyond hte numebr of nodes, which is not desirable.
	 *  Before writing the new mesh, I will clear this, if needed.
	 */
	 bool errorInStructure = false;
	 if (unusedNodeCounter>0){
	 	std::map<int, int> mapOfNewNodeIds;
	 	int indexCurrentNodes = 0;
	 	int indexNewNodes = 0;
	 	//no modification on the boolean, no need to copy: cont address
	 	for (auto const &nodeStateBool : unusedNodes){
	 		if (nodeStateBool){
	 			//this node is unused, do not record.
	 			indexCurrentNodes++;
	 			continue;
	 		}
	 		mapOfNewNodeIds[indexCurrentNodes] = indexNewNodes;
	 		indexCurrentNodes++;
	 		indexNewNodes++;
	 	}
		//Now I have the map, replace node ids.
		for (triangle* tri_ptr : triangles){
			//I want to update the node id, I need the address not the copy.
			for (auto &currNodeId : tri_ptr->nodeIds){
				std::map<int,int>::iterator nodeIterator;
				nodeIterator = mapOfNewNodeIds.find(currNodeId);
				if (  nodeIterator != mapOfDuplicateNodes.end() ) {
					//This should always be the case, adding the if clause for safety
					//swapping the node ids.
					currNodeId = nodeIterator->second;
				}
				else{
					std::cerr<<"There is error in node recording for triangle "<<tri_ptr->index<<std::endl;
					errorInStructure = true;
				}	
			}
		}
	 }
	 return errorInStructure;
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
		saveFileMesh<<nNodes - unusedNodeCounter;
		saveFileMesh<<std::endl;
		for (int i=0;i<nNodes; ++i){
			if (find(unusedNodes.begin(), unusedNodes.end(),i) != unusedNodes.end()){
				std::cout<<"skipping unused node"<<std::endl;
				continue;
			}
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
			//I don't need to update anything, but I don't need the cost of copy either.
			//So use a constant address.
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
