/*
 * LoadPoint.cpp
 *
 *  Created on: Jan 28, 2022
 *      Author: Keven L. Ates
 */

#include "LoadPoint.hpp"

#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <fmt/format.h>

LoadPoint::LoadPoint(void)
{
    this->clear();
}

LoadPoint::~LoadPoint(void)
{
    this->clear();
}

void LoadPoint::clear(void)
{
    siMemberID = 0;
    siID = 0;
    adLoadVect[0] = 0.0;
    adLoadVect[1] = 0.0;
    dDistance = 0.0;
}

std::string LoadPoint::report()
{
    std::string strFormat;
    strFormat = strFormat + "   {:3d}     {:3d}   " + SF12_3f + " " + SF12_3f + "    " + SF12_3f + "\n";

    return
        fmt::vformat(
            strFormat.c_str(),
            fmt::make_format_args(
                this->siMemberID, this->siID,
                this->adLoadVect[X], this->adLoadVect[Y],
                this->dDistance
            )
        );
}
