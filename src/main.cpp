//******************************************************************************
//     Comprehensive Analyzer for Plane Structures
//
// COPYRIGHT: 2020
//
// AUTHOR: Keven L. Ates
//
// PURPOSE:
//
//     The purpose of this program is to model stress applied to a
//     'plane frame' structure.  It uses the Displacement (or Stiffness)
//     Matrix Method to solve an indeterminate structure model.
//
//     The 'plane frame' structure model is a two dimensional theoretical
//     frame that exists completely in a single plane.  Forces applied
//     to the 'plane frame' result in computed stresses tabulated for
//     review.
//
//     It emulates the original PPSA process closely.
//
//     This program is intended to be used as an engineering tool.  It assists
//     the user in the analysis of 'plane frame' wood structures.
//
//******************************************************************************

#include "PPSAFile.hpp"
#include "Report.hpp"
#include "PlaneFrame.hpp"

#include <getopt.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <ostream>


void help(int iHelp)
{
    if (iHelp > 1)
    {
        Report::reportPreambleDirect();
    }
    std::cout << std::string() +
            "USAGE: caps [[-i|--input] <infile>] [[-o|--output] <outfile>] [-h|-?|--help]\n" +
            "\n" +
            "OPTIONS: Processed before all non-options\n" +
            "\n" +
            "  -i <file>         Input file - PPSA formatted input.\n" +
            "  --input <file>      A '-' as the input file processes input\n" +
            "                      from stdin. Default: IN.DAT\n" +
            "                      The last option input overrides previous option inputs.\n" +
            "                      If no input option is specified, the first non-option\n" +
            "                      is treated as the input file.\n" +
            "                      (see additional help for format information)\n" +
            "\n" +
            "  -o <file>         Output file - tabular results file.\n" +
            "  --output <file>     A '-' as the output file prints the results\n" +
            "                      to stdout. Default: OUT.DAT\n" +
            "                      The last output option overrides previous output options.\n" +
            "                      The last non-option is always treated as the output file.\n" +
            "\n" +
            "  -h, -?            Print this help\n" +
            "  --help              Multiple help options expands the help with\n" +
            "                      additional information. Max: 3\n" +
            "\n" +
            "PURPOSE:\n" +
            "\n" +
            "The purpose of this program is to model stress applied to a\n" +
            "'plane frame' structure.  It uses the Displacement (or Stiffness)\n" +
            "Matrix Method to solve an indeterminate structure model.\n" +
            "\n" +
            "The 'plane frame' structure model is a two dimensional theoretical\n" +
            "frame that exists completely in a single plane.  Forces applied\n" +
            "to the 'plane frame' result in computed stresses tabulated for\n" +
            "review.\n" +
            "\n" +
            "This program is intended to emulates the original PPSA process closely.\n" +
            "\n" +
            "This program is intended to be used as an engineering tool.  It assists the\n" +
            "user in the analysis of 'plane frame' wood structures.  Accuracy of the analog\n" +
            "and interpretation of the structural adequacy are the responsibility of the\n" +
            "user.  The authors assume no responsibility, explicit or implied.\n" +
            "*** USE AT YOUR OWN RISK!!! ***\n";
    if (iHelp > 1)
    {
        std::cout << std::string("\n") +
            "FILE FORMATS:\n" +
            "\n" +
            "There are two (2) input formats and one (1) output format.\n" +
            "\n" +
            "Input 1: PPSA Standard format\n" +
            "    This is a column specific input as described by the\n" +
            "    PPSA documentation.\n" +
            "\n" +
            "Input 2: PPSA FREE format\n" +
            "    This is a free form input as described by the PPSA\n" +
            "    documentation.\n" +
            "\n" +
            "Output:\n" +
            "    The report output closely follows the PPSA documentation.\n";
    }
    if (iHelp > 2)
    {
        std::cout << std::string("\n") +
            "RESTRICTIONS:\n" +
            "\n" +
            "An input file is read as character strings and parsed for input\n" +
            "values.  The standard column based format is converted to the\n" +
            "FREE format for processing.  The process parses the data for\n" +
            "analysis.\n" +
            "\n" +
            "Standard Input: follows the PPSA documentation.  Values are\n" +
            "  limited to the documented column sizes.\n" +
            "\n" +
            "FREE Input: all values are separated by white space as described\n" +
            "  in the PPSA documentation.  Values sizes are limited by the\n" +
            "  codes variable declarations.\n" +
            "\n" +
            "Problem Size: is limited by memory.\n" +
            "\n" +
            "Member Properties: are limited by memory.\n" +
            "1. The Member Property Type (MTYPE) must be a valid type\n" +
            "   (see PPSA documentation: blank, 0-9, A-D).\n" +
            "2. Normal and Composite properties may have fictitious members\n" +
            "   defined by a zero for the Allowable Compression Stress (Fc).\n" +
            "   The Allowable Bending Stress determines whether Shear Modulus\n" +
            "   calculations will be used for a fictitious member:\n" +
            "   a. A Shear Modulus value of 9000 indicates the Shear Modulus\n" +
            "      calculations are applied.\n" +
            "   b. For any other value, Shear Modulus is not calculated.\n" +
            "3. Cross-Sectional properties do not have fictitious members.\n" +
            "   Shear Modulus calculations are determined by the Shear Modulus\n" +
            "   value:\n" +
            "   a. A Shear Modulus value other than zero (0) indicates that\n" +
            "      Shear Modulus calculations are applied.\n" +
            "   b. A zero value indicates no Shear Modulus calculation.\n" +
            "\n" +
            "Stress Factors: are limited to three (3) values.\n" +
            "\n" +
            "Node Coordinates: are limited by memory.\n" +
            "\n" +
            "Structural Assembly Members: are limited by memory.\n" +
            "  A Volume Factor of one (1.0) is used for all materials.  It can\n" +
            "  only be changed if option OPT-C is used and a value is entered\n" +
            "  for the Volume Factor. The Volume Factor must be > 0.0 and <= 1.0.\n" +
            "\n" +
            "Reaction Nodes: are limited by memory.\n" +
            "\n" +
            "Load Information and Interaction Interpretation:\n" +
            "  The Interaction Equation Index values of 0 to 3 indicate NDS 91\n" +
            "  rules. Values of 4 to 7 indicate the older NDS 86 rules.\n" +
            "  The Trapezoidal Loads Flag is read and set whether option OPT-3\n" +
            "  was given or not--it defaults to zero.\n" +
            "\n" +
            "Concentrated Loads: are limited by memory.\n" +
            "\n" +
            "Uniform Loads: are limited by memory.\n" +
            "  For a given member, only the first uniform load defined for\n" +
            "  it is applied.  Any others are ignored.\n" +
            "\n" +
            "Nodal Loads: are limited by memory.\n" +
            "  Each load is added to the nodes respective X, Y, or Moment.\n" +
            "\n" +
            "Trapezoidal Loads: are limited by memory.\n";
    }
    std::cout << "\n";
    return;
}

