#include <fstream>
#include <sstream>
#include <string>

#include "G4Box.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "GLG4Sim/GLG4param.hh"
#include "LSCSim/LSCDetectorConstruction.hh"
#include "LSCSim/LSCPMTSD.hh"

using namespace std;
using namespace CLHEP;

void LSCDetectorConstruction::ConstructDetector_LSC_Cylinder(
    G4VPhysicalVolume * worldphys, LSCPMTSD * pmtsd, GLG4param & geom_db)
{
  auto worldLog = worldphys->GetLogicalVolume();

  // TODO find replacement of the worldLog using GetDaughter()
 
  // visual attributes
  auto visssteel = new G4VisAttributes();
  visssteel->SetForceSolid(true);
  // visssteel->SetColour(G4Colour::Black());  
  visssteel->SetColour(G4Colour(0.64, 0.7, 0.73, 1));

  auto visacrylic = new G4VisAttributes();
  // visacrylic->SetForceCloud(true);
  visacrylic->SetForceSolid(true);
  visacrylic->SetColour(G4Colour(1, 1, 1, 0.1)); //white
  // visacrylic->SetColour(G4Colour(0.0, 0.0, 1.0, 0.5));

  auto viswater = new G4VisAttributes();
  viswater->SetForceSolid(true);
  // viswater->SetColour(G4Colour(0,0,1,0.7)); // blue
  viswater->SetColour(G4Colour(0.03, 0.49, 0.69, 0.7));

  auto vislslab = new G4VisAttributes();
  vislslab->SetForceSolid(true);
  // vislslab->SetColour(G4Colour(0, 1, 0, 1)); // green
  vislslab->SetColour(G4Colour(0.12, 0.26, 0.41, 0.8));


  // Veto
  G4double vetoR = cm * geom_db["veto_radius"];
  G4double vetoH = cm * geom_db["veto_height"];
  G4double vetoT = cm * geom_db["veto_thickness"];
  auto VetoTankTubs =
      new G4Tubs("VetoTankTubs", 0, vetoR, vetoH / 2, 0, 360 * deg);
  auto VetoTankLog = new G4LogicalVolume(
      VetoTankTubs, G4Material::GetMaterial("Steel"), "VetoTankLog", 0, 0, 0);
  VetoTankLog->SetVisAttributes(visssteel);
  auto VetoTankPhys =
      new G4PVPlacement(0, G4ThreeVector(), VetoTankLog, "VetoTankPhys",
                        worldLog, false, 0, fGeomCheck);

  auto VetoLiquidTubs = new G4Tubs("VetoLiquidTubs", 0, vetoR - vetoT,
                                   vetoH / 2 - vetoT, 0, 360 * deg);
  auto VetoLiquidLog =
      new G4LogicalVolume(VetoLiquidTubs, G4Material::GetMaterial("Water"),
                          "VetoLiquidLog", 0, 0, 0);
  VetoLiquidLog->SetVisAttributes(viswater);
  auto VetoLiquidPhys =
      new G4PVPlacement(0, G4ThreeVector(), VetoLiquidLog, "VetoLiquidPhys",
                        VetoTankLog, false, 0, fGeomCheck);

  new G4LogicalBorderSurface("veto_logsurf1", VetoTankPhys, VetoLiquidPhys,
                             Stainless_opsurf);
  new G4LogicalBorderSurface("veto_logsurf2", VetoLiquidPhys, VetoTankPhys,
                             Stainless_opsurf);

  // Buffer
  G4double bufferR = cm * geom_db["buffer_radius"];
  G4double bufferH = cm * geom_db["buffer_height"];
  G4double bufferT = cm * geom_db["buffer_thickness"];
  auto BufferTankTubs =
      new G4Tubs("BufferTankTubs", 0, bufferR, bufferH / 2, 0, 360 * deg);
  auto BufferTankLog =
      new G4LogicalVolume(BufferTankTubs, G4Material::GetMaterial("Steel"),
                          "BufferTankLog", 0, 0, 0);
  // BufferTankLog->SetVisAttributes(G4VisAttributes::GetInvisible());
  BufferTankLog->SetVisAttributes(G4Colour::Black());
  auto BufferTankPhys =
      new G4PVPlacement(0, G4ThreeVector(), BufferTankLog, "BufferTankPhys",
                        worldLog, false, fGeomCheck);

  auto BufferLiquidTubs = new G4Tubs("BufferLiquidTubs", 0, bufferR - bufferT,
                                     bufferH / 2 - bufferT, 0, 360 * deg);
  auto BufferLiquidLog = new G4LogicalVolume(
      BufferLiquidTubs, G4Material::GetMaterial("Water"), "BufferLog", 0, 0, 0);
  // BufferLiquidLog->SetVisAttributes(G4VisAttributes::GetInvisible());
  BufferLiquidLog->SetVisAttributes(G4Colour(0, 0, 1, 0.1)); // blue
  auto BufferLiquidPhys =
      new G4PVPlacement(0, G4ThreeVector(), BufferLiquidLog, "BufferLiquidPhys",
                        BufferTankLog, false, fGeomCheck);

  new G4LogicalBorderSurface("buffer_logsurf1", BufferTankPhys,
                             BufferLiquidPhys, Stainless_opsurf);
  new G4LogicalBorderSurface("buffer_logsurf2", BufferLiquidPhys,
                             BufferTankPhys, Stainless_opsurf);

  // Target
  G4double targetR = cm * geom_db["target_radius"];
  G4double targetH = cm * geom_db["target_height"];
  G4double targetT = cm * geom_db["target_thickness"];

  auto TargetTankTubs =
      new G4Tubs("TargetTankTubs", 0, targetR, targetH / 2, 0, 360 * deg);
  auto TargetTankLog =
      new G4LogicalVolume(TargetTankTubs, G4Material::GetMaterial("Acrylic"),
                          "TargetTankLog", 0, 0, 0);
  // TargetTankLog->SetVisAttributes(G4VisAttributes::GetInvisible());
  TargetTankLog->SetVisAttributes(G4Colour(1, 1, 1, 0.1)); // white
  auto TargetTankPhys =
      new G4PVPlacement(0, G4ThreeVector(), TargetTankLog, "TargetTankPhys",
                        BufferLiquidLog, false, fGeomCheck);

  auto TargetLSTubs = new G4Tubs("TargetLSTubs", 0, targetR - targetT,
                                 targetH / 2 - targetT, 0, 360 * deg);
  auto TargetLSLog = new G4LogicalVolume(
      TargetLSTubs, G4Material::GetMaterial("LS_LAB"), "TargetLSLog", 0, 0, 0);
  // TargetLSLog->SetVisAttributes(G4VisAttributes::GetInvisible());
  TargetLSLog->SetVisAttributes(G4Colour(0, 1, 0, 0.1)); // green
  auto TargetLSPhys =
      new G4PVPlacement(0, G4ThreeVector(), TargetLSLog, "TargetLSPhys",
                        TargetTankLog, false, fGeomCheck);

  // Place PMTs
  ConstructDetector_LSC_PMT(BufferLiquidPhys, pmtsd);
}
