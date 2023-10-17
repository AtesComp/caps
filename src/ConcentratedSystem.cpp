/*
 * ConcentratedSystem.cpp
 *
 *  Created on: Jan 28, 2022
 *      Author: Keven L. Ates
 */

#include "ConcentratedSystem.hpp"

ConcentratedSystem::ConcentratedSystem(void)
{
    this->clear();
}

ConcentratedSystem::~ConcentratedSystem(void)
{
    this->clear();
}

void ConcentratedSystem::clear(void)
{
    dLoadDist = 0.0;
    adLoadVect[0] = 0.0;
    adLoadVect[1] = 0.0;
}
