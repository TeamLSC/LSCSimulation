#include "LSCSim/LSCDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalConstants.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RegionStore.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4Sphere.hh"
#include "G4String.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"
#include "G4UImanager.hh"
#include "G4UnionSolid.hh"
#include "GLG4Sim/GLG4param.hh"
#include "LSCSim/LSCPMTSD.hh"
#include "LSCSim/LSC_PMT_LogicalVolume.hh"

using namespace std;

void LSCDetectorConstruction::ConstructDetector_LSC_ExpHall(
    G4VPhysicalVolume * worldphys, GLG4param & geom_db)
{
  auto WorldLog = worldphys->GetLogicalVolume();

  // Visualization attributes
  auto visair = new G4VisAttributes();
  visair->SetForceSolid(true);
  visair->SetColour(G4Colour(0.3, 0.3, 0.3, 0.3));

  auto visssteel = new G4VisAttributes();
  visssteel->SetForceSolid(true);
  visssteel->SetColour(G4Colour(0.64, 0.7, 0.73, 1));

  auto viswater = new G4VisAttributes();
  viswater->SetForceSolid(true);
  viswater->SetColour(G4Colour(0.03, 0.49, 0.69, 0.7));


  // Cavern (Top)
  G4double cavernR = cm * geom_db["cavern_arch_radius"];
  G4double cavernH = cm * geom_db["cavern_arch_height"];
  G4double cavernT = cm * geom_db["cavern_arch_thickness"];
  G4double cavernA = deg * geom_db["cavern_arch_angle"];
  auto CavernTubs = new G4Tubs("CavernTubs", cavernR - cavernH / 8, cavernR,
                                cavernH / 2, 0 * deg, cavernA * 2);

  // Cavern (Bottom)
  G4double cavernX = cm * geom_db["cavern_arch_height"];
  G4double cavernY = cm * geom_db["cavern_arch_height"];
  G4double cavernZ = cm * geom_db["cavern_height"];
  auto CavernBox = new G4Box("CavernBox", cavernX / 2, cavernY / 2,
                              (cavernZ - cavernT) / 2.);
  auto CavernRot = new G4RotationMatrix();
  CavernRot->rotateZ(-cavernA);
  CavernRot->rotateY(-90 * deg);

  // Pit
  G4double pitR = cm * geom_db["pit_radius"];
  G4double pitH = cm * geom_db["pit_height"];
  auto PitTubs = new G4Tubs("PitTubs", 0, pitR, pitH / 2, 0, 360 * deg);

  // Cavern (Union of pit, tunnel, top and bottom caverns)
  auto CavernUni = new G4UnionSolid(
      "CavernUni", CavernBox, CavernTubs,
      G4Transform3D(*CavernRot, G4ThreeVector(0, 0,
                                              CavernBox->GetZHalfLength() -
                                                  (cavernR - cavernH / 10))));
  CavernUni = new G4UnionSolid(
      "CavernUni", PitTubs, CavernUni, 0,
      G4ThreeVector(0, 0, pitH / 2. + CavernBox->GetZHalfLength()));

  auto CavernLog = new G4LogicalVolume(
      CavernUni, G4Material::GetMaterial("Air"), "CavernLog");
  CavernLog->SetVisAttributes(visair);
  auto CavernPhys =
      new G4PVPlacement(0, G4ThreeVector(), CavernLog, "CavernPhys", WorldLog,
                        false, 0, fGeomCheck);
}
