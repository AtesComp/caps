/*
 * PPSAFile.cpp
 *
 *  Created on: Aug 25, 2020
 *      Author: Keven L. Ates
 */

#include "PPSAFile.hpp"

#include "SystemDef.hpp"

#include <iostream>
#include <fmt/format.h>
#include <iomanip>

const std::string PPSAFile::strEOF = "\n End of file (no marker found)!\n";
const std::string PPSAFile::strSpace = std::string().append(128, ' ');

PPSAFile::PPSAFile(void) : pifsInFile( nullptr ), isInFile( std::cin )
{
    this->strInFile = "STDIN";
}

PPSAFile::PPSAFile(std::string & strIn) : pifsInFile( new std::ifstream(strIn) ), isInFile( *pifsInFile )
{
    this->strInFile = strIn;
}

bool PPSAFile::checkPPSA(void)
{
    if ( ! isInFile.good() )
    {
        std::cerr << "\n ERROR: Could not open " << this->strInFile << " for input!" << std::endl;
        return false;
    }

    return true;
}

void PPSAFile::close(void)
{
    if ( this->pifsInFile != nullptr )
    {
        this->pifsInFile->close();
        delete this->pifsInFile;
        this->pifsInFile = nullptr;
    }
}

bool PPSAFile::isStandard(void)
{
    return (this->cFormatType == PPSA_STANDARD);
}

void PPSAFile::setFree(void)
{
    this->cFormatType = PPSA_FREE;
}

std::string PPSAFile::getFormat(void)
{
    return (this->isStandard() ? "STANDARD" : "FREE");
}

//**************************************************************************
//*
//* PPSAFile::getHeader
//*     This function inputs the data file's Identification
//*     lines (section 1) and processes the lines for special instructions.
//*
//**************************************************************************

bool PPSAFile::getHeader(PlaneFrame & pframe)
{
    //*
    //* Read file identification and problem size (section 1)...
    //*
    //***********************************************************************

    this->isInFile.peek();
    if ( this->isInFile.eof() )
    {
        std::cout << PPSAFile::strEOF;
        return false;
    }

    std::string strRead;
    strRead.append(256, 0); // ...really clear
    strRead.clear();

    std::getline( this->isInFile, strRead );
    this->strID = strRead; // ...store original input for later reporting (for STANDARD input)
    std::transform(strRead.begin(), strRead.end(), strRead.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });   // ...must UPPER before check and
                                                                                        //    for later processing...
    if (strRead.find("FREE") != std::string::npos) // ...check for "FREE"
    {
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        this->setFree();
        std::getline( this->isInFile, strRead );
        this->strID = strRead; // ...store original input for later reporting (for FREE input)
        std::transform(strRead.begin(), strRead.end(), strRead.begin(),
                       [](unsigned char c) -> unsigned char { return std::toupper(c); });  // ...must UPPER (as before)
                                                                                           //    for later processing
    }

    // Check for End of Input File...
    const std::string strENDDCALC = "ENDDCALC";
    if (strRead.find(strENDDCALC) != std::string::npos)
    {
        std::cout << "\n End Calculation (" << strENDDCALC << ") marker found!\n";
        return false;
    }
    if ( this->isInFile.eof() )
    {
        std::cout << PPSAFile::strEOF;
        return false;
    }

    this->strOpt = "";

    // Detect if Special Cross Section option is chosen...
    short int bOptionS = false;     // Option flag 'S'
    if (strRead.find("OPT-S") != std::string::npos)
    {
        bOptionS = true;
        this->strOpt = "Special Cross Section Analysis";
    }

    // Detect if Version 3 data file input option is chosen...
    short int bOption3 = false;     // Option flag '3'
    if (strRead.find("OPT-3") != std::string::npos)
    {
        bOption3 = true;
        this->strOpt = "PPSA Version 3 Input Analysis";
    }

    // Detect if Composite GLULAM, LVL, PSL option is chosen...
    short int bOptionC = false;     // Option flag 'C'
    if (strRead.find("OPT-C") != std::string::npos)
    {
        bOptionC = true;
        this->strOpt = "Composite (GLULAM, LVL, PSL) Analysis";
    }

    //
    // Check selected options for illegal combinations...
    //

    if (bOptionS == true && bOption3 == true)
    {
        std::cerr << "\n ERROR: OPT-S and OPT-3 can not be selected simultaneously!\n";
        return false;
    }
    if (bOptionS == true && bOptionC == true)
    {
        std::cerr << "\n ERROR: OPT-S and OPT-C can not be selected simultaneously!\n";
        return false;
    }
    if (bOptionC == true && bOption3 == true)
    {
        std::cerr << "\n ERROR: OPT-C and OPT-3 can not be selected simultaneously!\n";
        return false;
    }

    //
    // Set structure option...
    //

    pframe.clearOption();
    if (bOptionS == true)
        pframe.setOptionSpecialCrossSection();
    else if (bOption3 == true)
        pframe.setOptionVersion3();
    else if (bOptionC == true)
        pframe.setOptionComposite();

    return true;
}

//*
//* End of PPSAFile::getHeader
//**************************************************************************

