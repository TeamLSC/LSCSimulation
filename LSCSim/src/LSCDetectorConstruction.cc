#include "LSCSim/LSCDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalConstants.hh"
#include "G4SDManager.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "GLG4Sim/GLG4param.hh"
#include "LSCSim/LSCPMTSD.hh"

using namespace std;

LSCPMTSD * LSCDetectorConstruction::fPmt_SD = NULL;

LSCDetectorConstruction::LSCDetectorConstruction()
  : G4VUserDetectorConstruction(),
    G4UImessenger()
{
  fDetectorDir = new G4UIdirectory("/LSC/det/");

  fGeomCheck = 0;

  fGeomCheckOptCmd = new G4UIcmdWithAnInteger("/LSC/det/geomcheck", this);
  fMaterialDataFileCmd = new G4UIcmdWithAString("/LSC/det/materialdata", this);
  fGeometryDataFileCmd = new G4UIcmdWithAString("/LSC/det/geometrydata", this);
  fPMTPositionDataFileCmd = new G4UIcmdWithAString("/LSC/det/pmtposdata", this);
  fWhichDetectorCmd = new G4UIcmdWithAString("/LSC/det/detector", this);
  fLightConcentratorCmd = new G4UIcmdWithAnInteger("/LSC/det/lightconcentrator", this);
  fLightConProfileCmd = new G4UIcmdWithAString("/LSC/det/lightconprofile", this);

  fGeometryDataFile = "";
  fPMTPositionDataFile = "";
  fMaterialDataFile = "";
  fWhichDetector = "";
  fLightConcentrator = 0; // default is no light concentrator
  fLightConProfile = "";
}

LSCDetectorConstruction::~LSCDetectorConstruction()
{
  delete fDetectorDir;
  delete fGeomCheckOptCmd;
  delete fMaterialDataFileCmd;
  delete fGeometryDataFileCmd;
  delete fPMTPositionDataFileCmd;
  delete fWhichDetectorCmd;
  delete fLightConcentratorCmd;
  delete fLightConProfileCmd;
}

void LSCDetectorConstruction::SetNewValue(G4UIcommand * command,
                                          G4String newValues)
{
  if (command == fGeomCheckOptCmd) {
    istringstream is(newValues);
    is >> fGeomCheck;
  }
  if (command == fGeometryDataFileCmd) {
    if (fGeometryDataFile.empty()) fGeometryDataFile = newValues;
  }
  if (command == fMaterialDataFileCmd) {
    if (fMaterialDataFile.empty()) fMaterialDataFile = newValues;
    G4cout << fMaterialDataFile << G4endl;
  }
  if (command == fPMTPositionDataFileCmd) {
    if (fPMTPositionDataFile.empty()) fPMTPositionDataFile = newValues;
  }
  if (command == fWhichDetectorCmd) {
    if (fWhichDetector.empty()) fWhichDetector = newValues;
  }
  if (command == fLightConcentratorCmd) {
    istringstream is(newValues);
    is >> fLightConcentrator;
  }
  if (command == fLightConProfileCmd) {
    if (fLightConProfile.empty()) fLightConProfile = newValues;
  }
}

G4VPhysicalVolume * LSCDetectorConstruction::Construct()
{
  ConstructMaterials();
  return ConstructDetector();
}

G4VPhysicalVolume * LSCDetectorConstruction::ConstructDetector()
{
  GLG4param & geom_db(GLG4param::GetDB());

  if (fGeometryDataFile.empty()) {
    G4String msg = "Error, geometery data file could not be opened.\n";
    G4cerr << msg << G4endl;
    G4Exception("LSCDetectorConstruction::LSCDetectorConstruction", "",
                FatalException, msg);
  }
  else {
    geom_db.ReadFile(fGeometryDataFile.c_str());
  }

  // World (Rock)
  G4double worldX = cm * geom_db["worldx"];
  G4double worldY = cm * geom_db["worldy"];
  G4double worldZ = cm * geom_db["worldz"];
  auto WorldBox = new G4Box("WorldBox", worldX / 2, worldY / 2, worldZ / 2);
  auto WorldLog = new G4LogicalVolume(WorldBox, G4Material::GetMaterial("Rock"),
                                      "WorldLog", 0, 0, 0);
  WorldLog->SetVisAttributes(G4VisAttributes::GetInvisible());
  auto WorldPhys = new G4PVPlacement(0, G4ThreeVector(), WorldLog, "WorldPhys",
                                     0, false, 0, fGeomCheck);

  ///////////////////////////////////////////////////////////////////////////
  // --- PMT sensitive detector
  ///////////////////////////////////////////////////////////////////////////
  G4SDManager * fSDman = G4SDManager::GetSDMpointer();
  LSCPMTSD * pmtSDInner = new LSCPMTSD("/LSC/PMT/inner");
  fSDman->AddNewDetector(pmtSDInner);

  if (fWhichDetector == "LSCC" || fWhichDetector == "CYLINDER") {
    ConstructDetector_LSC_ExpHall(WorldPhys, geom_db);
    ConstructDetector_LSC_Cylinder(WorldPhys, pmtSDInner, geom_db);
  }
  else if (fWhichDetector == "LSCS" || fWhichDetector == "SPHERE") {
    ConstructDetector_LSC_ExpHall(WorldPhys, geom_db);
    ConstructDetector_LSC_Sphere(WorldPhys, pmtSDInner, geom_db);
  }
  else if (fWhichDetector == "PROTO")
    ConstructDetector_Prototype(WorldPhys, pmtSDInner, geom_db);

  return WorldPhys;
}
