/*
 * NodeDegree.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef NODEDEGREE_HPP_
#define NODEDEGREE_HPP_

#include "Node.hpp"

// Node Degrees of Usage...

class NodeDegree
{
public:
	Node * node;            // Node Degree's Node

	short int siSequence;   // Node Sequence Indicator
    short int siDegrees;    // Node Degree of Usage
};

#endif /* NODEDEGREE_HPP_ */