//**************************************************************************
//*
//* PPSAFile::getStructure
//*     This function inputs the data file's structural
//*     information lines (section 2 - 7) and processes relevant
//*     information related to the structure.
//*
//**************************************************************************

bool PPSAFile::getStructure(PlaneFrame & pframe)
{
    // Read in the Problem Size (section 2)...
    if ( ! this->getStructureProblemSize(pframe.ps) )
        return false;

    // Read in the Member Properties Data (section 3)...
    if ( ! this->getStructureMaterialProperties( pframe.mprops,
                                                 pframe.isSpecialCrossSection(),
                                                 pframe.isComposite() ) )
        return false;

    // Read in the Stress Adjustment Factors (section 4)...
    if ( ! this->getStructureStressAdjustmentFactors(pframe.sf) )
        return false;

    // Read in the Node Coordinates (section 5).....
    if ( ! this->getStructureNodes(pframe.nodes) )
        return false;

    // Read in the Structural Assembly Members (section 6).....
    if ( ! this->getStructureMembers( pframe.members, pframe.isComposite() ) )
        return false;

    // Read in the Reactions (Bearings) (section 7).....
    if ( ! this->getStructureReactions(pframe.react) )
        return false;

    return true;
}

//*
//* End of PPSAFile::getStructure
//*************************************************************************

