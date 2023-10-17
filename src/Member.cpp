/*
 * Member.cpp
 *
 *  Created on: Aug 15, 2020
 *      Author: Keven L. Ates
 */

#include "Member.hpp"
#include "LoadTrapezoidalExtra.hpp"

#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <iomanip>
#include <iostream>
#include <fmt/format.h>
#include <algorithm>

const double Member::dInteriorModifier = 0.8;
bool Member::bIsComposite = false;

Member::Member(void)
{
    this->conc.clear();
    this->dist.clear();
    this->clear();
}

Member::~Member(void)
{
    this->clear();
}

void Member::clear(void)
{
    this->siID = 0;
    this->siNegNodeID = 0;
    this->siPosNodeID = 0;
    this->siMatPropID = 0;
    this->bJointNeg = false;
    this->bJointPos = false;
    this->dEffColumnLength_IP = 0.0;
    this->dEffColumnLength_PP = 0.0;
    this->dEffBendingLength = 0.0;
    this->dVolumeFactor = 0.0;

    this->dLength = 0.0;
    this->dCos = 0.0;
    this->dSin = 0.0;

    this->adNodeVectorNeg[X] = 0.0;
    this->adNodeVectorNeg[Y] = 0.0;
    this->adNodeVectorPos[X] = 0.0;
    this->adNodeVectorPos[Y] = 0.0;

    // MaterialProperty *mp is a reference; deleted by MaterialProperty
    this->mp = nullptr;
    // Node *nodeNeg is a reference; deleted by Node
    this->nodeNeg = nullptr;
    // Node *nodePos is a reference; deleted by Node
    this->nodePos = nullptr;

    this->clearLoadAnalysis();
}

void Member::clearLoadAnalysis(void)
{
    if ( ! this->conc.empty() )
    {
        for ( ConcentratedSystem * concCurr : this->conc )
            delete concCurr;
        this->conc.clear();
    }

    if ( ! this->dist.empty() )
    {
        for ( DistributedSystem * distCurr : this->dist )
            delete distCurr;
        this->dist.clear();
    }

    this->adLocalForce[0] = 0.0;
    this->adLocalForce[1] = 0.0;
    this->adLocalForce[2] = 0.0;
    this->adLocalForce[3] = 0.0;
    this->adLocalForce[4] = 0.0;
    this->adLocalForce[5] = 0.0;

    this->adGlobalForce[0] = 0.0;
    this->adGlobalForce[1] = 0.0;
    this->adGlobalForce[2] = 0.0;
    this->adGlobalForce[3] = 0.0;
    this->adGlobalForce[4] = 0.0;
    this->adGlobalForce[5] = 0.0;

    this->ma.clear();
}

void Member::deriveCommonValues(void)
{
    // Ensure volume factor for Composite is 1.0 if not set...
    if ( Member::bIsComposite && this->dVolumeFactor == 0.0 )
        this->dVolumeFactor = 1.0;
}

bool Member::isType2Composite(void)
{
    return (this->mp->cProperty == 'C' || this->mp->cProperty == 'D');
}

bool Member::isLaterallySupported(void)
{
    // Lateral Support flag...
    //      true  =    Lateral Support OR  Perp-Plane thickness is greater than height.
    //      false = No Lateral Support AND Perp-Plane thickness is less than or equal to height.
    return ( this->mp->cProperty == '0' || this->mp->cProperty == '2' ||
             ( this->mp->cProperty > '3' && this->mp->cProperty < '9' ) ||
             this->mp->cProperty == 'A' ||
             this->mp->dThick > this->mp->dHeight );
    //return ( ! ( ( this->mp->cProperty == '1' || this->mp->cProperty == '3' || this->mp->cProperty == '9' ||
    //               this->mp->cProperty == 'B' || this->mp->cProperty == 'C' || this->mp->cProperty == 'D' ) &&
    //             this->mp->dThick <= this->mp->dHeight ) );
    //return ( ! ( this->mp->cProperty == '1' || this->mp->cProperty == '3' || this->mp->cProperty == '9' ||
    //             this->mp->cProperty == 'B' || this->mp->cProperty == 'C' || this->mp->cProperty == 'D' ) ||
    //            this->mp->dThick > this->mp->dHeight );
    //return ( ( this->mp->cProperty != '1' && this->mp->cProperty != '3' && this->mp->cProperty != '9' &&
    //           this->mp->cProperty != 'B' && this->mp->cProperty != 'C' && this->mp->cProperty != 'D' ) ||
    //         this->mp->dThick > this->mp->dHeight );
}

bool Member::isInteriorMember(void)
{
    // Interior Member flag...
    //      true = Interior Member.
    //      false = Not an Interior Member.
    return (this->mp->cProperty == '2' || this->mp->cProperty == '3' ||
            this->mp->cProperty == 'A' || this->mp->cProperty == 'B' ||
            this->mp->cProperty == 'D');
}

bool Member::isTrussChord(void)
{
    // Truss Chord flag...
    //      Truss Chord == NDS defined Truss member
    //      true = NDS Truss Chord type.
    //      false = Not an NDS Truss Chord type.
    return (this->mp->cProperty == '4' || this->mp->cProperty == '5' ||
            this->mp->cProperty == '6' || this->mp->cProperty == '7');
}

bool Member::isMaterialMachineTested(void)
{
    // Machine Tested flag...
    //      Machine Tested == Machine Stress Rated (MSR)
    //      Machine Evaluated Lumber (MEL) is comparable to Visually Graded by NDS
    //      true = MSR type (coefficient of variation in Modulus of Elasticity of 0.11 or less)
    //      false = Visually Graded or MEL type (coefficient of variation in Modulus of Elasticity of 0.25)
    return (this->mp->cProperty == '6' || this->mp->cProperty == '7' ||
            this->mp->cProperty == '8' || this->mp->cProperty == '9' ||
            this->mp->cProperty == 'A' || this->mp->cProperty == 'B' ||
            this->mp->cProperty == 'C' || this->mp->cProperty == 'D' );
}

//***************************************************************************
//*
//* Member::process
//*     Analyze Member information related to the Nodes that define the
//*     member's location.
//*
//***************************************************************************

bool Member::process(std::vector<MaterialProperty *> & mps, std::vector<Node *> & nodes)
{
    //
    // Find Member Property of Member in Member Properties...
    //

    if ( mps.empty() )
    {
        std::cerr << "\n ERROR: No Member Properties for Member processing!\n";
        return false;
    }

    if ( nodes.empty() )
    {
        std::cerr << "\n ERROR: No Nodes for Member processing!\n";
        return false;
    }

    for (MaterialProperty * mpCurr : mps)
    {
        if (this->siMatPropID == mpCurr->siID)
        {
            // Set Member Property pointer for Member...
            this->mp = mpCurr;
            break;
        }
    }
    if (this->mp == nullptr)
    {
        std::cerr << "\n ERROR: Material property (" << this->siMatPropID << ") in Member (" << this->siID  << ")\n"
                  <<   "        not found in Material Properties!\n";
        return false;
    }

    //
    // Find NEG node of Member in Nodes...
    //

    for (Node * nodeCurr : nodes)
    {
        if (this->siNegNodeID == nodeCurr->siID)
        {
            // Set Negative Node pointer for Member and Node Sequence...
            this->nodeNeg = nodeCurr;
            nodeCurr->siSequence = 1; // ...set node to "Used" (temporary use)
            nodeCurr->siDegrees++;
            break;
        }
    }
    if (this->nodeNeg == nullptr)
    {
        std::cerr << "\n ERROR: Neg Node (" << this->siNegNodeID << ") in  Member (" << this->siID << ")\n"
                  <<   "        not found in Nodes!\n";
        return false;
    }

    //
    // Find POS node of Member in Nodes...
    //

    for (Node * nodeCurr : nodes)
    {
        if (this->siPosNodeID == nodeCurr->siID)
        {
            // Set Positive Node pointer for Member and Node Sequence...
            this->nodePos = nodeCurr;
            nodeCurr->siSequence = 1; // ...set node to "Used" (temporary use)
            nodeCurr->siDegrees++;
            break;
        }
    }
    if (this->nodePos == nullptr)
    {
        std::cerr << "\n ERROR: Pos Node (" << this->siPosNodeID << ") in  Member (" << this->siID << ")\n"
                  <<   "        not found in Nodes!\n";
        return false;
    }

    //
    // Load D Matrix...
    //

    double X_dist = this->nodePos->adPoint[X] - this->nodeNeg->adPoint[X];
    double Y_dist = this->nodePos->adPoint[Y] - this->nodeNeg->adPoint[Y];
    this->dLength = sqrt(X_dist * X_dist + Y_dist * Y_dist);
    if (this->dLength == 0.0)
    {
        std::cerr << "\n ERROR: Member " << this->siID << " has no length!!\n"
                  <<   "        All members must have a length between nodes.\n";
        return false;
    }
    this->dCos = X_dist / this->dLength; // Normal X
    this->dSin = Y_dist / this->dLength; // Normal Y

    // Set up member's Negative Node Roll Vector...
    this->adNodeVectorNeg[X] = this->dCos;
    this->adNodeVectorNeg[Y] = this->dSin;

    // Set up member's Positive Node Roll Vector...
    this->adNodeVectorPos[X] = this->dCos;
    this->adNodeVectorPos[Y] = this->dSin;

    //
    // A Member's Joint connection flags set a Node's MemberFixture to
    // FIXED (false) if the Node has at least one Member rigidly connected.
    // If all Member's attached to the node are FREE (true), the Node's
    // MemberFixture is set to FREE (true, default).
    // See PlaneFrame::findNodeFreedom() for more information on how this
    // setting affects the moment calculation at a Node.
    //
    this->nodeNeg->bMemberFixture = ( this->nodeNeg->bMemberFixture && this->bJointNeg );
    this->nodePos->bMemberFixture = ( this->nodePos->bMemberFixture && this->bJointPos );

    return true;
}

//*
//* End of Member::process
//***************************************************************************

//***************************************************************************
//*
//* Member::loadMember
//*     This function sets up load forces for a member and
//*     contributes to a global force matrix.
//*
//*     The LOAD TYPES are divided into two categories:
//*
//*         "CONCENTRATED" Systems          "DISTRIBUTED" Systems
//*             Point                           Uniform
//*             Nodal                           Trapezoidal
//*
//***************************************************************************

