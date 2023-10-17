/*
 * StiffnessMatrix.cpp
 *
 *  Created on: Aug 14, 2020
 *      Author: Keven L. Ates
 */

#include "StiffnessMatrix.hpp"
#include "Member.hpp"

#include "SystemDef.hpp"

#include <iostream>
#include <fmt/format.h>

StiffnessMatrix::StiffnessMatrix(void)
{
    this->adLocation = nullptr;
    this->adDiagonal = nullptr;
    this->clear();
}

StiffnessMatrix::~StiffnessMatrix(void)
{
    this->clear();
}

void StiffnessMatrix::clear(void)
{
    if (this->adLocation != nullptr)
        delete this->adLocation;
    this->adLocation = nullptr;

    if (this->adDiagonal != nullptr)
        delete this->adDiagonal;
    this->adDiagonal = nullptr;

    this->uiArraySize = 0;
    this->siMatrixSize = 0;
    this->siBandwidth = 0;
    this->siCut = 0;
}

//***************************************************************************
//*
//* StiffnessMatrix::create
//*     This function creates the System Stiffness Matrix (SSM) storage given
//*     its matrix size and upper bandwidth.  It populates the matrix with
//*     the structure's given members.
//*
//***************************************************************************

bool StiffnessMatrix::create(short int siNewMatrixSize, short int siNewBandwidth, std::vector<Member *> & members)
{
    //************************************************************
    //* Set up the System Stiffness Matrix...
    //************************************************************

    this->siMatrixSize = siNewMatrixSize;
    this->siBandwidth = siNewBandwidth;
    this->siCut = this->siMatrixSize - this->siBandwidth;

    // Original Formula:
    // ArraySize = Bandwidth * Cut + ( Bandwidth ^ 2 + Bandwidth ) / 2
    //
    // Must insure whatever is divided by 2 is even,
    //   otherwise the integer calculation will fail!!!
    //
    // A = B * C + ( B^2 + B ) / 2
    //   First Reduction:
    //   = B * ( M - B ) + ( B^2 + B ) / 2
    //   = B * ( M - B ) + B * ( B + 1 ) / 2
    //   = B * ( M - B + ( B + 1 ) / 2 )
    //   = B * ( 2M - 2B + B + 1 ) / 2
    //   = B * ( 2M - B + 1 ) / 2
    //   = B * ( M + M - B + 1 ) / 2
    //   = B * ( M + C + 1 ) / 2
    //   Second Reduction:
    //   = B * C + ( B * ( B + 1 ) ) / 2
    //   = B * ( C + ( B + 1 ) / 2 )
    //   = B * ( 2C + B + 1 ) / 2
    //   = B * ( C + C + B + 1 ) / 2
    //   = B * ( C + M + 1 ) / 2
    //
    // Both are equivalent:
    //   = ( B * ( M + C + 1 ) ) / 2
    //
    // If M odd,  B odd,  then C even => O * ( O + E + 1 ) = O * ( O + 1 ) = O * E = E
    // If M odd,  B even, then C odd  => E * ( O + O + 1 ) = E * ( E + 1 ) = E * O = E
    // If M even, B odd,  then C odd  => O * ( E + O + 1 ) = O * ( O + 1 ) = O * E = E
    // If M even, B even, then C even => E * ( E + E + 1 ) = E * ( E + 1 ) = E * O = E

    this->uiArraySize = ( (unsigned int) this->siBandwidth *
                          (unsigned int) ( this->siMatrixSize + this->siCut + 1 )
                        ) / 2;

    // DEBUG: Band Matrix Integer Math...
    // ----------------------------------------
    //unsigned int uiArrayTest = ( (unsigned int) ( this->siCut + this->siMatrixSize + 1 ) / 2 ) *
    //                           (unsigned int) this->siBandwidth;
    //std::cerr << "\n DEBUG: BAND MATRIX INTEGER MATH!\n"
    //          <<   "        Matrix Side = " << this->siMatrixSize << ",\n"
    //          <<   "          Bandwidth = " << this->siBandwidth  << ",\n"
    //          <<   "                Cut = " << this->siCut        << ",\n"
    //          <<   "         Array Size = " << this->uiArraySize  << ",\n"
    //          <<   "         Array Test = " << uiArrayTest        << "\n";
    // ----------------------------------------

    // Allocate SSM Location Array and clear to 0...
    this->adLocation = new double[this->uiArraySize]();
    if (this->adLocation == nullptr)
    {
        std::cerr << "\n ERROR: OUT OF MEMORY!\n"
                  <<   "        System Stiffness Matrix too large!\n"
                  <<   "        Matrix Side = " << this->siMatrixSize << ",\n"
                  <<   "          Bandwidth = " << this->siBandwidth  << ",\n"
                  <<   "                Cut = " << this->siCut        << ",\n"
                  <<   "         Array Size = " << this->uiArraySize  << "\n";
        this->clear();
        return false;
    }

    //************************************************************
    //* Load the System Stiffness Matrix...
    //************************************************************

    for (Member * memberCurr : members)
    {
        double adSSM_Local[6][6];

        // Calculate the Local Member Stiffness Matrix...
        memberCurr->calcStiffnessMatrix(adSSM_Local);

        //*
        //* Set up Local to Global Matrix Conversion values...
        //************************************************************

        double adSSM_LocalToGlobal[6][6];
        for (short int siIndex = 0; siIndex < 6; siIndex++)
        {
            adSSM_LocalToGlobal[siIndex][0] = adSSM_Local[siIndex][0] * memberCurr->adNodeVectorNeg[X] -
                                              adSSM_Local[siIndex][1] * memberCurr->adNodeVectorNeg[Y];
            adSSM_LocalToGlobal[siIndex][1] = adSSM_Local[siIndex][0] * memberCurr->adNodeVectorNeg[Y] +
                                              adSSM_Local[siIndex][1] * memberCurr->adNodeVectorNeg[X];
            adSSM_LocalToGlobal[siIndex][2] = adSSM_Local[siIndex][2];
            adSSM_LocalToGlobal[siIndex][3] = adSSM_Local[siIndex][3] * memberCurr->adNodeVectorPos[X] -
                                              adSSM_Local[siIndex][4] * memberCurr->adNodeVectorPos[Y];
            adSSM_LocalToGlobal[siIndex][4] = adSSM_Local[siIndex][3] * memberCurr->adNodeVectorPos[Y] +
                                              adSSM_Local[siIndex][4] * memberCurr->adNodeVectorPos[X];
            adSSM_LocalToGlobal[siIndex][5] = adSSM_Local[siIndex][5];
        }

        double adSSM_Global[6][6];
        for (short int siIndex = 0; siIndex < 6; siIndex++)
        {
            adSSM_Global[0][siIndex] = adSSM_LocalToGlobal[0][siIndex] * memberCurr->adNodeVectorNeg[X] -
                                       adSSM_LocalToGlobal[1][siIndex] * memberCurr->adNodeVectorNeg[Y];
            adSSM_Global[1][siIndex] = adSSM_LocalToGlobal[0][siIndex] * memberCurr->adNodeVectorNeg[Y] +
                                       adSSM_LocalToGlobal[1][siIndex] * memberCurr->adNodeVectorNeg[X];
            adSSM_Global[2][siIndex] = adSSM_LocalToGlobal[2][siIndex];
            adSSM_Global[3][siIndex] = adSSM_LocalToGlobal[3][siIndex] * memberCurr->adNodeVectorPos[X] -
                                       adSSM_LocalToGlobal[4][siIndex] * memberCurr->adNodeVectorPos[Y];
            adSSM_Global[4][siIndex] = adSSM_LocalToGlobal[3][siIndex] * memberCurr->adNodeVectorPos[Y] +
                                       adSSM_LocalToGlobal[4][siIndex] * memberCurr->adNodeVectorPos[X];
            adSSM_Global[5][siIndex] = adSSM_LocalToGlobal[5][siIndex];
        }

        //*
        //* Store Global Member Stiffness Matrix into System Stiffness Matrix...
        //************************************************************

        short int asiPositionIndex[6];
        asiPositionIndex[0] = memberCurr->nodeNeg->siHorzFlag;
        asiPositionIndex[1] = memberCurr->nodeNeg->siVertFlag;
        asiPositionIndex[2] = memberCurr->nodeNeg->siRotFlag;
        asiPositionIndex[3] = memberCurr->nodePos->siHorzFlag;
        asiPositionIndex[4] = memberCurr->nodePos->siVertFlag;
        asiPositionIndex[5] = memberCurr->nodePos->siRotFlag;

        for (short int siIndex1 = 0; siIndex1 < 6; siIndex1++)
        {
            if (asiPositionIndex[siIndex1]) // ...element is FREE...
            {
                for (short int siIndex2 = siIndex1; siIndex2 < 6; siIndex2++)
                {
                    if (asiPositionIndex[siIndex2]) // ...element is FREE...
                    {
                        // ...set matrix position...
                        unsigned int siPosition =
                                this->position( asiPositionIndex[siIndex1] - 1,
                                                asiPositionIndex[siIndex2] - 1 );
                        // ...set matrix position value to selected SSM Global value...
                        this->adLocation[siPosition] += adSSM_Global[siIndex1][siIndex2];
                    }
                }
            }
        }
    }

    //************************************************************
    //* Store System Stiffness Matrix Diagonal for use in Column
    //* Calculations...
    //************************************************************

    // Allocate SSM Diagonal Array and clear to 0...
    double * adDiagonalNew = new double[siNewMatrixSize]();
    if (adDiagonalNew == nullptr)
    {
        std::cerr << "\n ERROR: OUT OF MEMORY!\n"
                  <<   "        Diagonal too large!\n"
                  <<   "        Diagonal = Matrix Side = " << this->siMatrixSize << "\n";
        this->clear();
        return false;
    }
    // Copy SSM Location Array's diagonal to SSM Diagonal Array...
    for (short int siIndex = 0; siIndex < siNewMatrixSize; siIndex++)
    {
        adDiagonalNew[siIndex] = this->adLocation[ this->position(siIndex, siIndex) ];
    }
    this->adDiagonal = adDiagonalNew;

    return true;
}