//**************************************************************************
//*
//*  MAIN:  This is the main function for the CAPS program. The purpose
//*         of this function is to print the header, validate the input file
//*         and handle the high level processing of the plane analysis.
//*[
//**************************************************************************

int main(int iArgC, char * pacArgV[])
{
    std::string strInFile = "IN.DAT";   // Default input file name
    std::string strOutFile = "OUT.DAT"; // Default output file name

    char cOption;
    bool bHelp = false;
    int iHelp = 0;
    bool bOptContinue = true;
    bool bInFile = false;
    int iRetVal = 0;

    //
    //  Process arguments...
    //

    while (bOptContinue)
    {
        int iOptIndex = 0;
        char strOpts[] = "i:o:h?";
        static struct option optsLong[] =
        {
            { "input",  required_argument, 0, 'i' },
            { "output", required_argument, 0, 'o' },
            { "help",   no_argument,       0, 'h' },
            { 0,        0,                 0, 0 }
        };

        cOption = getopt_long(iArgC, pacArgV, strOpts, optsLong, &iOptIndex);
        if (cOption != -1)
        {
            switch (cOption)
            {
                case 0: // Long Option Only...
                    switch (iOptIndex)
                    {
                        case 0: // input
                            strInFile = optarg;
                            bInFile = true;
                            break;

                        case 1: // output
                            strOutFile = optarg;
                            break;

                        case 2: // help
                            bHelp = true;
                            iHelp++;
                            break;

                    }
                    break;

                case 'i': // input
                    strInFile = optarg;
                    bInFile = true;
                    break;

                case 'o': // output
                    strOutFile = optarg;
                    break;

                case 'h': // help
                case '?': // help
                    bHelp = true;
                    iHelp++;
                    break;

                default: // any other option is invalid
                    bHelp = true;
                    bOptContinue = false;
                    iRetVal = 1;
                    break;
            }
        }
        else
            bOptContinue = false;
    }

    while (optind < iArgC) // more arguments exist...
    {
        // DEBUG
        //std::cout << optind << ": " << pacArgV[optind] << std::endl;

        if (! bInFile) // ...no input file yet...
        {
            strInFile = pacArgV[optind]; // ...interpret argument as an input filename
            bInFile = true;
        }
        else // ...set output file...
            strOutFile = pacArgV[optind]; // ...interpret argument as an output filename

        optind++;
    }

    if (bHelp) // ...HELP!...
    {
        help(iHelp);
        return iRetVal;
    }

    //
    //  Open the Input file...
    //

    PPSAFile * pinPPSA = nullptr;
    if (strInFile == "-") // Input is the standard input (cin)
        pinPPSA = new PPSAFile();
    else
        pinPPSA = new PPSAFile( strInFile );
    if ( ! pinPPSA->checkPPSA() )
    {
        iRetVal = 1;
        help(iHelp);
        return iRetVal;
    }

    //
    //  Open the Output file...
    //

    Report * pOutCAPS = nullptr;
    if (strOutFile == "-") // Output is the standard output (cout)
        pOutCAPS = new Report();
    else
        pOutCAPS = new Report( strOutFile );
    if ( ! pOutCAPS->checkReport() )
    {
        iRetVal = 1;
        pinPPSA->close();
        help(iHelp);
        return iRetVal;
    }

    // Setup Plane Frame and its reporting...
    PlaneFrame pframe;
    pOutCAPS->setPlaneFrame( &pframe );

    //
    //  Print the CAPS Introduction heading...
    //

    pOutCAPS->reportPreamble();

    //
    // Set up main loop...
    //

    short int siCount = 1;
    bool bContinue = true;
    while (bContinue)
    {
        // Read the File Header (Section 1) from file...
        if ( ! pinPPSA->getHeader(pframe) )
        {
            bContinue = false;
            break;
        }

        pOutCAPS->setTitle(siCount, pinPPSA->getFormat(), pinPPSA->strID, pinPPSA->strOpt);
        pOutCAPS->reportTitle();

        // Read the Plane Frame Structure (Sections 2 - 7) from file...
        if ( ! pinPPSA->getStructure(pframe) )
        {
            bContinue = false;
            break;
        }

        pOutCAPS->reportStructureReading();

        // Set up the Plane Frame Structure...
        if ( ! pframe.setup() )
        {
            bContinue = false;
            break;
        }

        // Report the Plane Frame structure (Tables 1 - 5)...
        pOutCAPS->reportStructure();

        //
        // Set up loop for multiple loading conditions...
        //********************************************************************************

        for (short int siLoadCase = 1; siLoadCase <= pframe.ps.siLoads; siLoadCase++)
        {
            // Clear the loads and related load analysis info on the Plane Frame...
            pframe.clearLoads();

            pOutCAPS->setLoadCase(siLoadCase);
            pOutCAPS->reportLoadCaseReading();

            // Read CAPS structure loading (Sections 8 - 12) from file...
            if ( pinPPSA->getLoadCase(pframe) == false )
            {
                bContinue = false;
                break;
            }

            // Report structure Loading Case (Table 5)...
            pOutCAPS->reportLoadCase(pinPPSA->isWarningForTPIConcLoad(), pinPPSA->isWarningForTPITrapLoad() );

            pOutCAPS->reportLoadCaseProcessing();

            // Process the structure loading for External Forces...
            if ( ! pframe.processLoads() )
            {
                bContinue = false;
                break;
            }

            pOutCAPS->reportLoadCaseLoading();

            // Apply the loading to the members and setup Internal Forces...
            if ( ! pframe.loadMembers() )
            {
                bContinue = false;
                break;
            }

            pOutCAPS->reportCalcDisplacementMatrix();

            // Calculate the System Displacement Matrix...
            if ( ! pframe.createDisplacementMatrix() )
            {
                bContinue = false;
                break;
            }

            pOutCAPS->reportCalcMemberForces();

            // Calculate the System Displacement Matrix...
            pframe.calcMemberForces();

            pOutCAPS->reportCalcActions();

            // Calculate and Output member end actions and reactions...
            if ( ! pframe.calcActions() )
            {
                bContinue = false;
                break;
            }

            pOutCAPS->reportAnalyzeStructure();

            // Analyzing the Structure: Find maximum axial, moment, and shear...
            //     Perform NDS Design Calculations...
            pframe.analyzeStructureForces();

            pOutCAPS->reportCalcDeflections();

            // Calculate maximum member deflections...
            if ( ! pframe.calcDeflections() )
            {
                bContinue = false;
                break;
            }

            pOutCAPS->reportResults();

            pOutCAPS->reportLoadCaseNext();

            // Perform another loading condition on the matrix if possible...
        }

        // Clear the Plane Frame information...
        pframe.clear();

        // Update Structure Counter...
        ++siCount;

        // Analyze the next Plane Frame system in the input file...
    }

    // Clear the Plane Frame information...
    pframe.clear();

    //
    // Close the files and exit the program...
    //

    pinPPSA->close();  // Not Req: destructor closes
    pOutCAPS->close(); // Not Req: destructor closes

    return 0;
}

//***************
//* End of MAIN *
//**************************************************************************
