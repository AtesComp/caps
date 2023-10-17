/*
 * LoadTrapezoidal.cpp
 *
 *  Created on: Jan 28, 2022
 *      Author: Keven L. Ates
 */

#include "LoadTrapezoidal.hpp"

#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <fmt/format.h>

LoadTrapezoidal::LoadTrapezoidal(void)
{
    this->clear();
}

LoadTrapezoidal::~LoadTrapezoidal(void)
{
    this->clear();
}

void LoadTrapezoidal::clear(void)
{
    this->siMemberID = 0;
    this->siID = 0;
    this->adLoad[0] = 0.0;
    this->adLoad[1] = 0.0;
    this->adDist[0] = 0.0;
    this->adDist[1] = 0.0;
    this->dTheta = 0.0;
    this->adStart[0] = 0.0;
    this->adStart[1] = 0.0;
    this->adEnd[0] = 0.0;
    this->adEnd[1] = 0.0;
}

std::string LoadTrapezoidal::report(void)
{
    std::string strFormat;
    strFormat = strFormat +
        "   {:3d}      {:2d}              " +
        SF9_3f + " " + SF9_3f + " " +
        SF9_3f + " " + SF9_3f + "  " +
        SF9_4f + "\n";

    return
        fmt::vformat(
            strFormat,
            fmt::make_format_args(
                this->siMemberID, this->siID,
                this->adLoad[X], this->adLoad[Y], //adLoadVect
                this->adDist[0], this->adDist[1],
                this->dTheta
            )
        );
}
