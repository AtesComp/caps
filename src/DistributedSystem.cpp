/*
 * DistributedSystem.cpp
 *
 *  Created on: Jan 28, 2022
 *      Author: Keven L. Ates
 */

#include "DistributedSystem.hpp"

DistributedSystem::DistributedSystem(void)
{
    this->clear();
}

DistributedSystem::~DistributedSystem(void)
{
    this->clear();
}

void DistributedSystem::clear(void)
{
    adLoadvect[0] = 0.0;
    adLoadvect[1] = 0.0;
}
