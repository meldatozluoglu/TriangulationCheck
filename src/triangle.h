/*
 * triangle.h
 *
 *  Created on: 12 Mar 2019
 *      Author: melda
 */

#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include <vector>

class triangle{
protected:
	double area;
	//int nodeIds[3];
	void	rotateNodes();
public:
	triangle(int node0, int node1,int  node2, int index);
	~triangle();
	int	index;
	double 	calculateAreaAndRotation(std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords);
	bool 	calculateAngles(double thresholdCosine, std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords);
	double 	getArea();
	int nodeIds[3];
};


#endif /* TRIANGLE_H_ */
