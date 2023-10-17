/*
 * MaterialProperty.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef MATERIALPROPERTY_HPP_
#define MATERIALPROPERTY_HPP_

#include <ostream>
#include <string>

// Material Property Definition...
//      NORMAL / GluLam, LVL, PSL / Special Cross-section

class MaterialProperty
{
public:
    short int siID;                 // Group Number
    double dMoE_IP;                 // Modulus of Elasticity: In-Plane
    double dMoE_PP;                 // Modulus of Elasticity: Perp-Plane
    double dMoE_Axial;              // Modulus of Elasticity: Axial
    double dShearModulus;           // Shear Modulus
    double dThick;                  // Thickness
    double dHeight;                 // Height
    double dArea;                   // Area
    double dMoI;                    // Moment of Inertia
    double dSectionModulus;         // Section Modulus
    double dForceBend;              // Allowable Bending
    double dForceComp;              // Allowable Compression
    double dForceTens;              // Allowable Tension
    char cProperty;                 // Property Indicator:
                                    // 0: [S   ] Laterally [S]upported   Material as by sheathing, etc
                                    // 1: [U   ] Laterally [U]nsupported Material
                                    // 2: [SI  ] Laterally [S]upported   [I]nterior Material as by bracing
                                    // 3: [UI  ] Laterally [U]nsupported [I]nterior Material
                                    // 4: [TD  ] [T]russ Chord, seasoned ([D]ry), supported           (NDS)
                                    // 5: [TG  ] [T]russ Chord, [G]reen,          supported           (NDS)
                                    // 6: [MTD ] [M]SR/MEL [T]russ Chord, seasoned ([D]ry), supported (NDS)
                                    // 7: [MTG ] [M]SR/MEL [T]russ Chord, [G]reen,          supported (NDS)
                                    // 8: [MS  ] [M]SR/MEL Laterally [S]upported   Material
                                    // 9: [MU  ] [M]SR/MEL Laterally [U]nsupported Material
                                    // A: [MSI ] [M]SR/MEL Laterally [S]upported   [I]nterior Material
                                    // B: [MUI ] [M]SR/MEL Laterally [U]nsupported [I]nterior Material
                                    // C: [MU2 ] [M]SR/MEL Laterally [U]nsupported Material Type [2] (see 9)
                                    // D: [MUI2] [M]SR/MEL Laterally [U]nsupported [I]nterior Material Type [2] (see B)

    MaterialProperty(void);
    ~MaterialProperty(void);
    void clear(void);
    std::string typeToString(void);

    void deriveCommonValues(bool, bool);
    bool isFictitious(void);
    void report(std::ostream &, bool, bool);

private:
    bool bFictitious;
};

#endif /* MATERIALPROPERTY_HPP_ */
