#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include <vector>

class triangle{
protected:
	double 	area;		///< Area of trinagle
	double 	calculateNorm (double dx, double dy);	///< The function to calculate the norm of a 2D vector
	void	rotateNodes();				///< This function swaps the node order to ensure consistent rotation of nodes

public:
	triangle(int node0, int node1,int  node2, int index);	///< constructor
	~triangle();						///< destructor
	int	index;						///< index of the triangle
	int nodeIds[3];						///< node ids
	double 	calculateAreaAndRotation(std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords);	///< This function calculates the triangle area, and meanwhile corrects for the node rotation inferred from sign of the area. 
	bool 	calculateAngles(double thresholdCosine, std::vector<double>& nodeXCoords, std::vector<double>& nodeYCoords);	///< This function checks the angles of the triangle for skewness.
	double 	getArea();					///< returns area of triangle 
};
#endif /* TRIANGLE_H_ */
