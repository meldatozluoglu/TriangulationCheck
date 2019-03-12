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
	double 	calculateNorm (double dx, double dy);
	double 	dotP (double dx, double dy);
	void	rotateNodes();

public:
	triangle(int node0, int node1,int  node2, int index);
	~triangle();
	int	index;
	int nodeIds[3];
	double 	calculateAreaAndRotation(std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords);
	bool 	calculateAngles(double thresholdCosine, std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords);
	double 	getArea();
};
#endif /* TRIANGLE_H_ */