bool Member::loadMember(std::vector<LoadPoint *> & pl,
                        std::vector<LoadUniform *> & ul,
                        std::vector<LoadTrapezoidal *> & tl,
                        double * adForceMatrix)
{
    double adConcForce[6] = { 0.0 };
    double adDistForce[6] = { 0.0 };

    //*
    //* Analyze Member Information for Stiffness and Force Vectors.....
    //***********************************************************************

    //
    // Calculate Fixed End Forces...
    //

    double dReciprocalLength = 1.0 / this->dLength;
    double dLength2 = this->dLength * this->dLength;
    double dLength3 = dLength2 * this->dLength;
    double dFixEndForce = 0.0;
    if (this->mp->dShearModulus != 0.0) // not Fictitious
        dFixEndForce = (this->mp->dMoE_Axial * (this->mp->dHeight * this->mp->dHeight)) / (2.0 * this->mp->dShearModulus * dLength2);
    double dForceRatio = (1.0 - dFixEndForce) / (2.0 + dFixEndForce);

    //
    // Compile Concentrated Loads for Member...
    //

    for (LoadPoint * plCurr : pl)
    {
        if (plCurr->siMemberID != this->siID)
            continue;

        //
        // Allocate Memory for Concentrated System...
        //

        ConcentratedSystem * concCurr = nullptr;
        if ( ( concCurr = new ConcentratedSystem ) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      << "          Concentrated System!\n";
            return false;
        }

        //
        // Set up "CONCENTRATED" System values...
        //

        double adConcForceCurr[6] = { 0.0 };
        double adConcVector[2] = { 0.0 };

        double dLoadDistFromNeg = plCurr->dDistance;
        double dLoadDistFromPos = this->dLength - plCurr->dDistance;
        adConcVector[X] = this->dSin * plCurr->adLoadVect[Y] + this->dCos * plCurr->adLoadVect[X]; // Fx
        adConcVector[Y] = this->dCos * plCurr->adLoadVect[Y] - this->dSin * plCurr->adLoadVect[X]; // Fy

        adConcForceCurr[0] = (-adConcVector[X]) * dLoadDistFromPos * dReciprocalLength; // -Fx b / L
        adConcForceCurr[3] = (-adConcVector[X]) * dLoadDistFromNeg * dReciprocalLength; // -Fx a / L
        if (this->bJointNeg && this->bJointPos) // (1 && 1)
        {
            adConcForceCurr[1] = (-adConcVector[Y]) * dLoadDistFromPos * dReciprocalLength; // -Fy b / L
            adConcForceCurr[2] = 0.0; // No Moment
            adConcForceCurr[4] = (-adConcVector[Y]) * dLoadDistFromNeg * dReciprocalLength; // -Fy a / L
            adConcForceCurr[5] = 0.0; // No Moment
        }
        else // (1 && 0), (0 && 1), (0 && 0)
        {
            adConcForceCurr[2] = (-adConcVector[Y]) * (dLoadDistFromPos * dLoadDistFromPos) * dLoadDistFromNeg / dLength2; // -Fy a b^2 / L^2 [Y Bend Moment @ Neg]
            adConcForceCurr[5] =   adConcVector[Y]  * (dLoadDistFromNeg * dLoadDistFromNeg) * dLoadDistFromPos / dLength2; //  Fy a^2 b / L^2 [Y Bend Moment @ Pos]

            if (this->bJointNeg)      // (1 && 0)
            {
                adConcForceCurr[5] -= (dForceRatio * adConcForceCurr[2]); //  Fy a^2 b / L^2 + FR Fy a b^2 / L^2 = ( Fy a b / L^2 )( a + FR b )
                adConcForceCurr[2] = 0.0; // No Moment
                adConcForceCurr[4] = -(adConcForceCurr[5] + adConcVector[Y] * dLoadDistFromNeg) * dReciprocalLength; // - ( ( Fy a b / L^2 )( a + FR b ) + Fy a ) / L =
                                                                              // - ( Fy a / L ) ( b ( a + FR b ) / L^2 + 1 )
                adConcForceCurr[1] = -(adConcForceCurr[4] + adConcVector[Y]); //   ( Fy a / L ) ( b ( a + FR b ) / L^2 + 1 - Fy
            }
            else if (this->bJointPos) // (0 && 1)
            {
                adConcForceCurr[2] -= (dForceRatio * adConcForceCurr[5]); // -Fy a b^2 / L^2 - FR Fy a^2 b / L^2 = -( Fy a b / L^2 )( b + FR a )
                adConcForceCurr[5] = 0.0; // No Moment
                adConcForceCurr[1] =  (adConcForceCurr[2] - adConcVector[Y] * dLoadDistFromPos) * dReciprocalLength; //   (-( Fy a b / L^2 )( b + FR a ) - Fy b ) / L =
                                                                              // - ( Fy b / L ) ( a ( b + FR a ) / L^2 + 1 )
                adConcForceCurr[4] = -(adConcForceCurr[1] + adConcVector[Y]); //   ( Fy b / L ) ( a ( b + FR a ) / L^2 + 1 ) - Fy
            }
            else                      // (0 && 0)
            {
                adConcForceCurr[1] =  (adConcForceCurr[2] + adConcForceCurr[5] - adConcVector[Y] * dLoadDistFromPos) * dReciprocalLength;
                                  //  ( -Fy a b^2 / L^2 + Fy a^2 b / L^2 - Fy b ) / L
                                  // -( Fy b / L ) ( a b / L^2 - a a / L^2 + 1)
                                  // -( Fy b / L ) ( a ( b - a ) / L^2 + 1 )
                                  // -5/32 Fy; a=3/4, b = 1/4
                adConcForceCurr[4] = -(adConcForceCurr[1] + adConcVector[Y]);
                                  // ( Fy b / L ) ( a ( b - a ) / L^2 + 1 ) - Fy
                                  // -27/32 Fy; a=3/4, b = 1/4
                //  Symmetric Eq   = -(adConcForceCurr[2] + adConcForceCurr[5] + adConcVector[Y] * dLoadDistFromNeg) * dReciprocalLength;
                                  // -( -Fy a b^2 / L^2 + Fy a^2 b / L^2 + Fy a ) / L =
                                  // -( Fy a / L ) (-b b / L^2 + a b / L^2 + 1 ) =
                                  // -( Fy a / L ) ( b ( a - b ) / L^2 + 1 ) --> Symmetric
                                  // -27/32 Fy; a=3/4, b = 1/4
                                  // Equivalence Check:
                                  // -( Fy a / L ) ( b ( a - b ) / L^2 + 1 )
                                  // -( Fy a / L ) ( ( a b - b^2 ) / L^2 + L^2 / L^2 )
                                  // -( Fy a / L ) ( L^2 + a b - b^2 ) / L^2 )
                                  // -( Fy a / L ) ( ( a + b )^2 + a b - b^2 ) / L^2 )
                                  // -( Fy a / L ) ( a^2 + 2 a b + b^2 + a b - b^2 ) / L^2 )
                                  // -( Fy a / L ) ( a^2 + 3 a b ) / L^2 )
                                  // -( Fy / L ) (a^3 + 3 a^2 b ) / L^2 )
                                  // -( Fy / L ) (a^3 + 3 a^2 b + 3 a b^2 + b^3 - 3 a b^2 - b^3 ) / L^2 )
                                  // -( Fy / L ) (L^3 - 3 a b^2 - b^3 ) / L^2 )
                                  //  ( Fy / L ) (3 a b^2 + b^3 - L^3 ) / L^2 )
                                  //  ( Fy / L ) (a b^2 - a^2 b + a^2 b + 2 a b^2 + b^3 - L^3 ) / L^2 )
                                  //  ( Fy / L ) (a b^2 - a^2 b + b ( a^2 + 2 a b + b^2 ) - L^3 ) / L^2 )
                                  //  ( Fy / L ) (a b^2 - a^2 b + b L^2 - L^3 ) / L^2 )
                                  //  Fy ( b ( a b - a^2 + L^2 ) - L^3 ) / L^3
                                  //  Fy ( b ( a b - a^2 + L^2 ) / L^3 - 1 )
                                  //  Fy ( ( b / L ) ( a b - a^2 + L^2) / L^2 - 1 )
                                  //  Fy ( ( b / L ) ( ( a b - a^2 ) / L^2 + 1 ) - 1 )
                                  //  ( Fy b / L ) ( a ( b - a ) / L^2 + 1 ) - Fy --> Equivalent
                                  //
                                  // -5/32 Fy + -27/32 Fy = -Fy --> Total Force
            }
        }

        adConcForce[0] += adConcForceCurr[0];
        adConcForce[1] += adConcForceCurr[1];
        adConcForce[2] += adConcForceCurr[2];
        adConcForce[3] += adConcForceCurr[3];
        adConcForce[4] += adConcForceCurr[4];
        adConcForce[5] += adConcForceCurr[5];

        //
        // Store "CONCENTRATED" Member values...
        //

        concCurr->dLoadDist = dLoadDistFromNeg;
        concCurr->adLoadVect[X] = adConcVector[X];
        concCurr->adLoadVect[Y] = adConcVector[Y];

        this->conc.push_back(concCurr);
    }

    //
    // Compile Uniform Loads for Member...
    //

    for (LoadUniform * ulCurr : ul)
    {
        if (ulCurr->siMemberID != this->siID)
            continue;

        //
        // Allocate Memory for Distributed System...
        //

        DistributedSystem * distCurr = nullptr;
        if ( (distCurr = new DistributedSystem ) == nullptr )
        {
            std::cerr << "\n Error: OUT OF MEMORY!!\n"
                      << "          Distributed System!\n";
            return false;
        }

        //
        // Set up "DISTRIBUTED" System values...
        //

        double dDistForceCurr[6] = { 0.0 };
        double dDistVector[2] = { 0.0 };

        if (ulCurr->adLoadVect[X] != 0.0 || ulCurr->adLoadVect[Y] != 0.0)
        {
            dDistVector[X] =
                ulCurr->adLoadVect[Y] * this->dSin * fabs(this->dCos) +
                ulCurr->adLoadVect[X] * this->dCos * fabs(this->dSin);

            dDistVector[Y] =
                ulCurr->adLoadVect[Y] * this->dCos * fabs(this->dCos) -
                ulCurr->adLoadVect[X] * this->dSin * fabs(this->dSin);

            dDistForceCurr[0] = (-dDistVector[X]) * this->dLength / 2.0;
            dDistForceCurr[1] = (-dDistVector[Y]) * this->dLength / 2.0;
            dDistForceCurr[3] = dDistForceCurr[0];
            dDistForceCurr[4] = dDistForceCurr[1];

            if (this->bJointNeg && this->bJointPos) // (1 && 1)
            {
                dDistForceCurr[2] = 0.0;
                dDistForceCurr[5] = 0.0;
            }
            else                          // (1 && 0), (0 && 1), (0 && 0)
            {
                dDistForceCurr[5] = dDistVector[Y] * dLength2 / 12.0;
                dDistForceCurr[2] = -dDistForceCurr[5];

                if (this->bJointNeg)      // (1 && 0)
                {
                    dDistForceCurr[2] = 0.0;
                    dDistForceCurr[5] *= (1.0 + dForceRatio);
                    dDistForceCurr[4] = -(dDistForceCurr[5] + dDistVector[Y] * dLength2 / 2.0) * dReciprocalLength;
                    dDistForceCurr[1] = -(dDistForceCurr[4] + dDistVector[Y] * this->dLength);
                }
                else if (this->bJointPos) // (0 && 1)
                {
                    dDistForceCurr[5] = 0.0;
                    dDistForceCurr[2] *= (1.0 + dForceRatio);
                    dDistForceCurr[1] =  (dDistForceCurr[2] - dDistVector[Y] * dLength2 / 2.0) * dReciprocalLength;
                    dDistForceCurr[4] = -(dDistForceCurr[1] + dDistVector[Y] * this->dLength);
                }
            }
        }

        adDistForce[0] += dDistForceCurr[0];
        adDistForce[1] += dDistForceCurr[1];
        adDistForce[2] += dDistForceCurr[2];
        adDistForce[3] += dDistForceCurr[3];
        adDistForce[4] += dDistForceCurr[4];
        adDistForce[5] += dDistForceCurr[5];

        //
        // Store "DISTRIBUTED" Member values...
        //

        distCurr->adLoadvect[X] = dDistVector[X];
        distCurr->adLoadvect[Y] = dDistVector[Y];

        this->dist.push_back(distCurr);
    }

    //
    // Compile Nodal Loads for Member...
    //
    //      This was performed in PlaneFrame::processLoads() and applied to the node forces.
    //

    //
    // Compile Trapezoidal Loads for Member...
    //

    double dShearArea = this->mp->dShearModulus * this->mp->dArea;
    double dEI = this->mp->dMoE_Axial * this->mp->dMoI;

    for (LoadTrapezoidal * tlCurr : tl)
    {
        if (tlCurr->siMemberID == this->siID)
        {
            if (tlCurr->adDist[0] != 0.0 || tlCurr->adDist[1] != 0.0)
            {
                double dLengthStart = 0.0;
                double dVectorStart[2] = { 0.0 };
                double dLengthEnd = 0.0;
                double dVectorEnd[2] = { 0.0 };

                // (0 && 0), (0 && 1), (1 && 1)
                if ( ! this->bJointNeg || this->bJointPos )
                {
                    dLengthStart = tlCurr->adDist[0];
                    dLengthEnd   = this->dLength - tlCurr->adDist[1];
                    dVectorStart[X] = tlCurr->adStart[X];
                    dVectorStart[Y] = tlCurr->adStart[Y];
                    dVectorEnd[X]   = tlCurr->adEnd[X];
                    dVectorEnd[Y]   = tlCurr->adEnd[Y];
                }
                // (1 && 0)
                else // (this->neg_joint && ! this->pos_joint)
                {   // SWAP...
                    dLengthStart = this->dLength - tlCurr->adDist[1];
                    dLengthEnd   = tlCurr->adDist[0];
                    dVectorStart[X] = tlCurr->adEnd[X];
                    dVectorStart[Y] = tlCurr->adEnd[Y];
                    dVectorEnd[X]   = tlCurr->adStart[X];
                    dVectorEnd[Y]   = tlCurr->adStart[Y];
                }

                double dLengthTrap  = tlCurr->adDist[1] - tlCurr->adDist[0];
                double dLengthTrap2 = dLengthTrap * dLengthTrap;

                double dDeltaX = dVectorStart[X] - dVectorEnd[X];
                double dDeltaY = dVectorStart[Y] - dVectorEnd[Y];
                double dR1 = dVectorEnd[Y] * dLengthTrap +
                                   dDeltaY * dLengthTrap / 2.0;
                double dM1 = dVectorEnd[Y] * dLengthTrap       * (dLengthStart + dLengthTrap / 2.0) +
                                   dDeltaY * dLengthTrap / 2.0 * (dLengthStart + dLengthTrap / 3.0);

                double dX1 = 0.0, dX2 = 0.0, dX4 = 0.0, dX5 = 0.0;
                if (this->bJointNeg && this->bJointPos) // 1 && 1
                {
                    double dTemp = dM1 * dReciprocalLength;
                    dX1 = dTemp - dR1;
                    dX2 = 0.0;
                    dX4 = (-dTemp);
                    dX5 = 0.0;
                }
                else // (0 && 0), (0 && 1), (1 && 0)
                {
                    double start_len2 = dLengthStart * dLengthStart;
                    double start_len3 = dLengthStart * start_len2;
                    double trap_len3 = dLengthTrap * dLengthTrap2;

                    double dD1 = ((-dR1) * start_len2 * this->dLength / 2.0 +
                                dM1 * dLengthStart * this->dLength +
                                dR1 * start_len3 / 3.0 -
                                dM1 * start_len2 / 2.0 +
                                dVectorEnd[Y] * dLengthEnd  * trap_len3 / 6.0 +
                                dDeltaY * dLengthEnd  * trap_len3 / 24.0 +
                                dVectorEnd[Y] * dLengthTrap * trap_len3 / 8.0 +
                                dDeltaY * dLengthTrap * trap_len3 / 30.0) / dEI;

                    double dD11 = dLength3 / (3.0 * dEI);
                    if (dShearArea != 0.0)
                    {
                        dD1 += (dVectorEnd[Y] * dLengthTrap2 / 2.0 +
                                dDeltaY * dLengthTrap2 / 6.0 +
                                dR1 * dLengthStart) / dShearArea;
                        dD11 += this->dLength / dShearArea;
                    }

                    // For // (0 && 1), (1 && 0)
                    double dTemp4 = dD1 / dD11;
                    double dTemp5 = 0.0;
                    if ( ! this->bJointNeg && ! this->bJointPos ) // (0 && 0)
                    {
                        double d2 = ((-dR1) * start_len2 / 2.0 +
                                     dM1 * dLengthStart +
                                     dVectorEnd[Y]  * trap_len3 / 6.0 +
                                     dDeltaY * trap_len3 / 24.0) / dEI;
                        double d12 = dLength2 / (2.0 * dEI);
                        double d22 = this->dLength / dEI;
                        dTemp5 = ((-d2) + d12 * dD1 / dD11) / (d22 - (d12 * d12) / dD11);
                        dTemp4 = (dD1 + d12 * dTemp5) / dD11;
                    }

                    // For (0 && 1), (0 && 0)
                    dX1 = dTemp4 - dR1;
                    dX2 = (-(dM1 + dTemp5 - dTemp4 * this->dLength));
                    dX4 = (-dTemp4);
                    dX5 = dTemp5;

                    if ( this->bJointNeg && ! this->bJointPos ) // (1 && 0)
                    {   // SWAP BACK...
                        dX4 = dX1;
                        dX5 = (-dX2);
                        dX1 = (-dTemp4);
                        dX2 = 0.0;
                    }
                }

                double dForceApplied =
                    ((dVectorStart[X] / 2.0 - dDeltaX / 6.0) * dLengthTrap2 * dReciprocalLength +
                      dVectorStart[X] * dLengthTrap * dLengthEnd * dReciprocalLength -
                      dDeltaX  * dLengthTrap * dLengthEnd * dReciprocalLength / 2.0);

                adDistForce[0] -= dForceApplied;
                adDistForce[1] += dX1;
                adDistForce[2] += dX2;
                adDistForce[3] += (dForceApplied - (dVectorStart[X] + dVectorEnd[X]) * dLengthTrap / 2.0);
                adDistForce[4] += dX4;
                adDistForce[5] += dX5;
            }
        }
    }

    double adNegGlobalForce[6];
    for (short int siIndex = 0; siIndex < 6; siIndex++)
    {
        this->adGlobalForce[siIndex] = adConcForce[siIndex] + adDistForce[siIndex];
        adNegGlobalForce[siIndex] = ( - this->adGlobalForce[siIndex] );
    }
    this->translateLocalToGlobal(adNegGlobalForce, adForceMatrix);

    return true;
}