//*
//* End of StiffnessMatrix::create
//***************************************************************************

short int StiffnessMatrix::getMatrixSize(void)
{
    return this->siMatrixSize;
}

short int StiffnessMatrix::getBandwidth(void)
{
    return this->siBandwidth;
}

double * StiffnessMatrix::getDiagonal(void)
{
    return this->adDiagonal;
}

//***************************************************************************
//*
//* StiffnessMatrix::position
//*     This function calculates the position in the linear array representing
//*     matrix array given a row and column. Since the matrix is
//*     symmetric and only the upper half of the matrix is stored,
//*     the row and column are switched if the column is less than
//*     the row.
//*
//***************************************************************************

unsigned int StiffnessMatrix::position(short int siRow, short int siCol)
{
    // Upper Band Matrix = Condensed Band Array = Bandwidth * ( 2 * Matrix Size - Bandwidth - 1) / 2
    // x x x x 0 0 0 0 0     x x x x
    // 0 x x x x 0 0 0 0     x x x x
    // 0 0 x x x x 0 0 0     x x x x
    // 0 0 0 x x x x 0 0  =  x x x x
    // 0 0 0 0 x x x x 0     x x x x
    // 0 0 0 0 0 x x x x     x x x x
    // 0 0 0 0 0 0 x x x     x x x
    // 0 0 0 0 0 0 0 x x     x x
    // 0 0 0 0 0 0 0 0 x     x

    if (siCol < siRow)
    {   // SWAP...
        short int temp = siRow;
        siRow = siCol;
        siCol = temp;
    }

    unsigned int uiRowOffset;
    //if (siRow > (this->siMatrixSize - this->siBandwidth))
    if (siRow > this->siCut)
    {
        unsigned int uiRowLength = this->siMatrixSize - siRow;
        uiRowOffset = this->uiArraySize - (uiRowLength * uiRowLength + uiRowLength) / 2;
    }
    else
    {
        // uiRowLength = this->siBandwidth;
        uiRowOffset = this->siBandwidth * siRow;
    }

    // Position = Row Offset + Column Offset
    return uiRowOffset + (siCol - siRow);
}

