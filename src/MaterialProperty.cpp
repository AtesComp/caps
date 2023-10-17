/*
 * MaterialProperty.cpp
 *
 *  Created on: Aug 19, 2020
 *      Author: Keven L. Ates
 */

#include "MaterialProperty.hpp"
#include "PlaneFrame.hpp"
#include "ReportFormat.hpp"

#include <iomanip>
#include <fmt/format.h>

MaterialProperty::MaterialProperty(void)
{
    this->clear();
}

MaterialProperty::~MaterialProperty(void)
{
    this->clear();
}

void MaterialProperty::clear(void)
{
    this->siID = 0;
    this->dMoE_IP = 0.0;
    this->dMoE_PP = 0.0;
    this->dMoE_Axial = 0.0;
    this->dShearModulus = 0.0;
    this->dThick = 0.0;
    this->dHeight = 0.0;
    this->dArea = 0.0;
    this->dMoI = 0.0;
    this->dSectionModulus = 0.0;
    this->dForceBend = 0.0;
    this->dForceComp = 0.0;
    this->dForceTens = 0.0;
    this->cProperty = 0;
    this->bFictitious = false;
}

//***************************************************************************
//*
//* Derive undeclared Common Values for a Material Property.
//*
//***************************************************************************

void MaterialProperty::deriveCommonValues(bool bIsSpecialCrossSection, bool bIsComposite)
{
    //*
    //* Common calculations for Materials
    //***********************************************************************

    // Special Cross-Section material...
    if ( bIsSpecialCrossSection )
    {
        this->dHeight = 2.0 * this->dMoI / this->dSectionModulus;

        //
        // Set up Fictitious Material information...
        //
        // If the input Shear Modulus variable is set to zero (0.0), it
        // indicates a "Fictitious Material".
        //
        if ( this->dShearModulus == 0.0)
            this->bFictitious = true;
    }
    // Other material...
    else
    {
        // Normal material (not Composite)...
        if ( ! bIsComposite )
        {
            // Set the Shear Modulus value (for Standard Option only)...
            // NOTE: This value is calculated as Modulus of Elasticity of
            //       the Material in ratio to an engineering approximation.
            this->dShearModulus = this->dMoE_Axial / 19.2;
        }

        // Set 'other' values for Normal and Composite material...
        this->dArea = this->dThick * this->dHeight;
        //           I = area MOI = t * h^3 / 12
        this->dMoI = this->dThick * pow(this->dHeight, 3.0) / 12.0;
        //                      I / c = I / (h / 2) = 2I / h = t * h^2 / 6
        this->dSectionModulus = this->dThick * pow(this->dHeight, 2.0) / 6.0;

        //
        // Set up Fictitious Material information...
        //
        // If the input Fc variable is set to zero (0.0), it indicates a
        // "Fictitious Material".  Additionally, if the input Fb variable is
        // set to 9000, it is used as a flag to indicate the use of the
        // current Shear Modulus value in the Material calculations.
        // Otherwise, the Shear Modulus variable is set to zero (0.0) and
        // used as a flag to adjust the member calculations.
        //
        if (this->dForceComp == 0.0)
        {
            this->bFictitious = true;
            if (this->dForceBend != 9000.0) // If Fictitious wo/ Fb flag,
                this->dShearModulus = 0.0;
        }
    }

    //
    // Adjust Modulus of Elasticity values...
    // NOTE: Option 'S' and 'C' have input inclusions for some or all of
    //       these values, otherwise the In-Plane and/or Perpendicular-Plane
    //       are made equal to the Axial.
    //
    if (this->dMoE_IP == 0.0)
        this->dMoE_IP = this->dMoE_Axial;
    if (this->dMoE_PP == 0.0)
        this->dMoE_PP = this->dMoE_Axial;
}

bool MaterialProperty::isFictitious(void)
{
    return this->bFictitious;
}

//***************************************************************************
//*
//* Reports a Material Property type as a string.
//*
//***************************************************************************

