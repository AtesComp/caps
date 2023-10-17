/*
 * DistributedSystem.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef DISTRIBUTEDSYSTEM_HPP_
#define DISTRIBUTEDSYSTEM_HPP_

//  Distributed System Definition...

class DistributedSystem
{
public:
    double adLoadvect[2];                // Load Vector

    DistributedSystem(void);
    ~DistributedSystem(void);
    void clear(void);
};

#endif /* DISTRIBUTEDSYSTEM_HPP_ */