//*
//* End of Member::loadMember
//***************************************************************************

void Member::calcForces(double * adDisplaceGlobal)
{
    double adForceLocal[6][6];
    this->calcStiffnessMatrix(adForceLocal);

    double adDisplaceLocal[6];
    this->translateGlobalToLocal(adDisplaceLocal, adDisplaceGlobal);

    for (short int siIndex1 = 0; siIndex1 < 6; siIndex1++)
    {
        for (short int siIndex2 = 0; siIndex2 < 6; siIndex2++)
        {
            this->adLocalForce[siIndex1] += (adForceLocal[siIndex1][siIndex2] * adDisplaceLocal[siIndex2]);
        }
        this->adLocalForce[siIndex1] += this->adGlobalForce[siIndex1];
    }
}

//***************************************************************************
//*
//* Member::calcDeflection
//*     This function calculates the displacements for the structure.
//*
//***************************************************************************

bool Member::calcDeflection(bool bSpecialCrossSection, double * adGlobalDisplace, std::vector<LoadTrapezoidal *> & tl,
                                        short int siDivisions)
{
    // No calculation required for Fictitious & NOT Special Cross-Section...
    if ( this->mp->isFictitious() && ! bSpecialCrossSection)
        return true;

    //*
    //* Member Deflection Calculation...
    //**************************************

    double dLength2 = this->dLength  * this->dLength;
    double dLength3 = dLength2 * this->dLength;

    double adLocalDisplace[6] = { 0.0 };
    this->translateGlobalToLocal(adLocalDisplace, adGlobalDisplace);

    //
    // Compile Trapezoidal Load Constants for Member...
    //
    // TODO: Verify trapezoidal load calcs and against comparable uniform load since
    //       a uniform load is a simplified trapezoidal load.
    //
    std::vector<LoadTrapezoidalExtra *> tle;
    for (LoadTrapezoidal * tlCurr : tl)
    {
        if (tlCurr->siMemberID == this->siID)
        {
            double dStart = tlCurr->adDist[0]; // ...distance from member's neg end to start of load
            double dEnd   = tlCurr->adDist[1]; // ...distance from member's neg end to end of load
            double dTLength = dEnd - dStart; // ...length of trapezoidal load
            //if (dStart != 0.0 || dEnd != 0.0)
            if (dTLength != 0.0)
            {
                LoadTrapezoidalExtra * tleCurr = new LoadTrapezoidalExtra;
                if (tleCurr == nullptr)
                {
                    std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                              << "          Trapezoidal Load Extras!!\n\n";
                    return false;
                }
                tleCurr->tl = tlCurr;

                double dStart2 = dStart  * dStart;
                double dStart3 = dStart2 * dStart;
                double dStart4 = dStart3 * dStart;
                double dStartY = tlCurr->adStart[Y];

                double dEnd2 = dEnd * dEnd;
                double dEndY = tlCurr->adEnd[Y];

                double dTLength2 = dTLength  * dTLength;
                double dTLength3 = dTLength2 * dTLength;
                double dTLength4 = dTLength3 * dTLength;
                double dTLength5 = dTLength4 * dTLength;

                //
                // Start of trapezoidal load not at NegEnd...
                //
                // TODO: If dStart == 0.0, so what, shouldn't they calc to 0 since skipping will be 0...they don't!
                //
                if (dStart != 0.0)
                {
                    tleCurr->vam1 = dTLength * ((3.0 * this->dLength - 2.0 * dTLength - 3.0 * dStart) * dEndY +
                                                (3.0 * this->dLength -       dTLength - 3.0 * dStart) * dStartY);

                    tleCurr->mam3 = 10.0 * tleCurr->vam1;

                    tleCurr->mam1 =
                        dTLength * ((-40.0 * dTLength  -  60.0 * dStart) * dLength2 +
                                    ( 45.0 * dTLength2 + 120.0 * dStart * dTLength  + 90.0 * dStart2) * this->dLength -
                                    ( 12.0 * dTLength3 -  45.0 * dStart * dTLength2 - 60.0 * dStart2 * dTLength - 30.0 * dStart3)) * dEndY +
                        dTLength * ((-20.0 * dTLength  -  60.0 * dStart) * dLength2 +
                                    ( 15.0 * dTLength2 +  60.0 * dStart * dTLength  + 90.0 * dStart2) * this->dLength -
                                    (  3.0 * dTLength3 -  15.0 * dStart * dTLength2 - 30.0 * dStart2 * dTLength - 30.0 * dStart3)) * dStartY;
                }

                //
                // The trapezoidal load...
                //
                tleCurr->mbm5 =  3.0 * this->dLength * (dEndY          - dStartY);

                tleCurr->mbm4 = 15.0 * this->dLength * (dEnd * dStartY - dStart * dEndY);

                tleCurr->mbm3 = 10.0 * (( 3.0 * dStart2 * this->dLength - dTLength2 * ( 3.0 * dStart + 3.0 * this->dLength + 2.0 * dTLength)) * dEndY +
                                        (-3.0 * dEnd2   * this->dLength + dTLength2 * ( 3.0 * dStart +                             dTLength)) * dStartY);

                tleCurr->mbm2 = 30.0 * this->dLength * dStart2 * ((3.0 * dTLength + dStart) * dStartY -
                                                                                    dStart  * dEndY);

                tleCurr->mbm1 = (20.0 * dTLength2 * (3.0 * dStart + 2.0 * dTLength) * dLength2 +
                                 15.0 * (dStart4                             - 6.0 * dStart2 * dTLength2 -
                                                   8.0 * dStart  * dTLength3 - 3.0 * dTLength4) * this->dLength +
                               // or (dTLength3 * (8.0 * dStart              - 3.0 * dTLength)) * this->dLength + ???
                                 30.0 * dStart3 * dTLength2 +
                                 60.0 * dStart2 * dTLength3 +
                                 45.0 * dStart  * dTLength4 +
                                 12.0           * dTLength5
                                ) * dEndY +
                                (20.0 * dTLength2 * (3.0 * dStart +       dTLength) * dLength2 -
                                 15.0 * (dStart4 + 4.0 * dStart3 * dTLength  + 6.0 * dStart2 * dTLength2 +
                                                   4.0 * dStart  * dTLength3 +       dTLength4) * this->dLength +
                                 30.0 * dStart3 * dTLength2 +
                                 30.0 * dStart2 * dTLength3 +
                                 15.0 * dStart  * dTLength4 +
                                  3.0           * dTLength5
                                ) * dStartY;

                tleCurr->mbm0 = dStart4 * this->dLength * ((3.0 * dStart + 15.0 * dTLength) * dStartY -
                                                           (3.0 * dStart                  ) * dEndY);

                tleCurr->vbm3 = this->dLength * (dEndY - dStartY);

                tleCurr->vbm2 = this->dLength * (3.0 * dEnd   * dStartY -
                                                 3.0 * dStart * dEndY);

                tleCurr->vbm1 = ( 3.0 * (dStart2                           - dTLength2) * this->dLength +
                                  3.0 * dStart * dTLength2 +
                                  2.0 * dTLength3
                                ) * dEndY +
                                (-3.0 * (dStart2 + 2.0 * dStart * dTLength + dTLength2) * this->dLength +
                                  3.0 * dStart * dTLength2 +
                                        dTLength3
                                ) * dStartY;

                tleCurr->vbm0 = this->dLength * ((dStart3 + 3.0 * dStart2 * dTLength) * dStartY -
                                                  dStart3                             * dEndY);

                //
                // End of trapezoidal load not at PosEnd...
                //
                // TODO: If dEnd == dLength, so what, shouldn't they calc to 0 since skipping will be 0...they don't!
                //
                if (dEnd != this->dLength)
                {
                    tleCurr->vcm1 =  dTLength * ((3.0 * dStart +       dTLength) * dStartY +
                                                 (3.0 * dStart + 2.0 * dTLength) * dEndY);

                    tleCurr->vcm0 = -this->dLength * tleCurr->vcm1;

                    tleCurr->mcm3 =  10.0 * tleCurr->vcm1;

                    tleCurr->mcm2 = -30.0 * this->dLength * dTLength * ((3.0 * dStart +       dTLength) * dStartY -
                                                                        (3.0 * dStart + 2.0 * dTLength) * dEndY);

                    tleCurr->mcm1 = dTLength * ( 30.0 * dStart3 +
                                                 30.0 * dStart2 * dTLength +
                                                 15.0 * dStart  * dTLength2 +
                                                  3.0 * dTLength3 +
                                                 20.0 * (3.0 * dStart + dTLength) * dLength2
                                               ) * dStartY +
                                    dTLength * ( 30.0 * dStart3 +
                                                 60.0 * dStart2 * dTLength +
                                                 45.0 * dStart  * dTLength2 +
                                                 12.0 * dTLength3 +
                                                 20.0 * (3.0 * dStart + 2.0 * dTLength) * dLength2
                                               ) * dEndY;

                    tleCurr->mcm0 = -dTLength * ( 30.0 * dStart3 +
                                                  30.0 * dStart2 * dTLength +
                                                  15.0 * dStart  * dTLength2 +
                                                   3.0 * dTLength3
                                                ) * this->dLength * dStartY -
                                     dTLength * ( 30.0 * dStart3 +
                                                  60.0 * dStart2 * dTLength +
                                                  45.0 * dStart  * dTLength2 +
                                                  12.0 * dTLength3
                                                ) * this->dLength * dEndY;
                }
                tle.push_back(tleCurr);
            }
        }
    }

    //*
    //* Calculate Max Displacement =
    //*     Sum of (Beam Deflection + Stress Deflection) at division points along Member...
    //**************************************

    double dEI = this->mp->dMoE_IP * this->mp->dMoI;
    double dShearArea = this->mp->dShearModulus * this->mp->dArea; // Shear Area = G * A = ShearModulus * (thick * height)

    // Check along member at siDivisions internal points + start + end = siDivisions + 2
    double dReciprocalDivision = 1.0 / (double)(siDivisions + 1);
    for (short int siDivFactor = 0; siDivFactor <= siDivisions + 1; siDivFactor++)
    {
        double dDivLengthNeg  = this->dLength * ((double)siDivFactor * dReciprocalDivision);
        double dDivLengthNeg2 = dDivLengthNeg  * dDivLengthNeg;
        double dDivLengthNeg3 = dDivLengthNeg2 * dDivLengthNeg;
        double dDivLengthNeg4 = dDivLengthNeg3 * dDivLengthNeg;
        double dDivLengthNeg5 = dDivLengthNeg4 * dDivLengthNeg;
        double dDivLengthPos  = this->dLength - dDivLengthNeg;
        double dDivLengthPos2 = dDivLengthPos  * dDivLengthPos;

        //
        // Add Displacement from Distributed Systems...
        //
        //  Beam Deflection
        //      yb = Wu x / (24 EI) (l^3 - 2 l x^2 + x^3)
        //  Shear Deflection
        //      ys = Wu x v / (2 G A l)

        double y1b = 0.0;
        double y1s = 0.0;
        for (DistributedSystem * distCurr : this->dist)
        {
            y1b += distCurr->adLoadvect[Y] * dDivLengthNeg / 24.0 * ( dLength3 - 2.0 * this->dLength * dDivLengthNeg2 + dDivLengthNeg3 );
            // ...we'll divide by EI later...

            if (dShearArea != 0.0) // ...Fictitious check: divide by 0...
                y1s += distCurr->adLoadvect[Y] * dDivLengthNeg * dDivLengthPos / (2.0 * this->dLength);
                // ...we'll divide by shear area (G A) later...
        }

        //
        // Add Displacement from Node Moments...
        //
        // yx = - Wm x / (6 E I) (2 L + x^2 / L - 3 x)

        // Moment Deflection from Neg End
        //double y2b = this->dLocalForce[2] * dDivLengthPos / 6.0 * ( dLength - dDivLengthPos2 / dLength );
        double y2b =   this->adLocalForce[2] * dDivLengthNeg / 6.0 * ( 2.0 * this->dLength + dDivLengthNeg2 / this->dLength - 3.0 * dDivLengthNeg );
        // ...we'll divide by EI later...

        // Moment Deflection from Pos End
        //double y3b = this->dLocalForce[5] * dDivLengthNeg / 6.0 * ( dDivLengthNeg2 / dLength -  dLength);
        double y3b = - this->adLocalForce[5] * dDivLengthPos / 6.0 * ( 2.0 * this->dLength + dDivLengthPos2 / this->dLength - 3.0 * dDivLengthPos );
        // ...we'll divide by EI later...

        //
        // Add Displacement from Concentrated Systems...
        //
        //  Beam Deflection between neg_end and load:
        //      yb = - Wp b x / (6 E I l) (l^2 - x^2 - b^2) =
        //           - Load * pos_to_load * DivDistWithinNeg / ( 6 * ModulusOfElast * MomentOfInertia * LengthOfMember ) * ( LengthOfMember^2 - DivDistWithinNeg^2 - pos_to_load^2 )
        //  Beam Deflection between load and pos_end:
        //      yb = - Wp a v / (6 E I l) (l^2 - v^2 - a^2) =
        //           - Load * neg_to_load * DivDistWithinPos / ( 6 * ModulusOfElast * MomentOfInertia * LengthOfMember ) * ( LengthOfMember^2 - DivDistWithinPos^2 - neg_to_load^2 )
        //  Beam Deflection at load:
        //      yb = - Wp a^2 b^2 / (3 E I l) =
        //           - Load * neg_to_load^2 * pos_to_load^2 / ( 2 * ModulusOfElast * MomentOfInertia * LengthOfMember )
        //
        //  Stress Deflection between neg_end and load:
        //      ys = - Wp b x / (G A l) =
        //           - Load * pos_to_load * DivDistWithinNeg / ( ShearMod * Area * LengthOfMember )
        //  Stress Deflection between load and pos_end:
        //      ys = - Wp a v / (G A l) =
        //           - Load * neg_to_load * DivDistWithinPos / ( ShearMod * Area * LengthOfMember )
        //  Stress Deflection at load:
        //      ys = - Wp a b / (G A l) = MAX
        //           - Load * neg_to_load * pos_to_load / ( ShearMod * Area * LengthOfMember )

        double y4b = 0.0;
        double y4s = 0.0;
        for (ConcentratedSystem * concCurr : this->conc)
        {
            if (concCurr->adLoadVect[Y] != 0.0)
            {
                double neg_to_load = concCurr->dLoadDist;
                double pos_to_load = this->dLength - neg_to_load;
                double neg_to_load2 = neg_to_load * neg_to_load;
                double pos_to_load2 = pos_to_load * pos_to_load;

                if (dDivLengthNeg <= neg_to_load)
                {
                    y4b += concCurr->adLoadVect[Y] * pos_to_load * dDivLengthNeg / (6.0 * this->dLength) * (dLength2 - dDivLengthNeg2 - pos_to_load2);
                    // ...we'll divide by EI later...

                    if (dShearArea != 0.0) // ...Fictitious check: divide by 0...
                        y4s += concCurr->adLoadVect[Y] * pos_to_load * dDivLengthNeg / this->dLength;
                        // ...we'll divide by shear area (G A) later...
                }
                else
                {
                    y4b += concCurr->adLoadVect[Y] * neg_to_load * dDivLengthPos / (6.0 * this->dLength) * (dLength2 - dDivLengthPos2 - neg_to_load2);
                    // ...we'll divide by EI later...

                    if (dShearArea != 0.0) // ...Fictitious check: divide by 0...
                        y4s += concCurr->adLoadVect[Y] * neg_to_load * dDivLengthPos / this->dLength;
                        // ...we'll divide by shear area (G A) later...
                }
            }
        }

        //
        // Add Displacement as Local from Global Displacements...
        //
        double y5 = adLocalDisplace[1] + ( adLocalDisplace[4] - adLocalDisplace[1] ) * dDivLengthNeg / this->dLength;
        // = adLD[1] + ( adLD[4] * dDLN - adLD[1] *           dDLN           ) / dLength;
        // = adLD[1] + ( adLD[4] * dDLN - adLD[1] *      (dLength - dDLP)    ) / dLength;
        // = adLD[1] + ( adLD[4] * dDLN - adLD[1] * dLength + adLD[1] * dDLP ) / dLength;
        // = adLD[1] + ( adLD[4] * dDLN                     + adLD[1] * dDLP ) / dLength - (adLD[1] * dLength) / dLength;
        // = adLD[1] + ( adLD[4] * dDLN                     + adLD[1] * dDLP ) / dLength - adLD[1];
        // =           ( adLD[4] * dDLN                     + adLD[1] * dDLP ) / dLength;
        // = (adLocalDisplace[1] * dDivLengthPos + adLocalDisplace[4] * dDivLengthNeg) / this->dLength;

        //
        // Add Displacement from Trapezoidal Load Extras...
        //

        double adm = 0.0;
        double adv = 0.0;
        for (LoadTrapezoidalExtra * tleCurr : tle)
        {
            double dStart = tleCurr->tl->adDist[0];
            double dEnd   = tleCurr->tl->adDist[1];
            double dTrapLength = dEnd - dStart;

            if (dDivLengthNeg < dStart) // Length to Division is before start of trapezoidal load...
            {
                adm -= (tleCurr->mam3 * dDivLengthNeg3 +
                        tleCurr->mam1 * dDivLengthNeg
                       ) / (360.0 * this->dLength);

                if (dShearArea != 0.0) // ...Fictitious check: divide by 0...
                    adv += tleCurr->vam1 * dDivLengthNeg / (6.0 * this->dLength);
            }
            else if (dDivLengthNeg <= dEnd) // Length to Division is within trapezoidal load length...
            {
                adm += (tleCurr->mbm5 * dDivLengthNeg5 +
                        tleCurr->mbm4 * dDivLengthNeg4 +
                        tleCurr->mbm3 * dDivLengthNeg3 +
                        tleCurr->mbm2 * dDivLengthNeg2 +
                        tleCurr->mbm1 * dDivLengthNeg +
                        tleCurr->mbm0
                       ) / (360.0 * dTrapLength * this->dLength);

                if (dShearArea != 0.0) // ...Fictitious check: divide by 0...
                    adv -= (tleCurr->vbm3 * dDivLengthNeg3 +
                            tleCurr->vbm2 * dDivLengthNeg2 +
                            tleCurr->vbm1 * dDivLengthNeg +
                            tleCurr->vbm0
                           ) / (6.0 * dTrapLength * this->dLength);
            }
            else // Length to Division is after end of trapezoidal load...
            {
                adm += (tleCurr->mcm3 * dDivLengthNeg3 +
                        tleCurr->mcm2 * dDivLengthNeg2 +
                        tleCurr->mcm1 * dDivLengthNeg +
                        tleCurr->mcm0
                       ) / (360.0 * this->dLength);

                if (dShearArea != 0.0) // ...Fictitious check: divide by 0...
                    adv -= (tleCurr->vcm1 * dDivLengthNeg +
                            tleCurr->vcm0
                           ) / (6.0 * this->dLength);
            }
        }

        //
        // Calculate Maximum Displacement 'delta'...
        //

        double temp = (y1b + y2b + y3b + y4b + adm) / dEI + // Bending
                      (y1s + y4s + adv) / dShearArea +      // Shear
                      y5;                                   // Local from Global

        if ( fabs(temp) > fabs(this->ma.dDelta) )
        {
            this->ma.dDelta = temp;
            this->ma.dDeltaLength = dDivLengthNeg;
        }
    }

    //
    // Clear Trapezoidal Load Extras...
    //

    if ( ! tle.empty() )
    {
        for (LoadTrapezoidalExtra * tleCurr : tle)
            delete tleCurr;
        tle.clear();
    }

    return true;
}

