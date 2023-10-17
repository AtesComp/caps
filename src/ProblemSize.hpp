/*
 * ProblemSize.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef PROBLEMSIZE_HPP_
#define PROBLEMSIZE_HPP_

#include <string>

// Problem Size Definition...

class ProblemSize
{
public:
    long unsigned int uiNodes;      // Number of Nodes
    long unsigned int uiMembers;    // Number of Members
    short int siRolPin;             // Number of Roller Joints
    short int siFixPin;             // Number of Pinned Joints
    short int siRolRgd;             // Number of FIRL Joints
    short int siFixRgd;             // Number of Fixed Joints
    bool bNoPrintIn;                // Print Input Indicator
    short int siPrintOut;           // Print Output Indicator
    short int siLoads;              // Number of Load Cases
    short int siDivisions;          // Number of Division Parts

    // Computed values...
    short int siNodeFreedom;        // Number of Total Coordinates
    short int siNodePinned;         // Number of Usage Coordinates
    long unsigned int uiReactions;  // Number of Reactions

    ProblemSize(void);
    ~ProblemSize(void);
    void clear(void);

    std::string report();
};

#endif /* PROBLEMSIZE_HPP_ */
