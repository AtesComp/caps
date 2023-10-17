/*
 * LoadUniform.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef LOADUNIFORM_HPP_
#define LOADUNIFORM_HPP_

#include <string>

// Uniform Loads Definition: A "DISTRIBUTED" System...

class LoadUniform
{
public:
    short int siMemberID;       // Member to be loaded
    double adLoadVect[2];       // Load Vector (x and y load)

    LoadUniform(void);
    ~LoadUniform(void);
    void clear(void);
    std::string report(void);
};

#endif /* LOADUNIFORM_HPP_ */