//*
//* End of Member::calcDeflection
//***************************************************************************

//***************************************************************************
//*
//* Member::calcStiffnessMatrix
//*     This function calculates the K matrix for the given member.
//*
//***************************************************************************

void Member::calcStiffnessMatrix(double adStiffnessMatrixLocal[6][6])
{
    // Clear the member's stiffness matrix...
    //for (short int siIndex1 = 0; siIndex1 < 6; siIndex1++)
    //{
    //    for (short int siIndex2 = 0; siIndex2 < 6; siIndex2++)
    //        adStiffnessMatrixLocal[siIndex1][siIndex2] = 0.0;
    //}
    memset(adStiffnessMatrixLocal, 0.0, 36 * sizeof(double));

    if (this->dLength == 0.0)
        return;

    double dLength2 = this->dLength * this->dLength;
    double dLength3 = dLength2 * this->dLength;

    double dShearArea = this->mp->dShearModulus * this->mp->dArea;
    double d1ei = this->mp->dMoE_IP * this->mp->dMoI;
    double d3ei = 3.0 * d1ei;

    //*
    //* All Joint Connections...
    //****************************************************

    double f0 = this->mp->dArea * this->mp->dMoE_Axial / this->dLength;

    adStiffnessMatrixLocal[0][0] =  f0;
    adStiffnessMatrixLocal[0][3] = -f0;

    adStiffnessMatrixLocal[3][0] = -f0;
    adStiffnessMatrixLocal[3][3] =  f0;

    //*
    //* Joints: FREE & FIXED or FIXED & FREE...
    //****************************************************

    //if ((member->neg_joint || member->pos_joint) && !(member->neg_joint && member->pos_joint))
    if (this->bJointNeg != this->bJointPos)         // 1 != 0 || 0 != 1
    {
        double f1 = 0.0, f2 = 0.0, f3 = 0.0;
        if (dShearArea != 0.0)
        {
            f1 = d3ei / (dLength3 + (d3ei * this->dLength / dShearArea));
            f2 = d3ei / (dLength2 + (d3ei / dShearArea));
            f3 = f2 * this->dLength;
        }
        else // Fictitious
        {
            f1 = d3ei / dLength3;
            f2 = d3ei / dLength2;
            f3 = d3ei / this->dLength;
        }

        // FREE & FIXED...
        if (this->bJointNeg)          // 1 && 0
        {
            adStiffnessMatrixLocal[1][1] =  f1;
            adStiffnessMatrixLocal[1][4] = -f1;
            adStiffnessMatrixLocal[1][5] =  f2;

            adStiffnessMatrixLocal[4][1] = -f1;
            adStiffnessMatrixLocal[4][4] =  f1;
            adStiffnessMatrixLocal[4][5] = -f2;

            adStiffnessMatrixLocal[5][1] =  f2;
            adStiffnessMatrixLocal[5][4] = -f2;
            adStiffnessMatrixLocal[5][5] =  f3;
        }

        // FIXED & FREE...
        else if (this->bJointPos)     // 0 && 1
        {
            adStiffnessMatrixLocal[1][1] =  f1;
            adStiffnessMatrixLocal[1][2] =  f2;
            adStiffnessMatrixLocal[1][4] = -f1;

            adStiffnessMatrixLocal[2][1] =  f2;
            adStiffnessMatrixLocal[2][2] =  f3;
            adStiffnessMatrixLocal[2][4] = -f2;

            adStiffnessMatrixLocal[4][1] = -f1;
            adStiffnessMatrixLocal[4][2] = -f2;
            adStiffnessMatrixLocal[4][4] =  f1;
        }
    }

    //*
    //* Joints: FIXED & FIXED...
    //****************************************************

    else if ( ! this->bJointNeg && ! this->bJointPos)  // 0 && 0
    {
        double  d4ei =  4.0 * d1ei;
        double  d6ei =  6.0 * d1ei;
        double d12ei = 12.0 * d1ei;

        double f1 = 0.0, f2 = 0.0, f3 = 0.0, f4 = 0.0;
        if (dShearArea != 0.0)
        {
            f1 = d12ei / (dLength3 + (d12ei * this->dLength / dShearArea));
            f2 =  d6ei / (dLength2 + (d12ei                / dShearArea));
            f3 = (d4ei / this->dLength) * ((d3ei + dShearArea * dLength2) / (d12ei + dShearArea * dLength2));
            f4 = f2 * this->dLength - f3;
        }
        else // Fictitious
        {
            f1 = d12ei / dLength3;
            f2 =  d6ei / dLength2;
            f3 =  d4ei / this->dLength;
            f4 = (2.0 * d1ei) / this->dLength;
        }

        adStiffnessMatrixLocal[1][1] =  f1;
        adStiffnessMatrixLocal[1][2] =  f2;
        adStiffnessMatrixLocal[1][4] = -f1;
        adStiffnessMatrixLocal[1][5] =  f2;

        adStiffnessMatrixLocal[2][1] =  f2;
        adStiffnessMatrixLocal[2][2] =  f3;
        adStiffnessMatrixLocal[2][4] = -f2;
        adStiffnessMatrixLocal[2][5] =  f4;

        adStiffnessMatrixLocal[4][1] = -f1;
        adStiffnessMatrixLocal[4][2] = -f2;
        adStiffnessMatrixLocal[4][4] =  f1;
        adStiffnessMatrixLocal[4][5] = -f2;

        adStiffnessMatrixLocal[5][1] =  f2;
        adStiffnessMatrixLocal[5][2] =  f4;
        adStiffnessMatrixLocal[5][4] = -f2;
        adStiffnessMatrixLocal[5][5] =  f3;
    }

    //*
    //* Joints: FREE & FREE...
    //****************************************************

    // else, both ends are FREE, so there is no stiffness.

    return;
}

