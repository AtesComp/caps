/*
 * Reaction.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef REACTION_HPP_
#define REACTION_HPP_

#include "Node.hpp"
#include "Member.hpp"

#include <string>
#include <vector>

struct ReactionCounts
{
    short int siFixRgd = 0;
    short int siFixPin = 0;
    short int siRolRgd = 0;
    short int siRolPin = 0;
};

// Reaction (bearing) Definition...

class Reaction
{
public:
    // Class Constants...
    static const char * const strType[4];   // Available Reaction Types
    static const short int siFIX;           // FIX  Reaction Type
    static const short int siPIN;           // PIN  Reaction Type
    static const short int siROLL;          // ROLL Reaction Type
    static const short int siFIRL;          // FIRL Reaction Type

    short int siNodeID;             // Reaction's Node Number

    short int siType;               // Reaction Type
    								// 0 = "FIX" : FIX RGD: 3 parts are held ( Horz, Vert, and Rot  )
    								// 1 = "PIN" : FIX PIN: 2 parts are held ( Horz and Vert        )
    								// 2 = "ROLL": ROL PIN: 1 part is held   ( Horz or Vert         )
    								// 3 = "FIRL": ROL RGD: 2 parts are held ( Horz or Vert, and Rot)
    double dVector[2];              // Reaction Vector for Roll types: (X, Y)

    Node * nodeReact;               // Pointer to Node for reaction

    std::vector<Member *> vectMembersNeg;   // Vector of Members who's negative end is the reaction
    std::vector<Member *> vectMembersPos;   // Vector of Members who's positive end is the reaction

    double dHorzReaction;           // Reaction's Horizontal Energy
    double dVertReaction;           // Reaction's Vertical Energy
    double dRotReaction;            // Reaction's Rotational Energy

    Reaction(void);
    ~Reaction(void);
    void clear(void);

    std::string report();
    std::string typeToString(void);

    bool linkNodes(std::vector<Node *> &);
    bool linkMembers(std::vector<Member *> &);
    void processNodeForces(void);
    void processMemberForces(void);
    void processAction(ReactionCounts &);
};

#endif /* REACTION_HPP_ */
