/*
 * Report.cpp
 *
 *  Created on: Aug 25, 2020
 *      Author: Keven L. Ates
 */

#include "Report.hpp"
#include "ReportFormat.hpp"
#include "Version.hpp"

#include "PlaneFrame.hpp"

#include <iostream>
#include <iomanip>
#include <fmt/format.h>

const char Report::cBackup = '\b';
const std::string Report::strNoLoad = " ---------- NONE ----------\n\n";

const std::string Report::strHeading = std::string() +
        "\n" +
        "     ************************************************************************\n" +
        "     ***     PROGRAM: CAPS                                                ***\n" +
        "     ***     Comprehensive Analyzer for Plane Structures                  ***\n" +
        "     ***          Version " + fmt::format("{: 2d}.{:02d}", VERSION_MAJOR, VERSION_MINOR) +
                                       "  Release " + RELEASE_DATE +
                                                           "                       ***\n" +
        "     ***          COPYRIGHT (" + COPYRIGHT +
                                         ")                                        ***\n" +
        "     ***                                                                  ***\n" +
        "     ***     This program is intended to facilitate the analysis of       ***\n" +
        "     ***     wood structures.  It creates a structural response           ***\n" +
        "     ***     report from the input analog.  Accuracy of the analog        ***\n" +
        "     ***     and interpretation of the structural adequacy are the        ***\n" +
        "     ***     responsibility of the user.  The authors assume no           ***\n" +
        "     ***     responsibility, explicit or implied.                         ***\n" +
        "     ***                                                                  ***\n" +
        "     ***                     USE AT YOUR OWN RISK!!!                      ***\n" +
        "     ***                                                                  ***\n" +
        "     ************************************************************************\n" +
        "\n" +
        " ================================================================================\n\n";

Report::Report(void) : pofsOutFile( nullptr ), osOutFile( std::cout )
{
    this->bNotTerminalOut = false;
    this->strOutFile = "STDOUT";
}

Report::Report(std::string & strOut) : pofsOutFile( new std::ofstream(strOut) ), osOutFile( *pofsOutFile )
{
    this->strOutFile = strOut;
}

void Report::setQuiet(void)
{
    this->bQuiet = true;
}

bool Report::checkReport(void)
{
    if ( ! osOutFile.good() )
    {
        std::cerr << " ERROR: Could not open " << this->strOutFile << " for output!" << std::endl;
        return false;
    }
    return true;
}

void Report::close(void)
{
    if ( this->pofsOutFile != nullptr )
    {
        this->pofsOutFile->close();
        delete this->pofsOutFile;
        this->pofsOutFile = nullptr;
    }
}

void Report::setPlaneFrame(PlaneFrame * pf)
{
    this->pframe = pf;
}

void Report::setTitle(short int siIndex, std::string strFormat, std::string strID, std::string strOpt)
{
    this->siTitleIndex = siIndex;
    this->strTitleFormat = strFormat;
    this->strTitleID = strID;
    this->strTitleOpt = strOpt;
}

void Report::setLoadCase(short int siLC)
{
    this->siLoadCase = siLC;
}

PlaneFrame * Report::getPlaneFrame(void)
{
    return this->pframe;
}

void Report::reportPreambleDirect(void)
{
    std::cout << Report::strHeading;
}

void Report::reportPreamble(void)
{
    if ( bNotTerminalOut ) // ..if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
            this->reportPreamble( std::cout ); // ...then, produce some terminal output
        this->reportPreamble( this->osOutFile ); // ...output to file
    }
    else if ( ! this->bQuiet ) // ...output to terminal...
        this->reportPreamble( this->osOutFile );
}

void Report::reportPreamble(std::ostream & osOut)
{
    osOut << Report::strHeading;
}

void Report::reportTitle(void)
{
    if ( this->bNotTerminalOut ) // ..if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Structure Read indicator...
            std::cout << " STRUCTURE " << std::setw(3) << std::right << this->siTitleIndex << std::left << ": "
                      << std::setw(TAGLINE_WIDTH) <<  "Reading Structure..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
        this->reportTitle( this->osOutFile ); // ...output to file
    }
    else if ( ! this->bQuiet ) // ...output to terminal...
        this->reportTitle( this->osOutFile );
}

void Report::reportTitle(std::ostream & osOut)
{
    // Print the Structure count and file identification...
    osOut << " STRUCTURE " << std::setw(3) << std::right << this->siTitleIndex << "\n";
    osOut << " --------------------\n";
    osOut << " Format: " << this->strTitleFormat << "\n";
    osOut << "     ID: " << this->strTitleID << "\n";
    if ( this->strTitleOpt.size() )
        osOut << " Option: " << this->strTitleOpt << "\n";
    osOut << " --------------------------------------------------------------------------------\n";
}

