/*
 * StressFactors.cpp
 *
 *  Created on: Oct 7, 2020
 *      Author: Keven L. Ates
 */

#include "StressFactors.hpp"
#include "ReportFormat.hpp"

#include <fmt/format.h>

StressFactors::StressFactors(void)
{
this->clear();
}

StressFactors::~StressFactors(void)
{
    this->clear();
}

void StressFactors::clear(void)
{
    this->factor[0] = 0;
    factor[1] = 0;
    factor[2] = 0;
}

std::string StressFactors::report(short int siStressFactorIndex)
{
    if ( siStressFactorIndex == 0 )
        return " Bad Stress Factor Index in input. Range: blank,0-3 (blank & 0 indicate 1). Defaulting to 1.0";
    else
        return
            fmt::format(SF5_2f, this->factor[ siStressFactorIndex - 1 ]);
}