//*
//* End of Member::calcStiffnessMatrix
//***************************************************************************

//***************************************************************************
//*
//* Member::calcBending1986
//*     This function calculates the allowable bending stress using the
//*     1986 NDS for Standard and Composite systems.
//*
//***************************************************************************

void Member::calcBending1986(void)
{
    this->ma.siSlenderRatioType = 0;

    //*
    //* Calculate Allowable Bending Force...
    //*********************************************

    if (this->ma.dForceCompWork == 0.0)
        return;

    //
    // Use the full Bending Force if laterally supported...
    //

    if ( this->isLaterallySupported() )
        this->ma.dForceBendPrime = this->ma.dForceBendWork;

    //
    // Otherwise, calculate the Allowable Bending Force when not laterally supported.....
    //

    else
    {
        // Slenderness Override:
        //      true  = override the slenderness ratio, the Bending Length is given.
        //      false = do not override the slenderness ratio, the Bending Length is calculated.

        bool bSlendernessOverride = true;
        if (this->dEffBendingLength <= 0.0) // ...calculate Effective Bending Length...
        {
            bSlendernessOverride = false;
            double dLoD = this->dLength / this->mp->dHeight;
            if (dLoD > 14.3)
                this->dEffBendingLength = this->dLength * 1.84;
            else // dLoD <= 14.3
                this->dEffBendingLength = this->dLength * 1.63 + this->mp->dHeight * 3.0;
        }

        double dCs = sqrt(this->dEffBendingLength * this->mp->dHeight / (this->mp->dThick * this->mp->dThick));
        double dCk = sqrt(this->mp->dMoE_PP / this->ma.dForceBendWork);
        if ( this->isMaterialMachineTested() )
            dCk *= 0.956;
        else
            dCk *= 0.811;

        //
        // Short Beam calculation...
        //

        if (dCs <= 10.0)
            // if Material is Machine Tested (MSR/MEL) or not...
            this->ma.dForceBendPrime = this->ma.dForceBendWork;

        //
        // Intermediate Beam calculation...
        //

        else if (dCs <= dCk)
            // if Material is Machine Tested (MSR/MEL) or not, but dependent on Ck...
            this->ma.dForceBendPrime = this->ma.dForceBendWork * (1.0 - pow((dCs / dCk), 4) / 3.0);

        //
        // Long Beam calculation...
        //

        else if (dCs > dCk && dCs <= 50.0)
        {
            if ( this->isMaterialMachineTested() )
                this->ma.dForceBendPrime = 0.609 * this->mp->dMoE_PP / (dCs * dCs);
            else
                this->ma.dForceBendPrime = 0.438 * this->mp->dMoE_PP / (dCs * dCs);
        }

        //
        // Slenderness Ratio > 50.0.....
        //

        else if (dCs > 50.0)
        {
            this->ma.siSlenderRatioType = 1;
            if (bSlendernessOverride)
                this->ma.siSlenderRatioType = 2;
        }
    }

    //********************************
    //* Adjust Panel Bending Force for height...
    //********************************

    if (this->mp->dHeight > 12.0)
    {
        double height_adj = pow((12.0 / this->mp->dHeight), (1.0 / 9.0)) * this->ma.dForceBendWork;

        if (height_adj <= this->ma.dForceBendPrime)
            this->ma.dForceBendPrime = height_adj;

        if (height_adj <= this->ma.dForceBendWork)
            this->ma.dForceBendWork = height_adj;

        if (this->ma.dForceBendWork <= this->ma.dForceBendPrime)
            this->ma.dForceBendPrime = this->ma.dForceBendWork;
    }
}
//*
//* End of Member::calcBending1986
//***************************************************************************

