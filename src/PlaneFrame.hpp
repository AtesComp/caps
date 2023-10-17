/*
 * PlaneFrame.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef PLANEFRAME_HPP_
#define PLANEFRAME_HPP_

#include "ProblemSize.hpp"
#include "MaterialProperty.hpp"
#include "Node.hpp"
#include "Member.hpp"
#include "Reaction.hpp"
#include "StressFactors.hpp"
#include "LoadAndInteraction.hpp"
#include "LoadPoint.hpp"
#include "LoadUniform.hpp"
#include "LoadNodal.hpp"
#include "LoadTrapezoidal.hpp"
#include "StiffnessMatrix.hpp"

#include <vector>

// CAPS Plane Frame...

class PlaneFrame
{
public:
    ProblemSize ps;                         // Problem Size
    std::vector<MaterialProperty *> mprops; // Member Properties
    StressFactors sf;                       // Stress Factors
    std::vector<Node *> nodes;              // Nodes
    std::vector<Member *> members;          // Structural Assembly Members
    std::vector<Reaction *> react;          // Reactions (Bearings)
    LoadAndInteraction li;                  // Load and interaction equation
    std::vector<LoadPoint *> pl;            // Concentrated (point on member) Load
    std::vector<LoadUniform *> ul;          // Uniform Load
    std::vector<LoadNodal *> nl;            // Nodal (point at node) Load
    std::vector<LoadTrapezoidal *> tl;      // Trapezoidal Load

    double dCombinedExtHorz;                // Combined External Horizontal Energy
    double dCombinedExtVert;                // Combined External Vertical Energy
    double dCombinedExtRot;                 // Combined External Rotational Energy

    double dCombinedIntHorz;                // Combined Internal Horizontal Energy
    double dCombinedIntVert;                // Combined Internal Vertical Energy
    double dCombinedIntRot;                 // Combined Internal Rotational Energy

    double dTotalHorz;                      // Total Horizontal Energy
    double dTotalVert;                      // Total Vertical Energy
    double dTotalRot;                       // Total Rotational Energy

    StiffnessMatrix ssm;                    // System Stiffness Matrix
    double * adForceMatrix;                 // pointer to Force Matrix
    double * adDisplaceMatrix;              // pointer to Displacement Matrix

    bool bNoteCompression50;                // Member Note Flags
    bool bNoteCompression75;
    bool bNoteTension;
    bool bNoteStressDifference;
    bool bNoteBuckle;
    bool bNoteBending;
    bool bNoteEuler;
    bool bNoteTPI;

    short int siPerpendicular;

    // Methods...

    PlaneFrame(void);
    ~PlaneFrame(void);
    void clear(void);
    void clearOption(void);
    void clearLoads(void);
    void clearLoadAnalysis(void);
    void clearMemberNoteFlags(void);
    bool setup(void);
    bool processLoads(void);
    bool loadMembers(void);
    bool createDisplacementMatrix(void);
    void calcMemberForces(void);
    bool calcActions(void);
    void analyzeStructureForces(void);
    bool calcDeflections(void);
    void setOptionSpecialCrossSection(void);
    void setOptionVersion3(void);
    void setOptionComposite(void);
    char getOption(void);
    bool isSpecialCrossSection(void);
    bool isVersion3(void);
    bool isComposite(void);
    double getShearFactor(void);

private:
    char cProcessingOption;         // Option Indicator for processing
                                    //  0  = Standard
                                    // ‘S’ = Special Cross Section
                                    // ‘C’ = Composite (GLULAM, LVL, PSL)
                                    // '3’ = PPSA3 Input Option
    double dShearFactor;

    bool processMembers(void);
    bool processReactions(void);
    bool processReactionFreedom(void);
    bool findNodeFreedom(void);
    bool minimizeBandwidth(void);
    short int findSystemStiffnessMatrixSize(void);
    short int findSystemStiffnessMatrixBandwidth(void);
};

#endif /* PLANEFRAME_HPP_ */
