#include <iostream>
#include <getopt.h>

#include "GLG4Sim/GLG4param.hh"
#include "LSCSim/LSCEventGen.hh"

using namespace std;

G4String fn_geom   = "geometry.dat";
G4String fn_source = "source.dat";
G4String fn_solar  = "solar.dat";
G4String fn_reactor= "reactor.dat";

G4String detshape = "sphere";

bool exists(const G4String &fname);
int reactor(int nevent, double Ev);
int source(int nevent, double Ev, bool flag_uniform_vtx=false);
int source_Cr51(int nevent, double Ev, bool flag_uniform_vtx=false);
int solar(int nevent, double Ev);
void PrintHelp();

int main(int argc, char** argv)
{
    if (argc < 2) {
        PrintHelp();
        return -1;
    }

    int opt;

    int nevent = 1;
    double Ev = -1;  // neutrino energy

    G4String evt_type = "reactor";
    G4String fn_setting;

    while ((opt = getopt(argc, argv, "t:n:g:e:s:h:d")) != -1) {
        switch (opt) {
            case 't': evt_type = G4String(optarg); break;
            case 'n': nevent = atoi(optarg); break;
            case 'g': fn_geom = G4String(optarg); break;
            case 'e': Ev = atof(optarg); break;
            case 's': 
                if (evt_type == "source") {
                    fn_source = G4String(optarg);
                }
                else if (evt_type == "solar") {
                    fn_solar = G4String(optarg);
                }
                else if (evt_type == "Cr51") {
                    fn_source = G4String(optarg);
                }
                else {
                    fn_reactor = G4String(optarg); 
                }
                break;
            case 'd': detshape= G4String(optarg); break;
            case 'h': PrintHelp(); break;
            default: PrintHelp();
        }
    }

    if      (evt_type == "reactor") reactor(nevent, Ev);
    else if (evt_type == "source")  source (nevent, Ev);
    else if (evt_type == "solar")   solar  (nevent, Ev);
    else if (evt_type == "Cr51")    source_Cr51(nevent, Ev);
    else PrintHelp();

    return 0;
}

int reactor(int nevent, double Ev)
{
    LSCEventGen_IBD* evtgen = new LSCEventGen_IBD();
    evtgen->SetDetShape(detshape);
    
    bool rndEv = false;
    if (Ev == -1) rndEv = true;

    double theta = -1;
    G4ThreeVector uv(1, 0, 0); // direction of incoming neutrino
    G4ThreeVector pos_src;
    
    if (!exists(fn_geom)) {
        cout << "File does not exist: " << fn_geom<< endl;
        return -1;
    }
    evtgen->ReadGeometryFile(fn_geom);

    GLG4param& src_db = GLG4param::GetDB();
    if (!exists(fn_reactor)) {
        cout << "File does not exist: " << fn_reactor << endl;
        return -1;
    }
    src_db.ReadFile(fn_reactor);

    pos_src = G4ThreeVector(src_db["x"], src_db["y"], src_db["z"]);

    double emin = src_db["emin_reactor"];
    double emax = src_db["emax_reactor"];

    uv = pos_src / pos_src.mag();

    for (int i=0; i<nevent; i++) {
        if (rndEv) Ev = G4UniformRand() * (emax - emin) + emin; // reactor neutrino 1.8 ~ 10 MeV

        evtgen->GeneratePosition();
        evtgen->GenerateEvent(Ev, uv, theta); 
        evtgen->Print_HEPEvt();
    }

    return 0;
}


int source(int nevent, double Ev, bool flag_uniform_vtx)
{
    LSCEventGen_IBD* evtgen = new LSCEventGen_IBD();
    evtgen->SetDetShape(detshape);

    bool rndEv = false;
    if (Ev == -1) rndEv = true;

    double theta = -1;
    G4ThreeVector uv(1, 0, 0); 
    G4ThreeVector pos_src;

    if (!exists(fn_geom)) {
        cout << "File does not exist: " << fn_geom << endl;
        return -1;
    }
    evtgen->ReadGeometryFile(fn_geom);

    if (!exists(fn_source)) {
        cout << "File does not exist: " << fn_source << endl;
        return -1;
    }
    GLG4param& src_db = GLG4param::GetDB();
    src_db.ReadFile(fn_source);

    double emin = src_db["emin_source"];
    double emax = src_db["emax_source"];
    double L;
    int flagMC = 1;

    pos_src = G4ThreeVector(src_db["sourcex"], src_db["sourcey"], src_db["sourcez"]);
    pos_src *= 10; // cm -> mm;
    evtgen->SetPositionSource(pos_src);

    for (int i=0; i<nevent; i++) {
        flagMC = 0;
        if (rndEv) Ev = G4UniformRand() * (emax - emin) + emin; // source neutrino 1.8 ~ 10 MeV

        while (!flagMC) {
            evtgen->GeneratePosition();
            uv = evtgen->GetPosition() - pos_src;
            L = sqrt(uv.dot(uv));

            if (flag_uniform_vtx)
                flagMC = 1;
            else
                flagMC = evtgen->GeneratePosition_sourceMC(L);
        }

        evtgen->GenerateEvent(Ev, uv, theta); 
        evtgen->Print_HEPEvt();
    }

    return 0;
}