//***************************************************************************
//*
//* Member::calcBending1991
//*     This function calculates the allowable bending using the 1991 NDS
//*     for Standard and Composite systems.
//*
//***************************************************************************

void Member::calcBending1991(void)
{
    this->ma.siSlenderRatioType = 0;

    //*
    //* Calculate Allowable Bending Force...
    //*********************************************

    if (this->ma.dForceCompWork == 0.0)
        return;
    //
    // If the Member is a Composite Beam, use the given volume factor...
    //

    double dEffectiveVolumeFactor = 1.0;
    if (Member::bIsComposite)
        dEffectiveVolumeFactor = this->dVolumeFactor;

    //
    // Else the Member is not a Composite Beam, calculate the volume factor...
    //

    else
    {
        if (this->mp->dHeight > 12.0 && this->mp->dThick >= 5.0)
            dEffectiveVolumeFactor = pow((12.0 / this->mp->dHeight), (1.0 / 9.0));
    }

    //
    // Use the Allowable Bending Force modified by volume factor if laterally supported...
    //

    if ( this->isLaterallySupported() )
    {
        this->ma.dForceBendPrime = this->ma.dForceBendWork * dEffectiveVolumeFactor;
        return;
    }

    //
    // Otherwise, calculate the Allowable Bending Force when not laterally supported.....
    //

    // Slenderness Override:
    //      true  = override the slenderness ratio, the Bending Length is given.
    //      false = do not override the slenderness ratio, the Bending Length is calculated.
    bool bOverride = true;
    if (this->dEffBendingLength <= 0.0)
    {
        bOverride = false;
        double dLoD = this->dLength / this->mp->dHeight;
        if (dLoD < 7.0)
            this->dEffBendingLength = this->dLength * 2.06;
        else if (dLoD >= 7.0 && dLoD <= 14.3)
            this->dEffBendingLength = this->dLength * 1.63 + this->mp->dHeight * 3.0;
        else
            this->dEffBendingLength = this->dLength * 1.84;
    }

    //
    // Slenderness Ratio > 50.0...
    //

    double dRb = sqrt(this->dEffBendingLength * this->mp->dHeight / (this->mp->dThick * this->mp->dThick));
    if (dRb > 50.0)
    {
        this->ma.siSlenderRatioType = 1;
        if (bOverride)
            this->ma.siSlenderRatioType = 2;
    }

    //
    // Check for Member E value: COV <= 0.11
    //

    double dKbe = 0.438;
    if ( this->isMaterialMachineTested() )
        dKbe = 0.609;

    double dFbe = dKbe * this->mp->dMoE_PP / (dRb * dRb);
    double dRatio = dFbe;
    // If Member is a Type 2 Composite Beam...
    if ( Member::bIsComposite && this->isType2Composite() )
        dRatio /= this->ma.dForceBendWork;
    // Otherwise, all other Members...
    else
        dRatio /= (this->ma.dForceBendWork * dEffectiveVolumeFactor);

    double d1Ratio = 1.0 + dRatio;
    double dCl = d1Ratio / 1.9 - sqrt(pow((d1Ratio / 1.9), 2.0) - dRatio / 0.95);

    //*
    //* Adjust for Depth or Volume...
    //***********************************

    //
    // If Member is a Type 2 Composite Beam...
    //

    if ( Member::bIsComposite && this->isType2Composite() )
    {
        if (dEffectiveVolumeFactor <= dCl)
            this->ma.dForceBendPrime = this->ma.dForceBendWork * dEffectiveVolumeFactor;
        else
            this->ma.dForceBendPrime = this->ma.dForceBendWork * dCl;
    }

    //
    // Otherwise, all other Members...
    //

    else
        this->ma.dForceBendPrime = this->ma.dForceBendWork * dCl * dEffectiveVolumeFactor;
}

//*
//* End of Member::allowableBending1991
//***************************************************************************

//***************************************************************************
//*
//* Member::calcTension
//*     This function calculates the stress index and L/D for a tension
//*     member.
//*
//***************************************************************************

void Member::calcTension(bool & bStressDiffNote, bool & bTensionNote)
{
    //
    // Determine In-Plane L/D...
    //
    double dLoD_IPCalc = 0.0;
    if (this->dEffColumnLength_IP != 0.0)
        dLoD_IPCalc = this->dEffColumnLength_IP / this->mp->dHeight;
    else
    {
        dLoD_IPCalc = this->dLength / this->mp->dHeight;
        if ( this->isInteriorMember() )
            dLoD_IPCalc *= Member::dInteriorModifier;
    }

    //
    // Determine Perp-Plane L/D when member is not laterally supported...
    //
    double dLoD_PPCalc = 0.0;
    if ( ! this->isLaterallySupported() )
    {
        if (this->dEffColumnLength_PP != 0.0)
            dLoD_PPCalc = this->dEffColumnLength_PP / this->mp->dThick;
        else
        {
            dLoD_PPCalc = this->dLength / this->mp->dThick;
            if ( this->isInteriorMember() )
                dLoD_PPCalc *= Member::dInteriorModifier;
        }
    }

    this->ma.dLoD_IP = dLoD_IPCalc;
    this->ma.dLoD_PP = dLoD_PPCalc;

    //
    // Find which Tension L/D is critical...
    //
    this->ma.dLoD_Critical = dLoD_IPCalc;
    if (dLoD_IPCalc < dLoD_PPCalc)
        this->ma.dLoD_Critical = dLoD_PPCalc;
    if (this->ma.dLoD_Critical > 80.0)
    {
        this->ma.strMemberNotes[1] = 'T';
        bTensionNote = true;
    }

    //
    // Determine In-Plane Tension Force...
    //
    double dAction_IPCalc =
        fabs(this->ma.dBend  / this->ma.dForceBendWork) +
        fabs(this->ma.dAxial / this->ma.dForceTensWork);

    //
    // Determine Perp-Plane Tension Force...
    //
    double dAction_PPCalc =
        (fabs(this->ma.dBend) - fabs(this->ma.dAxial)) / this->ma.dForceBendPrime;

    //
    // Find which Plane Tension Force is critical...
    //
    this->ma.dAction = dAction_IPCalc;
    if (dAction_PPCalc > dAction_IPCalc)
    {
        this->ma.dAction = dAction_PPCalc;
        bStressDiffNote = true;
        this->ma.strMemberNotes[0] = 'D';
    }
}

//*
//* End of Member::calcTension
//***************************************************************************

//***************************************************************************
//*
//* Member::calcCompression1986
//*     This function calculates the stress index and L/D ratio for a
//*     compression member as per NDS 1986.
//*
//***************************************************************************

void Member::calcCompression1986(double dDiagonal[], bool bUsingTPI, double dEndActionTPI, double dBendLocTPI,
                                 bool & bNoteBuckle, short int & siNotePerp, bool & bNoteTPIOver, bool & bNoteComp50)
{
    //
    // Determine In-Plane L/D...
    //
    double dLoD_IPCalc = 0.0;
    double dEffectiveLength = this->dLength;
    if (this->dEffColumnLength_IP != 0.0)
        dEffectiveLength = this->dEffColumnLength_IP;
    else
    {
        if ((dEffectiveLength / this->mp->dHeight) > 11.0)
        {
            if ( this->isInteriorMember() )
                dEffectiveLength *= Member::dInteriorModifier;
            double dEffColumnLength = this->calcColumn(dDiagonal);
            if (dEffectiveLength > dEffColumnLength)
                dEffectiveLength = dEffColumnLength;
        }
    }
    dLoD_IPCalc = dEffectiveLength / this->mp->dHeight;

    //
    // Determine Perp-Plane L/D when member is not laterally supported...
    //
    double dLoD_PPCalc = 0.0;
    if ( ! this->isLaterallySupported() )
    {
        if (this->dEffColumnLength_PP != 0.0)
            dLoD_PPCalc = this->dEffColumnLength_PP / this->mp->dThick;
        else
        {
            dLoD_PPCalc = this->dLength / this->mp->dThick;
            if ( this->isInteriorMember() )
                dLoD_PPCalc *= Member::dInteriorModifier;
        }
    }

    //
    // Set Euler and Splice Coefficients...
    //
    double dEuler = 0.300;
    double dSplice = 0.671;
    if ( this->isMaterialMachineTested() )
    {
        dEuler = 0.418;
        dSplice = 0.792;
    }

    //
    // Calculate 'CT' for Special Truss Chord...
    //      NDS: Special Design Considerations - Wood Trusses
    //
    double dCT = 1.0;
    if ( this->isTrussChord() && this->mp->dHeight <= 3.5)
    {
        double dLe = dEffectiveLength;
        if (dLe > 96.0)
        {
            dLe = 96.0;
            bNoteBuckle = true;
            this->ma.strMemberNotes[1] = 'S';
        }

        double dSeasoned = 2300.0;
        double dGreen = 1200.0;
        double dKM = dGreen; // ...assume worst case
        // KT Factors:            = 1 - 1.645*COVe
        double dFactor_TrussChord = 1 - 1.645 * 0.25; // 0.58875; // COVe  = 0.25
        //double dFactor_MEL        = 1 - 1.645 * 0.15; // 0.75325; // COVe  = 0.15
        double dFactor_MSR        = 1 - 1.645 * 0.11; // 0.81905; // COVe <= 0.11
        double dKT = dFactor_TrussChord; // ...assume worst case
        switch (this->mp->cProperty)
        {
            case '4': // Truss Chord, Seasoned...
                dKM = dSeasoned;
                dKT = dFactor_TrussChord;
                break;
            case '5': // Truss Chord, Green...
                dKM = dGreen;
                dKT = dFactor_TrussChord;
                break;
            case '6': // MSR, Seasoned...
                dKM = dSeasoned;
                dKT = dFactor_MSR;
                break;
            case '7': // MSR, Green...
                dKM = dGreen;
                dKT = dFactor_MSR;
                break;
        }
        dCT += ( dKM * dLe / (dKT * this->mp->dMoE_IP) );
    }

    //
    // Calculate Combined Bending and Axial Stress Index...
    //
    double dForceCompPrime = 0.0;
    double dAdjust = 0.0;
    if (dLoD_IPCalc <= 11.0)
    {
        dForceCompPrime = this->ma.dForceCompWork;
    }
    else
    {
        double dBreakVal = dSplice * sqrt(dCT * this->mp->dMoE_IP / this->ma.dForceCompWork);
        if (dLoD_IPCalc < dBreakVal)
        {
            dForceCompPrime =
                this->ma.dForceCompWork *
                ( 1.0 - pow(dLoD_IPCalc, 4.0) / ( 3.0 * pow(dBreakVal, 4.0) ) );
            dAdjust = (dLoD_IPCalc - 11.0) / (dBreakVal - 11.0);
        }
        else // dLoD_IPCalc >= dBreakVal
        {
            dForceCompPrime = dCT * dEuler * this->mp->dMoE_IP / (dLoD_IPCalc * dLoD_IPCalc);
            dAdjust = 1.0;
        }
    }

    double dAction_IPCalc =
        fabs(this->ma.dBend) /
            (this->ma.dForceBendPrime - dAdjust * fabs(this->ma.dAxial)) +
        fabs(this->ma.dAxial) / dForceCompPrime;

    double dAction_PPCalc = 0.0;
    if ( ! this->isLaterallySupported() )
    {
        dAdjust = 0.0;
        if (dLoD_PPCalc <= 11.0)
            dForceCompPrime = this->ma.dForceCompWork;
        else
        {
            double dBreakVal = dSplice * sqrt(this->mp->dMoE_PP / this->ma.dForceCompWork);
            if (dLoD_PPCalc < dBreakVal)
            {
                dForceCompPrime =
                    this->ma.dForceCompWork *
                    ( 1.0 - pow(dLoD_PPCalc, 4.0) / ( 3.0 * pow(dBreakVal, 4.0) ) );
                dAdjust = (dLoD_PPCalc - 11.0) / (dBreakVal - 11.0);
            }
            else // dLoD_PPCalc >= dBreakVal
            {
                dForceCompPrime = dEuler * this->mp->dMoE_PP / (dLoD_PPCalc * dLoD_PPCalc);
                dAdjust = 1.0;
            }
        }

        double dCs =
            sqrt( this->dEffBendingLength * this->mp->dHeight /
                    (this->mp->dThick * this->mp->dThick)
            );
        if (dCs < 10.0)
            dAdjust = 0.0;

        dAction_PPCalc =
            fabs(this->ma.dBend) /
                (this->ma.dForceBendPrime - dAdjust * fabs(this->ma.dAxial)) +
            fabs(this->ma.dAxial) / dForceCompPrime;
    }

    this->ma.dLoD_IP = dLoD_IPCalc;
    this->ma.dLoD_PP = dLoD_PPCalc;

    //
    // Check which Plane Compression Force is critical...
    //
    this->ma.dAction = dAction_IPCalc;
    this->ma.dLoD_Critical = dLoD_IPCalc;
    if (dAction_PPCalc > dAction_IPCalc)
    {
        this->ma.dAction = dAction_PPCalc;
        this->ma.dLoD_Critical = dLoD_PPCalc;
        this->ma.strMemberNotes[4] = 'P';
        ++siNotePerp;
    }

    this->ma.dForceCompPrime = (-dForceCompPrime);

    //
    // Use TPI override calculation only when TPI Force > Euler Force...
    //
    if (bUsingTPI && dEndActionTPI > this->ma.dLoD_Critical)
    {
        double dThickHeight = this->mp->dThick * this->mp->dHeight;
        double dThickHeight2 = dThickHeight * this->mp->dHeight;
        // Default to Pos End Forces...
        double dForceBend = this->adLocalForce[5];
        double dForceAxial = this->adLocalForce[3];
        if (dBendLocTPI == 0.0) // Neg End
        {
            // Set to Neg End Forces...
            dForceBend = -this->adLocalForce[2];
            dForceAxial = -this->adLocalForce[0];
        }
        this->ma.dBend  = dForceBend * 6.0 / dThickHeight2;
        this->ma.dAxial = dForceAxial / dThickHeight;

        this->ma.dMaxBendLoc  = dBendLocTPI;
        this->ma.dMaxAxialLoc = dBendLocTPI;

        this->ma.dLoD_Critical = dEndActionTPI;

        this->ma.dForceCompPrime = (-this->ma.dForceCompWork);

        // If a Perp-Plane L/D was in effect...
        if (this->ma.strMemberNotes[4] == 'P')
        {
            // ...remove it...
            --siNotePerp;
            bNoteTPIOver = true;
            this->ma.strMemberNotes[4] = '@';
            this->ma.dLoD_Critical = this->ma.dLoD_IP;
        }
    }

    //
    // Set flag if L/D limits exceeded...
    //
    if (this->ma.dLoD_Critical > 50.0)
    {
        this->ma.strMemberNotes[0] = 'C';
        bNoteComp50 = true;
    }
}

