/*
 * LoadAndInteraction.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef LOADANDINTERACTION_HPP_
#define LOADANDINTERACTION_HPP_

#include <ostream>

// Load Information and Interaction Interpretation Definition...

class LoadAndInteraction
{
public:
    // Concentrated Loads
    bool bPointLoads;               // Concentrated Loads Exist?
    bool bNodeLoads;                // Nodal Loads Exist?
    // Distributed Loads
    bool bUniformLoads;             // Uniform Loads Exist?
    bool bTrapezoidalLoads;         // Trapezoidal Loads flag Exist?

    short int siStressFactorIndex;  // Stress Factor Indicator for StressFactor use:
                                    // 0 or 1: Use Stress Factor 1
                                    //      2: Use Stress Factor 2
                                    //      3: Use Stress Factor 3
                                    // Default == 1
                                    // Thereafter, 0 indicates problem

    LoadAndInteraction(void);
    ~LoadAndInteraction(void);
    void clear(void);
    void setInteraction(short int);
    short int getNDS(void);
    bool isNDS1991(void);
    bool isNDS1986(void);
    bool calcAtBendAndAxialMax(void);
    bool calcAtBendMax(void);
    bool calcAtAxialMax(void);
    bool calcUsingTPI(void);

private:
    short int interaction_input; // Original Input: Interaction Equation Indicator
    short int interaction;      // Interaction Equation Indicator
                                // 0 & 4: Calc Bend & Axial stress max.
                                // 1 & 5: Calc Bend max. & Axial at bend.
                                // 2 & 6: Calc Axial max. & Bend at axial.
                                // 3 & 7: TPI method (no conc or trap loads).
                                // (NDS rules: 0-3 = '91 / 4-7 = '86)
                                // Default == 0
    short nds;                  // NDS Rules
};

#endif /* LOADANDINTERACTION_HPP_ */
