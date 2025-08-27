#ifndef TESTS_H
#define TESTS_H


void TestGEOS();
void TestGEOS_AVX();
void TestGEOS_Neon();

void TestReprojectEqToMerc();
void TestReprojectEqToMerc_AVX();
void TestReprojectEqToMerc_Neon();

void TestReprojectAEQDToMerc();
void TestReprojectAEQDToMerc_AVX();

void TestReprojectLambertToEq();

void TestReprojectionMercToPolar();

void TestOblique();

void TestWrapAround();

void TestCalculations();

#endif
