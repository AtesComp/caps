/*
 * LoadTrapezoidal.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef LOADTRAPEZOIDAL_HPP_
#define LOADTRAPEZOIDAL_HPP_

#include <string>

// Trapezoidal Loads Definition: A "DISTRIBUTED" System...

class LoadTrapezoidal
{
public:
    short int siMemberID;       // Member to be loaded
    short int siID;             // ID of the load
    double adLoad[2];           // Load:     0=Start 1=End
    double adDist[2];           // Distance: 0=Start 1=End
    double dTheta;              // Angle of Load to Member

    // Calculated values...
    double adStart[2];          // Starting (X,Y) on member
    double adEnd[2];            // Ending (X,Y) on Member

    LoadTrapezoidal(void);
    ~LoadTrapezoidal(void);
    void clear(void);
    std::string report(void);
};

#endif /* LOADTRAPEZOIDAL_HPP_ */
