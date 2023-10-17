/*
 * Report.hpp
 *
 *  Created on: Aug 25, 2020
 *      Author: Keven L. Ates
 */

#ifndef REPORT_HPP_
#define REPORT_HPP_


#include <fstream>
#include <ostream>
#include <string>

#define TAGLINE_WIDTH 40

class PlaneFrame;

class Report
{
public:
    Report(void);
    Report(std::string &);
    void setQuiet(void);
    bool checkReport(void);
    void setPlaneFrame(PlaneFrame *);
    void setTitle(short int, std::string, std::string, std::string);
    void setLoadCase(short int);

    PlaneFrame * getPlaneFrame(void);

    static void reportPreambleDirect(void);
    void reportPreamble(void);
    void reportTitle(void);

    void reportStructureReading(void);
    void reportStructure(void);

    void reportLoadCaseReading(void);
    void reportLoadCase(bool, bool);
    void reportLoadCaseProcessing(void);
    void reportLoadCaseLoading(void);

    void reportCalcDisplacementMatrix(void);
    void reportCalcMemberForces(void);
    void reportCalcActions(void);
    void reportAnalyzeStructure(void);
    void reportCalcDeflections(void);

    void reportResults(void);

    void reportLoadCaseNext(void);

    void close(void);

private:
    static const char cBackup;
    static const std::string strNoLoad;
    static const std::string strHeading;

    std::ofstream * pofsOutFile = nullptr;
    std::ostream & osOutFile;
    std::string strOutFile;

    bool bQuiet = false;
    PlaneFrame * pframe = nullptr;
    bool bNotTerminalOut = true;
    short int siTitleIndex = 0;
    std::string strTitleFormat;
    std::string strTitleID;
    std::string strTitleOpt;
    short int siLoadCase = 0;

    void reportPreamble(std::ostream &);
    void reportTitle(std::ostream &);

    void reportStructureMaterialProperties(void);   // Table 1
    void reportStructureNodeCoordinates(void);      // Table 2
    void reportStructureMemberAssemblies(void);     // Table 3
    void reportStructureReactions(void);            // Table 4

    void reportStructureSSM(void);                  // Table 5

    void reportLoadCaseLoads(void);                 // Table 6
    void reportLoadCaseLoadsPoint(void);            // Table 6 A
    void reportLoadCaseLoadsUniform(void);          // Table 6 B
    void reportLoadCaseLoadsNodal(void);            // Table 6 C
    void reportLoadCaseLoadsTrap(void);             // Table 6 D

    void reportResultsReactions(void);              // Table 7
    void reportResultsEndActions(void);             // Table 8
    void reportResultsStressAxialBend(void);        // Table 9
    void reportResultsStressShear(void);            // Table 10
    void reportResultsDeflection(void);             // Table 11
    void reportResultsDisplacement(void);           // Table 12

    std::string formatReal(double, unsigned long int, unsigned long int );
};

#endif /* REPORT_HPP_ */
