/*
 * LoadPoint.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef LOADPOINT_HPP_
#define LOADPOINT_HPP_

#include <string>

// Concentrated (point) Loads Definition: A "CONCENTRATED" System...

class LoadPoint
{
public:
    short int siMemberID;       // Member to be loaded
    short int siID;             // ID of the PLOAD
    double adLoadVect[2];       // Load Vector (x and y load)
    double dDistance;           // Distance to load from neg

    LoadPoint(void);
    ~LoadPoint(void);
    void clear(void);
    std::string report(void);
};

#endif /* LOADPOINT_HPP_ */
