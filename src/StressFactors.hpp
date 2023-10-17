/*
 * StressFactors.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef STRESSFACTORS_HPP_
#define STRESSFACTORS_HPP_

#include <string>

// Stress Factor Definition...
//      Stress Factor Indicator is with the Load Information and
//      Interaction Interpretation Definition (f_ind)

class StressFactors
{
public:
    double factor[3];               // Stress Factors

    StressFactors(void);
    ~StressFactors(void);
    void clear(void);
    std::string report(short int siStressFactorIndex);
};

#endif /* STRESSFACTORS_HPP_ */