//*
//* End of Member::calcCompression1986
//***************************************************************************

//***************************************************************************
//*
//* Member::calcCompression1991
//*     This function calculates the stress index and L/D ratio for a
//*     compression member as per NDS 1991.
//*
//***************************************************************************

void Member::calcCompression1991(double dDiagonal[], bool bUsingTPI, double dEndActionTPI, double dBendLocTPI,
                                 bool & bBuckleNote, short int & siPerpNote, bool & bNoteTPIOver, bool & bComp75Note,
                                 bool & bComp50Note, bool & bEulerNote)
{
    //
    // Determine In-Plane L/D...
    //
    double dLoD_IPCalc = 0.0;
    double dEffectiveLength = this->dLength;
    if (this->dEffColumnLength_IP != 0.0)
        dEffectiveLength = this->dEffColumnLength_IP;
    else
    {
        if ( this->isInteriorMember() )
            dEffectiveLength *= Member::dInteriorModifier;
        double dEffColumnLength = this->calcColumn(dDiagonal);
        if (dEffectiveLength > dEffColumnLength)
            dEffectiveLength = dEffColumnLength;
    }
    dLoD_IPCalc = dEffectiveLength / this->mp->dHeight;

    //
    // Determine Perp-Plane L/D when member is not laterally supported...
    //
    double dLoD_PPCalc = 0.0;
    if ( ! this->isLaterallySupported() )
    {
        if (this->dEffColumnLength_PP != 0.0)
            dLoD_PPCalc = this->dEffColumnLength_PP / this->mp->dThick;
        else
        {
            dLoD_PPCalc = this->dLength / this->mp->dThick;
            if ( this->isInteriorMember() )
                dLoD_PPCalc *= Member::dInteriorModifier;
        }
    }

    //
    // Calculate 'CT' for Special Truss Chords...
    //      NDS: Special Design Considerations - Wood Trusses
    //
    double dCT = 1.0;
    if ( this->isTrussChord() && this->mp->dHeight <= 3.5 )
    {
        double dLe = dEffectiveLength;
        if (dLe > 96.0)
        {
            dLe = 96.0;
            bBuckleNote = true;
            this->ma.strMemberNotes[1] = 'S';
        }

        double dSeasoned = 2300.0;
        double dGreen = 1200.0;
        double dKM = dGreen; // ...assume worst case
        // KT Factors:            = 1 - 1.645 * COVe
        double dFactor_TrussChord = 1 - 1.645 * 0.25; // 0.58875; // COVe  = 0.25
        //double dFactor_MEL        = 1 - 1.645 * 0.15; // 0.75325; // COVe  = 0.15
        double dFactor_MSR        = 1 - 1.645 * 0.11; // 0.81905; // COVe <= 0.11
        double dKT = dFactor_TrussChord; // ...assume worst case
        switch (this->mp->cProperty)
        {
            case '4': // Truss Chord, Seasoned...
                dKM = dSeasoned;
                dKT = dFactor_TrussChord;
                break;
            case '5': // Truss Chord, Green...
                dKM = dGreen;
                dKT = dFactor_TrussChord;
                break;
            case '6': // MSR, Seasoned...
                dKM = dSeasoned;
                dKT = dFactor_MSR;
                break;
            case '7': // MSR, Green...
                dKM = dGreen;
                dKT = dFactor_MSR;
                break;
        }
        dCT += ( dKM * dLe / (dKT * this->mp->dMoE_IP) );
    }

    this->ma.dLoD_IP = dLoD_IPCalc;
    this->ma.dLoD_PP = dLoD_PPCalc;
    this->ma.dLoD_Critical = dLoD_IPCalc;

    // Set Euler Coefficient...
    double dEuler = 0.300;
    if ( this->isMaterialMachineTested() )
        dEuler = 0.418;

    //
    // Find which Plane 'Fc_e' is critical...
    //
    double dForceCompEulerIP =
        dEuler * dCT * this->mp->dMoE_IP /
            (dLoD_IPCalc * dLoD_IPCalc);
    double dForceCompEulerLeast = dForceCompEulerIP;
    if ( ! this->isLaterallySupported() )
    {
        double dForceCompEulerPP =
            dEuler * this->mp->dMoE_PP /
                (dLoD_PPCalc * dLoD_PPCalc);
        if (dForceCompEulerPP < dForceCompEulerIP)
        {
            dForceCompEulerLeast = dForceCompEulerPP;
            this->ma.dLoD_Critical = dLoD_PPCalc;
            this->ma.strMemberNotes[4] = 'P';
            ++siPerpNote;
        }
    }

    //
    // Calculate 'Fc Prime' value...
    //
    double dC = 0.8;
    if ( this->isType2Composite() )
        dC = 0.9;

    double dRatio = dForceCompEulerLeast / this->ma.dForceCompWork;
    double d1Ratio = 1.0 + dRatio;
    double d2C = 2.0 * dC;

    double dCp =
        d1Ratio / d2C -
        sqrt( (d1Ratio * d1Ratio) / (d2C * d2C) - dRatio / dC );

    double dForceCompPrime = this->ma.dForceCompWork * dCp;

    //
    // Axial stress to Euler stress ratio...
    //
    //  This calculation uses dForceCompEulerIP, not dForceCompEulerLease,
    //  as it needs to specifically check the In-Plane ratio, not the least of
    //  In-Plane and Perp-Plane. A higher value produces a lower action ratio
    //  in the check after this equation.  See below...
    double dAxialIPCalc = fabs(this->ma.dAxial) / dForceCompEulerIP;

    //
    // Calculate Compression Stress Index (CSI)...
    //
    if (dAxialIPCalc < 1.0) // ..actual stress < Euler stress (stress passes)...
    {
        double dAxialCalc = fabs(this->ma.dAxial / dForceCompPrime); // w.r.t. dForceCompEulerLeast for Axial Stress Index
        if (fabs(this->ma.dBend) <= 1.0) // If Bending is not an important stress factor...
            this->ma.dAction = dAxialCalc; // ...then, use the worst case Axial Compression Index
        else // ...otherwise, calculate total Compression Stress Index (CSI = BSI + ASI)...
        {
            // NOTE: dAxialCalc is bound to dForceCompEulerLeast in the prior calculation.
            //       dAxialCalc is related to the ASI portion of the CSI.
            //       The BSI portion of the CSI is only related to In-Plane action.  Therefore,
            //       DO NOT use the dForceCompEulerLeast value for the BSI portion--use the
            //       dForceCompEulerIP (In-Plane Euler) value directly.
            //  Since:
            //     fabs(this->ma.dAxial) / dForceCompEulerIP) == dAxialIPCalc
            //  and is pre-calculated above, use it directly in this equation.
            //  The value:
            //    ( 1.0 - dAxialIPCalc )
            //  is the BSI percent remaining as related to the In-Plane ASI percentage:
            //    Bending% + Axial% = 100%
            this->ma.dAction =
                /*ASI*/ dAxialCalc * dAxialCalc +
                /*BSI*/ fabs( this->ma.dBend ) /
                                ( this->ma.dForceBendPrime * ( 1.0 - dAxialIPCalc ) );
        }
    }
    else // ...actual stress >= Euler stress (stress failure)...
    {
        this->ma.dAction = dAxialIPCalc;
        this->ma.strMemberNotes[3] = 'E'; // Actual Compression Stress exceeds Euler Stress
        bEulerNote = true;
    }

    //  Show Compression as a negative force...
    this->ma.dForceCompPrime = (-dForceCompPrime);

    //
    // Use TPI override calculation only when TPI Force > Standard CSI Force...
    //
    if (bUsingTPI && dEndActionTPI > this->ma.dAction)
    {
        double dThickHeight = this->mp->dThick * this->mp->dHeight;
        double dThickHeight2 = dThickHeight * this->mp->dHeight;
        // Default to Pos End Forces...
        double dForceBend = this->adLocalForce[5];
        double dForceAxial = this->adLocalForce[3];
        if (dBendLocTPI == 0.0)
        {
            // Change to Neg End Forces...
            dForceBend = -this->adLocalForce[2];
            dForceAxial = -this->adLocalForce[0];
        }
        this->ma.dBend  = dForceBend * 6.0 / dThickHeight2;
        this->ma.dAxial = dForceAxial / dThickHeight;

        this->ma.dMaxBendLoc  = dBendLocTPI;
        this->ma.dMaxAxialLoc = dBendLocTPI;

        this->ma.dAction = dEndActionTPI;

        this->ma.dForceCompPrime = (-this->ma.dForceCompWork);

        // If a Perp-Plane L/D was in effect...
        if (this->ma.strMemberNotes[4] == 'P')
        {
            // ...remove it...
            --siPerpNote;
            bNoteTPIOver = true;
            this->ma.strMemberNotes[4] = '@';
            this->ma.dLoD_Critical = this->ma.dLoD_IP;
        }
    }

    //
    // Set flag if L/D limits exceeded...
    //
    if (this->ma.dLoD_Critical > 75.0)
    {
        this->ma.strMemberNotes[0] = '*';
        bComp75Note = true;
        this->ma.bCompFlag = true;
    }
    else if (this->ma.dLoD_Critical > 50.0)
    {
        this->ma.strMemberNotes[0] = 'C';
        bComp50Note = true;
    }
}

