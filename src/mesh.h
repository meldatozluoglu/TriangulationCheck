#ifndef MESH_H_
#define MESH_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

class triangle;

class mesh{

protected:
	double 			averageArea;		///< Average are of all tringles in the mesh, needed for defining the outlier triangles.
	const char * 	inputMeshFileName;		///< The input mesh file name as read from user input to the executable
	bool			inputFileSpecified;	///< The boolen stating if the input file is specified
	std::map<int, int> mapOfDuplicateNodes;		///< the map of nodes that are duplicate coordinates {(int) index of node to be overwritten},{(int) index of the node to remain}
	int 			nNodes;			///< Number of nodes in the mesh as read from input
	int 			nTriangles;		///< Numebr of triangles in the mesh as read from input
	std::string 	outputDirectory;		///< The directory path string for the output
	bool 			thereAreDuplicateNodes;	///< boolean stating if there are duplicates in the nodes
	double 			thresholdAreaRatio;	///< the threshold area ratio to generate error for triangle size, default value 10 percent of mean.
	double 			thresholdCosine;	///< the cosine of the threshold angle, below which will gnerate an error, default value is set to 20 degrees.
	std::vector <triangle*> triangles;		///< vector of triangle pointers
	std::vector<int>unhealthyTriangles;		///< the indexes of triengles with outlier areas are too skew
	int 			unusedNodeCounter;	///< the number of nodes that are not part of any triangle, these are cleaned.
	std::vector<int> unusedNodes;			///< The vector of indexes of the unused nodes.

	bool 	checkFileStatus(std::ifstream& file, std::string fileName); 	///< Checks if hte file has been successfullly opened
	bool 	checkHealthOfTriangles();					///< This function checks the node rotation and angles of triangles
	void	markDuplicateNodes();						///< This function flags and bookkeeps for duplicate nodes to later replace all duplicates with the first occurace in triangle#nodeIds
	bool	markTooSmallAreas();						///< This functions flags triangles with too small area, set by the threshold mesh#thresholdAreaRatio
	void	markUnusedNodes();						///< This function flags unused nodes to exclude tehm form the cleaned mesh
	bool	readInputMesh();						///< This function reads the input mesh into node position arrays and triangle objects.
	bool 	readNodes(std::ifstream& inputMeshFile);			///< This function reads node coordinates from file
	bool 	readTriangles(std::ifstream& inputMeshFile);			///< This function reads node ids from file and generates triengles
	void 	updateTriangleDuplicateNodes();					///< This function replaces duplicate node ids with the first occurance in triangles.
	bool	updateTriangleNodeIds();					///< This function reorders the ode ids in triangles.
public:
	mesh();		///< constructor
	~mesh();	///< destructor

	std::vector <double> nodeXCoords;	///< Vector of node x positions
	std::vector <double> nodeYCoords;	///< Vector of node y positions
	std::vector <double> nodeBorderInfo;	///< Vector of booleasn stating if the node is at mesh border. 


	bool	healthCheckMesh();		///< This function calls multple check functions to identify possible problem in node format, or triangle area nad angle distribution of the mesh.
	bool	readExecutableInputs(int argc, char * argv[]);	///< reads in user input
	bool	readInputFileName(int& inputArgumentIndex, int argc, char **argv);	///< This function reads in input file path and name from user input
	bool 	readOutputDirectory(int& inputArgumentIndex, int argc, char **argv);	///< This function reads in output directory path from user input
	void	writeNewMesh();				///< This function writes new mesh after claen up
};

#endif /* MESH_H_ */
