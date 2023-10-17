/*
 * LoadNodal.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef LOADNODAL_HPP_
#define LOADNODAL_HPP_

#include <string>

// System Coordinate (node) Loads Definition: A "CONCENTRATED" System...

class LoadNodal
{
public:
    short int siNodeID;             // Node number to be loaded
    short int siLoadDirection;      // Load Direction Indicator
                                    // Load Direction = 1: Force applied in x direction to the node.
                                    //                  2: Force applied in y direction to the node.
                                    //                  3: Force applied as moment to the node.
    double dLoad;                   // Node Load

    LoadNodal(void);
    ~LoadNodal(void);
    void clear(void);
    std::string report();
};

#endif /* LOADNODAL_HPP_ */