//*
//* End of StiffnessMatrix::position
//***************************************************************************

//***************************************************************************
//*
//* StiffnessMatrix::decompose
//*     This function decomposes the structure's K Matrix into the
//*     A Matrix.
//*
//***************************************************************************

bool StiffnessMatrix::decompose()
{
    unsigned int uiPos[4];

    for (short int siRow2 = 0; siRow2 < (this->siMatrixSize - 1); siRow2++)
    {
        for (short int siRow1 = (siRow2 + 1); (siRow1 - siRow2) < this->siBandwidth && siRow1 < this->siMatrixSize; siRow1++)
        {
            for (short int siCol = siRow1; siCol < (this->siBandwidth + siRow2) && siCol < this->siMatrixSize; siCol++)
            {
                uiPos[0] = this->position(siRow1, siCol);
                uiPos[1] = this->position(siRow2, siRow1);
                uiPos[2] = this->position(siRow2, siCol);
                uiPos[3] = this->position(siRow2, siRow2);
                if (this->adLocation[uiPos[3]] != 0.0)
                    this->adLocation[uiPos[0]] -= (this->adLocation[uiPos[1]] * this->adLocation[uiPos[2]] / adLocation[uiPos[3]]);
                else
                {
                    std::cerr << "\n WARNING: Stiffness Matrix is SINGULAR!\n"
                              <<   "          Unable to find solution for Plane Frame!\n";
                    return false;
                }
            }
        }
    }

    return true;
}