std::string MaterialProperty::typeToString(void)
{
    // Convert Material type code to Material string:
    //  S = Supported
    //  U = Unsupported
    //  I = Interior Material (web, otherwise chord)
    //  T = Truss Chord Material (Supported)
    //  D = Seasoned
    //  G = Green
    //  M = Machine Stress Rated (MSR) OR Machine Evaluated Lumber (MEL)
    //  2 = Type 2 Composite

    if (this->dForceComp == 0.0) // Fictitious
        return "FICT";

    switch (this->cProperty)
    {
        case ' ':
        case '0': // Supported Material
            return "S   ";
            break;
        case '1': // Unsupported Material
            return "U   ";
            break;
        case '2': // Supported Interior Material
            return "SI  ";
            break;
        case '3': // Unsupported Interior Material
            return "UI  ";
            break;
        case '4': // Truss Chord, Seasoned
            return "TD  ";
            break;
        case '5': // Truss Chord, Green
            return "TG  ";
            break;
        case '6': // MSR/MEL Truss Chord, Seasoned
            return "MTD ";
            break;
        case '7': // MSR/MEL Truss Chord, Green
            return "MTG ";
            break;
        case '8': // MSR/MEL Supported Material
            return "MS  ";
            break;
        case '9': // MSR/MEL Unsupported Material
            return "MU  ";
            break;
        case 'A': // MSR/MEL Supported Interior Material
            return "MSI ";
            break;
        case 'B': // MSR/MEL Unsupported Interior Material
            return "MUI ";
            break;
        case 'C': // MSR/MEL Unsupported Composite, Type 2
            return "MU2 ";
            break;
        case 'D': // MSR/MEL Unsupported Interior Composite, Type 2
            return "MUI2";
            break;
    }

    return "????";
}

//***************************************************************************
//*
//* Report Material Properties...
//*
//***************************************************************************

void MaterialProperty::report(std::ostream & osOut, bool bIsSpecialCrossSection, bool bIsComposite)
{
    //
    // Table 1 body...
    //
    osOut << std::right;

    // Print the current Material Property Group ID...
    osOut << "   " << std::setw(3) << this->siID << "  ";

    if ( bIsSpecialCrossSection )
    {
        osOut << std::setw(9) << fmt::format(SF9_3f, this->dArea) << " ";
        osOut << std::setw(9) << fmt::format(SF9_3f, this->dMoI) << " ";
        osOut << std::setw(9) << fmt::format(SF9_3f, this->dSectionModulus) << "   ";
        osOut << std::setw(10) << fmt::format(SF10_3E, this->dMoE_IP) << " ";
        osOut << std::setw(10) << fmt::format(SF10_3E, this->dMoE_Axial) << " ";
        osOut << std::setw(10) << fmt::format(SF10_3E, this->dShearModulus) << "\n";
    }
    else
    {
        osOut << " " << std::setw(4) << this->typeToString() << " ";
        osOut << std::setw(6) << fmt::format(SF6_0f, this->dForceBend);
        osOut << std::setw(6)<< fmt::format(SF6_0f, this->dForceComp);
        osOut << std::setw(6)<< fmt::format(SF6_0f, this->dForceTens);
        osOut << std::setw(7) << fmt::format(SF7_3f, this->dThick);
        osOut << std::setw(7) << fmt::format(SF7_3f, this->dHeight) << " ";

        if ( bIsComposite )
        {
            osOut << std::setw(10) << fmt::format(SF10_3E, this->dMoE_IP);
            osOut << std::setw(10) << fmt::format(SF10_3E, this->dMoE_PP);
            osOut << std::setw(10) << fmt::format(SF10_3E, this->dMoE_Axial);
            osOut << std::setw(10) << fmt::format(SF10_3E, this->dShearModulus) << "\n";
        }
        else
        {
            osOut << std::setw(10) << fmt::format(SF12_4E, this->dMoE_Axial);
            osOut << std::setw(10) << fmt::format(SF12_4E, this->dShearModulus) << "\n";
        }
    }
}