//*
//* End of Member::calcCompression1991
//***************************************************************************

//***************************************************************************
//*
//* CALC_COLUMN: This function calculates the effective column length for
//*     a compression member.
//*
//***************************************************************************

double Member::calcColumn(double adDiagonal[])
{
    short int siRow1, siRow2;
    double dEffectiveLength = 0.0;
    double dX1 = 0.0, dY1 = 0.0, dX2 = 0.0, dY2 = 0.0;
    double dEI, dLengthSqr;
    double dEI1, dEI2, dLength1 = 0.0, dLength2 = 0.0;

    //
    // Calculate the Row locations for the Nodes of the Member...
    //

    dLengthSqr = this->dLength * this->dLength;
    siRow1 = this->nodeNeg->siRotFlag;
    siRow2 = this->nodePos->siRotFlag;

    dEI = this->mp->dMoE_IP * this->mp->dMoI;

    //
    // Calculate the Row locations for the Nodes of the Member...
    //

    dEI1 = dEI;
    dEI2 = dEI;
    if (this->mp->dShearModulus != 0.0) // not Fictitious
    {
        dEI1 = dEI * (3.0 * dEI + this->mp->dShearModulus * this->mp->dArea * dLengthSqr) /
                (12.0 * dEI + this->mp->dShearModulus * this->mp->dArea * dLengthSqr);
        dEI2 = dEI / (1.0 + 3.0 * dEI / (this->mp->dShearModulus * this->mp->dArea * dLengthSqr));
    }

    if (siRow1)
    {
        dX1 = adDiagonal[siRow1 - 1] - 3.0 * dEI2 / this->dLength;
        dY1 = adDiagonal[siRow1 - 1] - 4.0 * dEI1 / this->dLength;
    }
    if (siRow2)
    {
        dX2 = adDiagonal[siRow2 - 1] - 3.0 * dEI2 / this->dLength;
        dY2 = adDiagonal[siRow2 - 1] - 4.0 * dEI1 / this->dLength;
    }

    //
    // Negative End Pinned and Positive End Pinned (P - P)...
    //

    if (this->bJointNeg && this->bJointPos)
        dEffectiveLength = this->dLength;

    //
    // Negative End Pinned and Positive End Rigid (P - R)...
    //

    else if (this->bJointNeg)
    {
        if (!siRow2) // Positive End fixed by reaction
            dEffectiveLength = this->dLength * 0.7;
        else
        {
            if (dX2 != 0.0)
                dLength2 = 3.0 * dEI2 / dX2;
        }
    }

    //
    // Negative End Rigid and Positive End Pinned (R - P)...
    //

    else if (this->bJointPos)
    {
        if (!siRow1) // Negative End fixed by reaction
            dEffectiveLength = this->dLength * 0.7;
        else
        {
            if (dX1 != 0.0)
                dLength1 = 3.0 * dEI2 / dX1;
        }
    }

    //
    // Negative End Rigid and Positive End Rigid (R - R)...
    //

    else
    {
        if (!siRow1 && !siRow2) // Both Ends fixed by reaction
            dEffectiveLength = this->dLength * 0.5;
        else if (!siRow1) // Negative End fixed by reaction
        {
            dLength1 = this->dLength / 60.0;
            if (dY2 != 0.0)
                dLength2 = 3.0 * dEI1 / dY2;
        }
        else if (!siRow2) // Positive End fixed by reaction
        {
            dLength2 = this->dLength / 60.0;
            if (dY1 != 0.0)
                dLength1 = 3.0 * dEI1 / dY1;
        }
        else
        {
            if (dY1 != 0.0)
                dLength1 = 3.0 * dEI1 / dY1;

            if (dY2 != 0.0)
                dLength2 = 3.0 * dEI1 / dY2;
        }
    }

    //
    // Calculate the effective length if not already set...
    //

    if (dEffectiveLength == 0.0)
    {
        double neg_node_factor = 0.0;
        if (dLength1 != 0.0)
            neg_node_factor = this->dLength / dLength1;

        double pos_node_factor = 0.0;
        if (dLength2 != 0.0)
            pos_node_factor = this->dLength / dLength2;

        double factor = 1.0;
        if (neg_node_factor != 0.0 || pos_node_factor != 0.0)
        {
            double pi2 = M_PIl * M_PIl;
            factor =
                sqrt(1.0 / ((pi2 + 16.0 * neg_node_factor) / (pi2 + 8.0 * neg_node_factor) *
                            (pi2 + 16.0 * pos_node_factor) / (pi2 + 8.0 * pos_node_factor)));
        }
        dEffectiveLength = this->dLength * factor;
    }

    return dEffectiveLength;
}

//**********************
//* End of CALC_COLUMN *
//***************************************************************************

//***************************************************************************
//*
//* TRANSLATE_LOCAL_TO_GLOBAL: This function translates a Local
//*     Coordinate Structure of a given member into the Global
//*     Coordinate Structure.
//*
//***************************************************************************

void Member::translateLocalToGlobal(double * adLocal, double * adGlobal)
{
    if (this->nodeNeg->siHorzFlag)
    {
        adGlobal[this->nodeNeg->siHorzFlag - 1] +=
                adLocal[0] * this->adNodeVectorNeg[X] -
                adLocal[1] * this->adNodeVectorNeg[Y];
    }

    if (this->nodeNeg->siVertFlag)
    {
        adGlobal[this->nodeNeg->siVertFlag - 1] +=
                adLocal[1] * this->adNodeVectorNeg[X] +
                adLocal[0] * this->adNodeVectorNeg[Y];
    }

    if (this->nodeNeg->siRotFlag)
    {
        adGlobal[this->nodeNeg->siRotFlag - 1] += adLocal[2];
    }

    if (this->nodePos->siHorzFlag)
    {
        adGlobal[this->nodePos->siHorzFlag - 1] +=
                adLocal[3] * this->adNodeVectorPos[X] -
                adLocal[4] * this->adNodeVectorPos[Y];
    }

    if (this->nodePos->siVertFlag)
    {
        adGlobal[this->nodePos->siVertFlag - 1] +=
                adLocal[4] * this->adNodeVectorPos[X] +
                adLocal[3] * this->adNodeVectorPos[Y];
    }

    if (this->nodePos->siRotFlag)
    {
        adGlobal[this->nodePos->siRotFlag - 1] += adLocal[5];
    }

    return;
}

//************************************
//* End of TRANSLATE_LOCAL_TO_GLOBAL *
//***************************************************************************

//***************************************************************************
//*
//* TRANSLATE_GLOBAL_TO_LOCAL: This function translates from the Global
//*     Coordinate Structure to a Local Coordinate Structure of a given
//*     member.
//*
//***************************************************************************

void Member::translateGlobalToLocal(double * adLocal, double * adGlobal)
{
    double adExchange[6] = { 0.0 };

    if (this->nodeNeg->siHorzFlag)
        adExchange[0] = adGlobal[this->nodeNeg->siHorzFlag - 1];
    if (this->nodeNeg->siVertFlag)
        adExchange[1] = adGlobal[this->nodeNeg->siVertFlag - 1];
    if (this->nodeNeg->siRotFlag)
        adExchange[2] = adGlobal[this->nodeNeg->siRotFlag - 1];

    if (this->nodePos->siHorzFlag)
        adExchange[3] = adGlobal[this->nodePos->siHorzFlag - 1];
    if (this->nodePos->siVertFlag)
        adExchange[4] = adGlobal[this->nodePos->siVertFlag - 1];
    if (this->nodePos->siRotFlag)
        adExchange[5] = adGlobal[this->nodePos->siRotFlag - 1];

    adLocal[0] = adExchange[0] * this->adNodeVectorNeg[X] + adExchange[1] * this->adNodeVectorNeg[Y];
    adLocal[1] = adExchange[1] * this->adNodeVectorNeg[X] - adExchange[0] * this->adNodeVectorNeg[Y];
    adLocal[2] = adExchange[2];
    adLocal[3] = adExchange[3] * this->adNodeVectorPos[X] + adExchange[4] * this->adNodeVectorPos[Y];
    adLocal[4] = adExchange[4] * this->adNodeVectorPos[X] - adExchange[3] * this->adNodeVectorPos[Y];
    adLocal[5] = adExchange[5];

    return;
}

//*************************************
//* End of TRANSLATE GLOBAL_ TO LOCAL *
//***************************************************************************

std::string Member::reportAssembly()
{
    //
    // Table 3 body...
    //
    std::string strNegJointType = "RIGID ";
    if (this->bJointNeg)
        strNegJointType = "PINNED";
    std::string strPosJointType = "RIGID ";
    if (this->bJointPos)
        strPosJointType = "PINNED";

    std::string strFormat;
    strFormat = strFormat + "   {:4d}     {:4d}    {:6}    {:4d}    {:6}    {:4d}\n";
    return
        fmt::vformat(
            strFormat,
            fmt::make_format_args(
                this->siID,
                this->siNegNodeID, strNegJointType,
                this->siPosNodeID, strPosJointType,
                this->siMatPropID
            )
        );
}

std::string Member::reportAdditions(bool bIsComposite)
{
    std::string strNA = "   N/A   ";
    //
    // Table 6 body...
    //
    std::string strLength = ( this->dLength   == 0.0 ? strNA : fmt::format(SF9_3f, this->dLength) );
    std::string strECLIP = ( this->dEffColumnLength_IP == 0.0 ? strNA : fmt::format(SF9_3f, this->dEffColumnLength_IP) );
    std::string strECLPP = ( this->dEffColumnLength_PP == 0.0 ? strNA : fmt::format(SF9_3f, this->dEffColumnLength_PP) );
    std::string strEBL = ( this->dEffBendingLength == 0.0 ? strNA : fmt::format(SF9_3f, this->dEffBendingLength) );
    std::string strVF = "";
    if ( bIsComposite ) {
        strVF = ( this->dVolumeFactor == 0.0 ? strNA : fmt::format(SF9_3f, this->dVolumeFactor) );
    }
    std::string strFormat;
    strFormat = strFormat + "   {:4d}  {:9}  {:9}   {:9}      {:9}     {:9}\n";
    return
        fmt::vformat(
            strFormat,
            fmt::make_format_args(
                this->siID, strLength, strECLIP, strECLPP, strEBL, strVF
            )
        );
}