void Report::reportStructureReading(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Structure Process indicator...
            std::cout << std::setw(TAGLINE_WIDTH) <<  "Reading Structure..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportStructure(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Structure Report indicator...
            std::cout << std::setw(TAGLINE_WIDTH) <<  "Reporting Structure..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }

        if ( this->pframe->ps.bNoPrintIn )
            return;

        // Output to file...

        // Report the Problem Size...
        this->osOutFile << this->pframe->ps.report();
        // Report the Table of Member Properties...
        this->reportStructureMaterialProperties();
        // Report the Table of Node Coordinates...
        this->reportStructureNodeCoordinates();
        // Report the Table of Member Assemblies...
        this->reportStructureMemberAssemblies();
        // Report the Table of Reactions...
        this->reportStructureReactions();
        // Report the Table of System Stiffness Matix...
        this->reportStructureSSM();
    }
    else if ( ! this->bQuiet ) // ...output to terminal...
    {
        if ( this->pframe->ps.bNoPrintIn )
            return;

        // Report the Problem Size...
        this->osOutFile << this->pframe->ps.report();
        // Report the Table of Member Properties...
        this->reportStructureMaterialProperties();
        // Report the Table of Node Coordinates...
        this->reportStructureNodeCoordinates();
        // Report the Table of Member Assemblies...
        this->reportStructureMemberAssemblies();
        // Report the Table of Reactions...
        this->reportStructureReactions();
        // Report the Table of System Stiffness Matix...
        this->reportStructureSSM();
    }
}

void Report::reportStructureMaterialProperties(void)
{
    //*
    //* Report the Table of Member Properties...
    //*****************************************

    std::string strTitle    = " TABLE  1:        Material Properties";
    std::string strOptS     =                                      " (Special Cross-Section Option)";
    std::string strOptC     =                                      " (Composite Option - GluLam/PSL/LVL)";

    std::string strMemberTypeLegend =
        "   ----------------------------------------------------------------------\n"
        "   Legend: Member Type\n"
        "   ----------------------------------------------------------------------\n"
        "     S = Supported                     U = Unsupported\n"
        "     T = Truss Chord (Supported)       I = Interior Member (Web)\n"
        "     D = Seasoned (Dry)                G = Green\n"
        "     M = Machine: Stress Rated (MSR), Evaluated Lumber (MEL)\n"
        "     2 = Type 2 Composite\n"
        "   ----------------------------------------------------------------------";

    std::string strAMS      = "              Allowable Material Stresses in PSI";
    std::string strNLD      = "                @ Normal Load Duration (100%)";

    std::string strOptSCol1 = "  Group   X-Sect   Moment of   Section   Modulus of Elasticity     Shear";
    std::string strOptSCol2 = "    ID     Area     Inertia    Modulus    Bending     Axial       Modulus";

    std::string strCommCol1 = "  Group  Member   Allowable                     ";
    std::string strCommCol2 = "    ID   Type  Bend  Comp  Tens   Width  Depth  ";
    std::string strOptCCol1 =                                                 "   Modulus of Elasticity      Shear";
    std::string strOptCCol2 =                                                 " InPlane  PerpPlane   Axial   Modulus";
    std::string strNormCol1 =                                                 "Modulus of    Shear";
    std::string strNormCol2 =                                                 "Elasticity    Modulus";

    std::string strTable; // Gathers Table Header

    strTable = strTitle;
    if ( this->pframe->isSpecialCrossSection() )
        strTable += strOptS;
    else if ( this->pframe->isComposite() )
        strTable += strOptC;

    if ( ! this->pframe->isSpecialCrossSection() )
        strTable += "\n\n" +
                    strMemberTypeLegend;

    strTable += "\n\n" +
                strAMS + "\n";

    if ( this->pframe->isSpecialCrossSection() )
        strTable += "\n" +
                    strOptSCol1 + "\n" +
                    strOptSCol2;
    else
    {
        strTable += strNLD + "\n\n" +
                    strCommCol1;
        if ( this->pframe->isComposite() )
            strTable += strOptCCol1;
        else
            strTable += strNormCol1;
        strTable += "\n" +
                    strCommCol2;
        if ( this->pframe->isComposite() )
            strTable += strOptCCol2;
        else
            strTable += strNormCol2;
    }
    strTable += "\n\n";

    //
    // Report Table Header...
    //
    this->osOutFile << strTable;

    //
    // Start Member Property loop...
    //
    for (MaterialProperty * mpCurr : this->pframe->mprops)
    {
        mpCurr->report( this->osOutFile, this->pframe->isSpecialCrossSection(), this->pframe->isComposite() );
    }
    this->osOutFile << "\n";
}

void Report::reportStructureNodeCoordinates(void)
{
    //*
    //* Report the Table of Node Coordinates...
    //*****************************************

    this->osOutFile
        << " TABLE  2:        Node Coordinates\n\n";

    this->osOutFile
        << "   Node    (X-Coordinate  Y-Coordinate)\n"
        << "    ID     (   inches   ,    inches   )\n\n";

    //
    // Start Node Coordinate loop...
    //
    for (Node * nodeCurr : this->pframe->nodes)
    {
        nodeCurr->report( this->osOutFile );
    }
    this->osOutFile << "\n";
}

