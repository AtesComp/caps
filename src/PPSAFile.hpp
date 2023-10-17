/*
 * PPSAFile.hpp
 *
 *  Created on: Aug 13, 2020
 *      Author: Keven L. Ates
 */

#ifndef PPSAFILE_HPP_
#define PPSAFILE_HPP_

#include <istream>
#include <fstream>

#include "PlaneFrame.hpp"

#define PPSA_STANDARD 'S'
#define PPSA_FREE 'F'

// PPSA File...

class PPSAFile
{
public:
    static const std::string strEOF;
    static const std::string strSpace;  // Setup string compare for spaces

    std::string strID;                  // Input Identification Title
    std::string strOpt;                 // Detailed Option from Title

    PPSAFile(void);
    PPSAFile(std::string &);
    bool checkPPSA(void);
    bool isStandard(void);
    void setFree(void);
    std::string getFormat(void);

    bool getHeader(PlaneFrame &);
    bool getStructure(PlaneFrame &);
    bool getLoadCase(PlaneFrame &);
    bool isWarningForTPIConcLoad(void);
    bool isWarningForTPITrapLoad(void);

    void close(void);

private:
    std::ifstream * pifsInFile = nullptr;
    std::istream & isInFile;
    std::string strInFile;

    char cFormatType = PPSA_STANDARD;   // File type flag for input file format
                                        // S = Standard (positional)
                                        // F = Free (space delimited)
    bool bWarnTPIConcLoad = false;
    bool bWarnTPITrapLoad = false;

    bool getStructureProblemSize(ProblemSize &);
    bool getStructureMaterialProperties(std::vector<MaterialProperty *> &, bool, bool);
    bool getStructureStressAdjustmentFactors(StressFactors &);
    bool getStructureNodes(std::vector<Node *> &);
    bool getStructureMembers(std::vector<Member *> &, bool);
    bool getStructureReactions(std::vector<Reaction *> &);

    bool getLoadCaseLoadArrangementAndInteractionInterpretation(LoadAndInteraction &);
    bool getLoadCaseConcentratedLoads(std::vector<LoadPoint *> &, bool);
    bool getLoadCaseUniformLoads(std::vector<LoadUniform *> &);
    bool getLoadCaseNodalLoads(std::vector<LoadNodal *> &);
    bool getLoadCaseTrapezoidalLoads(std::vector<LoadTrapezoidal *> &, bool);
};

#endif /* PPSAFILE_HPP_ */
