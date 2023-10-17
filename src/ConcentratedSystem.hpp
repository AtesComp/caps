/*
 * ConcentratedSystem.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef CONCENTRATEDSYSTEM_HPP_
#define CONCENTRATEDSYSTEM_HPP_

// Concentrated System Definition...

class ConcentratedSystem
{
public:
    double dLoadDist;               // Load Distance
    double adLoadVect[2];           // Load Vector

    ConcentratedSystem(void);
    ~ConcentratedSystem(void);
    void clear(void);
};

#endif /* CONCENTRATEDSYSTEM_HPP_ */
