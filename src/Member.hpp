/*
 * Member.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef MEMBER_HPP_
#define MEMBER_HPP_

#include "MaterialProperty.hpp"
#include "Node.hpp"
#include "LoadPoint.hpp"
#include "LoadUniform.hpp"
#include "LoadTrapezoidal.hpp"
#include "ConcentratedSystem.hpp"
#include "DistributedSystem.hpp"
#include "MemberAnalysis.hpp"

#include <ostream>
#include <vector>

// Member Definition...

class Member
{
public:
    static const double dInteriorModifier;  // Global Interior Modifier
    static bool bIsComposite;               // Global Composite Indicator

    short int siID;                 // Member ID
    short int siNegNodeID;          // Negative Node Number
    short int siPosNodeID;          // Positive Node Number
    short int siMatPropID;          // Material Property ID
    bool bJointNeg;                 // Negative Joint Indicator
    bool bJointPos;                 // Positive Joint Indicator

    // As per the PPSA II documentation:
    // The Structural Assembly lines give the user the option of independently
    // specifying effective column lengths for each member in planes parallel
    // and perpendicular to the plane of the structure and specifying effective
    // bending length for beams lacking continuous edge support and/or torsional
    // restraint at their bearing points (NDS 3.3.3 and 4.4).
    double dEffColumnLength_IP;     // Effective Column Length: In-Plane
    double dEffColumnLength_PP;     // Effective Column Length: Perpendicular-Plane
    double dEffBendingLength;       // Effective Bending Length

    double dVolumeFactor;           // Volume Factor (Option C: Composite)

    // Calculated Values:
    double dLength;                 // Member's Length
    double dCos;                    // Member's Angular Cosine
    double dSin;                    // Member's Angular Sine
    double adNodeVectorNeg[2];      // Member's Negative Node Vector
    double adNodeVectorPos[2];      // Member's Positive Node Vector
    MaterialProperty * mp;          // Member's Material Property
    Node * nodeNeg;                 // Member's Negative Node
    Node * nodePos;                 // Member's Positive Node
    std::vector<ConcentratedSystem *> conc; // Member's Concentrated Load System
    std::vector<DistributedSystem *> dist;  // Member's Distributed Load System
    double adGlobalForce[6];        // Member's Global Force System
    double adLocalForce[6];         // Member's Local Force System

    MemberAnalysis ma;              // Member's Calculated Analysis Values

    Member(void);
    ~Member(void);
    void clear(void);
    void clearLoadAnalysis(void);

    void deriveCommonValues(void);
    bool isType2Composite(void);
    bool isLaterallySupported(void);
    bool isInteriorMember(void);
    bool isTrussChord(void);
    bool isMaterialMachineTested(void);
    bool process(std::vector<MaterialProperty *> &, std::vector<Node *> &);
    bool loadMember(std::vector<LoadPoint *> &,
                    std::vector<LoadUniform *> &,
                    std::vector<LoadTrapezoidal *> &,
                    double *);
    void calcForces(double *);
    void calcStiffnessMatrix(double [][6]);
    void calcBending1986(void);
    void calcBending1991(void);
    void calcTension(bool &, bool &);
    void calcCompression1986(double [], bool, double, double,
                             bool &, short int &, bool &, bool &);
    void calcCompression1991(double [], bool, double, double,
                             bool &, short int &, bool &,
                             bool &, bool &, bool &);
    bool calcDeflection(bool, double *, std::vector<LoadTrapezoidal *> &, short int);
    std::string reportAssembly();
    std::string reportAdditions(bool);

private:
    void translateLocalToGlobal(double *, double *);
    void translateGlobalToLocal(double *, double *);
    double calcColumn(double []);
};

#endif /* MEMBER_HPP_ */