void Report::reportStructureMemberAssemblies(void)
{
    //*
    //* Report the Table of Member Assemblies...
    //**********************************

    this->osOutFile
        << " TABLE  3:        Member Layout\n\n";

    this->osOutFile
        << "  Member     Negative End      Positive End     Group\n"
        << "    ID      Node  Condition   Node  Condition     ID\n\n";

    //
    // Start Member loop...
    //
    for (Member * memberCurr : this->pframe->members)
    {
        this->osOutFile << memberCurr->reportAssembly();
    }
    this->osOutFile << "\n";

    //*
    //* Report the Table of Additional Member Information and Overrides...
    //*********************************

    // As per the PPSA II documentation:reportStructureReactions
    // The Structural Assembly lines give the user the option of independently
    // specifying effective column lengths for each member in planes parallel
    // and perpendicular to the plane of the structure and specifying effective
    // bending length for beams lacking continuous edge support and/or torsional
    // restraint at their bearing points (NDS 3.3.3 and 4.4).
    std::string strTitle    = " TABLE  3 A:      Member's Processed Material Properties";
    std::string strCommCol1 = "  Member  Member  Effective Column Lengths  Effective Bending";
    std::string strCommCol2 = "  Number  Length    In-Plane   Perp-Plane       Length";
    std::string strCommCol3 = "          (IN)       (IN)        (IN)           (IN)";
    std::string strOptCCol1 =                                                              "  Volume";
    std::string strOptCCol2 =                                                       "         Factor";
    std::string strOptCCol3 =                                                     "             %";
    std::string strTable;

    strTable = strTitle + "\n\n" +
               strCommCol1;
    if ( this->pframe->isComposite() )
        strTable += strOptCCol1;
    strTable += "\n" +
                strCommCol2;
    if ( this->pframe->isComposite() )
        strTable += strOptCCol2;
    strTable += "\n" +
            strCommCol3;
    if ( this->pframe->isComposite() )
        strTable += strOptCCol3;
    strTable += "\n\n";
    this->osOutFile << strTable;

    //
    // Start Member Information loop...
    //
    for (Member * memberCurr : this->pframe->members)
    {
        this->osOutFile << memberCurr->reportAdditions(this->pframe->isComposite() );
    }
    this->osOutFile << "\n";
}

void Report::reportStructureReactions(void)
{
    //*
    //* Report the Table of Reactions...
    //***********************************

    this->osOutFile
        << " TABLE  4:        Reaction Conditions\n\n";

    this->osOutFile
        << "   Node   Reaction    Horizontal     Vertical\n"
        << "    ID      Type     Displacement  Displacement\n\n";

    //
    // Start Reaction Definition loop...
    //
    for (Reaction * reactCurr : this->pframe->react)
    {
        this->osOutFile << reactCurr->report();
    }
    this->osOutFile << "\n";
}

void Report::reportStructureSSM(void)
{
    //*
    //* Report the Table of System Stiffness Matrix...
    //***********************************

    this->osOutFile
        << " TABLE  5:        System Stiffness Matrix\n\n";

    //
    // Print the System Stiffness Matrix report...
    //
    this->osOutFile << this->pframe->ssm.report();
    this->osOutFile << "\n";
}

