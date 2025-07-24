#ifndef __LightCon_hh__
#define __LightCon_hh__

#include <unistd.h>
#include "G4LogicalVolume.hh"

class LightCon {
public:
    LightCon() {};
    ~LightCon() {}

    G4LogicalVolume* Construct_LightCon(G4String ifname);
    
    G4double GetRMax() { return r_max; }
    G4double GetZMax() { return z_max; }
private:
    G4double r_max = 0;
    G4double z_max = 0;

};

#endif