bool PPSAFile::getStructureProblemSize(ProblemSize & ps)
{
    //*
    //* Read in the Problem Size (section 2)...
    //*
    //***********************************************************************

    std::string strProcess;
    strProcess.append(60, 0); // ...really clear
    strProcess.clear();

    std::getline( this->isInFile, strProcess );
    if ( this->isInFile.eof() )
    {
        std::cout << PPSAFile::strEOF;
        return false;
    }
    std::stringstream strRead;
    strRead.str(strProcess);

    //
    // Convert Standard format to Free format (partial)...
    //

    if ( this->isStandard() )
    {

        short int siIndex = strProcess.size();
        if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[ 1] = '0';
        if (siIndex >  4 && strProcess.compare( 2, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[ 3] = '0';
        if (siIndex >  6 && strProcess.compare( 4, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[ 5] = '0';
        if (siIndex >  8 && strProcess.compare( 6, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[ 7] = '0';
        if (siIndex > 10 && strProcess.compare( 8, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[ 9] = '0';
        if (siIndex > 12 && strProcess.compare(10, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[11] = '0';
        if (siIndex > 13 && strProcess.compare(12, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[12] = '0';
        if (siIndex > 14 && strProcess.compare(13, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[13] = '0';
        if (siIndex > 16 && strProcess.compare(14, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[15] = '0';
        if (siIndex > 18 && strProcess.compare(16, 2, PPSAFile::strSpace, 0, 2) == 0)
            strProcess[17] = '0';

        strRead.clear();
        strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                << fmt::format("{:.2s}", &strProcess[ 2]) << " "
                << fmt::format("{:.2s}", &strProcess[ 4]) << " "
                << fmt::format("{:.2s}", &strProcess[ 6]) << " "
                << fmt::format("{:.2s}", &strProcess[ 8]) << " "
                << fmt::format("{:.2s}", &strProcess[10]) << " "
                << fmt::format("{:.1s}", &strProcess[12]) << " "
                << fmt::format("{:.1s}", &strProcess[13]) << " "
                << fmt::format("{:.2s}", &strProcess[14]) << " "
                << fmt::format("{:.2s}", &strProcess[16]);
    }

    //
    // Read FREE format...
    //

    short int siNoPrintIn = 0;
    ProblemSize psCurr;
    strRead >> psCurr.uiNodes >> psCurr.uiMembers >> psCurr.siRolPin >> psCurr.siFixPin >> psCurr.siRolRgd
            >> psCurr.siFixRgd >> siNoPrintIn >> psCurr.siPrintOut >> psCurr.siLoads
            >> psCurr.siDivisions;

    // Adjust problem size values...

    if (psCurr.siLoads < 1)
        psCurr.siLoads = 1;
    if (psCurr.siDivisions == 0)
        psCurr.siDivisions = 24;
    psCurr.bNoPrintIn = (siNoPrintIn != 0);

    // Store Problem Size to Plane Frame System Definition...
    ps = psCurr;

    return true;
}

bool PPSAFile::getStructureMaterialProperties(std::vector<MaterialProperty *> & mps,
                                              bool bSpecialCrossSection, bool bComposite)
{
    //*
    //* Read in the Material Property Data (per option) (section 3)...
    //*
    //***********************************************************************

    short int siEOS = false;    // End of Section marker
    std::string strProcess;
    std::stringstream strRead;
    MaterialProperty * mpCurr = nullptr;
    do
    {
        //
        // Allocate memory for Material Property Groups...
        //

        if ( (mpCurr = new MaterialProperty) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      <<   "        Material Property!\n";
            return false;
        }

        //
        // Process line from file for Material Property Groups...
        //

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Process per Cross-Section Option (OPT-S)...
        //
        if ( bSpecialCrossSection )
        {
            //
            // Convert Standard format to Free format...
            //

            if ( this->isStandard() )
            {
                short int siIndex = strProcess.size();
                if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                    strProcess[ 1] = '0';
                if (siIndex >  9 && strProcess.compare( 2, 7, PPSAFile::strSpace, 0, 7) == 0)
                    strProcess[ 8] = '0';
                if (siIndex > 16 && strProcess.compare( 9, 7, PPSAFile::strSpace, 0, 7) == 0)
                    strProcess[15] = '0';
                if (siIndex > 23 && strProcess.compare(16, 7, PPSAFile::strSpace, 0, 7) == 0)
                    strProcess[22] = '0';
                if (siIndex > 29 && strProcess.compare(23, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[28] = '0';
                if (siIndex > 35 && strProcess.compare(29, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[34] = '0';
                if (siIndex > 41 && strProcess.compare(35, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[40] = '0';
                if (siIndex > 42 && strProcess.compare(41, 1, PPSAFile::strSpace, 0, 1) == 0)
                    strProcess[41] = '0';

                strRead.clear();
                strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                        << fmt::format("{:.7s}", &strProcess[ 2]) << " "
                        << fmt::format("{:.7s}", &strProcess[ 9]) << " "
                        << fmt::format("{:.7s}", &strProcess[16]) << " "
                        << fmt::format("{:.6s}", &strProcess[23]) << " "
                        << fmt::format("{:.6s}", &strProcess[29]) << " "
                        << fmt::format("{:.6s}", &strProcess[35]) << " "
                        << fmt::format("{:.1s}", &strProcess[41]);
            }

            //
            // FREE format...
            //

            strRead >> mpCurr->siID >> mpCurr->dArea >> mpCurr->dMoI >> mpCurr->dSectionModulus
                    >> mpCurr->dMoE_IP >> mpCurr->dMoE_Axial >> mpCurr->dShearModulus >> siEOS;
        }

        //
        // Process per Composites Option (GluLam, LVL, PSL) (OPT-C)...
        //      Composite products such as GluLam / LVL / PSL
        //
        else if ( bComposite )
        {
            //
            // Convert Standard format to Free format...
            //

            if ( this->isStandard() )
            {
                short int siIndex = strProcess.size();
                if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                    strProcess[ 1] = '0';
                if (siIndex >  7 && strProcess.compare( 2, 5, PPSAFile::strSpace, 0, 5) == 0)
                    strProcess[ 6] = '0';
                if (siIndex > 12 && strProcess.compare( 7, 5, PPSAFile::strSpace, 0, 5) == 0)
                    strProcess[11] = '0';
                if (siIndex > 18 && strProcess.compare(12, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[17] = '0';
                if (siIndex > 24 && strProcess.compare(18, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[23] = '0';
                if (siIndex > 30 && strProcess.compare(24, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[29] = '0';
                if (siIndex > 34 && strProcess.compare(30, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[33] = '0';
                if (siIndex > 38 && strProcess.compare(34, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[37] = '0';
                if (siIndex > 42 && strProcess.compare(38, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[41] = '0';
                if (siIndex > 46 && strProcess.compare(42, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[45] = '0';
                if (siIndex > 47 && strProcess.compare(46, 1, PPSAFile::strSpace, 0, 1) == 0)
                    strProcess[46] = '0';
                if (siIndex > 48 && strProcess.compare(47, 1, PPSAFile::strSpace, 0, 1) == 0)
                    strProcess[47] = '0';

                strRead.clear();
                strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                        << fmt::format("{:.5s}", &strProcess[ 2]) << " "
                        << fmt::format("{:.5s}", &strProcess[ 7]) << " "
                        << fmt::format("{:.6s}", &strProcess[12]) << " "
                        << fmt::format("{:.6s}", &strProcess[18]) << " "
                        << fmt::format("{:.6s}", &strProcess[24]) << " "
                        << fmt::format("{:.4s}", &strProcess[30]) << " "
                        << fmt::format("{:.4s}", &strProcess[34]) << " "
                        << fmt::format("{:.4s}", &strProcess[38]) << " "
                        << fmt::format("{:.4s}", &strProcess[42]) << " "
                        << fmt::format("{:.1s}", &strProcess[46]) << " "
                        << fmt::format("{:.1s}", &strProcess[47]);
            }

            //
            // FREE format...
            //

            char cProperty = 0; // Member Type property indicator
            strRead >> mpCurr->siID >> mpCurr->dThick >> mpCurr->dHeight
                    >> mpCurr->dMoE_IP >> mpCurr->dMoE_PP >> mpCurr->dMoE_Axial
                    >> mpCurr->dShearModulus >> mpCurr->dForceBend >> mpCurr->dForceComp
                    >> mpCurr->dForceTens >> cProperty >> siEOS;
            mpCurr->cProperty = toupper( cProperty );
        }

        //
        // Process per Normal input, no option selected...
        //
        else
        {
            //
            // Convert Standard format to Free format...
            //

            if ( this->isStandard() )
            {
                short int siIndex = strProcess.size();
                if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                    strProcess[ 1] = '0';
                if (siIndex >  7 && strProcess.compare( 2, 5, PPSAFile::strSpace, 0, 5) == 0)
                    strProcess[ 6] = '0';
                if (siIndex > 12 && strProcess.compare( 7, 5, PPSAFile::strSpace, 0, 5) == 0)
                    strProcess[11] = '0';
                if (siIndex > 18 && strProcess.compare(12, 6, PPSAFile::strSpace, 0, 6) == 0)
                    strProcess[17] = '0';
                if (siIndex > 22 && strProcess.compare(18, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[21] = '0';
                if (siIndex > 26 && strProcess.compare(22, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[25] = '0';
                if (siIndex > 30 && strProcess.compare(26, 4, PPSAFile::strSpace, 0, 4) == 0)
                    strProcess[29] = '0';
                if (siIndex > 31 && strProcess.compare(30, 1, PPSAFile::strSpace, 0, 1) == 0)
                    strProcess[30] = '0';
                if (siIndex > 32 && strProcess.compare(31, 1, PPSAFile::strSpace, 0, 1) == 0)
                    strProcess[31] = '0';

                strRead.clear();
                strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                        << fmt::format("{:.5s}", &strProcess[ 2]) << " "
                        << fmt::format("{:.5s}", &strProcess[ 7]) << " "
                        << fmt::format("{:.6s}", &strProcess[12]) << " "
                        << fmt::format("{:.4s}", &strProcess[18]) << " "
                        << fmt::format("{:.4s}", &strProcess[22]) << " "
                        << fmt::format("{:.4s}", &strProcess[26]) << " "
                        << fmt::format("{:.1s}", &strProcess[30]) << " "
                        << fmt::format("{:.1s}", &strProcess[31]);
            }

            //
            // FREE format...
            //

            char cProperty = 0; // Member Type property indicator
            strRead >> mpCurr->siID >> mpCurr->dThick >> mpCurr->dHeight
                    >> mpCurr->dMoE_Axial >> mpCurr->dForceBend >> mpCurr->dForceComp >> mpCurr->dForceTens
                    >> cProperty >> siEOS;
            mpCurr->cProperty = toupper( cProperty );
        }

        // Common calculations for Material Properties...
        mpCurr->deriveCommonValues( bSpecialCrossSection, bComposite );

        //
        // Store Member Property...
        //
        mps.push_back(mpCurr);

        // Get another member property if not marked as last...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::getStructureStressAdjustmentFactors(StressFactors & sf)
{
    //*
    //* Read in the Stress Adjustment Factors (section 4)...
    //*
    //***********************************************************************

    std::string strProcess;
    strProcess.append(60, 0); // ...really clear
    strProcess.clear();

    std::getline( this->isInFile, strProcess );
    if ( this->isInFile.eof() )
    {
        std::cout << PPSAFile::strEOF;
        return false;
    }
    std::stringstream strRead;
    strRead.str(strProcess); // ...store original input for later output if FREE

    //
    // Convert Standard format to Free format...
    //

    if ( this->isStandard() )
    {
        short int siIndex = strProcess.size();
        if (siIndex >  5 && strProcess.compare( 0, 5, PPSAFile::strSpace, 0, 5) == 0)
            strProcess[ 4] = '0';
        if (siIndex > 10 && strProcess.compare( 5, 5, PPSAFile::strSpace, 0, 5) == 0)
            strProcess[ 9] = '0';
        if (siIndex > 15 && strProcess.compare(10, 5, PPSAFile::strSpace, 0, 5) == 0)
            strProcess[14] = '0';

        strRead.clear();
        strRead << fmt::format("{:.5s}", &strProcess[ 0]) << " "
                << fmt::format("{:.5s}", &strProcess[ 5]) << " "
                << fmt::format("{:.5s}", &strProcess[10]);
    }

    //
    // FREE format...
    //

    // DIRECTLY store Stress Adjustment Factors to Plane Frame System Definition...
    strRead >> sf.factor[0] >> sf.factor[1] >> sf.factor[2];

    return true;
}

bool PPSAFile::getStructureNodes(std::vector<Node *> & nodes)
{
    //*
    //* Read in the Node Coordinates (section 5).....
    //*
    //***********************************************************************

    short int siEOS = false;    // End of Section marker
    std::string strProcess;
    std::stringstream strRead;
    Node * nodeCurr = nullptr;
    do
    {
        //
        // Allocate memory for Node Coordinates...
        //

        if ( ( nodeCurr = new Node ) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      <<   "        Node Coordinate!\n";
            return false;
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int siIndex = strProcess.size();
            if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 1] = '0';
            if (siIndex >  9 && strProcess.compare( 2, 7, PPSAFile::strSpace, 0, 7) == 0)
                strProcess[ 8] = '0';
            if (siIndex > 16 && strProcess.compare( 9, 7, PPSAFile::strSpace, 0, 7) == 0)
                strProcess[15] = '0';
            if (siIndex > 17 && strProcess.compare(16, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[16] = '0';

            strRead.clear();
            strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.7s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.7s}", &strProcess[ 9]) << " "
                    << fmt::format("{:.1s}", &strProcess[16]);
        }

        //
        // FREE format...
        //

        strRead >> nodeCurr->siID >> nodeCurr->adPoint[X] >> nodeCurr->adPoint[Y] >> siEOS;

        //
        // Store Node...
        //
        nodes.push_back(nodeCurr);

        // Get another node coordinate if not marked as last...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::getStructureMembers(std::vector<Member *> & members, bool bComposite)
{
    //*
    //* Read in the Structural Assembly Members (section 6).....
    //*
    //***********************************************************************

    short int siEOS = false;    // End of Section marker
    std::string strProcess;
    std::stringstream strRead;
    do
    {
        //
        // Allocate memory for a Structural Assembly member...
        //

        Member * memberCurr = nullptr;
        if ( ( memberCurr = new Member ) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      <<   "        Structural Assembly!\n";
            return false;
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int siIndex = strProcess.size();
            if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 1] = '0';
            if (siIndex >  4 && strProcess.compare( 2, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 3] = '0';
            if (siIndex >  6 && strProcess.compare( 4, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 5] = '0';
            if (siIndex >  8 && strProcess.compare( 6, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 7] = '0';
            if (siIndex >  9 && strProcess.compare( 8, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[ 8] = '0';
            if (siIndex > 10 && strProcess.compare( 9, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[ 9] = '0';
            if (siIndex > 11 && strProcess.compare(10, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[10] = '0';
            if (siIndex > 17 && strProcess.compare(11, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[16] = '0';
            if (siIndex > 23 && strProcess.compare(17, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[22] = '0';
            if (siIndex > 29 && strProcess.compare(23, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[28] = '0';
            if (siIndex > 35 && strProcess.compare(29, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[34] = '0';

            strRead.clear();
            strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.2s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.2s}", &strProcess[ 4]) << " "
                    << fmt::format("{:.2s}", &strProcess[ 6]) << " "
                    << fmt::format("{:.1s}", &strProcess[ 8]) << " "
                    << fmt::format("{:.1s}", &strProcess[ 9]) << " "
                    << fmt::format("{:.1s}", &strProcess[10]) << " "
                    << fmt::format("{:.6s}", &strProcess[11]) << " "
                    << fmt::format("{:.6s}", &strProcess[17]) << " "
                    << fmt::format("{:.6s}", &strProcess[23]) << " "
                    << fmt::format("{:.6s}", &strProcess[29]);
       }

        //
        // FREE format [check for volume_factor variable]...
        //

        strRead >> memberCurr->siID >> memberCurr->siNegNodeID
                >> memberCurr->siPosNodeID >> memberCurr->siMatPropID
                >> memberCurr->bJointNeg >> memberCurr->bJointPos >> siEOS
                >> memberCurr->dEffColumnLength_IP >> memberCurr->dEffColumnLength_PP
                >> memberCurr->dEffBendingLength >> memberCurr->dVolumeFactor;

        //
        // Check Option 'C' input.....
        //

        if ( bComposite )
        {
            if (memberCurr->dVolumeFactor <= 0.0 || memberCurr->dVolumeFactor > 1.0)
            {
                std::cerr << "\n ERROR: Volume Factor must be GREATER THAN 0.0 and LESS THAN OR EQUAL TO 1.0!\n"
                          <<   "        Error detected on Member Number " << memberCurr->siID << std::endl
                          <<   "        CAPS program aborted...\n";
                return false;
            }
        }


        // Common calculations for Members...
        memberCurr->deriveCommonValues();

        // Store Structural Assembly Members to Plane Frame System Definition...
        members.push_back(memberCurr);

        // Get another Structural Assembly member if not marked as last...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::getStructureReactions(std::vector<Reaction *> & react)
{
    //*
    //* Read in the Reactions (Bearings) (section 7).....
    //*
    //***********************************************************************

    short int siEOS = false;        // ...End of Section marker
    std::string strProcess;
    std::stringstream strRead;
    Reaction * reactCurr = nullptr;
    do
    {
        //
        // Allocate memory for a Reaction...
        //

        if ( (reactCurr = new Reaction ) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      <<   "        Reaction!\n";
            return false;
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int siIndex = strProcess.size();
            if (siIndex >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 1] = '0';
            if (siIndex >  6 && strProcess.compare( 2, 4, PPSAFile::strSpace, 0, 4) == 0)
                strProcess.replace(2, 4, "PIN ");
            if (siIndex > 11 && strProcess.compare( 6, 5, PPSAFile::strSpace, 0, 5) == 0)
                strProcess[10] = '0';
            if (siIndex > 16 && strProcess.compare(11, 5, PPSAFile::strSpace, 0, 5) == 0)
                strProcess[15] = '0';
            if (siIndex > 17 && strProcess.compare(16, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[16] = '0';

            strRead.clear();
            strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.4s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.5s}", &strProcess[ 6]) << " "
                    << fmt::format("{:.5s}", &strProcess[11]) << " "
                    << fmt::format("{:.1s}", &strProcess[16]);
        }

        //
        // FREE format...
        //

        strRead >> reactCurr->siNodeID >> strProcess
                >> reactCurr->dVector[X] >> reactCurr->dVector[Y] >> siEOS;

        strProcess[4] = 0; // hard limit
        std::transform(strProcess.begin(), strProcess.begin() += 4, strProcess.begin(),
                       [](unsigned char c) -> unsigned char { return std::toupper(c); });
        reactCurr->siType = Reaction::siPIN; // Default to "PIN"
        if (strProcess.compare(Reaction::strType[0]) == 0) // FIX
            reactCurr->siType = Reaction::siFIX;
        else if (strProcess.compare(Reaction::strType[2]) == 0) // ROLL
            reactCurr->siType = Reaction::siROLL;
        else if (strProcess.compare(Reaction::strType[3]) == 0) // FIRL
            reactCurr->siType = Reaction::siFIRL;

        //
        // Store Reaction...
        //
        react.push_back(reactCurr);

        // Get another Reaction if not marked as last...
    } while (siEOS == false);

    return true;
}

//***************************************************************************
//*
//* PPSAFile::getLoadCase
//*     This function inputs the loading requirements for the
//*     structure.
//*
//*     The LOAD TYPES are divided into two categories:
//*
//*     "CONCENTRATED" Systems              "DISTRIBUTED" Systems
//*         Concentrated                        Uniform
//*         Nodal                               Trapezoidal
//*
//***************************************************************************

bool PPSAFile::getLoadCase(PlaneFrame & pframe)
{
    // Read in the Load Arrangement and Interaction Interpretation (section 8)...
    if ( ! this->getLoadCaseLoadArrangementAndInteractionInterpretation(pframe.li) )
        return false;

    // Check Option for any discrepancy with Load Arrangement and
    //      skip loads that cannot be processed...
    this->bWarnTPIConcLoad = false;
    this->bWarnTPITrapLoad = false;
    bool bSkip = false;
    if ( pframe.li.calcUsingTPI() )
    {
        if (pframe.li.bPointLoads)
        {
            this->bWarnTPIConcLoad = true;
            bSkip = true;
        }
        if (pframe.li.bTrapezoidalLoads)
        {
            this->bWarnTPITrapLoad = true;
            bSkip = true;
        }
    }

    // Read in the Concentrated Loads (section 9)...
    if ( pframe.li.bPointLoads )
    {
        if ( ! this->getLoadCaseConcentratedLoads(pframe.pl, bSkip) )
            return false;
    }

    // Read in the Uniform Loads (section 10)...
    if ( pframe.li.bUniformLoads )
    {
        if ( ! this->getLoadCaseUniformLoads(pframe.ul) )
            return false;
    }

    // Read in the Nodal Loads (section 11)...
    if ( pframe.li.bNodeLoads)
    {
        if ( ! this->getLoadCaseNodalLoads(pframe.nl) )
            return false;
    }

    // Read in the Trapezoidal Loads (section 12)...
    if ( pframe.li.bTrapezoidalLoads )
    {
        if ( ! this->getLoadCaseTrapezoidalLoads(pframe.tl, bSkip) )
            return false;
    }

    return true;
}

//*
//* End of PPSAFile::getLoadCase
//***************************************************************************

bool PPSAFile::getLoadCaseLoadArrangementAndInteractionInterpretation(LoadAndInteraction & li)
{
    //*
    //* Read in the Load Arrangement and Interaction Interpretation (section 8)...
    //*
    //***********************************************************************

    std::string strProcess;
    strProcess.append(60, 0); // ...really clear
    strProcess.clear();

    std::getline( this->isInFile, strProcess );
    if ( this->isInFile.eof() )
    {
        std::cout << PPSAFile::strEOF;
        return false;
    }
    std::stringstream strRead;
    strRead.str(strProcess); // ...store original input for later output if FREE

    //
    // Convert Standard format to Free format...
    //

    if ( this->isStandard() )
    {
        short int siIndex = strProcess.size();
        if (siIndex >  1 && strProcess.compare( 0, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[ 0] = '0';
        if (siIndex >  2 && strProcess.compare( 1, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[ 1] = '0';
        if (siIndex >  3 && strProcess.compare( 2, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[ 2] = '0';
        if (siIndex >  4 && strProcess.compare( 3, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[ 3] = '0';
        if (siIndex >  5 && strProcess.compare( 4, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[4] = '0';
        if (siIndex >  6 && strProcess.compare( 5, 1, PPSAFile::strSpace, 0, 1) == 0)
            strProcess[5] = '0';

        strRead.clear();
        strRead << fmt::format("{:.1s}", &strProcess[ 0]) << " "
                << fmt::format("{:.1s}", &strProcess[ 1]) << " "
                << fmt::format("{:.1s}", &strProcess[ 2]) << " "
                << fmt::format("{:.1s}", &strProcess[ 3]) << " "
                << fmt::format("{:.1s}", &strProcess[ 4]) << " "
                << fmt::format("{:.1s}", &strProcess[ 5]);
    }

    //
    // FREE format...
    //

    short int siUsePointLoads = 0;
    short int siUseUniformLoads = 0;
    short int siUseNodeLoads = 0;
    short int siUseTrapLoads = 0;
    short int siInteraction = 0;
    short int siStressFactorIndex = 0;
    strRead >> siUsePointLoads >> siUseUniformLoads >> siUseNodeLoads
            >> siInteraction >> siStressFactorIndex >> siUseTrapLoads;
    // Adjust Stress Factor Index input...
    if (siStressFactorIndex == 0)
        siStressFactorIndex = 1;

    // Load flags...
    li.bPointLoads       = (siUsePointLoads != 0);
    li.bUniformLoads     = (siUseUniformLoads != 0);
    li.bNodeLoads        = (siUseNodeLoads != 0);
    li.bTrapezoidalLoads = (siUseTrapLoads != 0);
    li.siStressFactorIndex = siStressFactorIndex;
    li.setInteraction(siInteraction);

    // Check Stress Factor Index...
    if (siStressFactorIndex < 1 || siStressFactorIndex > 3)
        li.siStressFactorIndex = 0; // ...problem index

    return true;
}

bool PPSAFile::getLoadCaseConcentratedLoads(std::vector<LoadPoint *> & pl, bool bSkip)
{
    //*
    //* Read in the Concentrated Loads (section 9)...
    //*     "CONCENTRATED" System
    //*
    //***********************************************************************

    std::string strProcess;
    std::stringstream strRead;
    short int siEOS = false;
    do
    {
        //
        // Allocate memory for a Point Load...
        //

        LoadPoint * plCurr = nullptr;
        if ( ! bSkip )
        {
            if ( ( plCurr = new LoadPoint ) == nullptr )
            {
                std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                          <<   "        Concentrated Load!\n";
                return false;
            }
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int ind = strProcess.size();
            if (ind >  2 && strProcess.compare( 0, 2, PPSAFile::strSpace, 0, 2) == 0)
                strProcess[ 1] = '0';
            if (ind >  3 && strProcess.compare( 2, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[ 2] = '0';
            if (ind >  9 && strProcess.compare( 3, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[ 8] = '0';
            if (ind > 15 && strProcess.compare( 9, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[14] = '0';
            if (ind > 21 && strProcess.compare(15, 6, PPSAFile::strSpace, 0, 6) == 0)
                strProcess[20] = '0';
            if (ind > 22 && strProcess.compare(21, 1, PPSAFile::strSpace, 0, 1) == 0)
                strProcess[21] = '0';

            strRead.clear();
            strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.1s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.6s}", &strProcess[ 3]) << " "
                    << fmt::format("{:.6s}", &strProcess[ 9]) << " "
                    << fmt::format("{:.6s}", &strProcess[15]) << " "
                    << fmt::format("{:.1s}", &strProcess[21]);
        }

        //
        // FREE format...
        //

        if (bSkip)
        {
            short int siSkip;
            double dSkip;
            strRead >> siSkip >> siSkip >> dSkip >> dSkip >> dSkip >> siEOS;
        }
        else
        {
            strRead >> plCurr->siMemberID >> plCurr->siID
                    >> plCurr->adLoadVect[X] >> plCurr->adLoadVect[Y] >> plCurr->dDistance >> siEOS;

            // Store Concentrated Loads to Plane Frame Structure...
            pl.push_back(plCurr);
        }

        // Get another LoadPoint, if not end of section...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::getLoadCaseUniformLoads(std::vector<LoadUniform *> & ul)
{
    //*
    //* Read in the Uniform Loads (section 10)...
    //*     "DISTRIBUTED" System
    //*
    //***********************************************************************

    std::string strProcess;
    std::stringstream strRead;
    short int siEOS = false;
    do
    {
        //
        // Allocate memory for a Uniform Load...
        //

        LoadUniform * ulCurr = nullptr;
        if ( ( ulCurr = new LoadUniform ) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      <<   "        Uniform Load!\n";
            return false;
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int siIndex = strProcess.size();
            if (siIndex >  2 && strProcess.compare( 0,  2, PPSAFile::strSpace, 0,  2) == 0)
                strProcess[ 1] = '0';
            if (siIndex > 12 && strProcess.compare( 2, 10, PPSAFile::strSpace, 0, 10) == 0)
                strProcess[11] = '0';
            if (siIndex > 22 && strProcess.compare(12, 10, PPSAFile::strSpace, 0, 10) == 0)
                strProcess[21] = '0';
            if (siIndex > 23 && strProcess.compare(22,  1, PPSAFile::strSpace, 0,  1) == 0)
                strProcess[22] = '0';

            strRead.clear();
            strRead << fmt::format( "{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.10s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.10s}", &strProcess[12]) << " "
                    << fmt::format( "{:.1s}", &strProcess[22]);
        }

        //
        // FREE format...
        //

        strRead >> ulCurr->siMemberID >> ulCurr->adLoadVect[X] >> ulCurr->adLoadVect[Y] >> siEOS;

        // Store Uniform Loads to Plane Frame Structure...
        ul.push_back(ulCurr);

        // Get another LoadUniform, if not end of section...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::getLoadCaseNodalLoads(std::vector<LoadNodal *> & nl)
{
    //*
    //* Read in the Nodal Loads (section 11)...
    //*     "CONCENTRATED" System
    //*
    //***********************************************************************

    std::string strProcess;
    std::stringstream strRead;
    short int siEOS = false;
    do
    {
        //
        // Allocate memory for a Nodal Load...
        //

        LoadNodal * nlCurr = nullptr;
        if ( ( nlCurr = new LoadNodal ) == nullptr )
        {
            std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                      <<   "        Node Load!\n";
            return false;
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int siIndex = strProcess.size();
            if (siIndex >  2 && strProcess.compare( 0,  2, PPSAFile::strSpace, 0,  2) == 0)
                strProcess[ 1] = '0';
            if (siIndex >  3 && strProcess.compare( 2,  1, PPSAFile::strSpace, 0,  1) == 0)
                strProcess[ 2] = '0';
            if (siIndex > 11 && strProcess.compare( 3,  8, PPSAFile::strSpace, 0,  8) == 0)
                strProcess[10] = '0';
            if (siIndex > 12 && strProcess.compare(11,  1, PPSAFile::strSpace, 0,  1) == 0)
                strProcess[11] = '0';

            strRead.clear();
            strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.1s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.8s}", &strProcess[ 3]) << " "
                    << fmt::format("{:.1s}", &strProcess[11]);
        }

        //
        // FREE format...
        //

        strRead >> nlCurr->siNodeID >> nlCurr->siLoadDirection >> nlCurr->dLoad >> siEOS;

        if (nlCurr->siLoadDirection < 1 || nlCurr->siLoadDirection > 3)
            nlCurr->siLoadDirection = 2; // ...default bad load direction to Y (gravity)

        // Store Nodal Load...
        nl.push_back(nlCurr);

        // Get another LoadNodal, if not end of section...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::getLoadCaseTrapezoidalLoads(std::vector<LoadTrapezoidal *> & tl, bool bSkip)
{
    //*
    //* Read in the Trapezoidal Loads (section 12)...
    //*     "DISTRIBUTED" System
    //*
    //***********************************************************************

    std::string strProcess;
    std::stringstream strRead;
    short int siEOS = false;
    do
    {
        //
        // Allocate memory for a Trapezoidal Load...
        //

        LoadTrapezoidal * tlCurr = nullptr;
        if ( ! bSkip )
        {
            if ( ( tlCurr = new LoadTrapezoidal ) == nullptr )
            {
                std::cerr << "\n ERROR: OUT OF MEMORY!!\n"
                          <<   "        Trapezoidal Load!\n";
                return false;
            }
        }

        strProcess.clear();
        strProcess.append(60, 0); // ...really clear
        strProcess.clear();

        std::getline( this->isInFile, strProcess );
        if ( this->isInFile.eof() )
        {
            std::cout << PPSAFile::strEOF;
            return false;
        }
        strRead.clear();
        strRead.str(strProcess); // ...store original input for later output if FREE

        //
        // Convert Standard format to Free format...
        //

        if ( this->isStandard() )
        {
            short int siIndex = strProcess.size();
            if (siIndex >  2 && strProcess.compare( 0,  2, PPSAFile::strSpace, 0,  2) == 0)
                strProcess[ 1] = '0';
            if (siIndex >  4 && strProcess.compare( 2,  2, PPSAFile::strSpace, 0,  2) == 0)
                strProcess[ 3] = '0';
            if (siIndex > 12 && strProcess.compare( 4,  8, PPSAFile::strSpace, 0,  8) == 0)
                strProcess[11] = '0';
            if (siIndex > 20 && strProcess.compare(12,  8, PPSAFile::strSpace, 0,  8) == 0)
                strProcess[19] = '0';
            if (siIndex > 28 && strProcess.compare(20,  8, PPSAFile::strSpace, 0,  8) == 0)
                strProcess[27] = '0';
            if (siIndex > 36 && strProcess.compare(28,  8, PPSAFile::strSpace, 0,  8) == 0)
                strProcess[35] = '0';
            if (siIndex > 41 && strProcess.compare(36,  5, PPSAFile::strSpace, 0,  5) == 0)
                strProcess[40] = '0';
            if (siIndex > 42 && strProcess.compare(41,  1, PPSAFile::strSpace, 0,  1) == 0)
                strProcess[41] = '0';

            strRead.clear();
            strRead << fmt::format("{:.2s}", &strProcess[ 0]) << " "
                    << fmt::format("{:.2s}", &strProcess[ 2]) << " "
                    << fmt::format("{:.8s}", &strProcess[ 4]) << " "
                    << fmt::format("{:.8s}", &strProcess[12]) << " "
                    << fmt::format("{:.8s}", &strProcess[20]) << " "
                    << fmt::format("{:.8s}", &strProcess[28]) << " "
                    << fmt::format("{:.5s}", &strProcess[36]) << " "
                    << fmt::format("{:.1s}", &strProcess[41]);
        }

        //
        // FREE format...
        //

        if (bSkip)
        {
            short int siSkip;
            double dSkip;
            strRead >> siSkip >> siSkip >> dSkip >> dSkip >> dSkip >> dSkip >> dSkip >> siEOS;
        }
        else
        {
            strRead >> tlCurr->siMemberID >> tlCurr->siID >> tlCurr->adLoad[X] >> tlCurr->adLoad[Y]
                    >> tlCurr->adDist[X] >> tlCurr->adDist[Y] >> tlCurr->dTheta >> siEOS;

            // Store Trapezoidal Load...
            tl.push_back(tlCurr);
        }

        // Get another LoadTrapezoidal, if not end of section...
    } while (siEOS == false);

    return true;
}

bool PPSAFile::isWarningForTPIConcLoad(void)
{
    return PPSAFile::bWarnTPIConcLoad;
}

bool PPSAFile::isWarningForTPITrapLoad(void)
{
    return PPSAFile::bWarnTPITrapLoad;
}
