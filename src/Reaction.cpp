/*
 * Reaction.cpp
 *
 *  Created on: Aug 26, 2020
 *      Author: Keven L. Ates
 */

#include "Reaction.hpp"
#include "ReportFormat.hpp"

#include "ReportFormat.hpp"
#include "SystemDef.hpp"

#include <iostream>
#include <fmt/format.h>
#include <algorithm>

const char * const Reaction::strType[4] = { "FIX", "PIN", "ROLL", "FIRL" };
const short int Reaction::siFIX  = 0;
const short int Reaction::siPIN  = 1;
const short int Reaction::siROLL = 2;
const short int Reaction::siFIRL = 3;

Reaction::Reaction(void)
{
    this->clear();
}

Reaction::~Reaction(void)
{
    this->clear();
}

void Reaction::clear(void)
{
    this->siNodeID = 0;
    this->siType = Reaction::siPIN;

    this->dVector[0] = 0.0;
    this->dVector[1] = 0.0;

    this->nodeReact = nullptr;
    this->vectMembersNeg.clear();
    this->vectMembersPos.clear();

    this->dHorzReaction = 0.0;
    this->dVertReaction = 0.0;
    this->dRotReaction = 0.0;
}

std::string Reaction::report()
{
    //
    // Table 4 body...
    //
    std::string strFormat;
    strFormat = strFormat + "   {:4d}     {:4}   " + SF13_4f + " " + SF13_4f + "\n";
    return fmt::vformat(
                strFormat,
                fmt::make_format_args(
                    this->siNodeID,
                    this->typeToString(),
                    this->dVector[0], this->dVector[1]
                )
            );
}

std::string Reaction::typeToString(void)
{
    return Reaction::strType[this->siType];
}

bool Reaction::linkNodes(std::vector<Node *> & nodes)
{
    Node * nodeFound = nullptr;
    for (Node * nodeCurr : nodes)
    {
        if (this->siNodeID == nodeCurr->siID && nodeCurr->siSequence != 0)
        {
            nodeFound = nodeCurr;
            break;
        }
    }
    if (nodeFound == nullptr)
    {
        std::cerr << "\n ERROR: Reaction Node " << this->siNodeID << std::endl
                  <<   "        not found in Nodes!\n";
        return false;
    }

    // Set Reaction Node...
    this->nodeReact = nodeFound;

    return true;
}

bool Reaction::linkMembers(std::vector<Member *> & members)
{
    for (Member * memberCurr : members)
    {
        if (memberCurr->siNegNodeID == this->siNodeID)
        {
            this->vectMembersNeg.push_back(memberCurr);
        }
        if (memberCurr->siPosNodeID == this->siNodeID)
        {
            this->vectMembersPos.push_back(memberCurr);
        }
    }

    if (this->vectMembersNeg.empty() && this->vectMembersPos.empty())
    {
        std::cerr << "\n ERROR: Reaction Node " << this->siNodeID << std::endl
                  <<   "        not found in Member End Nodes!\n";
        return false;
    }

    return true;
}

void Reaction::processNodeForces(void)
{
    if ( this->nodeReact->nodeload != nullptr )
    {
        if (this->nodeReact->nodeload->siLoadDirection == 1)         // X Direction
            this->dHorzReaction -= this->nodeReact->nodeload->dLoad;
        else if (this->nodeReact->nodeload->siLoadDirection == 2)    // Y Direction
            this->dVertReaction -= this->nodeReact->nodeload->dLoad;
        else if (this->nodeReact->nodeload->siLoadDirection == 3)    // Moment
            this->dRotReaction  -= this->nodeReact->nodeload->dLoad;
    }
}

void Reaction::processMemberForces(void)
{
    for (Member * memberCurr : this->vectMembersNeg)
    {
        this->dHorzReaction += (memberCurr->dCos * memberCurr->adLocalForce[0] - memberCurr->dSin * memberCurr->adLocalForce[1]);
        this->dVertReaction += (memberCurr->dSin * memberCurr->adLocalForce[0] + memberCurr->dCos * memberCurr->adLocalForce[1]);
        this->dRotReaction  += memberCurr->adLocalForce[2];
    }

    for (Member * memberCurr : this->vectMembersPos)
    {
        this->dHorzReaction += (memberCurr->dCos * memberCurr->adLocalForce[3] - memberCurr->dSin * memberCurr->adLocalForce[4]);
        this->dVertReaction += (memberCurr->dSin * memberCurr->adLocalForce[3] + memberCurr->dCos * memberCurr->adLocalForce[4]);
        this->dRotReaction  += memberCurr->adLocalForce[5];
    }
}

