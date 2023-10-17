/*
 * StiffnessMatrix.hpp
 *
 *  Created on: Feb 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef STIFFNESSMATRIX_HPP_
#define STIFFNESSMATRIX_HPP_

#include "Member.hpp"

#include <string>

// System Stiffness Matrix (S Matrix)...

class StiffnessMatrix
{
public:
    StiffnessMatrix(void);
    ~StiffnessMatrix(void);
    void clear(void);

    bool create(short int, short int, std::vector<Member *> &);
    short int getMatrixSize(void);
    short int getBandwidth(void);
    double * getDiagonal(void);
    bool decompose(void);
    std::string report();
    void calcDisplacement(double *, double *);

private:
    double * adLocation;            // pointer to first Matrix location
    double * adDiagonal;            // pointer to Matrix diagonal (for Column)
    unsigned int uiArraySize;       // # of elements in the Square Matrix
    short int siMatrixSize;         // Matrix size (the Square Matrix side length)
    short int siBandwidth;          // Banded Matrix upper bandwidth
    short int siCut;                // Cut = MatrixSize - Bandwidth, the unused triangular matrix portion

    unsigned int position(short int, short int);
};

#endif /* STIFFNESSMATRIX_HPP_ */
