/*
 * ProblemSize.cpp
 *
 *  Created on: Aug 26, 2020
 *      Author: Keven L. Ates
 */

#include "ProblemSize.hpp"

#include <fmt/format.h>

ProblemSize::ProblemSize(void)
{
    this->clear();
}

ProblemSize::~ProblemSize(void)
{
    this->clear();
}

void ProblemSize::clear(void)
{
    this->uiNodes = 0;
    this->uiMembers = 0;
    this->siRolPin = 0;
    this->siFixPin = 0;
    this->siRolRgd = 0;
    this->siFixRgd = 0;
    this->bNoPrintIn = false;
    this->siPrintOut = 0;
    this->siLoads = 0;
    this->siDivisions = 0;
    this->siNodeFreedom = 0;
    this->siNodePinned = 0;
    this->uiReactions = 0;
}

std::string ProblemSize::report()
{
    //***************************
    //*
    //* Write Problem Size.....
    //*
    //***************************

    std::string strInputReport[6] =
    {   "True",
        "False"
    };
    std::string strOutputControl[6] =
    {   "All Tables",
        "Only Tables 7 and 8",
        "Only Table 9",
        "Only Tables 9 and 10",
        "Only Tables 11 and 12",
        "Only Tables 9 - 12"
    };

    std::string strFormat;
    strFormat = strFormat +
        " Nodes .............. {:d}\n" +
        " Members ............ {:d}\n" +
        " Roller Supports .... {:d}\n" +
        " Pinned Supports .... {:d}\n" +
        " FIRL   Supports .... {:d}\n" +
        " Fixed  Supports .... {:d}\n" +
        " Report Input ....... {:s}\n" +
        " Report Results ..... {:s}\n" +
        " Load Cases ......... {:d}\n" +
        " Member Divisions ... {:d} + start + end\n" +
        "\n" +
        " {:=<80s}\n" +
        "\n";
    return
        fmt::vformat(
            strFormat,
            fmt::make_format_args(
                this->uiNodes,
                this->uiMembers,
                this->siRolPin,
                this->siFixPin,
                this->siRolRgd,
                this->siFixRgd,
                strInputReport[ this->bNoPrintIn ],
                strOutputControl[ this->siPrintOut ],
                this->siLoads,
                this->siDivisions,
                ""
            )
        );
}