void Report::reportLoadCaseReading(void)
{
    // If the general output is not to the terminal output...
    if (  this->bNotTerminalOut )
    {
        if ( ! this->bQuiet )
        {
            // Print the Load Case Read indicator...
            std::cout << "LOAD CASE " << std::setw(3) << std::right << this->siLoadCase << std::left << ": "
                      << std::setw(TAGLINE_WIDTH) <<  "Reading Load Case..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

//***************************************************************************
//*
//* Report::reportLoadCase
//*     Outputs the loading requirement tables for the structure.
//*
//*     The LOAD TYPES are divided into two categories:
//*
//*         "CONCENTRATED" Systems          "DISTRIBUTED" Systems
//*             Point                           Uniform
//*             Nodal                           Trapezoidal
//*
//***************************************************************************

void Report::reportLoadCase(bool bWarnTPIConcLoad, bool bWarnTPITrapLoad)
{
    if (bWarnTPIConcLoad)
    {
        this->osOutFile
            << " WARNING: STRUCTURE: " << std::setw(3) << std::right << this->siTitleIndex << "\n"
            << "                 ID: " << this->strTitleID << "\n"
            << "          LOAD CASE: " << std::setw(3) << std::right << siLoadCase << "\n"
            << "          Concentrated Loads not permitted with TPI method!\n"
            << "          Ignoring Concentrated Loads in this load case...\n\n";
        return;
    }
    if (bWarnTPITrapLoad)
    {
        this->osOutFile
            << " WARNING: STRUCTURE: " << std::setw(3) << std::right << this->siTitleIndex << "\n"
            << "                 ID: " << this->strTitleID << "\n"
            << "          LOAD CASE: " << std::setw(3) << std::right << siLoadCase << "\n"
            << "          Trapezoidal Loads not permitted with TPI method!\n"
            << "          Ignoring Trapezoidal Loads in this load case...\n\n";
        return;
    }

    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Load Case Report indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Reporting Load Case..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }

        if ( this->pframe->ps.bNoPrintIn )
            return;

        this->reportLoadCaseLoads(); // ...output to file
    }
    else if ( ! this->bQuiet ) // ...output to terminal...
    {
        if ( this->pframe->ps.bNoPrintIn )
            return;

        this->reportLoadCaseLoads();
    }
}

void Report::reportLoadCaseLoads(void)
{
    //*
    //* Report the Load Case ID...
    //*****************************************************

            //         1         2         3         4         5         6         7
    this->osOutFile << " ***** LOAD CASE: " << std::setw(3) << std::right << siLoadCase
                             << " ********************************************************\n\n";

    //*
    //* Report the Table of Stress Factor and Loads...
    //*****************************************************

    this->osOutFile << " TABLE  6:        Loads\n\n";

    this->osOutFile << "  Stress Adjustment Factor: ";
    this->osOutFile << this->pframe->sf.report(this->pframe->li.siStressFactorIndex) << "\n\n";

    this->reportLoadCaseLoadsPoint();
    this->reportLoadCaseLoadsUniform();
    this->reportLoadCaseLoadsNodal();
    this->reportLoadCaseLoadsTrap();
}

void Report::reportLoadCaseLoadsPoint(void)
{
    //*
    //* Report the Table of Concentrated (Point) Loads...
    //*****************************************************

    this->osOutFile << " TABLE  6 A:      Point Loads\n\n";

    if ( this->pframe->li.bPointLoads && ! this->pframe->pl.empty() )
    {
        this->osOutFile
            << "  Member   Load   Horizontal    Vertical   Distance to Point Load\n"
            << "  Number  Number  Compression  Compression   from Negative End\n"
            << "                     (LBS)        (LBS)            (IN)\n\n";

        for (LoadPoint * plCurr : this->pframe->pl)
        {
            this->osOutFile << plCurr->report();
        }
        this->osOutFile << "\n";
    }
    else
        this->osOutFile << Report::strNoLoad;
}

void Report::reportLoadCaseLoadsUniform(void)
{
    //*
    //* Report the Table of Uniform Loads...
    //*****************************************************

    this->osOutFile << " TABLE  6 B:      Uniform Loads\n\n";

    if ( this->pframe->li.bUniformLoads && ! this->pframe->ul.empty() )
    {
        this->osOutFile
            << "  Member  Horizontal    Vertical\n"
            << "  Number  Compression  Compression\n"
            << "            (LBS)        (LBS)\n\n";

        for (LoadUniform * ulCurr : this->pframe->ul)
        {
            this->osOutFile << ulCurr->report();
        }
        this->osOutFile << "\n";
    }
    else
        this->osOutFile << Report::strNoLoad;
}

void Report::reportLoadCaseLoadsNodal(void)
{
    //*
    //* Report the Table of Nodal Loads...
    //*****************************************************

    this->osOutFile << " TABLE  6 C:      Nodal Loads\n\n";

    if ( this->pframe->li.bNodeLoads && ! this->pframe->nl.empty() )
    {
        this->osOutFile
            << "   Node   Direction  Load in LBS or IN-LBS\n"
            << "  Number   of Load      as appropriate\n\n";

        for (LoadNodal * nlCurr : this->pframe->nl)
        {
            this->osOutFile << nlCurr->report();
        }
        this->osOutFile << "\n";
    }
    else
        this->osOutFile << Report::strNoLoad;
}

void Report::reportLoadCaseLoadsTrap(void)
{
    //*
    //* Report the Table of Trapezoidal Loads...
    //*****************************************************

    this->osOutFile << " TABLE  6 D:      Trapezoidal Loads\n\n";

    if ( this->pframe->li.bTrapezoidalLoads && ! this->pframe->tl.empty() )
    {
        this->osOutFile
            << "  Member   Load   From     | Starting   Ending   Starting   Ending     Angle\n"
            << "  Number  Number  Negative |   Load      Load    Distance  Distance   of Load\n"
            << "                  End ----->  (PLI)     (PLI)      (IN)      (IN)    (Degrees)\n\n";

        for (LoadTrapezoidal * tlCurr : this->pframe->tl)
        {
            this->osOutFile << tlCurr->report();
        }
        this->osOutFile << "\n";
    }
    else
        this->osOutFile << Report::strNoLoad;
}

void Report::reportLoadCaseProcessing(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Load Case Process indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Processing Load Case..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportLoadCaseLoading(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Member Process indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Loading Members..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportCalcDisplacementMatrix(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Calc Displacement Matrix indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Calcing Displacement Matrix..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportCalcMemberForces(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Calc Member Forces indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Calcing Member Forces..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportCalcActions(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Calc Structure Actions indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Calcing Structure Actions..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportAnalyzeStructure(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Structure Analysis Indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Analyzing Structure..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportCalcDeflections(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Calc Deflection indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Calcing Deflections..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }
}

void Report::reportResults(void)
{
//    Output Control – Control the output of the tabulated calculation results
//    0: Print all of the output tables (default)
//    1: Only Tables  7 and  8 are listed
//    2: Only Table   9 is listed
//    3: Only Tables  9 and 10 are listed
//    4: Only Tables 11 and 12 are listed
//    5: Only Tables  9  –  12 are listed

    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Print the Load Case Report indicator...
            std::cout << std::setw(TAGLINE_WIDTH) << "Reporting Load Case Results..."
                      << std::setfill(Report::cBackup) << std::setw(TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ');
        }
    }


    this->osOutFile
        << " *****************************************************************************\n"
        << " ********************************** RESULTS **********************************\n"
        << " *****************************************************************************\n\n";

    //
    // Report the Table of Plane Frame Load Actions (Tables 7 & 8)...
    //

    this->osOutFile
        << " * * * * * * * * * * * * * * *  ACTION ANALYSIS  * * * * * * * * * * * * * * *\n\n";
    // Check for Table 7 & 8 output...
    if ( this->pframe->ps.siPrintOut == 0 || this->pframe->ps.siPrintOut == 1 )
    {
        this->reportResultsReactions();
        this->reportResultsEndActions();
    }

    //
    // Report the Tables of Plane Frame Load Stresses (Tables 9 & 10)...
    //

    this->osOutFile
        << " * * * * * * * * * * * * * * *  STRESS ANALYSIS  * * * * * * * * * * * * * * *\n\n";
    // Check for Table 9 & 10 output...
    if ( this->pframe->ps.siPrintOut == 0 ||
         this->pframe->ps.siPrintOut == 2 ||
         this->pframe->ps.siPrintOut == 3 ||
         this->pframe->ps.siPrintOut == 5 )
    {
        this->reportResultsStressAxialBend();

        // Check for Table 10 output...
        if (this->pframe->ps.siPrintOut != 2)
            this->reportResultsStressShear();
    }

    //
    // Report the Tables of Plane Frame Deflections (Tables 11 & 12)...
    //

    this->osOutFile
        << " * * * * * * * * * * * * * *  DEFLECTION ANALYSIS * * * * * * * * * * * * * * \n\n";
    // Check for Tables 11 & 12 output...
    if ( this->pframe->ps.siPrintOut == 0 ||
         this->pframe->ps.siPrintOut == 4 ||
         this->pframe->ps.siPrintOut == 5 )
    {
        this->reportResultsDeflection();
        this->reportResultsDisplacement();
    }
}

void Report::reportResultsReactions(void)
{
    //*
    //* Report the Reaction results...
    //********************************************

    this->osOutFile
        << " TABLE  7:        Reactions\n\n";

    this->osOutFile
        << "  Reaction         Horz. Comp.      Vert. Comp.        Moment\n"
        << "    Node              (LBS)            (LBS)          (IN-LBS)\n\n";

    for (Reaction * reactCurr : this->pframe->react)
    {
        // TODO: Reformat as repost function: reactCurr->reportResults()
        this->osOutFile
            << "    " << std::setw(4) << std::right << reactCurr->siNodeID << "        "
            << fmt::format(SF14_3f, reactCurr->dHorzReaction) << "    "
            << fmt::format(SF14_3f, reactCurr->dVertReaction) << "     "
            << fmt::format(SF14_4E, reactCurr->dRotReaction) << "\n";
    }
    this->osOutFile << "\n";

    this->osOutFile
        << " TABLE  7 A:      Zero Check (Load - Reaction)\n\n";

    this->osOutFile
        << "                   Horz. Comp.      Vert. Comp.        Moment\n"
        << "                      (LBS)            (LBS)          (IN-LBS)\n\n";

    this->osOutFile
        << "        Load:  "
        << fmt::format(SF14_3f, this->pframe->dCombinedExtHorz) << "    "
        << fmt::format(SF14_3f, this->pframe->dCombinedExtVert) << "     "
        << fmt::format(SF14_4E, this->pframe->dCombinedExtRot) << "\n";
    this->osOutFile
        << "    Reaction:  "
        << fmt::format(SF14_3f, this->pframe->dCombinedIntHorz) << "    "
        << fmt::format(SF14_3f, this->pframe->dCombinedIntVert) << "     "
        << fmt::format(SF14_4E, this->pframe->dCombinedIntRot) << "\n";
    this->osOutFile
        << "  --------------------------------------------------------------------\n";
    this->osOutFile
        << "  Difference:  "
        << fmt::format(SF14_3f, this->pframe->dTotalHorz) << "    "
        << fmt::format(SF14_3f, this->pframe->dTotalVert) << "     "
        << ( ( fabs(this->pframe->dTotalRot) < 0.0005 ) ? // ...control Total Moment output a bit...
                fmt::format(SF14_4E, 0.0) :
                fmt::format(SF14_4E, this->pframe->dTotalRot) ) << "\n";
    this->osOutFile << "\n";
}

void Report::reportResultsEndActions(void)
{
    //*
    //* Report the Member End Action results...
    //********************************************

    this->osOutFile
        << " TABLE  8:        Member End Actions\n\n";

    this->osOutFile
        << "  Member    Location       Axial          Shear        Moment\n"
        << "  Number                   (LBS)          (LBS)       (IN-LBS)\n\n";

    for (Member * memberCurr : this->pframe->members)
    {
        // TODO: Reformat as repost function: memberCurr->reportResultsEndActions()
        this->osOutFile
            << "   " << std::setw(4) << std::right << memberCurr->siID
            << " ---[ Neg End "
            << fmt::format(SF14_3f, memberCurr->adLocalForce[0]) << " "
            << fmt::format(SF14_3f, memberCurr->adLocalForce[1]) << " "
            << fmt::format(SF14_3f, memberCurr->adLocalForce[2]) << "\n";

        this->osOutFile << "   ";
        std::string strFictitiousNote = "    ";
        if ( memberCurr->mp->isFictitious() )
            strFictitiousNote = "FICT";
        this->osOutFile << strFictitiousNote;
        this->osOutFile
            << "    [ Pos End "
            << fmt::format(SF14_3f, memberCurr->adLocalForce[3]) << " "
            << fmt::format(SF14_3f, memberCurr->adLocalForce[4]) << " "
            << fmt::format(SF14_3f, memberCurr->adLocalForce[5]) << "\n\n";
    }
}

void Report::reportResultsStressAxialBend(void)
{
    //*
    //* Report the Table of Axial and Bending Stress Analysis...
    //***********************************************************

    this->osOutFile << " TABLE  9:        ";
    if ( this->pframe->isSpecialCrossSection() )
        this->osOutFile << "Maximum Axial Force and Moment (Special Cross-Section)\n";
    else
        this->osOutFile << "NDS-" << std::setw(4) << std::right << this->pframe->li.getNDS() << " Interaction Analysis\n";

    this->osOutFile << "                  ";
    if ( this->pframe->li.calcUsingTPI() )
        this->osOutFile << "TPI Interaction: Truss with Uniform and Nodal Loads\n\n";
    else if ( this->pframe->li.calcAtBendAndAxialMax() )
        this->osOutFile << "Using Maximum Axial Force and Maximum Moment\n\n";
    else // if ( this->pframe->li.calcAtBendMax() || this->pframe->li.calcAtAxialMax() )
    {
        this->osOutFile << "Calculated at position of Maximum ";
        if ( this->pframe->li.calcAtBendMax() )
            this->osOutFile << "Moment\n\n";
        else // this->pframe->li.calcAtAxialMax()
            this->osOutFile << "Axial Force\n\n";
    }

    if ( this->pframe->isSpecialCrossSection() )
    {
        this->osOutFile
            << "  Member       Loc from Neg End          Axial        Bending\n"
            << "  Number      Axial        Bending       Stress       Stress\n"
            << "              (IN)          (IN)         (PSI)        (PSI)\n\n";
    }
    else
    {
        this->osOutFile
            << "  Member  Prop  Notes     Max       Loc from Neg End     Axial      Bending\n"
            << "  Number  Type            Int       Axial      Bend      Stress     Stress       L/D\n"
            << "                          Val       (IN)       (IN)      (PSI)      (PSI)\n\n";
    }

    //
    // Member Loop...
    //

    for (Member * memberCurr : this->pframe->members)
    {
        // TODO: Reformat as repost function: memberCurr->reportResultsAxialBending()
        this->osOutFile << "   " << std::setw(4) << std::right << memberCurr->siID;

        // Fictitious members that are NOT Special Cross-Section...
        if ( memberCurr->mp->isFictitious() && ! this->pframe->isSpecialCrossSection() )
        {
            this->osOutFile << "     Fictitious Member: No Interaction Analysis performed.\n";
        }

        // All other members, including Fictitious Special Cross-Section...
        else
        {
            if ( memberCurr->mp->isFictitious() )
                this->osOutFile << " F ";
            else
                this->osOutFile << "   ";

            //
            // Print for Special Cross-section Member...
            //
            if ( this->pframe->isSpecialCrossSection() )
            {
                this->osOutFile
                    << fmt::format(SF11_3f, memberCurr->ma.dMaxAxialLoc) << "  "
                    << fmt::format(SF11_3f, memberCurr->ma.dMaxBendLoc) << "   "
                    << fmt::format(SF11_3f, memberCurr->ma.dAxial) << "  "
                    << fmt::format(SF11_3f, memberCurr->ma.dBend) << "\n";
            }

            //
            // Print for Other Member...
            //
            else
            {
                std::string strMemberProperty = "    ";
                // if ( ! memberCurr->ma.bMixedForces && memberCurr->ma.siSlenderRatioType == 0)
                    strMemberProperty = memberCurr->mp->typeToString();

                    this->osOutFile << strMemberProperty << "  ";

                //
                // Member Notes...
                //
                bool bOtherNote = false;
                std::string strOtherNote;
                if (memberCurr->ma.siSlenderRatioType != 0) // Slenderness Ratio > 50.0...
                {
                    bOtherNote = true;
                    strOtherNote = "Bending Slenderness Ratio exceeds 50: NDS 3.3.3.5\n";
                    if (memberCurr->ma.siSlenderRatioType == 1)
                    {
                        memberCurr->ma.strMemberNotes[2] = ' ';
                    }
                    else if (memberCurr->ma.siSlenderRatioType == 2) // ...slenderness override...
                    {
                        memberCurr->ma.strMemberNotes[2] = 'B';
                        this->pframe->bNoteBending = true;
                    }
                    else // Unknown Slenderness Ratio flag...
                    {
                        strOtherNote = "Bending Slenderness Ratio exceeds 50: FLAG ERROR: " +
                                       fmt::format("{:d}", memberCurr->ma.siSlenderRatioType) + "\n";
                    }
                }
                else if (memberCurr->ma.bCompFlag && memberCurr->ma.dAxial < -1.0) // L/D > 75.0...
                {
                    bOtherNote = true;
                    strOtherNote = "L/D ratio for Compression member exceeds 75.0!\n";
                }
                else if ( memberCurr->ma.bMixedForces ) // Mixed Forces...
                {
                    bOtherNote = true;
                    strOtherNote = "Mixed Tension and Compression: Special Analysis required!\n";
                }

                //
                // Print...
                //
                this->osOutFile << memberCurr->ma.strMemberNotes << " ";
                if (bOtherNote)
                    this->osOutFile  << strOtherNote;
                else
                {
                    this->osOutFile
                        << fmt::format(SF10_3f, memberCurr->ma.dAction) << " "
                        << fmt::format(SF10_3f, memberCurr->ma.dMaxAxialLoc) << " "
                        << fmt::format(SF10_3f, memberCurr->ma.dMaxBendLoc) << " "
                        << fmt::format(SF10_3f, memberCurr->ma.dAxial) << " "
                        << fmt::format(SF10_3f, memberCurr->ma.dBend) << " "
                        << fmt::format(SF10_3f, memberCurr->ma.dLoD_Critical) << "\n";
                }
            }
        }
    }

    this->osOutFile << "\n";

    if ( ! this->pframe->isSpecialCrossSection() )
    {
        bool bPrinted = false;

        //*
        //* Report the Member Notes ledger...
        //***********************************************************

        //
        // Index 0 Notes...
        //
        if (this->pframe->bNoteStressDifference)
        {
            // 0: Tension
            this->osOutFile << "     D  Max. Interaction value is Stress Difference: NDS 3.10.1\n";
            bPrinted = true;
        }
        if (this->pframe->bNoteCompression50)
        {
            // 0: Compression 1986/1991
            this->osOutFile << "     C  L/D Ratio for Compression Member exceeds 50.0!\n";
            bPrinted = true;
        }
        if (this->pframe->bNoteCompression75)
        {
            // 0: Compression 1991
            this->osOutFile << "     *  L/D Ratio for Compression Member exceeds 75.0!\n";
            bPrinted = true;
        }

        //
        // Index 1 Notes...
        //
        if (this->pframe->bNoteTension)
        {
            // 1: Tension
            this->osOutFile << "     T  L/D Ratio for Tension Member exceeds 80.0!\n";
            bPrinted = true;
        }
        if (this->pframe->bNoteBuckle)
        {
            // 1: Compression 1986/1991
            this->osOutFile << "     S  Special Chord Effective Buckling length exceeds 96.0 inches!\n";
            bPrinted = true;
        }

        //
        // Index 2 Notes...
        //
        if (this->pframe->bNoteBending)
        {
            // 2: Effective Bending Not Given
            this->osOutFile << "     B  Effective Bending length not supplied: NDS 3.3.3.4\n";
            bPrinted = true;
        }

        //
        // Index 3 Notes...
        //
        if (this->pframe->bNoteEuler)
        {
            // 3: Compression 1991
            this->osOutFile << "     E  Actual Compression Stress exceeds Euler Stress!\n";
            bPrinted = true;
        }

        //
        // Index 4 Notes...
        //
        if (this->pframe->siPerpendicular)
        {
            // 4: Compression 1986/1991
            this->osOutFile << "     P  Interaction value critical for Perpendicular Plane!\n";
            bPrinted = true;
        }

        if (this->pframe->bNoteTPI)
        {
            // 4: Compression 1986/1991
            this->osOutFile << "     @  TPI Interaction overrides critical Perpendicular Plane!\n";
            bPrinted = true;
        }

        if (bPrinted)
            this->osOutFile << "\n";

        //*
        //*- Report the Table of Adjusted Axial and Bending Stresses...
        //***********************************************************

        this->osOutFile
            << " TABLE  9 A:      Member Force Analysis Data\n\n";

        this->osOutFile
            << "                         Final Adjusted Stresses              Effective Lengths\n"
            << "                     --------------------------------  --------------------------------\n"
            << "  Member     Length     FAxial     FBend      FBend'    In-Plane  Perp-Plane   Bending\n"
            << "  Number      (in)      (PSI)      (PSI)      (PSI)        L/D        L/D      Length\n\n";

        for (Member * memberCurr : this->pframe->members)
        {
            // TODO: Reformat as repost function: memberCurr->reportResultsAdjustedAxialBending()
            this->osOutFile << "   " << std::setw(4) << std::right << memberCurr->siID;

            if ( memberCurr->mp->isFictitious() && ! this->pframe->isSpecialCrossSection() )
                this->osOutFile << "     Fictitious Member: No Member Force Analysis performed.\n";
            else
            {
                if ( memberCurr->mp->isFictitious() )
                    this->osOutFile << " F ";
                else
                    this->osOutFile << "   ";
                this->osOutFile
                    << fmt::format(SF10_3f, memberCurr->dLength) << " "
                    << fmt::format(SF10_3f, memberCurr->ma.dForceCompPrime) << " "
                    << fmt::format(SF10_3f, memberCurr->ma.dForceBendWork) << " "
                    << fmt::format(SF10_3f, memberCurr->ma.dForceBendPrime) << "  "
                    << fmt::format(SF10_3f, memberCurr->ma.dLoD_IP) << " "
                    << fmt::format(SF10_3f, memberCurr->ma.dLoD_PP) << " "
                    << fmt::format(SF10_3f, memberCurr->dEffBendingLength) << "\n";
            }
        }
        this->osOutFile << "\n";
    }
}

void Report::reportResultsStressShear(void)
{
    //*
    //* Report the Table of Shear Stress Analysis...
    //***********************************************************

    this->osOutFile
        << " TABLE 10:        Shear Stress Analysis\n\n";

    this->osOutFile
        << "              Maximum          Location from       Member\n"
        << "  Member    Shear Stress       Negative End        Length\n"
        << "  Number         (PSI)             (in)             (in)\n\n";

    for (Member * memberCurr : this->pframe->members)
    {
        // TODO: Reformat as repost function: memberCurr->reportResultsShear()
        this->osOutFile << "   " << std::setw(4) << std::right << memberCurr->siID;

        if ( memberCurr->mp->isFictitious() && ! this->pframe->isSpecialCrossSection() )
            this->osOutFile << "     Fictitious Member: No Shear Stress Analysis performed.\n";
        else
        {
            if ( memberCurr->mp->isFictitious() )
                this->osOutFile << "  F  ";
            else
                this->osOutFile << "     ";
            this->osOutFile
                << fmt::format(SF12_3f, memberCurr->ma.dMaxShear) << "     "
                << fmt::format(SF12_3f, memberCurr->ma.dMaxShearLoc) << "     "
                << fmt::format(SF12_3f, memberCurr->dLength) << "\n";
        }
    }
    this->osOutFile << "\n";
}

void Report::reportResultsDeflection(void)
{
    //*
    //* Report the Table of Member Deflection...
    //***********************************************************

    this->osOutFile
        << " TABLE 11:        Maximum Member Deflections\n\n";

    this->osOutFile
        << "              Maximum       Location from     Member\n"
        << "  Member     Deflection     Negative End      Length\n"
        << "  Number        (in)            (in)           (in)\n\n";

    //
    // Member Deflection Calculation...
    //

    for (Member * memberCurr : this->pframe->members)
    {
        // TODO: Reformat as repost function: memberCurr->reportResultsDeflection()
        this->osOutFile << "   " << std::setw(4) << std::right << memberCurr->siID;

        if ( memberCurr->mp->isFictitious() && ! this->pframe->isSpecialCrossSection() )
            this->osOutFile << "     Fictitious Member: No Member Deflection Analysis performed.\n";
        else
        {
            if ( memberCurr->mp->isFictitious() )
                this->osOutFile << " F ";
            else
                this->osOutFile << "   ";
            this->osOutFile
                << this->formatReal(memberCurr->ma.dDelta, 12, 4) << "    "
                << this->formatReal(memberCurr->ma.dDeltaLength, 12, 4) << "   "
                << this->formatReal(memberCurr->dLength, 12, 4) << "\n";
        }
    }
    this->osOutFile << "\n";
}

void Report::reportResultsDisplacement(void)
{
    //*
    //* Report the Table of Node Displacements...
    //***********************************************************

    this->osOutFile
        << " TABLE 12:        Node Displacements\n\n";

    this->osOutFile
        << "          Horiz Displacement     Vertical           Rotational\n"
        << "   Node   or Roller Direction  Displacement        Displacement\n"
        << "  Number      (inches)          (inches)      (radians)     (degrees)\n\n";

    for (Node * nodeCurr : this->pframe->nodes)
    {
        // TODO: Reformat as repost function: nodeCurr->reportResultsDisplacement()
        if ( ! nodeCurr->siSequence )
            continue;

        double dHorzDisplace = 0.0;
        double dVertDisplace = 0.0;
        double dRotDisplace  = 0.0;

        if (nodeCurr->siHorzFlag)
            dHorzDisplace = this->pframe->adDisplaceMatrix[nodeCurr->siHorzFlag - 1];
        if (nodeCurr->siVertFlag)
            dVertDisplace = this->pframe->adDisplaceMatrix[nodeCurr->siVertFlag - 1];
        if (nodeCurr->siRotFlag)
            dRotDisplace  = this->pframe->adDisplaceMatrix[nodeCurr->siRotFlag  - 1];

        this->osOutFile
            << "   " << std::setw(4) << std::right << nodeCurr->siID
            << "      " << this->formatReal(dHorzDisplace, 9, 6)
            << "         " << this->formatReal(dVertDisplace, 9, 6)
            << "   " << fmt::format(SF12_4E, dRotDisplace);

        bool bNeg = false;
        if (dRotDisplace < 0.0)
            bNeg = true;

        // Convert rotation angle radians to degrees...
        double dDegrees = fabs(dRotDisplace) * 180.0 * M_1_PIl;

        short int iDegree = (int) floor(dDegrees);
        double dDegmin = (dDegrees - iDegree) * 60;
        short int iDegmin = (int) floor(dDegmin);
        double dDegsec = (dDegmin - iDegmin) * 60;

        if (bNeg)
            iDegree = -iDegree;
        this->osOutFile
            << "  " << std::setw(4) << iDegree
            << " " << std::setw(2) << iDegmin
            << "'" << fmt::format(SF7_3f, dDegsec) << "\"\n";
    }
    this->osOutFile << "\n";
}

void Report::reportLoadCaseNext(void)
{
    if ( this->bNotTerminalOut ) // ...if the general output is not to the terminal output...
    {
        if ( ! this->bQuiet )
        {
            // Reset to Load Case Number...
            std::cout << std::setw(TAGLINE_WIDTH) << " " // ...clear stage note, then backup to LOAD CASE...
                      << std::setfill(Report::cBackup) << std::setw(15 + TAGLINE_WIDTH) << Report::cBackup << std::setfill(' ')
                      << std::setw(15) << " " // ...clear LOAD CASE...
                      << std::setfill(Report::cBackup) << std::setw(15) << Report::cBackup << std::setfill(' ');
        }
    }
}

std::string Report::formatReal(double dValue, unsigned long int uliField, unsigned long int uliPrecision)
{

    std::string strRet, strUnable = "********";

    strRet = fmt::format("{: {}.{}f}", dValue, uliField, uliPrecision);
    if (strRet.size() > uliField)
    {
        if (uliField < 8)
            return fmt::format("{:*>{}}", '*', uliField);
        uliPrecision = uliField - 8;
        return fmt::format("{: #{}.{}E}", dValue, uliField, uliPrecision);
    }
    else
        return strRet;
}