void Reaction::processAction(ReactionCounts & reactionCounts)
{
    //
    // Set the Coordinate Flags for the Reaction Node in Node List...
    //      Also set any Reaction Vector changes for ROLL reactions...
    //
    // 0 = "FIX" : FIX RGD: 3 parts are held ( Horz, Vert, and Rot  )
    // 1 = "PIN" : FIX PIN: 2 parts are held ( Horz and Vert        )
    // 2 = "ROLL": ROL PIN: 1 part is held   ( Horz or Vert         )
    // 3 = "FIRL": ROL RGD: 2 parts are held ( Horz or Vert, and Rot)
    //

    if (this->siType == Reaction::siFIX) // FIX
    {
        reactionCounts.siFixRgd++;
        this->nodeReact->siHorzFlag = true; // FIXED
        this->nodeReact->siVertFlag = true; // FIXED
        this->nodeReact->siRotFlag = true; // FIXED
        // NOTE: DO NOT set a Node's Member Fixture to FIXED here
        //       (this->nodeReact->bMemberFixture = false) as it only
        //       applies to Member processing.  The Reaction's Fixture
        //       has already been considered.
    }
    else if (this->siType == Reaction::siPIN) // PIN
    {
        reactionCounts.siFixPin++;
        this->nodeReact->siHorzFlag = true; // FIXED
        this->nodeReact->siVertFlag = true; // FIXED
    }
    else // Roll Types...
    {
        if (this->siType == Reaction::siFIRL) // FIRL
        {
            reactionCounts.siRolRgd++;
            this->nodeReact->siRotFlag = true; // FIXED
            // NOTE: DO NOT set a Node's Member Fixture to FIXED here
            //       (this->nodeReact->bMemberFixture = false) as it only
            //       applies to Member processing.  The Reaction's Fixture
            //       has already been considered.
        }
        else // (this->type == Reaction::siROLL) ROLL
            reactionCounts.siRolPin++;

        // FIRLs and ROLLs can roll at an angle, so check for Vector Movement...
        if (this->dVector[X] == 0.0) // ...X held, only Y movement
            this->nodeReact->siHorzFlag = true; // FIXED
        else
        {   // Y Held OR Both FREE
            this->nodeReact->siVertFlag = true; // FIXED, Y held...maybe
            if (this->dVector[Y] != 0.0) // Both FREE: Vector Movement
            {   // VertFlag == TRUE && X & Y Vector != 0.0
                // Setup Vector info...
                double hyp = sqrt(this->dVector[X] * this->dVector[X] +
                                  this->dVector[Y] * this->dVector[Y]);
                double vect_cos = this->dVector[X] / hyp;
                double vect_sin = this->dVector[Y] / hyp;

                //
                // Recalculate the Reaction's Nodes' Roll Vector...
                //   Since only one Reaction can be found at a Node, we are only updating one end of
                //   each Member found at the Reaction Node.  If there were another reaction, it
                //   would overwrite the previous Roll Vector.
                //   To add vectors, the previous values need to be used in the calculation as
                //   shown in the comments.
                //

                // ...of a related Member's Negative Node...
                for (Member * memberCurr : this->vectMembersNeg)
                {
                    memberCurr->adNodeVectorNeg[X] = memberCurr->dCos * vect_cos + memberCurr->dSin * vect_sin;
                    memberCurr->adNodeVectorNeg[Y] = memberCurr->dSin * vect_cos - memberCurr->dCos * vect_sin;
                    // Alternative Additive:
                    //double vectX = memberCurr->adNodeVectorNeg[X];
                    //double vectY = memberCurr->adNodeVectorNeg[Y];
                    //memberCurr->adNodeVectorNeg[X] = vectX * vect_cos + vectY * vect_sin;
                    //memberCurr->adNodeVectorNeg[Y] = vectY * vect_cos - vectX * vect_sin;
                }

                // ...of a related Member's Positive Node...
                for (Member * memberCurr : this->vectMembersPos)
                {
                    memberCurr->adNodeVectorPos[X] = memberCurr->dCos * vect_cos + memberCurr->dSin * vect_sin;
                    memberCurr->adNodeVectorPos[Y] = memberCurr->dSin * vect_cos - memberCurr->dCos * vect_sin;
                    // Alternative Additive:
                    //double vectX = memberCurr->adNodeVectorPos[X];
                    //double vectY = memberCurr->adNodeVectorPos[Y];
                    //memberCurr->adNodeVectorPos[X] = vectX * vect_cos + vectY * vect_sin;
                    //memberCurr->adNodeVectorPos[Y] = vectY * vect_cos - vectX * vect_sin;
                }
            }
        }
    }
}

