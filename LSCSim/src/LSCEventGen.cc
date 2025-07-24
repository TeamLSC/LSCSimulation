
#include <iostream>
#include <sys/time.h>
#include <ctime>

#include "G4PhysicalConstants.hh"

#include "LSCSim/LSCEventGen.hh"
#include "GLG4Sim/GLG4param.hh"

using namespace std;

LSCEventGen::LSCEventGen(const char * arg_dbname) 
    : GLG4VVertexGen(arg_dbname) 
{
    SetSeed();
    _pos = G4ThreeVector(0, 0, 0);
    _pos_source = G4ThreeVector(0, 0, 0);
}

void LSCEventGen::SetSeed(long seed)
{
    if (seed < 0) {
        struct timeval tv; 
        gettimeofday(&tv, NULL);
        seed = tv.tv_sec*1000 + tv.tv_usec;
    }

    _rseed = seed;
    G4Random::setTheSeed(_rseed);
}

void LSCEventGen::Print_HEPEvt()
{
#if DEBUG
    cerr << "#DEBUG: Print_HEPEvt()" << endl;
#endif

    SetForm_HEPEvt();

    int print_pol = 0;

    if (_evt[0].PLX == 0 && _evt[0].PLY == 0 && _evt[0].PLZ == 0) 
        print_pol = 0;
    else
        print_pol = 1;

    cout << _evt.size() << endl; // number of primary particles

    for (int i=0; i<_evt.size(); i++) {
        cout << _evt[i].ISTHEP  << '\t';
        cout << _evt[i].IDHEP   << '\t';
        cout << _evt[i].JDAHEP1 << '\t';
        cout << _evt[i].JDAHEP2 << '\t';
        cout << scientific;
        cout << _evt[i].PHEP1   << '\t';
        cout << _evt[i].PHEP2   << '\t';
        cout << _evt[i].PHEP3   << '\t';
        cout << _evt[i].PHEP5   << '\t';
        cout << _evt[i].DT      << '\t';
        cout << _evt[i].X       << '\t';
        cout << _evt[i].Y       << '\t';
        cout << _evt[i].Z       << '\t';
        if (print_pol) {
            cout << _evt[i].PLX << '\t';
            cout << _evt[i].PLY << '\t';
            cout << _evt[i].PLZ << '\t';
        }
        cout << endl;
    }

    return;
}

void LSCEventGen::ReadGeometryFile(G4String fn)
{
    _fn_geometry = fn;
    _geom_db.ReadFile(_fn_geometry);

}

void LSCEventGen::GeneratePosition()
{
    double targetR = _geom_db["target_radius"];
    double targetH = _geom_db["target_height"];
    double targetT = _geom_db["target_thickness"];

    targetR *= 10; // cm -> mm
    targetH *= 10; // cm -> mm
    targetT *= 10; // cm -> mm

    double LS_R = targetR - targetT;
    double LS_Hmin = -(targetH/2 - targetT);
    double LS_Hmax = targetH/2 - targetT;

    if (_det_shape == "cylinder") {
        GeneratePosition_cylinder(LS_R, LS_Hmax - LS_Hmin);
    }
    else if (_det_shape == "sphere") {
        GeneratePosition_sphere(LS_R);
    }
}


G4ThreeVector LSCEventGen::GeneratePosition_cylinder(double R, double H)
{
    double LS_R = R;
    double LS_Hmin = -H/2;
    double LS_Hmax = H/2;

    double rho_det = sqrt(G4UniformRand()) * LS_R;
    double phi_det = G4UniformRand() * 2 * pi;
    double z_det = G4UniformRand() * (LS_Hmax - LS_Hmin) + LS_Hmin;

    G4ThreeVector pos;
    pos.setRhoPhiZ(rho_det, phi_det, z_det);
    _pos = pos;

    return pos;
}


G4ThreeVector LSCEventGen::GeneratePosition_sphere(double R)
{
    double x = G4UniformRand() * 2*R - R;
    double y = G4UniformRand() * 2*R - R;
    double z = G4UniformRand() * 2*R - R;

    if (R*R < x*x + y*y + z*z)
        return GeneratePosition_sphere(R);

    G4ThreeVector pos(x, y, z);
    _pos = pos;

    return pos;
}


