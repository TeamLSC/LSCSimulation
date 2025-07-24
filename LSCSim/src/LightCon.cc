#include <fstream>
#include <string>
#include <vector>

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Polycone.hh"
#include "G4SystemOfUnits.hh"

#include "LSCSim/LightCon.hh"

G4LogicalVolume* LightCon::Construct_LightCon(G4String ifname) {
    
    G4NistManager* nist = G4NistManager::Instance();
    //G4Material* pmt_mat = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    G4Material* pmt_mat = G4Material::GetMaterial("Steel");

    ///////Lightcon///////////////
    std::vector<G4double> r_vals, z_vals;
    std::ifstream infile((G4String) ifname);
    if (!infile.is_open()) {
        G4Exception("LightCon::Construct_LightCon()", "GeomSolids0001", FatalException, "Could not open light concentrator profile file.");
    }
    G4double x, z;

    r_max = 0;
    z_max = 0;

    while (infile >> x >> z) {
        r_vals.push_back(x * cm);
        z_vals.push_back(z * cm);
        if (x*cm > r_max) 
            r_max = x*cm;
        if (z*cm > z_max) 
            z_max = z*cm;
    }
    infile.close();

    size_t numZ = r_vals.size();
    if (numZ < 2) {
        G4Exception("DetectorConstruction::Construct()", "GeomSolids0003", FatalException, "Insufficient profile data.");
    }
   
    std::vector<G4double> r_vals_inner(numZ);
    std::vector<G4double> r_vals_outer(numZ);
    G4double thickness = 1.0 * mm;

    for (size_t i = 0; i < numZ; i++) {
        r_vals_inner[i] = r_vals[i];
        r_vals_outer[i] = r_vals[i] + thickness;
    }
    r_max += thickness;

    G4Polycone* solidConcentrator = new G4Polycone(
        "Concentrator", 0.0 * deg, 360.0 * deg, numZ,
        z_vals.data(), r_vals_inner.data(), r_vals_outer.data()
    );
   
    G4LogicalVolume* logicConcentrator = new G4LogicalVolume(solidConcentrator, pmt_mat, "Concentrator");

    return logicConcentrator;
}
