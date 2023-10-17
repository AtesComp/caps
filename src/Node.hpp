/*
 * Node.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef NODE_HPP_
#define NODE_HPP_

#include "LoadNodal.hpp"

#include <string>
#include <ostream>

//  Node Coordinate Definition...

class Node
{
public:
    static short int siUsedSize;
    static short int siLargestDegrees;

    short int siID;             // Node Identifier
    double adPoint[2];          // Location (X,Y)
    short int siSequence;       // Bandwidth Sequencer (0 = Not Sequenced)
    short int siDegrees;        // Degrees of Use (Number of Members using node)

    // Node's Degrees of Freedom (DoF):
    //   NOTE: The DoF indices are used in two ways.  Initially, they are flags used
    //         to determine if a DoF is FREE (0, false) or FIXED (true).  Later, they
    //         are used to set the System Stiffness Matrix: FIXED DoF are not used,
    //         FREE DoF are set to a matrix sequence location.
    short int siHorzFlag;       // Horizontal Freedom Index (FREE or FIXED)
    short int siVertFlag;       // Vertical   Freedom Index (FREE or FIXED)
    short int siRotFlag;        // Rotational Freedom Index (FREE or FIXED)

    bool bMemberFixture;        // Member Fixture Flag: true=FREE, false=FIXED
    LoadNodal * nodeload;       // Pointer to a Nodal Load, if present

    Node(void);
    ~Node(void);
    void clear(void);

    void report(std::ostream &);
};

#endif /* NODE_HPP_ */
