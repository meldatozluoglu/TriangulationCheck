#include "triangle.h"
#include <iostream>
#include <cmath>

triangle::triangle(int node0, int node1,int  node2, int index){
	nodeIds[0] = node0;
	nodeIds[1] = node1;
	nodeIds[2] = node2;
	this->index = index;
	area = 0;
}

triangle::~triangle(){

}

double calculateNorm (double dx, double dy){
	return pow(dx*dx + dy*dy,0.5);
}

double dotP (double dx, double dy){
	return pow(dx*dx + dy*dy,0.5);
}

bool triangle::calculateAngles(double thresholdCosine, std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords){
	double lengthThreshold = 1E-3;
	double innerAngleCosines[3];

	double dx01 = nodeXCoords[nodeIds[1]]-nodeXCoords[nodeIds[0]];
	double dy01 = nodeYCoords[nodeIds[1]]-nodeYCoords[nodeIds[0]];
	double dx02 = nodeXCoords[nodeIds[2]]-nodeXCoords[nodeIds[0]];
	double dy02 = nodeYCoords[nodeIds[2]]-nodeYCoords[nodeIds[0]];
	double dx12 = nodeXCoords[nodeIds[1]]-nodeXCoords[nodeIds[2]];
	double dy12 = nodeYCoords[nodeIds[1]]-nodeYCoords[nodeIds[2]];

	double norm01 = calculateNorm(dx01, dy01);
	double norm02 = calculateNorm(dx02, dy02);
	double norm12 = calculateNorm(dx12, dy12);

	double dotp0 =  dx01*dx02 + dy01*dy02;
	double dotp1 =  dx12*(-1.0*dx01) + dy12*(-1.0*dy01);
	double dotp2 =  (-1.0*dx12)*(-1.0*dx02) + (-1.0*dy12)*(-1.0*dy02);

	if (norm01 < lengthThreshold ||norm02 < lengthThreshold || norm12 < lengthThreshold ){
		innerAngleCosines[0] = 1;
		innerAngleCosines[1] = 1;
		innerAngleCosines[2] = 1;
		return true; //the element has too small angles
		std::cout<<"triangle "<<index<<" length too small: "<<norm01<<" "<<norm02<<" "<<norm12<<std::endl;
	}
	innerAngleCosines[0] = dotp0/norm01/norm02;
	if(innerAngleCosines[0] > thresholdCosine){
		std::cout<<"triangle "<<index<<" innerAngleCosines[0] too small: "<<innerAngleCosines[0]<<std::endl;
		return true; //the element has too small angles
	}
	innerAngleCosines[1] = dotp1/norm01/norm12;
	if(innerAngleCosines[1] > thresholdCosine){
		std::cout<<"triangle "<<index<<" innerAngleCosines[1] too small: "<<innerAngleCosines[1]<<std::endl;
		return true; //the element has too small angles
	}
	innerAngleCosines[2] = dotp2/norm02/norm12;
	if(innerAngleCosines[2] > thresholdCosine){
		std::cout<<"triangle "<<index<<" innerAngleCosines[2] too small: "<<innerAngleCosines[2]<<std::endl;
		return true; //the element has too small angles
	}
	return false;
}

double triangle::calculateAreaAndRotation(std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords){
	//calculate the area from the determinant:
	// A = 1/2 [x0y1 + x1y2 + x2y0 - x1y0 - x2y1 - x0y2]
	area += nodeXCoords[nodeIds[0]] * nodeYCoords[nodeIds[1]];
	area += nodeXCoords[nodeIds[1]] * nodeYCoords[nodeIds[2]];
	area += nodeXCoords[nodeIds[2]] * nodeYCoords[nodeIds[0]];
	area -= nodeXCoords[nodeIds[1]] * nodeYCoords[nodeIds[0]];
	area -= nodeXCoords[nodeIds[2]] * nodeYCoords[nodeIds[1]];
	area -= nodeXCoords[nodeIds[0]] * nodeYCoords[nodeIds[2]];
	if (area < 0){
		area*=-1.0;
		rotateNodes();
	}
	return area;
}

double triangle::getArea(){
	return area;
}

void triangle::rotateNodes(){
	int tmpNode0Id = nodeIds[0];
	nodeIds[0] = nodeIds[1];
	nodeIds[1] = tmpNode0Id;
	std::cout<<"corrected rotation for nodes trinagle with nodes: "<<nodeIds[0]<<" "<<nodeIds[1]<<" "<<nodeIds[2]<<std::endl;
}
