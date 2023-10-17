/*
 * LoadTrapezoidalExtra.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef LOADTRAPEZOIDALEXTRA_HPP_
#define LOADTRAPEZOIDALEXTRA_HPP_

#include "LoadTrapezoidal.hpp"

// Trapezoidal Load: Deflection calculation storage...

class LoadTrapezoidalExtra
{
public:
    LoadTrapezoidal * tl;           // pointer to trapezoidal load
    double mam1, mam3;
    double vam1;
    double mbm0, mbm1, mbm2, mbm3, mbm4, mbm5;
    double mcm0, mcm1, mcm2, mcm3;
    double vbm0, vbm1, vbm2, vbm3;
    double vcm0, vcm1;

    LoadTrapezoidalExtra(void)
    {
        this->clear();
    }

    ~LoadTrapezoidalExtra(void)
    {
        this->clear();
    }

    void clear(void)
    {
        this->tl = nullptr;

        this->vam1 = 0.0;
        this->mam1 = this->mam3 = 0.0;

        this->vbm0 = this->vbm1 = this->vbm2 = this->vbm3 = 0.0;
        this->mbm0 = this->mbm1 = this->mbm2 = this->mbm3 = this->mbm4 = this->mbm5 = 0.0;

        this->vcm0 = this->vcm1 = 0.0;
        this->mcm0 = this->mcm1 = this->mcm2 = this->mcm3 = 0.0;
    }
};

#endif /* LOADTRAPEZOIDALEXTRA_HPP_ */
