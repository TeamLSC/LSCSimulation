#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4Tubs.hh"

#include "LSCSim/LSCDetectorConstruction.hh"
#include "LSCSim/LSCPMTSD.hh"
#include "LSCSim/LSC_PMT_LogicalVolume.hh"
#include "LSCSim/LightCon.hh"

using namespace std;
using namespace CLHEP;

///////////////////////////////////////////////////////////////////////////
// --- make the fundamental inner PMT assembly
///////////////////////////////////////////////////////////////////////////
// TODO add PMT option macro
void LSCDetectorConstruction::ConstructDetector_LSC_PMT(
    G4VPhysicalVolume * motherPhys, LSCPMTSD * pmtsd)
{
  auto motherLog = motherPhys->GetLogicalVolume();

  auto _logiInnerPMT = new LSC_10inch_LogicalVolume(
      "InnerPMT", 
      motherLog->GetMaterial(),
      G4Material::GetMaterial("Glass"), 
      Photocathode_opsurf,
      G4Material::GetMaterial("PMT_Vac"), 
      G4Material::GetMaterial("Steel"),
      nullptr,
      pmtsd); // sensitive detector hook

  if (fPMTPositionDataFile.empty()) {
    G4String msg = "Error, pmt position data file could not be opened.\n";
    G4cerr << msg << G4endl;
    G4Exception("LSCDetectorConstruction::LSCDetectorConstruction_LSC_PMT", "",
                FatalException, msg);
  }

  if (fLightConcentrator && fLightConProfile.empty()) {
    G4String msg = "Warning, light concentrator profile file could not be opened.\n";
    G4cerr << msg << G4endl;
    //G4Exception("LSCDetectorConstruction::LSCDetectorConstruction", "",
    //            FatalException, msg);
    fLightConcentrator = false;
  }

  LightCon* lc = nullptr;
  G4LogicalVolume* lc_log = nullptr;
  G4PVPlacement *lc_phys = nullptr;
  G4PVPlacement *lc_phys2 = nullptr;


  double z_lc = 105*mm; // z-position of light concentrator
  G4double r_tube = 13.5*cm;
  G4double z_tube = 20.0*cm;

  if (fLightConcentrator) {
    lc = new LightCon();
    G4cout << "Constructing light concentrator with profile: "
            << fLightConProfile << G4endl;
    lc_log = lc->Construct_LightCon(fLightConProfile);
    lc_log->SetVisAttributes(new G4VisAttributes(G4Colour(0, 0, 1, 0.3)));
    r_tube = lc->GetRMax();
    z_tube = lc->GetZMax() + z_lc;

    G4cout << "r_tube = " << r_tube << endl;
    G4cout << "z_tube = " << z_tube << endl;
  }

  auto pmt_assembly = new G4Tubs("pmt_assembly", 0,  r_tube, z_tube, 
                            0, 360*deg);

  auto pmt_assembly_log = new G4LogicalVolume(pmt_assembly, 
                            motherLog->GetMaterial(), 
                            "pmt_assembly_log");

  pmt_assembly_log->SetVisAttributes(G4VisAttributes::GetInvisible());

  auto pmt_phys = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), 
                                    _logiInnerPMT, "pmt_phys", 
                                    pmt_assembly_log, false, 0, fGeomCheck);

  if (fLightConcentrator) {
    lc_phys = new G4PVPlacement(0, G4ThreeVector(0, 0, z_lc), 
                                lc_log, "lc_phys", 
                                pmt_assembly_log, false, 0, fGeomCheck);
    lc_phys2 = new G4PVPlacement(0, G4ThreeVector(0, 0, z_lc), 
                                "lc_phys2", lc_log, 
                                pmt_phys, false, 0, fGeomCheck);
  }

  // defining mirror surface 
  auto our_Mirror_opsurf = new G4OpticalSurface("mirror_opsurf");
  our_Mirror_opsurf->SetFinish(polishedfrontpainted);
  our_Mirror_opsurf->SetModel(glisur);
  our_Mirror_opsurf->SetType(dielectric_metal);
  our_Mirror_opsurf->SetPolish(0.999);
  G4MaterialPropertiesTable* propMirror = new G4MaterialPropertiesTable();
  propMirror->AddProperty("REFLECTIVITY", new G4MaterialPropertyVector());
  propMirror->AddEntry("REFLECTIVITY", twopi*hbarc / (800.0e-9 * m), 0.90);
  propMirror->AddEntry("REFLECTIVITY", twopi*hbarc / (200.0e-9 * m), 0.90);
  our_Mirror_opsurf->SetMaterialPropertiesTable(propMirror);

  char PMTname[64];
  char surfname[64];

  double coord_x, coord_y, coord_z;
  int pmtno, nring, region;

  string line;
  ifstream pmtposfile(fPMTPositionDataFile.c_str());

  while (getline(pmtposfile, line)) {
    if (line.empty() || line[0] == '#') continue;

    istringstream sline(line);
    sline >> pmtno >> coord_x >> coord_y >> coord_z >> nring >> region;

    sprintf(PMTname, "InnerPMTPhys%d", pmtno);

    G4double r =
        sqrt(coord_x * coord_x + coord_y * coord_y + coord_z * coord_z);
    G4double dx = -coord_x / r;
    G4double dy = -coord_y / r;
    G4double dz = -coord_z / r;

    double angle_z = atan2(dx, dy);
    double angle_x = atan2(dz, sqrt(dx * dx + dy * dy));

    string ftmp = fPMTPositionDataFile;  
    transform(ftmp.begin(), ftmp.end(), ftmp.begin(), ::tolower);
    if ((long)ftmp.find("sphere") >= 0) {
      angle_x = atan2(dz, sqrt(dx * dx + dy * dy));
    }
    else if (region == 0) { // cylinder wall
      angle_x = 0; 
    }
    else if (region == -1 || region == 1) { // top or bottom PMTs
        double normal_angle = (region > 0 ? -M_PI / 2 : M_PI / 2);
        angle_x = normal_angle;
    }

    auto PMT_rotation = new G4RotationMatrix();
    PMT_rotation->rotateZ(angle_z);
    PMT_rotation->rotateX(M_PI / 2.0 - angle_x);

    G4ThreeVector pmtpos(coord_x, coord_y, coord_z);

    auto pmt_assembly_phys = new G4PVPlacement(
        PMT_rotation, pmtpos, PMTname, pmt_assembly_log, motherPhys,
        false, pmtno - 1, fGeomCheck);

    if (fLightConcentrator) {
        sprintf(surfname, "surface%d_1", pmtno);
        new G4LogicalBorderSurface(surfname, pmt_assembly_phys, lc_phys, our_Mirror_opsurf);
        sprintf(surfname, "surface%d_2", pmtno);
        new G4LogicalBorderSurface(surfname, pmt_phys, lc_phys2, our_Mirror_opsurf);
    }
  }
}
