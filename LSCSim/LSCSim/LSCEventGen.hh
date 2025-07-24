// Event generator

#ifndef __LSCEvtGen_hh__
#define __LSCEvtGen_hh__ 

#include <vector>

#include "G4ThreeVector.hh"
#include "G4LorentzVector.hh"
#include "Randomize.hh"

#include "GLG4Sim/GLG4VertexGen.hh"
#include "GLG4Sim/GLG4param.hh"

typedef struct _Form_HEPEvt {
    int     ISTHEP; // status code
    long    IDHEP;  // HEP PDG code
    int     JDAHEP1; // first daughter
    int     JDAHEP2; // last daughter
    double  PHEP1;  // px in GeV
    double  PHEP2;  // py in GeV
    double  PHEP3;  // pz in GeV
    double  PHEP5;  // mass in GeV                                                        
    double  DT;     // vertex _delta_ time, in ns (*)                                     
    double  X;      // x vertex in mm                                                     
    double  Y;      // y vertex in mm                                                     
    double  Z;      // z vertex in mm                                                     
    double  PLX;    // x polarization                                                     
    double  PLY;    // y polarization                                                     
    double  PLZ;    // z polarization                                                     
} Form_HEPEvt;


class LSCEventGen : public GLG4VVertexGen 
{
public:
    LSCEventGen(const char * arg_dbname=""); 
    ~LSCEventGen() {}
    
    virtual void GeneratePrimaryVertex(G4Event * argEvent) = 0;
    virtual void SetState(G4String newValues) = 0;
    virtual G4String GetState() = 0;

    void GeneratePosition();
    G4ThreeVector GeneratePosition_cylinder(double R, double H);
    G4ThreeVector GeneratePosition_sphere(double R);
    virtual void SetForm_HEPEvt() = 0;
    void Print_HEPEvt();

    void ReadGeometryFile(G4String fn);

    // set functions
    void SetSeed(long seed=-1);
    void SetPositionSource(G4ThreeVector pos) { _pos_source = pos; }
    void SetDetShape(G4String detshape) { _det_shape = detshape; }

    // get functions
    long GetSeed() const { return _rseed; }
    G4ThreeVector GetPosition() const { return _pos; }
    G4ThreeVector GetPositionSource() const { return _pos_source; }

    int GeneratePosition_sourceMC(double L);
    double GetLmin();

protected:
    std::vector<Form_HEPEvt> _evt;
    G4ThreeVector _pos;
    G4ThreeVector _pos_source;
    G4String _fn_geometry;
    GLG4param& _geom_db = GLG4param::GetDB();
    G4String _det_shape;

private:
    long _rseed = 42;
};


class LSCEventGen_IBD : public LSCEventGen 
{
public:
    LSCEventGen_IBD(const char * arg_dbname="");
    ~LSCEventGen_IBD() {}

    virtual void GeneratePrimaryVertex(G4Event * argEvent);
    virtual void SetState(G4String newValues) {}
    virtual G4String GetState() { return ""; }

    void GenerateEvent(double Ev, G4ThreeVector uv, double theta=-1);
    void SetForm_HEPEvt();

    double GetEe1(double T, double theta);
    double GetDifCrossSection(double E, double theta);
    double GetDifCrossSection_costheta(double E, double costheta);
    
    void SetPDF(double E);
    
private:
    double _PDF[721];

    G4LorentzVector _pv0;  // momentum of the incoming neutrino
    G4LorentzVector _pp0;  // proton
    G4LorentzVector _pe1;  // positron
    G4LorentzVector _pn1;  // neutron
};


class LSCEventGen_ve : public LSCEventGen 
{
public:
    LSCEventGen_ve(const char * arg_dbname="");
    ~LSCEventGen_ve() {}

    virtual void GeneratePrimaryVertex(G4Event * argEvent);
    virtual void SetState(G4String newValues) {}
    virtual G4String GetState() { return ""; }

    void GenerateEvent(double Ev, G4ThreeVector uv, double theta=-1);
    void SetForm_HEPEvt();

    double GetEe1(double T, double theta);
    double DiLog(double y);

    double GetDifCrossSection(double E, double theta, G4String vflavour);
    double GetDifCrossSection_T(double E, double T, G4String vflavour);
    double GetDifCrossSection_costheta(double E, double costheta, G4String vflavour);

    void SetPDF(double E, G4String vflavour);

private:
    double _PDF[721];

    G4String _vflavour;

    G4LorentzVector _pv0; // momentum of the incoming neutrino
    G4LorentzVector _pe0; // initial electron
    G4LorentzVector _pv1; // neutrino
    G4LorentzVector _pe1; // electron
};

#endif

