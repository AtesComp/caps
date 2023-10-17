/*
 * LoadUniform.cpp
 *
 *  Created on: Jan 28, 2022
 *      Author: Keven L. Ates
 */

#include "LoadUniform.hpp"

#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <fmt/format.h>

LoadUniform::LoadUniform(void)
{
    this->clear();
}

LoadUniform::~LoadUniform(void)
{
    this->clear();
}

void LoadUniform::clear(void)
{
    siMemberID = 0;
    adLoadVect[0] = 0.0;
    adLoadVect[1] = 0.0;
}

std::string LoadUniform::report()
{
    std::string strFormat;
    strFormat = strFormat + "   {:3d}   " + SF12_3f + " " + SF12_3f + "\n";

    return
        fmt::vformat(
            strFormat,
            fmt::make_format_args(
                this->siMemberID,
                this->adLoadVect[X], this->adLoadVect[Y]
            )
        );
}
