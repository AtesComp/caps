/*
 * SystemDef.hpp
 *
 *  Created on: Oct 6, 2020
 *      Author: Keven L. Ates
 */

#ifndef SYSTEMDEF_HPP_
#define SYSTEMDEF_HPP_

//-----------------------------------
// Basic Definitions...
//-----------------------------------

#define X 0
#define Y 1

// pi = 3.14159265358979323846264338327950288419716939937510;

// 1 16th = 1.5875 mm EXACT
// 1 in = 25.4 mm EXACT
// 1 in = 2.54 cm EXACT
// 1 in = 0.0254 m EXACT
// 1 ft = 0.3048 m EXACT
// 1 yd = 0.9144 m EXACT
#define MillimeterPer16th 1.5875
#define MillimeterPerInch 25.4
#define CentimeterPerInch 2.54
#define MeterPerInch 0.0254
#define MeterPerFoot 0.3048
#define MeterPerYard 0.9144
// 1 mm = 0.0393700787401574803149606 in
// 1 cm = 0.393700787401574803149606 in
// 1 m  = 39.3700787401574803149606 in
// ft - in - 16ths
//  m - cm - mm

// 1 kg = 2.20462262184877580722974 lbs
// 1 lb = 0.45359237 kg EXACT
#define KilogramPerPound 0.45359237
// 1 g = 9.80665 m / s^2 EXACT?
#define GravitySI 9.80665
// 1 N = 1 kg m⋅ / s^2
// 1 lbf = 1 lb g = 4.4482216152605 N
// 1 N = 0.224808943099710482910039 lbf
// 1 kgf = 1 kg g = 9.80665 N
// 1 N = 0.101971621297792824257009 kgf
// 1 Pa = 1 N / m^2 = 1 kg / (m s^2) = 0.22480894244319 lbs / (39.3700787401 in)^2
// 1 psi = 1 lbf / in^2
// 1 psi = 6894.75729316836133672267 Pa
// 1 psi = 6.89475729316836133672267 kPa
#define kPaPerPsi 6.89475729316836133672267
// 1 MPa = 145.037737730209215154241 psi
// 1 kPa = 0.145037737730209215154241 psi

// Modulus of Elasticity = psi OR kPa
// Section Modulus = in^3 OR mm^3
// Moment Bending = lbf ft OR N m
// Stress Axial = psi OR kPa
// Stress Shear = psi OR kPa

#endif /* SYSTEMDEF_HPP_ */
