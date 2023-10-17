/*
 * LoadAndInteraction.cpp
 *
 *  Created on: Aug 20, 2020
 *      Author: Keven L. Ates
 */

#include "LoadAndInteraction.hpp"

LoadAndInteraction::LoadAndInteraction(void)
{
    this->clear();
}

LoadAndInteraction::~LoadAndInteraction(void)
{
    this->clear();
}

void LoadAndInteraction::clear(void)
{
    this->interaction_input = 0;
    this->interaction = 0;

    this->bPointLoads       = false;
    this->bNodeLoads        = false;
    this->bUniformLoads     = false;
    this->bTrapezoidalLoads = false;

    this->nds = 1991;

    this->siStressFactorIndex = 0;
}

void LoadAndInteraction::setInteraction(short int iInteractVal)
{
    this->interaction_input = this->interaction = iInteractVal;
    if (iInteractVal > 3)
    {
        this->nds = 1986;
        this->interaction = iInteractVal - 4;
    }
}

short int LoadAndInteraction::getNDS(void)
{
    return this->nds;
}

bool LoadAndInteraction::isNDS1991(void)
{
    return (this->nds == 1991);
}

bool LoadAndInteraction::isNDS1986(void)
{
    return (this->nds == 1986);
}

bool LoadAndInteraction::calcAtBendAndAxialMax(void)
{
    return (this->interaction == 0);
}

bool LoadAndInteraction::calcAtBendMax(void)
{
    return (this->interaction == 1);
}

bool LoadAndInteraction::calcAtAxialMax(void)
{
    return (this->interaction == 2);
}

bool LoadAndInteraction::calcUsingTPI(void)
{
    return (this->interaction == 3);
}