int solar(int nevent, double Ev)
{
    LSCEventGen_ve* evtgen = new LSCEventGen_ve();
    evtgen->SetDetShape(detshape);

    bool rndEv = false;
    if (Ev == -1) rndEv = true;

    double theta = -1;
    G4ThreeVector uv(0, 0, 1);
    G4ThreeVector pos_src;
    

    if (!exists(fn_geom)) {
        cout << "File does not exist: " << fn_geom << endl;
        return -1;
    }
    evtgen->ReadGeometryFile(fn_geom);

    if (!exists(fn_solar)) {
        cout << "File does not exist: " << fn_solar << endl;
        return -1;
    }
    GLG4param& src_db = GLG4param::GetDB();
    src_db.ReadFile(fn_solar);

    double emin = src_db["emin_solar"];
    double emax = src_db["emax_solar"];

    pos_src = G4ThreeVector(src_db["sunx"], src_db["suny"], src_db["sunz"]);
    pos_src *= 10; // cm -> mm;
    uv = pos_src / pos_src.mag();

    for (int i=0; i<nevent; i++) {
        if (rndEv) Ev = G4UniformRand() * (emax-emin) + emin; // solar neutrino 0.01 ~ 20.01 MeV

        evtgen->GeneratePosition();
        evtgen->GenerateEvent(Ev, uv, theta); 
        evtgen->Print_HEPEvt();
    }

    delete (evtgen);

    return 0;
}


int source_Cr51(int nevent, double Ev, bool flag_uniform_vtx)
{
    LSCEventGen_ve* evtgen = new LSCEventGen_ve();
    evtgen->SetDetShape(detshape);

    bool rndEv = false;
    if (Ev == -1) rndEv = true;

    double theta = -1;
    G4ThreeVector uv(1, 0, 0); 
    G4ThreeVector pos_src;

    if (!exists(fn_geom)) {
        cout << "File does not exist: " << fn_geom << endl;
        return -1;
    }
    evtgen->ReadGeometryFile(fn_geom);

    if (!exists(fn_source)) {
        cout << "File does not exist: " << fn_source << endl;
        return -1;
    }
    GLG4param& src_db = GLG4param::GetDB();
    src_db.ReadFile(fn_source);

    double Evs[4] = {0.747, 0.752, 0.427, 0.432}; // MeV
    double BRs[4] = {0.816, 0.085, 0.090, 0.009};
    double L;
    double N;
    int flagMC = 1;

    pos_src = G4ThreeVector(src_db["sourcex"], src_db["sourcey"], src_db["sourcez"]);
    pos_src *= 10; // cm -> mm;
    evtgen->SetPositionSource(pos_src);

    for (int i=0; i<nevent; i++) {
        flagMC = 0;
        if (rndEv) {
            N = G4UniformRand();

            if (N >= 0.0 && N <= BRs[0])
                Ev = 0.747; 
            else if (N <= BRs[0]+BRs[1]) 
                Ev = 0.752; 
            else if (N <= BRs[0]+BRs[1]+BRs[2]) 
                Ev = 0.427; 
            else 
                Ev = 0.432;
        }

        while (!flagMC) {
            evtgen->GeneratePosition();
            uv = evtgen->GetPosition() - pos_src;
            L = sqrt(uv.dot(uv));

            if (flag_uniform_vtx)
                flagMC = 1;
            else
                flagMC = evtgen->GeneratePosition_sourceMC(L);
        }

        evtgen->GenerateEvent(Ev, uv, theta); 
        evtgen->Print_HEPEvt();
    }

    return 0;
}


void PrintHelp()
{
    cout << endl;
    cout << "Usage: evtgen [-t event type] [-n # of event] [-g geometryfile]" << endl;
    cout << "              [-e neutrino energy] [-s sourcefile]" << endl;
    //cout << "              [-e neutrino energy] <setting file>" << endl;
    cout << "       event type - reactor, source, solar, Cr51 " << endl;
}

bool exists(const G4String &fname)
{
    ifstream f(fname);
    return f.good();
}
