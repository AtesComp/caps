/*
 * PlaneFrame.cpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#include "PlaneFrame.hpp"
#include "NodeDegree.hpp"
#include "LoadTrapezoidalExtra.hpp"

#include "SystemDef.hpp"

#include <iostream>
#include <exception>
#include <iomanip>
#include <cstddef>
#include <cstdlib>
#include <cmath>

PlaneFrame::PlaneFrame(void)
{
    this->mprops.clear();
    this->nodes.clear();
    Node::siUsedSize = 0;
    Node::siLargestDegrees = 0;
    this->members.clear();
    Member::bIsComposite = false;
    this->react.clear();
    this->pl.clear();
    this->ul.clear();
    this->nl.clear();
    this->tl.clear();
    this->adForceMatrix = nullptr;
    this->adDisplaceMatrix = nullptr;
    this->clear();
}

PlaneFrame::~PlaneFrame(void)
{
    this->clear();
}

//***************************************************************************
//*
//* PlaneFrame::clear
//*     This function clears the entire PFRAME structure.
//*
//***************************************************************************

void PlaneFrame::clear(void)
{
    this->clearOption();

    this->ps.clear();

    if ( ! this->mprops.empty() )
    {
        for ( MaterialProperty * mpCurr : this->mprops )
            delete mpCurr;
        this->mprops.clear();
    }

    this->sf.clear();

    if ( ! this->nodes.empty() )
    {
        for (Node * nodeCurr : this->nodes)
            delete nodeCurr;
        this->nodes.clear();
    }

    if ( ! this->members.empty() )
    {
        for (Member * memberCurr : this->members)
            delete memberCurr;
        this->members.clear();
    }

    if ( ! this->react.empty() )
    {
        for (Reaction * reactCurr : this->react)
            delete reactCurr;
        this->react.clear();
    }

    this->li.clear();

    this->clearLoads();

    this->ssm.clear();

    return;
}
//*
//* End of PlaneFrame::clear
//***************************************************************************

void PlaneFrame::clearOption(void)
{
    this->cProcessingOption = 0;
    Member::bIsComposite = false;
    this->dShearFactor = 1.5;
}

//***************************************************************************
//*
//* PlaneFrame::clearLoads
//*     This function clears the loads of a PlaneFrame object.
//*
//***************************************************************************

void PlaneFrame::clearLoads(void)
{
    this->dCombinedExtHorz = 0.0;
    this->dCombinedExtVert = 0.0;
    this->dCombinedExtRot = 0.0;

    this->dCombinedIntHorz = 0.0;
    this->dCombinedIntVert = 0.0;
    this->dCombinedIntRot = 0.0;

    this->dTotalHorz = 0.0;
    this->dTotalVert = 0.0;
    this->dTotalRot = 0.0;

    if ( ! this->pl.empty() )
    {
        for (LoadPoint * plCurr : this->pl)
            delete plCurr;
        this->pl.clear();
    }

    if ( ! this->ul.empty() )
    {
        for (LoadUniform * ulCurr : this->ul)
            delete ulCurr;
        this->ul.clear();
    }

    if ( ! this->nl.empty() )
    {
        for (LoadNodal * nlCurr : this->nl)
            delete nlCurr;
        this->nl.clear();
    }

    if ( ! this->tl.empty() )
    {
        for (LoadTrapezoidal * tlCurr : this->tl)
            delete tlCurr;
        this->tl.clear();
    }

    this->clearLoadAnalysis();

    return;
}
//*
//* End of PlaneFrame::clearLoads
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::clearLoadAnalysis
//*     This function clears the force and displacement matrices of a
//*     PlaneFrame object.
//*
//***************************************************************************

void PlaneFrame::clearLoadAnalysis(void)
{
    if (this->adForceMatrix != nullptr)
        delete this->adForceMatrix;
    this->adForceMatrix = nullptr;

    if (this->adDisplaceMatrix != nullptr)
        delete this->adDisplaceMatrix;
    this->adDisplaceMatrix = nullptr;

    for (Member * memberCurr : this->members)
    {
        memberCurr->clearLoadAnalysis();
    }

    this->clearMemberNoteFlags();

    return;
}
//*
//* End of PlaneFrame::clearLoadAnalysis
//***************************************************************************

void PlaneFrame::clearMemberNoteFlags(void)
{
    this->bNoteCompression50 = false;
    this->bNoteCompression75 = false;
    this->bNoteTension = false;
    this->bNoteStressDifference = false;
    this->bNoteBuckle = false;
    this->bNoteBending = false;
    this->bNoteEuler = false;
    this->bNoteTPI = false;

    this->siPerpendicular = 0;
}

void PlaneFrame::setOptionSpecialCrossSection(void)
{
    this->cProcessingOption = 'S';
    this->dShearFactor = 1.0;
}

void PlaneFrame::setOptionComposite(void)
{
    this->cProcessingOption = 'C';
    Member::bIsComposite = true;
    this->dShearFactor = 1.5;
}

void PlaneFrame::setOptionVersion3(void)
{
    this->cProcessingOption = '3';
    this->dShearFactor = 1.5;
}

char PlaneFrame::getOption(void)
{
    return this->cProcessingOption;
}

bool PlaneFrame::isSpecialCrossSection(void)
{
    return (this->cProcessingOption == 'S');
}

bool PlaneFrame::isVersion3(void)
{
    return (this->cProcessingOption == '3');
}

bool PlaneFrame::isComposite(void)
{
    return (this->cProcessingOption == 'C');
}

double PlaneFrame::getShearFactor(void)
{
    return this->dShearFactor;
}

//*************************************************************************
//*
//* PlaneFrame::setup
//*     This function sets up the plane frame structure for loading.  It
//*     looks for errors in format and sets up other structure variables.
//*
//*************************************************************************

bool PlaneFrame::setup(void)
{
    // Total number of possible Node Freedom parts (Horz, Vert, Rot)...
    this->ps.siNodeFreedom = this->ps.uiNodes * 3;

    // Number of Node Coordinate parts that have freedom of movement...
    //  Subtract the reaction node's parts that are rigid:
    //      ROL PIN: 1 part  is  held rigid ( Horz or Vert )
    //      FIX PIN: 2 parts are held rigid ( Horz and Vert )
    //      ROL RGD: 2 parts are held rigid ( Horz or Vert, and Rot )
    //      FIX RGD: 3 parts are held rigid ( Horz, Vert, and Rot )
    this->ps.siNodePinned = this->ps.siNodeFreedom -
            (this->ps.siRolPin + (this->ps.siFixPin + this->ps.siRolRgd) * 2 + this->ps.siFixRgd * 3);

    // Total number of reaction node definitions...
    this->ps.uiReactions = this->ps.siRolPin + this->ps.siFixPin + this->ps.siRolRgd + this->ps.siFixRgd;

    //
    // Check for proper number of nodes...
    //
    if (this->nodes.size() != this->ps.uiNodes)
    {
        std::cerr << "\n ERROR: Input file - Number of NODES does not agree with number specified!\n"
                  <<   "        CAPS program aborted...\n";
        return false;
    }

    //
    // Check for proper number of members...
    //
    if (this->members.size() != this->ps.uiMembers)
    {
        std::cerr << "\n ERROR: Input file - Number of MEMBERS does not agree with number specified!\n"
                  <<   "        CAPS program aborted...\n";
        return false;
    }

    //
    // Check for proper number of reactions...
    //
    if (this->react.size() != this->ps.uiReactions)
    {
        std::cerr << "\n ERROR: Input file - Number of REACTIONS does not agree with number specified!\n"
                  <<   "        CAPS program aborted...\n";
        return false;
    }

    //*************************************
    //*
    //* Process the structure's components...
    //*
    //*************************************

    // Process the Members for Node relationships...
    //   NOTE: Must be done before processReactions()
    if ( ! this->processMembers() )
        return false;

    // Process the Reactions for Node and Member relations...
    if ( ! this->processReactions() )
        return false;

    // Find the structure's Node Freedom...
    if ( ! this->findNodeFreedom() )
        return false;

    //  Minimize the structure's Node Bandwidth...
    if ( ! this->minimizeBandwidth() )
        return false;

    // Find the structure's System Stiffness Matrix Size...
    short int siMatrixSize = this->findSystemStiffnessMatrixSize();
    if (siMatrixSize != this->ps.siNodePinned)
    {
        std::cerr << "\n ERROR: Calculated System Stiffness Matrix size does\n"
                  <<   "        NOT match Pinned Node Coordinate Count!\n"
                  <<   "        SSM (" << siMatrixSize << ") != PNCC (" << this->ps.siNodePinned << ")\n";
        return false;
    }

    // Find the structure's System Stiffness Matrix Bandwidth...
    short int siBandwidth = this->findSystemStiffnessMatrixBandwidth();

    // Create the structure's System Stiffness Matrix...
    if ( ! this->ssm.create(siMatrixSize, siBandwidth, this->members) )
        return false;

    // Decompose the stiffness matrix into the A Matrix...
    if ( ! this->ssm.decompose() )
        return false;

    return true;
}
//*
//* End of PlaneFrame::setup
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::processMembers
//*     Analyze Structure for Member information related to the Nodes that
//*     define the member's location.
//*
//*     For a member: neg node is the negative end node
//*                   pos_node is the positive end node
//
//***************************************************************************

bool PlaneFrame::processMembers(void)
{
    for (Member * memberCurr : this->members)
    {
        if ( ! memberCurr->process(this->mprops, this->nodes) )
            return false;
    }

    return true;
}
//*
//* End of PlaneFrame::processMembers
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::processReactions
//*     Analyze Structure for Reaction information related to the Members and
//*     Nodes that define a reaction.
//*
//***************************************************************************

bool PlaneFrame::processReactions(void)
{
    for (Reaction * reactCurr : this->react)
    {
        // Link Members to Reactions...
        if ( ! reactCurr->linkMembers(this->members) )
            return false;

        // Link Nodes to Reactions...
        if ( ! reactCurr->linkNodes(this->nodes) )
            return false;
    }

    // Process the Reactions for Node Freedom...
    if ( ! this->processReactionFreedom() )
        return false;

    return true;
}
//*
//* End of PlaneFrame::processReactions
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::processReactionFreedom
//*     This function sets the RIGID or PINNED condition for the structure's
//*     reaction nodes.  If any reaction node does not have any rigid
//*     connections (i.e., no FIX nor FIRL nor RIGID attachment to at least
//*     one member), the node has Rotational Freedom of Movement (it is
//*     PINNED)...
//*
//***************************************************************************

bool PlaneFrame::processReactionFreedom(void)
{
    //* Process the Reaction Nodes...
    //***********************************

    ReactionCounts reactionCounts = { 0, 0, 0, 0 };

    for (Reaction * reactCurr : this->react)
    {
        reactCurr->processAction(reactionCounts);
    }

    //
    // Check the number of reactions for each Reaction Type...
    //

    std::string strErrRead = "NONE";
    bool bReactionErr = false;
    if (reactionCounts.siFixPin != this->ps.siFixPin)
    {
        bReactionErr = true;
        strErrRead = "PIN ";
    }
    else if (reactionCounts.siFixRgd != this->ps.siFixRgd)
    {
        bReactionErr = true;
        strErrRead = "FIX ";
    }
    else if (reactionCounts.siRolPin != this->ps.siRolPin)
    {
        bReactionErr = true;
        strErrRead = "ROLL";
    }
    else if (reactionCounts.siRolRgd != this->ps.siRolRgd)
    {
        bReactionErr = true;
        strErrRead = "FIRL";
    }

    //
    // If the number of Reactions is different from number specified,
    //      then print the error message.....
    //

    if (bReactionErr)
    {
        std::cerr << "\n ERROR: Number of " << std::setw(4) << strErrRead << " Reactions does not agree with number specified!\n";
        return false;
    }

    return true;

}
//*
//* End of PlaneFrame::processReactionFreedom
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::findNodeFreedom
//*     This function checks the FIXED or FREE condition for the structure's
//*     nodes.  If any Node does not have any RIGID connections,
//*     i.e., no FIX or FIRL reaction nor RIGID attachment to at least one member,
//      the node has Rotational Freedom of Movement (it is PINNED)...
//
//***************************************************************************

bool PlaneFrame::findNodeFreedom()
{
    for (Node * nodeCurr : this->nodes)
    {
        if ( nodeCurr->siSequence == 0 ) // Not Used...
            this->ps.siNodePinned -= 3;  // ...remove Horz, Vert, Rot
        else // Used...
        {
            // Count all the used nodes...
            Node::siUsedSize++;

            // Set largest Node Degrees...
            if (nodeCurr->siDegrees > Node::siLargestDegrees)
                Node::siLargestDegrees = nodeCurr->siDegrees;

            // PPSA II documentation
            // ================================================================================
            // Fixing intersecting members to a node causes them to rotate together and share
            // moments with other members that may likewise be fixed to the same node.  Pinning
            // a member end at a node causes a zero moment at that member end and creates
            // independence between rotation of the node and rotation of the member end.
            // The original PPSA (Suddarth 1972) required that at least one member end be fixed
            // to each node in the analog to avoid an arithmetic error within the computer.
            // PPSA II was structured to allow a node to be free of fixation from any member.
            // The node is then fixed against rotation in the analysis and a zero rotational
            // displacement will consequently be reported.
            // ================================================================================
            //
            // When all of a Node's Members are FREE (i.e., pinned at the node within
            // the Member::process() function), the following code carries out the above
            // directive--a completely free node is FIXED against rotation.  The member
            // ends are calculated as FREE to rotate--none of an end's moment is transferred to
            // any other member attached to the node.
            //
            //   NOTE: Reaction fixture has already been considered by the ProblemSize
            //         Reaction declarations.
            //
            // Therefore, when a node is FREE via Member analysis...
            if ( nodeCurr->bMemberFixture )
            {
                nodeCurr->siRotFlag = true; // ...set node rotation FIXED as we DO NOT want
                                            //    Member ends attached to the node to transfer
                                            //    energy.
                this->ps.siNodePinned--;    // ...and reduce unknowns for calculation: moment
                                            //    is zero (0).
            }
        }
    }

    return true;
}
//*
//* End of PlaneFrame::findNodeFreedom
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::minimizeBandwidth
//*     This function reduces the System Stiffness Matrix (SSM) "distance"
//*     between a Member's two end Nodes by assigning sequence numbers to
//*     each Node, analyzing the numeric difference between all connected
//*     Nodes, and re-sequencing the Nodes so the numeric difference between
//*     connected nodes is as minimal as possible.
//*
//*     Then, the minimized difference is the minimal width from the diagonal
//*     of the sparse, square SSM.
//*
//*     NOTE: Resequencing the Node numbering scheme takes place here. The
//*         program currently tries to minimize the bandwidth of the matrix,
//*         but the banding method is not 'fool proof' as a Node's Member
//*         count may be the same size as other counts (then, the selected
//*         Node may not be optimal).  UPDATE if a better 'fool proof'
//*         resequencing function is found.
//
//***************************************************************************

bool PlaneFrame::minimizeBandwidth(void)
{
    //*************************************************
    //* Allocate memory for Degree of Node Usage...
    //*************************************************

    NodeDegree * aNodeDegree = new NodeDegree[Node::siUsedSize];
    if ( aNodeDegree == nullptr )
    {
        std::cerr << "\n ERROR (minimizeBandwidth): Out of memory allocating Node Degree!\n";
        return false;
    }

    //**********************************************************
    //* Allocate memory for Adjacent Degree of Node Usage...
    //**********************************************************

    NodeDegree * aAdjacentNodeDegree = new NodeDegree[Node::siLargestDegrees];
    if ( aAdjacentNodeDegree == nullptr )
    {
    	std::cerr << "\n ERROR (minimizeBandwidth): Out of memory allocating Adjacent Node Degree!\n";
        return false;
    }

    //***********************************************************
    //* Load NodeDegree info from Nodes...
    //***********************************************************

    short int siIndex = 0;
    short int siCurrentIndex = 0;
    for (Node * nodeCurr : this->nodes)
    {
        if (nodeCurr->siSequence == 0) // ...not Used Node
            continue; // ...get next node

        // Set each Node Degree's values...
        aNodeDegree[siIndex].node = nodeCurr;
        aNodeDegree[siIndex].siSequence = 0;
        aNodeDegree[siIndex].siDegrees = nodeCurr->siDegrees;

        // Find Lowest Degree of Node Usage...
        if (aNodeDegree[siCurrentIndex].siDegrees > aNodeDegree[siIndex].siDegrees)
            siCurrentIndex = siIndex;

        ++siIndex;
    }

    //******************************************
    //* Sort Degree List for bandwidth...
    //******************************************

    //
    // Set First Node Degree's sequence number...
    //
    short int siLocation = 1;
    aNodeDegree[siCurrentIndex].siSequence = siLocation;

    //
    // Set Other Node Degrees' sequence numbers...
    //
    for (short int siNodeSequence = 1; siNodeSequence < Node::siUsedSize; siNodeSequence++)
    {
        //
        // Get the Adjacent Degree List...
        //
        short int siAdjacentNodeSize = 0;
        for (Member * memberCurr : this->members)
        {
            // For the Current Node, find a Member using it and it's opposing Node...
            Node * nodeOpposite = nullptr;
            if (aNodeDegree[siCurrentIndex].node == memberCurr->nodeNeg)
                nodeOpposite = memberCurr->nodePos;
            else if (aNodeDegree[siCurrentIndex].node == memberCurr->nodePos)
                nodeOpposite = memberCurr->nodeNeg;
            if (nodeOpposite == nullptr)
                continue;

            //
            // Add Adjacent Node NodeDegree to AdjacentNodeDegree List...
            //
            for ( siIndex = 0; siIndex < Node::siUsedSize; siIndex++, (siIndex == siCurrentIndex) ? siIndex++ : false )
            {
                // Find the same Member's opposite Node...
                if (aNodeDegree[siIndex].node == nodeOpposite)
                {
                    aAdjacentNodeDegree[siAdjacentNodeSize] = aNodeDegree[siIndex];
                    ++siAdjacentNodeSize;
                    break;
                }
            }
        }

        //
        // Sort Adjacent Degree List from Lowest to Highest...
        //
        short int siLowerIndex = 0;
        NodeDegree nodedegreeLow = aAdjacentNodeDegree[0];
        for (short int siLowIndex = 0; siLowIndex < siAdjacentNodeSize - 1; siLowIndex++)
        {
            siLowerIndex = siLowIndex;
            for (short int siANDIndex = siLowIndex + 1; siANDIndex < siAdjacentNodeSize; siANDIndex++)
            {
                if (aAdjacentNodeDegree[siLowerIndex].siDegrees > aAdjacentNodeDegree[siANDIndex].siDegrees)
                    siLowerIndex = siANDIndex;
            }
            if (siLowerIndex > siLowIndex) // SWAP...
            {
                nodedegreeLow = aAdjacentNodeDegree[siLowIndex];
                aAdjacentNodeDegree[siLowIndex] = aAdjacentNodeDegree[siLowerIndex];
                aAdjacentNodeDegree[siLowerIndex] = nodedegreeLow;
            }
        }

        //
        // Set AdjacentNodeDegrees to new sequence number...
        //
        for (short int siANDIndex = 0; siANDIndex < siAdjacentNodeSize; siANDIndex++)
        {
            // Find the NodeDegree of the current AdjacentNodeDegree...
            siIndex = 0;
            while (aNodeDegree[siIndex].node != aAdjacentNodeDegree[siANDIndex].node && siIndex < Node::siUsedSize)
                siIndex++;

            //  If the found NodeDegree has NOT already been set to a sequence...
            if (siIndex < Node::siUsedSize && ( ! aNodeDegree[siIndex].siSequence ) )
            {
                ++siLocation;
                aNodeDegree[siIndex].siSequence = siLocation; // ...set the sequence
            }
        }

        //
        // Set Next Current NodeDegree as the next lowest NodeDegree that has been set...
        //
        for (siIndex = 0; siIndex < Node::siUsedSize; siIndex++)
        {
            if (aNodeDegree[siIndex].siSequence == siNodeSequence + 1)
            {
                siCurrentIndex = siIndex;
                break;
            }
        }
    }

    //
    // Set the Node Sequence numbers to the "optimized" NodeDegree Sequence numbers...
    //
    for (siIndex = 0; siIndex < Node::siUsedSize; siIndex++)
        aNodeDegree[siIndex].node->siSequence = aNodeDegree[siIndex].siSequence;

    // Clear Degree Lists...
    delete aNodeDegree;
    delete aAdjacentNodeDegree;

    return true;
}
//*
//* End of PlaneFrame::minimizeBandwidth
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::findSystemStiffnessMatrixSize
//*     This function finds the Matrix Size (the length of a side of a
//*     Square Matrix) of the System Stiffness Matrix.
//*
//*     It uses the Degrees of Freedom (DoF) Indices for each node.  The
//*     indices are initially used as flags and transformed here to indices:
//*       All DoF Indices set ON (FIXED) are set to UNUSED (0).
//*       All DoF Indices set OFF (FREE) are set to a sequence of index
//*         locations into the System Stiffness Matrix.
//*     Therefore, the matrix array is index 1 based, not 0 based, as 0
//*     indicated unused.
//*
//*     The Matrix Size is calculated as the set of stiffness equations
//*     used to calculate for unknowns (FREE flags).  The last index sequence
//*     set is the size of the System Stiffness Matrix.
//*
//***************************************************************************

short int PlaneFrame::findSystemStiffnessMatrixSize(void)
{

    short int siMatrixIndex = 0;
    for (short int siSequence = 1; ; siSequence++)
    {
        // Select each node in SEQUENCE order....
        Node * nodeCurr = nullptr;
        for (Node * nodeFind : this->nodes)
        {
            if ( nodeFind->siSequence == siSequence)
            {
                nodeCurr = nodeFind;
                break;
            }
        }
        if (nodeCurr == nullptr)
            break;

        if (nodeCurr->siHorzFlag)     // ...node's horizontal is FIXED...
            nodeCurr->siHorzFlag = 0; // ...don't need it
        else // ...horizontal is FREE...
        {
            ++siMatrixIndex;
            // ...store its horizontal matrix index...
            nodeCurr->siHorzFlag = siMatrixIndex;
        }

        // TODO: Check for ROLL Vector, 'cause FIXED also means Vector Free
        //       when Y value != 0.0
        if (nodeCurr->siVertFlag)     // ...node's vertical is FIXED...
            nodeCurr->siVertFlag = 0; // ...don't need it
        else // ...vertical is FREE...
        {
            ++siMatrixIndex;
            // ...store its vertical matrix index...
            nodeCurr->siVertFlag = siMatrixIndex;
        }

        if (nodeCurr->siRotFlag)     // ...node's rotation is FIXED...
            nodeCurr->siRotFlag = 0; // ...don't need it
        else // ...rotation is FREE...
        {
            ++siMatrixIndex;
            // ...store its moment matrix index...
            nodeCurr->siRotFlag = siMatrixIndex;
        }
    }

    // The resulting last matrix index is the matrix size...
    return siMatrixIndex;
}
//*
//* End of PlaneFrame::findSystemStiffnessMatrixSize
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::findSystemStiffnessMatrixBandwidth
//*     This function finds the bandwidth of the System Stiffness Matrix.
//*     The bandwidth is the minimum bandwidth calculated from the given
//*     member's sequenced nodes.
//*
//***************************************************************************

short int PlaneFrame::findSystemStiffnessMatrixBandwidth(void)
{
    short int siBandwidth = 0;

    for (Member * memberCurr : this->members)
    {
        short int siLow = this->ps.siNodePinned + 1;
        short int siHigh = 0;
        short int asiOrder[6];
        asiOrder[0] = memberCurr->nodeNeg->siHorzFlag;
        asiOrder[1] = memberCurr->nodeNeg->siVertFlag;
        asiOrder[2] = memberCurr->nodeNeg->siRotFlag;
        asiOrder[3] = memberCurr->nodePos->siHorzFlag;
        asiOrder[4] = memberCurr->nodePos->siVertFlag;
        asiOrder[5] = memberCurr->nodePos->siRotFlag;

        for (short int count = 0; count < 6; count++)
        {
            if (asiOrder[count]) // ...DoF index set...
            {
                if (siLow > asiOrder[count])
                    siLow = asiOrder[count];
                if (siHigh < asiOrder[count])
                    siHigh = asiOrder[count];
            }
        }

        short int siWidth = abs(siHigh - siLow) + 1;
        if (siWidth > siBandwidth)
            siBandwidth = siWidth;
    }

    return siBandwidth;
}
//*
//* End of PlaneFrame::findSystemStiffnessMatrixBandwidth
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::processLoads
//*     This function processes the loading requirements for the structure.
//*
//*     The LOAD TYPES are divided into two categories:
//*
//*         "CONCENTRATED" Systems              "DISTRIBUTED" Systems
//*             Concentrated                        Uniform
//*             Nodal                               Trapezoidal
//*
//***************************************************************************

bool PlaneFrame::processLoads(void)
{
    //
    // Set up the Force Matrix...
    //

    double * adForceWork = new double[ this->ssm.getMatrixSize() ]();
    if (adForceWork == nullptr)
    {
        std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                  <<   "        Force Matrix too large!!\n";
        return false;
    }

    //*
    //* Set up the working Stress Maximums for
    //* Normal and GluLam/LVL/PSL Options.....
    //***********************************************

    if ( ! this->isSpecialCrossSection() )
    {
        //
        // Set up Stress Factor Indicator...
        //

        short int siSFIndicator = this->li.siStressFactorIndex;
        double dFactor = 1.0; // ...default for bad stress factor index
        if (siSFIndicator > 0)
            dFactor = this->sf.factor[siSFIndicator - 1];

        //
        // Set up factored Stress Maximums...
        //

        for (Member * memberCurr : this->members)
        {
            memberCurr->ma.dForceBendWork = memberCurr->mp->dForceBend * dFactor;
            memberCurr->ma.dForceCompWork = memberCurr->mp->dForceComp * dFactor;
            memberCurr->ma.dForceTensWork = memberCurr->mp->dForceTens * dFactor;
        }
    }

    //*
    //* Set up the Combined External Forces...
    //*******************************************************

    double dCombinedExtHorzWork = 0.0;
    double dCombinedExtVertWork = 0.0;
    double dCombinedExtRotWork  = 0.0;

    //*
    //* Analyze Concentrated Loads for Combined Energy...
    //*******************************************************

    for (LoadPoint * plCurr : this->pl)
    {
        dCombinedExtHorzWork += plCurr->adLoadVect[X];
        dCombinedExtVertWork += plCurr->adLoadVect[Y];

        // Get Member information...
        Member * memberFound = nullptr;
        for (Member * memberCurr : this->members)
        {
            if (memberCurr->siID == plCurr->siMemberID)
            {
                memberFound = memberCurr;
                break;
            }
        }
        if (memberFound == nullptr)
        {
            std::cerr << "\n ERROR: Member Number " << plCurr->siMemberID << std::endl
                      <<   "        not found in Concentrated Load List!\n";
            return false;
        }

        dCombinedExtRotWork += (memberFound->nodeNeg->adPoint[X] + memberFound->dCos * plCurr->dDistance) * plCurr->adLoadVect[Y] -
                               (memberFound->nodeNeg->adPoint[Y] + memberFound->dSin * plCurr->dDistance) * plCurr->adLoadVect[X];
    }

    //*
    //* Analyze Uniform Loads for Combined Energy...
    //**************************************************

    for (LoadUniform * ulCurr : this->ul)
    {
        // Get Member information...
        Member * memberFound = nullptr;
        for (Member * memberCurr : this->members)
        {
            if (memberCurr->siID == ulCurr->siMemberID)
            {
                memberFound = memberCurr;
                break;
            }
        }
        if (memberFound == nullptr)
        {
            std::cerr << "\n ERROR: Member Number " << ulCurr->siMemberID << std::endl
                      <<   "        not found in Uniform Load List!\n";
            return false;
        }

        double dLocalExtHorz = ulCurr->adLoadVect[X] * memberFound->dLength * fabs(memberFound->dSin);
        dCombinedExtHorzWork += dLocalExtHorz;
        double dLocalExtVert = ulCurr->adLoadVect[Y] * memberFound->dLength * fabs(memberFound->dCos);
        dCombinedExtVertWork += dLocalExtVert;
        dCombinedExtRotWork  += (dLocalExtVert * (memberFound->nodeNeg->adPoint[X] + memberFound->nodePos->adPoint[X]) / 2.0 -
                                 dLocalExtHorz * (memberFound->nodeNeg->adPoint[Y] + memberFound->nodePos->adPoint[Y]) / 2.0);
    }

    //*
    //* Analyze Nodal Loads for Combined Energy...
    //************************************************

    for (LoadNodal * nlCurr : this->nl)
    {
        // Get Node information...
        Node * nodeFound = nullptr;
        for (Node * nodeCurr : this->nodes)
        {
            if ( nodeCurr->siSequence && nodeCurr->siID == nlCurr->siNodeID )
            {
                nodeFound = nodeCurr;
                break;
            }
        }
        if (nodeFound == nullptr)
        {
            std::cerr << "\n ERROR: Node Number " << nlCurr->siNodeID << std::endl
                      <<   "        not found in Node Load List!\n";
            return false;
        }

        // Set Node Load in Node...
        nodeFound->nodeload = nlCurr;

        // X Direction Node Load...
        if (nlCurr->siLoadDirection == 1)
        {
            dCombinedExtHorzWork += nlCurr->dLoad;
            dCombinedExtRotWork  -= nlCurr->dLoad * nodeFound->adPoint[Y];
            if (nodeFound->siHorzFlag)
                adForceWork[nodeFound->siHorzFlag - 1] += nlCurr->dLoad;
        }
        // Y Direction Node Load...
        else if (nlCurr->siLoadDirection == 2)
        {
            dCombinedExtVertWork += nlCurr->dLoad;
            dCombinedExtRotWork  += nlCurr->dLoad * nodeFound->adPoint[X];
            if (nodeFound->siVertFlag)
                adForceWork[nodeFound->siVertFlag - 1] += nlCurr->dLoad;
        }
        // Moment Node Load...
        else if (nlCurr->siLoadDirection == 3)
        {
            dCombinedExtRotWork += nlCurr->dLoad;
            if (nodeFound->siRotFlag)
                adForceWork[nodeFound->siRotFlag - 1] += nlCurr->dLoad;
        }
        // Unknown direction for Node Load...
        else
        {
            std::cerr << "\n ERROR: Node Load Direction for Node Number " << nlCurr->siNodeID << std::endl
                      <<   "        not a valid Direction!\n"
                      <<   "     Valid Types are: 1] X Direction\n"
                      <<   "                      2] Y Direction\n"
                      <<   "                      3] Moment Rotation\n";
            return false;
        }
    }

    //*
    //* Analyze Trapezoidal Loads for Combined Energy...
    //******************************************************

    for (LoadTrapezoidal * tlCurr : this->tl)
    {
        // Get Member information...
        Member * memberFound = nullptr;
        for (Member * memberCurr : this->members)
        {
            if (memberCurr->siID == tlCurr->siMemberID)
            {
                memberFound = memberCurr;
                break;
            }
        }
        if (memberFound == nullptr)
        {
            std::cerr << "\n ERROR: Member Number " << tlCurr->siMemberID << std::endl
                      <<   "        not found in Trapezoidal Load List!\n";
            return false;
        }

        // Convert rotation angle degrees to radians...
        double theta_rad = tlCurr->dTheta / 180.0 * M_PIl;

        tlCurr->adStart[X] = tlCurr->adLoad[X] * sin(theta_rad);
        tlCurr->adStart[Y] = tlCurr->adLoad[X] * cos(theta_rad);

        tlCurr->adEnd[X]   = tlCurr->adLoad[Y] * sin(theta_rad);
        tlCurr->adEnd[Y]   = tlCurr->adLoad[Y] * cos(theta_rad);

        double dX1 = tlCurr->adStart[X] * fabs(memberFound->dCos) - tlCurr->adStart[Y] * fabs(memberFound->dSin);
        double dY1 = tlCurr->adStart[X] * fabs(memberFound->dSin) + tlCurr->adStart[Y] * fabs(memberFound->dCos);

        double dX2 = tlCurr->adEnd[X]   * fabs(memberFound->dCos) - tlCurr->adEnd[Y]   * fabs(memberFound->dSin);
        double dY2 = tlCurr->adEnd[X]   * fabs(memberFound->dSin) + tlCurr->adEnd[Y]   * fabs(memberFound->dCos);

        double dX3 = dX2 - dX1;
        double dY3 = dY2 - dY1;

        double d4 = tlCurr->adDist[1] - tlCurr->adDist[0];

        dCombinedExtHorzWork += (dX1 + dX2) / 2.0 * d4;
        dCombinedExtVertWork += (dY1 + dY2) / 2.0 * d4;
        dCombinedExtRotWork  += (
            dX1 * d4       * ((tlCurr->adDist[0] + d4       / 2.0) * fabs(memberFound->dSin) + memberFound->nodeNeg->adPoint[Y]) +
            dX3 * d4 / 2.0 * ((tlCurr->adDist[0] + d4 * 2.0 / 3.0) * fabs(memberFound->dSin) + memberFound->nodeNeg->adPoint[Y]) +
            dY1 * d4       * ((tlCurr->adDist[0] + d4       / 2.0) * fabs(memberFound->dCos) + memberFound->nodeNeg->adPoint[X]) +
            dY3 * d4 / 2.0 * ((tlCurr->adDist[0] + d4 * 2.0 / 3.0) * fabs(memberFound->dCos) + memberFound->nodeNeg->adPoint[X]) );
    }

    //
    // Store Combined External Forces...
    //

    this->adForceMatrix = adForceWork;

    this->dCombinedExtHorz = dCombinedExtHorzWork;
    this->dCombinedExtVert = dCombinedExtVertWork;
    this->dCombinedExtRot  = dCombinedExtRotWork;

    return true;
}
//*
//* End of PlaneFrame::processLoads
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::loadMembers
//*     This function sets the load forces for all member and collects the
//*     global force matrix.
//*
//*     The LOAD TYPES are divided into two categories:
//*
//*         "CONCENTRATED" Systems          "DISTRIBUTED" Systems
//*             Concentrated                    Uniform
//*             Nodal                           Trapezoidal
//*
//***************************************************************************

bool PlaneFrame::loadMembers(void)
{
    //*****************************************
    //* Analyze Member Information for
    //* Stiffness and Force Vectors.....
    //*****************************************

    for (Member * memberCurr : this->members)
    {
        if ( ! memberCurr->loadMember(this->pl, this->ul, this->tl, this->adForceMatrix) )
            return false;
    }

    return true;
}
//*
//* End of PlaneFrame::loadMembers
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::createDisplacementMatrix
//*     This function creates and populates the System Displacement Matrix
//*     for the structure.
//*
//***************************************************************************

bool PlaneFrame::createDisplacementMatrix(void)
{
    // Set up a new Displacement Matrix...

    this->adDisplaceMatrix = new double[ this->ssm.getMatrixSize() ]();
    if (this->adDisplaceMatrix == nullptr)
    {
        std::cerr << "\n Error: OUT OF MEMORY!!\n"
                  <<   "        Displacement Matrix too large!!\n";
        return false;
    }

    // Populate the new Displacement Matrix calculating the system Displacements
    //      via the system's Force Matrix...
    this->ssm.calcDisplacement(this->adDisplaceMatrix, this->adForceMatrix);

    return true;
}
//*
//* End of PlaneFrame::createDisplacementMatrix
//***************************************************************************

void PlaneFrame::calcMemberForces(void)
{
    for (Member * memberCurr : this->members)
    {
        // Calculate Local Member End Forces...
        memberCurr->calcForces(this->adDisplaceMatrix);
    }
}

//***************************************************************************
//*
//* PlaneFrame::calcActions
//*     This function calculates the end actions for the structure.
//*
//***************************************************************************

bool PlaneFrame::calcActions(void)
{
    //
    // Calculate Node and Member End Actions and Reactions...
    //

    for (Reaction * reactCurr : this->react)
    {
        reactCurr->processNodeForces();
        reactCurr->processMemberForces();
    }

    //
    // Calculate combined internal forces at reactions...
    //

    for (Reaction * reactCurr : this->react)
    {
        this->dCombinedIntHorz += reactCurr->dHorzReaction;
        this->dCombinedIntVert += reactCurr->dVertReaction;
        this->dCombinedIntRot  += reactCurr->dVertReaction * reactCurr->nodeReact->adPoint[X] -
                                  reactCurr->dHorzReaction * reactCurr->nodeReact->adPoint[Y] +
                                  reactCurr->dRotReaction;
    }

    //
    // Calculate Total Reaction from Load forces...
    //

    this->dTotalHorz = fabs(this->dCombinedExtHorz + this->dCombinedIntHorz);
    this->dTotalVert = fabs(this->dCombinedExtVert + this->dCombinedIntVert);
    this->dTotalRot  = fabs(this->dCombinedExtRot  + this->dCombinedIntRot);

    return true;
}
//*
//* End of PlaneFrame::calcActions *
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::analyzeStructureForces
//*     This function analyzes the structure's forces including adherence to
//*     NDS specifications.
//*
//***************************************************************************

void PlaneFrame::analyzeStructureForces(void)
{
    //*
    //* Start member Interaction and Shear Stress Analysis...
    //***********************************************************

    //
    // Member Loop...
    //

    for (Member * memberCurr : this->members)
    {
        if ( memberCurr->ma.dForceCompWork == 0.0 && ! this->isSpecialCrossSection() )
            continue;

        //
        // Set up and clear Member calculation variables...
        //

        bool bAxialTens = false;
        bool bAxialComp = false;

        double dMaxShear = 0.0;
        double dCurrAxial, dCurrShear, dCurrBend;

        //
        // Start Member division...
        //

        double dReciprocalDivision = 1.0 / (double)(this->ps.siDivisions + 1);
        for (short int siCount = 0; siCount <= this->ps.siDivisions + 1; siCount++)
        {
            double dIntraLengthMember = memberCurr->dLength * ((double)siCount * dReciprocalDivision);
            double dIntraLengthMember2 = dIntraLengthMember * dIntraLengthMember;

            //
            // Find Maximums for Axial, Shear, and Bending stresses
            //      and Location of Maximum occurrence...
            //
            //      dCurrAxial      =   Axial Force    at current location (length)
            //      dCurrShear      =   Shear Force    at current location (length)
            //      dCurrBend       =   Bending Moment at current location (length)
            //      dMaxAxial       =   Maximum Axial Force
            //      dMaxShear       =   Maximum Shear Force
            //      dMaxBend        =   Maximum Bending Moment
            //      dMaxAxialLoc    =   Location of Maximum Axial Force
            //      dMaxShearLoc    =   Location of Maximum Shear Force
            //      dMaxBendLoc     =   Location of Maximum Bending Moment
            //

            dCurrAxial = -memberCurr->adLocalForce[0];
            dCurrShear =  memberCurr->adLocalForce[1];
            dCurrBend  =  memberCurr->adLocalForce[1] * dIntraLengthMember - memberCurr->adLocalForce[2];

            //
            // Add Trapezoidal Load contributions...
            //

            double dForceAxialTemp = 0.0;
            double dForceShearTemp = 0.0;
            double dForceBendTemp  = 0.0;
            for (LoadTrapezoidal * tlCurr : this->tl)
            {
                if (tlCurr->siMemberID == memberCurr->siID)
                {
                    if (dIntraLengthMember > tlCurr->adDist[0] && (tlCurr->adDist[0] != 0.0 || tlCurr->adDist[1] != 0.0))
                    {
                        double dLengthTrap = tlCurr->adDist[1] - tlCurr->adDist[0];
                        //double dLengthTrap2 = dLengthTrap * dLengthTrap;
                        double dDeltaX = tlCurr->adEnd[X] - tlCurr->adStart[X];
                        double dDeltaY = tlCurr->adEnd[Y] - tlCurr->adStart[Y];

                        double dIntraLengthTrap  = dIntraLengthMember - tlCurr->adDist[0];
                        double dIntraLengthTrap2 = dIntraLengthTrap * dIntraLengthTrap;
                        double dIntraLengthTrap3 = dIntraLengthTrap * dIntraLengthTrap2;
                        if (dIntraLengthMember < tlCurr->adDist[1])
                        {
                            dForceAxialTemp -= (tlCurr->adStart[X] * dDeltaX * dIntraLengthTrap3 / (dLengthTrap * 2.0));
                            dForceShearTemp += (tlCurr->adStart[Y] * dDeltaY * dIntraLengthTrap3 / (dLengthTrap * 2.0));
                            dForceBendTemp  += (dIntraLengthTrap2 * (tlCurr->adStart[Y] +
                                                        dDeltaY * dIntraLengthTrap / (dLengthTrap * 3.0)) / 2.0);
                        }
                        else
                        {
                            dForceAxialTemp -= (dLengthTrap * (tlCurr->adStart[X] + dDeltaX / 2.0));
                            dForceShearTemp += (dLengthTrap * (tlCurr->adStart[Y] + dDeltaY / 2.0));
                            dForceBendTemp  += (dLengthTrap * (tlCurr->adStart[Y]  * (dIntraLengthTrap - dLengthTrap / 2.0) +
                                                  dDeltaY / 2.0 * (dIntraLengthTrap - dLengthTrap * 2.0 / 3.0)));
                        }
                    }
                }
            }

            dCurrAxial += dForceAxialTemp;
            dCurrShear += dForceShearTemp;
            dCurrBend  += dForceBendTemp;

            //
            // Add Distributed Load contributions...
            //

            for (DistributedSystem * distCurr : memberCurr->dist)
            {
                dCurrAxial -= distCurr->adLoadvect[X] * dIntraLengthMember;
                dCurrShear += distCurr->adLoadvect[Y] * dIntraLengthMember;
                dCurrBend  += distCurr->adLoadvect[Y] / 2.0 * dIntraLengthMember2;
            }

            //
            // Add Concentrated Load contributions...
            //

            for (ConcentratedSystem * concCurr : memberCurr->conc)
            {
                if (concCurr->adLoadVect[X] != 0.0 || concCurr->adLoadVect[Y] != 0.0)
                {
                    if (dIntraLengthMember > concCurr->dLoadDist)
                    {
                        dCurrAxial -= concCurr->adLoadVect[X];
                        dCurrShear += concCurr->adLoadVect[Y];
                        dCurrBend  += concCurr->adLoadVect[Y] * (dIntraLengthMember - concCurr->dLoadDist);
                    }
                }
            }

            if (dCurrAxial > 1.0)
                bAxialTens = true;
            else if (dCurrAxial < (-1.0))
                bAxialComp = true;

            //
            // Find the last greatest Axial and Bending force and
            //      location depending on the Interaction Index...
            //

            if ( this->li.calcAtBendAndAxialMax() )
            {
                if (fabs(dCurrAxial) >= fabs(memberCurr->ma.dMaxAxial))
                {
                    memberCurr->ma.dMaxAxial = dCurrAxial;
                    memberCurr->ma.dMaxAxialLoc = dIntraLengthMember;
                }
                if (fabs(dCurrBend) >= fabs(memberCurr->ma.dMaxBend))
                {
                    memberCurr->ma.dMaxBend = dCurrBend;
                    memberCurr->ma.dMaxBendLoc = dIntraLengthMember;
                }
            }
            else if ( this->li.calcAtAxialMax() )
            {
                if ((fabs(dCurrAxial) >= fabs(memberCurr->ma.dMaxAxial)) ||
                    (fabs(dCurrAxial) == fabs(memberCurr->ma.dMaxAxial) && fabs(dCurrBend) >= fabs(memberCurr->ma.dMaxBend)))
                {
                    memberCurr->ma.dMaxAxial = dCurrAxial;
                    memberCurr->ma.dMaxBend  = dCurrBend;
                    memberCurr->ma.dMaxAxialLoc = dIntraLengthMember;
                    memberCurr->ma.dMaxBendLoc  = dIntraLengthMember;
                }
            }
            else // this->li.calcAtBendMax() || this->li.calcUsingTPI()
            {
                if ((fabs(dCurrBend) >= fabs(memberCurr->ma.dMaxBend)) ||
                    (fabs(dCurrBend) == fabs(memberCurr->ma.dMaxBend) && fabs(dCurrAxial) >= fabs(memberCurr->ma.dMaxAxial)))
                {
                    memberCurr->ma.dMaxAxial = dCurrAxial;
                    memberCurr->ma.dMaxBend  = dCurrBend;
                    memberCurr->ma.dMaxAxialLoc = dIntraLengthMember;
                    memberCurr->ma.dMaxBendLoc  = dIntraLengthMember;
                }
            }

            //
            // Find the last greatest Shear force and location...
            //

            if (fabs(dCurrShear) >= fabs(dMaxShear))
            {
                dMaxShear = dCurrShear;
                memberCurr->ma.dMaxShearLoc = dIntraLengthMember;
            }
        }

        //
        // Check for mixed tension and compression...
        //

        double dEndActionTPI = 0.0;
        double dBendLocTPI = 0.0;
        if ( ! this->isSpecialCrossSection() )
        {
            memberCurr->ma.bMixedForces = (bAxialTens && bAxialComp);
            if ( ! memberCurr->ma.bMixedForces )
            {
                //
                // Get Allowable Bending Force...
                //
                if ( this->li.isNDS1986() )
                    memberCurr->calcBending1986();
                else if ( this->li.isNDS1991() )
                    memberCurr->calcBending1991();

                //
                // Check Slenderness Ratio > 50 (CS or RB > 50)...
                //

                if (memberCurr->ma.siSlenderRatioType == 0)
                {
                    //
                    // TPI Option: Find Panel Point Interaction values for Compression members...
                    //

                    if (this->li.calcUsingTPI() && bAxialTens == 0)
                    {
                        double dForceBendAreaHeight = memberCurr->mp->dThick *
                                                      (memberCurr->mp->dHeight * memberCurr->mp->dHeight) *
                                                      memberCurr->ma.dForceBendPrime;
                        double dCommonActionNeg = fabs(memberCurr->adLocalForce[2]) * 6.0 / dForceBendAreaHeight;
                        double dCommonActionPos = fabs(memberCurr->adLocalForce[5]) * 6.0 / dForceBendAreaHeight;
                        double dForceCompArea = memberCurr->ma.dForceCompWork * memberCurr->mp->dThick * memberCurr->mp->dHeight;
                        double dEndActionNeg = 0.0;
                        double dEndActionPos = 0.0;

                        //
                        // 1986 Beam Column Equation applied at panel points.
                        //
                        if ( this->li.isNDS1986() )
                        {
                            dEndActionNeg = dCommonActionNeg + fabs(memberCurr->adLocalForce[0]) / dForceCompArea;
                            dEndActionPos = dCommonActionPos + fabs(memberCurr->adLocalForce[3]) / dForceCompArea;
                        }

                        //
                        // 1991 Beam Column Equation applied at panel points.
                        //
                        else if ( this->li.isNDS1991() )
                        {
                            dEndActionNeg = dCommonActionNeg + pow((memberCurr->adLocalForce[0] / dForceCompArea), 2.0);
                            dEndActionPos = dCommonActionPos + pow((memberCurr->adLocalForce[3] / dForceCompArea), 2.0);
                        }

                        // Set to end with largest force...
                        dEndActionTPI = dEndActionNeg;
                        dBendLocTPI = 0.0; // Neg End
                        if (dEndActionPos > dEndActionNeg)
                        {
                            dEndActionTPI = dEndActionPos;
                            dBendLocTPI = memberCurr->dLength; // Pos End
                        }

                        // Default Bending Location to mid-panel...
                        memberCurr->ma.dMaxBendLoc = memberCurr->dLength / 2.0;

                        // If present, set Bending, Axial, and Location adjustments for the distributed load...
                        double dDistBendForce = 0.0;
                        double dDistAxialForce = 0.0;
                        for (DistributedSystem * distCurr : memberCurr->dist)
                        {
                            if (distCurr->adLoadvect[Y] != 0.0)
                            {
                                dDistBendForce  += distCurr->adLoadvect[Y];
                                dDistAxialForce += distCurr->adLoadvect[X];
                            }
                        }
                        if (dDistBendForce != 0.0)
                        {
                            double dDistForceLocation = (-memberCurr->adLocalForce[1]) / dDistBendForce;
                            if (dDistForceLocation >= 0.0 && dDistForceLocation <= memberCurr->dLength)
                                memberCurr->ma.dMaxBendLoc = dDistForceLocation;
                        }

                        // Set Bending Force...
                        memberCurr->ma.dMaxBend =
                                memberCurr->adLocalForce[1] * memberCurr->ma.dMaxBendLoc +
                                dDistBendForce  / 2.0 * memberCurr->ma.dMaxBendLoc * memberCurr->ma.dMaxBendLoc -
                                memberCurr->adLocalForce[2];

                        // Set Axial Location and Force...
                        memberCurr->ma.dMaxAxialLoc = memberCurr->ma.dMaxBendLoc;
                        memberCurr->ma.dMaxAxial = (-memberCurr->adLocalForce[0]) -
                                                   dDistAxialForce * memberCurr->ma.dMaxAxialLoc;
                    }
                }
            }
        }

        if ( ! memberCurr->ma.bMixedForces && memberCurr->ma.siSlenderRatioType == 0 )
        {
            memberCurr->ma.dAxial = memberCurr->ma.dMaxAxial / memberCurr->mp->dArea;
            memberCurr->ma.dMaxShear = dMaxShear / memberCurr->mp->dArea * this->getShearFactor();
            memberCurr->ma.dBend = memberCurr->ma.dMaxBend / memberCurr->mp->dSectionModulus;

            if ( ! this->isSpecialCrossSection() )
            {
                if (memberCurr->ma.dMaxAxial > 0.0)
                {
                    memberCurr->calcTension(this->bNoteStressDifference, this->bNoteTension);
                    memberCurr->ma.dForceCompPrime = memberCurr->ma.dForceTensWork;
                }
                else
                {
                    if ( this->li.isNDS1986() )
                        memberCurr->calcCompression1986(this->ssm.getDiagonal(),
                                                        this->li.calcUsingTPI(), dEndActionTPI, dBendLocTPI,
                                                        this->bNoteBuckle, this->siPerpendicular, this->bNoteTPI,
                                                        this->bNoteCompression50);

                    else if ( this->li.isNDS1991() )
                        memberCurr->calcCompression1991(this->ssm.getDiagonal(),
                                                        this->li.calcUsingTPI(), dEndActionTPI, dBendLocTPI,
                                                        this->bNoteBuckle, this->siPerpendicular, this->bNoteTPI,
                                                        this->bNoteCompression75, this->bNoteCompression50,
                                                        this->bNoteEuler);
                }
            }
        }
    }
}
//*
//* End of PlaneFrame::analyzeStructureForces
//***************************************************************************

//***************************************************************************
//*
//* PlaneFrame::calcDeflections
//*     This function calculates the displacements for the structure.
//*
//***************************************************************************

bool PlaneFrame::calcDeflections()
{
    for (Member * memberCurr : this->members)
    {
        // Member Deflection Calculation...
        memberCurr->calcDeflection( this->isSpecialCrossSection(),
                                    this->adDisplaceMatrix,
                                    this->tl, this->ps.siDivisions );
    }

    return true;
}
//*
//* End of PlaneFrame::calcDeflections
//***************************************************************************
