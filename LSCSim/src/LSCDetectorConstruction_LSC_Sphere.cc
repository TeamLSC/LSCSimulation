#include <fstream>
#include <sstream>
#include <string>

#include "G4Box.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4VPhysicalVolume.hh"
#include "GLG4Sim/GLG4param.hh"
#include "LSCSim/LSCDetectorConstruction.hh"
#include "LSCSim/LSCPMTSD.hh"

using namespace std;
using namespace CLHEP;

void LSCDetectorConstruction::ConstructDetector_LSC_Sphere(
    G4VPhysicalVolume * worldphys, LSCPMTSD * pmtsd, GLG4param & geom_db)
{
  auto worldLog = worldphys->GetLogicalVolume();

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
  VetoTankLog->SetVisAttributes(G4Colour::Black());
  auto VetoTankPhys =
      new G4PVPlacement(0, G4ThreeVector(), VetoTankLog, "VetoTankPhys",
                        worldLog, false, 0, fGeomCheck);

  auto VetoLiquidTubs = new G4Tubs("VetoLiquidTubs", 0, vetoR - vetoT,
                                   vetoH / 2 - vetoT, 0, 360 * deg);
  auto VetoLiquidLog =
      new G4LogicalVolume(VetoLiquidTubs, G4Material::GetMaterial("Water"),
                          "VetoLiquidLog", 0, 0, 0);
  VetoLiquidLog->SetVisAttributes(G4Colour(0, 0, 1, 0.1));
  auto VetoLiquidPhys =
      new G4PVPlacement(0, G4ThreeVector(), VetoLiquidLog, "VetoLiquidPhys",
                        VetoTankLog, false, 0, fGeomCheck);

  new G4LogicalBorderSurface("veto_logsurf1", VetoTankPhys, VetoLiquidPhys,
                             Stainless_opsurf);
  new G4LogicalBorderSurface("veto_logsurf2", VetoLiquidPhys, VetoTankPhys,
                             Stainless_opsurf);

  // Buffer
  G4double bufferR = cm * geom_db["buffer_radius"];
  G4double bufferT = cm * geom_db["buffer_thickness"];
  auto BufferTankSphere = new G4Sphere("BufferTankSphere", 0, bufferR, 0,
                                       360. * deg, 0, 180. * deg);
  auto BufferTankLog =
      new G4LogicalVolume(BufferTankSphere, G4Material::GetMaterial("Steel"),
                          "BufferTankLog", 0, 0, 0);
  BufferTankLog->SetVisAttributes(visssteel);
  auto BufferTankPhys =
      new G4PVPlacement(0, G4ThreeVector(), BufferTankLog, "BufferTankPhys",
                        worldLog, false, fGeomCheck);

  auto BufferLiquidSphere = 
      new G4Sphere("BufferLiquidSphere", 0, bufferR - bufferT, 
                   0, 360. * deg, 0, 180. * deg);
  auto BufferLiquidLog =
      new G4LogicalVolume(BufferLiquidSphere, G4Material::GetMaterial("Water"),
                          "BufferLog", 0, 0, 0);
  BufferLiquidLog->SetVisAttributes(viswater);
  auto BufferLiquidPhys =
      new G4PVPlacement(0, G4ThreeVector(), BufferLiquidLog, "BufferLiquidPhys",
                        BufferTankLog, false, fGeomCheck);

  new G4LogicalBorderSurface("buffer_logsurf1", BufferTankPhys,
                             BufferLiquidPhys, Stainless_opsurf);
  new G4LogicalBorderSurface("buffer_logsurf2", BufferLiquidPhys,
                             BufferTankPhys, Stainless_opsurf);

  // Target
  G4double targetR = cm * geom_db["target_radius"];
  G4double targetT = cm * geom_db["target_thickness"];

  auto TargetTankSphere =
      new G4Sphere("TargetTankSphere", 0, targetR, 
                   0, 360 * deg, 0, 180 * deg);
  auto TargetTankLog =
      new G4LogicalVolume(TargetTankSphere, G4Material::GetMaterial("Acrylic"),
                          "TargetTankLog", 0, 0, 0);
  TargetTankLog->SetVisAttributes(visacrylic);
  auto TargetTankPhys =
      new G4PVPlacement(0, G4ThreeVector(), TargetTankLog, "TargetTankPhys",
                        BufferLiquidLog, false, fGeomCheck);

  auto TargetLSSphere = 
      new G4Sphere("TargetLSSphere", 0, targetR - targetT,
                   0, 360 * deg, 0, 180 * deg);
  auto TargetLSLog = 
      new G4LogicalVolume(TargetLSSphere, G4Material::GetMaterial("LS_LAB"), 
                          "TargetLSLog", 0, 0, 0);
  // TODO Add target material macro
  TargetLSLog->SetVisAttributes(vislslab);
  auto TargetLSPhys =
      new G4PVPlacement(0, G4ThreeVector(), TargetLSLog, "TargetLSPhys",
                        TargetTankLog, false, fGeomCheck);
  G4cout << "Target mass:" << TargetLSLog->GetMass(true, false)/kg << " kg" << G4endl;

  // Place PMTs
  ConstructDetector_LSC_PMT(BufferLiquidPhys, pmtsd);
}
