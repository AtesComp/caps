/*
 * MemberAnalysis.hpp
 *
 *  Created on: Aug 20, 2020
 *      Author: Keven L. Ates
 */

#ifndef MEMBERANALYSIS_HPP_
#define MEMBERANALYSIS_HPP_

#include <string>

class MemberAnalysis
{
public:
    std::string strMemberNotes;

    double dForceCompWork;          // Compression Force - Factored Working Value
    double dForceCompPrime;         // Compression Force - Prime Working Value
    double dForceTensWork;          // Tensile Force - Factored Working Value
    double dForceBendWork;          // Bending Force - Factored Working Value
    double dForceBendPrime;         // Bending Force - Prime Working Value

    double dLoD_IP;
    double dLoD_PP;
    double dLoD_Critical;
    double dAction; // ...Compression Stress Index (Bending and Axial)

    double dMaxBend;
    double dMaxBendLoc;
    double dMaxAxial;
    double dMaxAxialLoc;
    double dMaxShear;
    double dMaxShearLoc;
    double dBend;
    double dAxial;

    bool bMixedForces;
    short int siSlenderRatioType;

    bool bCompFlag;

    double dDelta;
    double dDeltaLength;

    void clear(void)
    {
        this->strMemberNotes = "     ";

        this->dForceCompWork = 0.0;
        this->dForceCompPrime = 0.0;
        this->dForceTensWork = 0.0;
        this->dForceBendWork = 0.0;
        this->dForceBendPrime = 0.0;

        this->dLoD_IP = 0.0;
        this->dLoD_PP = 0.0;
        this->dLoD_Critical = 0.0;
        this->dAction = 0.0;

        this->dMaxBend = 0.0;
        this->dMaxBendLoc = 0.0;
        this->dMaxAxial = 0.0;
        this->dMaxAxialLoc = 0.0;
        this->dMaxShear = 0.0;
        this->dMaxShearLoc = 0.0;
        this->dBend = 0.0;
        this->dAxial = 0.0;

        this->bMixedForces = false;
        this->siSlenderRatioType = 0;

        this->bCompFlag = false;

        this->dDelta = 0.0;
        this->dDeltaLength = 0.0;
    }
};

#endif /* MEMBERANALYSIS_HPP_ */
