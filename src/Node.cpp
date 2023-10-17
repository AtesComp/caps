/*
 * Node.cpp
 *
 *  Created on: Aug 26, 2020
 *      Author: Keven L. Ates
 */

#include "Node.hpp"
#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <iomanip>
#include <fmt/format.h>

short int Node::siUsedSize = 0;
short int Node::siLargestDegrees = 0;

Node::Node(void)
{
    clear();
}

Node::~Node(void)
{
    clear();
}

void Node::clear(void)
{
    this->siID = 0;
    this->adPoint[0] = 0.0;
    this->adPoint[1] = 0.0;
    this->siSequence = 0;
    this->siDegrees = 0;
    this->siHorzFlag = 0; // ...assume FREE
    this->siVertFlag = 0; // ...assume FREE
    this->siRotFlag = 0;  // ...assume FREE
    this->bMemberFixture = true; // ...assume FREE
    this->nodeload = nullptr;
}

void Node::report(std::ostream & osOut)
{
    //
    // Table 2 body...
    //
    osOut << "   " << std::right;
    osOut << std::setw(3) << this->siID << "     ("
          << std::setw(12) << fmt::format(SF12_4f, this->adPoint[X]) << ", "
          << std::setw(12) << fmt::format(SF12_4f, this->adPoint[Y]) << ")";
    if (this->siSequence == 0)
        osOut << "   (UNUSED)";
    osOut << "\n";
}
