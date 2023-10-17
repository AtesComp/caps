# Comprehensive Analyzer for Plane Structures

## Introduction
The purpose of this program is to model stress applied to a 'plane frame' structure.  It uses the Displacement (or Stiffness) Matrix Method to solve an indeterminate structure model.

The 'plane frame' structure model is a two dimensional theoretical frame that exists completely in a single plane. Forces applied to the 'plane frame' result in computed stresses tabulated for review.

This program is intended to emulates the original PPSA process closely.

This program is intended to be used as an engineering tool. It assists the user in the analysis of 'plane frame' wood structures. Accuracy of the analog and interpretation of the structural adequacy are the responsibility of the user. The authors assume no responsibility, explicit or implied.

*** USE AT YOUR OWN RISK!!! ***

## File Formats

There are two (2) input formats.

1. PPSA Standard format - This is a column specific input as described by the PPSA documentation.

2. PPSA FREE format - This is a free form input as described by the PPSA documentation.

There is one (1) output format.

1. Output: The report output closely follows the PPSA documentation with a few significant changes.

## Restrictions

An input file is read as character strings and parsed for input values. The standard column based format is converted to the FREE format for processing. The process parses the data for analysis.

**Standard Input** follows the PPSA documentation. Values are limited to the documented column sizes.

**FREE Input** all values are separated by white space as described in the PPSA documentation. Values sizes are limited by the codes variable declarations.

**Problem Size** is limited by memory.

**Member Properties** are limited by memory.
1. The Member Property Type (MTYPE) must be a valid type (see PPSA documentation: blank, 0-9, A-D).
2. Normal and Composite properties may have fictitious members defined by a zero for the Allowable Compression Stress (Fc). The Allowable Bending Stress determines whether Shear Modulus calculations will be used for a fictitious member:
   a. A Shear Modulus value of 9000 indicates the Shear Modulus calculations are applied.
   b. For any other value, Shear Modulus is not calculated.
3. Cross-Sectional properties do not have fictitious members. Shear Modulus calculations are determined by the Shear Modulus value:
   a. A Shear Modulus value other than zero (0) indicates that Shear Modulus calculations are applied.
   b. A zero value indicates no Shear Modulus calculation.

**Stress Factors** are limited to three (3) values.

**Node Coordinates** are limited by memory.

**Structural Assembly Members** are limited by memory. A Volume Factor of one (1.0) is used for all materials. It can only be changed if option OPT-C is used and a value is entered for the Volume Factor. The Volume Factor must be > 0.0 and <= 1.0.

**Reaction Nodes** are limited by memory.

**Load Information and Interaction Interpretation** are limited to 8 values. The Interaction Equation Index values of 0 to 3 indicate NDS 91 rules. Values of 4 to 7 indicate the older NDS 86 rules. The Trapezoidal Loads Flag is read and set whether option OPT-3 was given or not--it defaults to zero.

**Concentrated Loads** are limited by memory.

**Uniform Loads** are limited by memory. For a given member, only the first uniform load defined for it is applied. Any others are ignored.

**Nodal Loads** are limited by memory. Each load is added to the nodes respective X, Y, or Moment.

**Trapezoidal Loads** are limited by memory.