//*
//* End of StiffnessMatrix::decompose
//***************************************************************************

std::string StiffnessMatrix::report()
{
    std::string strFormat;
    strFormat = strFormat +
        "      Cut  Matrix    Band  |      Array    Band  Matrix     Cut\n" +
        "     Size    Side    Size  |       Size    Size    Side    Size\n" +
        "   ------  ------  ------  |  ---------  ------  ------  ------\n" +
        "   {:6d} ={:6d} -{:6d}  |  {:9d} ={:6d} ({:6d} +{:6d} + 1 ) / 2\n";
    return fmt::vformat(
                strFormat,
                fmt::make_format_args(
                    this->siCut, this->siMatrixSize, this->siBandwidth,
                    this->uiArraySize, this->siBandwidth, this->siMatrixSize, this->siCut
                )
            );
}

//***************************************************************************
//*
//* StiffnessMatrix::displacement
//*     This function calculates the force displacements of the structure's
//*     K Matrix into the A Matrix.
//*
//***************************************************************************

void StiffnessMatrix::calcDisplacement(double * adDisplace, double * adForce)
{
    // Decompose Force Matrix...

    for (short int siRow = 0; siRow < (this->siMatrixSize - 1); siRow++)
    {
        short int siMaxCol = this->siBandwidth + siRow;
        if (siMaxCol > this->siMatrixSize)
            siMaxCol = this->siMatrixSize;
        for (short int siCol = (siRow + 1); siCol < siMaxCol; siCol++)
        {
            unsigned int uiPos = this->position(siRow, siCol);
            short int siDiagonal = this->position(siRow, siRow);
            if (this->adLocation[siDiagonal] != 0.0)
                adForce[siCol] -=
                        (this->adLocation[uiPos] * adForce[siRow] /
                                this->adLocation[siDiagonal]);
        }
    }

    // Calculate System Displacements...

    // Start with last diagonal value...
    short int siRow = this->siMatrixSize - 1;
    adDisplace[siRow] = adForce[siRow] /
                        this->adLocation[ this->position(siRow, siRow) ];
    for (siRow = (this->siMatrixSize - 2); siRow >= 0 ; siRow--)
    {
        short int siMaxCol = this->siBandwidth + siRow;
        if (siMaxCol > this->siMatrixSize)
            siMaxCol = this->siMatrixSize;
        double dSum = 0.0;
        for (short int siCol = (siRow + 1); siCol < siMaxCol; siCol++)
        {
            dSum += ( this->adLocation[ this->position(siRow, siCol) ] * adDisplace[siCol] );
        }
        adDisplace[siRow] = (adForce[siRow] - dSum) /
                            this->adLocation[ this->position(siRow, siRow) ];
    }
}

//*
//* End of StiffnessMatrix::displacement
//***************************************************************************
