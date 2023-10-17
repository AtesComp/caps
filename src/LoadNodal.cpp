/*
 * LoadNodal.cpp
 *
 *  Created on: Jan 28, 2022
 *      Author: Keven L. Ates
 */

#include "LoadNodal.hpp"

#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <fmt/format.h>

LoadNodal::LoadNodal(void)
{
    this->clear();
}

LoadNodal::~LoadNodal(void)
{
    this->clear();
}

void LoadNodal::clear(void)
{
    siNodeID = 0;
    siLoadDirection = 0;
    dLoad = 0.0;
}

std::string LoadNodal::report()
{
    std::string strLoadDir[3] = { "X", "Y", "Moment" };
    std::string strFormat;
    strFormat = strFormat + "   {:3d}      {:>6}       " + SF12_3f + "\n";

    return
        fmt::vformat(
            strFormat,
            fmt::make_format_args(
                this->siNodeID, strLoadDir[this->siLoadDirection - 1],
                this->dLoad
            )
        );
}
