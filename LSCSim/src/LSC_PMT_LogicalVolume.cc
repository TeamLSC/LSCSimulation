// This file is part of the GenericLAND software library.
// $Id: LSC_PMT_LogicalVolume.cc,v 1.2 2014/03/18 03:27:03 jslee Exp $
//
// LSC_PMT_LogicalVolume.cc
// defines classes for constructing PMT assemblies for GenericLAND
// Original by Glenn Horton-Smith, Dec 1999
// Modification history:
//  G.H-S.  2001/03/20:  Added LSCPMTOpticalModel for thin photocathode

#include "LSCSim/LSC_PMT_LogicalVolume.hh"

#include "G4LogicalBorderSurface.hh"
#include "G4Material.hh"
#include "G4OpticalSurface.hh"
#include "G4PVPlacement.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh" // for G4VisAttributes::Invisible
#include "GLG4Sim/GLG4TorusStack.hh"
#include "LSCSim/LSCPMTOpticalModel.hh"

using namespace CLHEP;

G4OpticalSurface * LSC_PMT_LogicalVolume::our_Mirror_opsurf = NULL;

////////////////////////////////////////////////////////////////
// LSC_PMT_LogicalVolume
//
LSC_PMT_LogicalVolume::LSC_PMT_LogicalVolume(
    const G4String & plabel, // label -- subvolume names are derived from this
    G4double r_bound,        // radius of bounding cylinder
    G4double hh_bound,       // half height of bounding cylinder
    G4Material * ExteriorMat // material which fills the bounding cylinder
    )
  : G4LogicalVolume(new G4Tubs(plabel + "_envelope_solid", 0.0, r_bound,
                               hh_bound, 0., 2. * M_PI),
                    ExteriorMat, plabel)
{
  if (our_Mirror_opsurf == NULL) {
    // construct a static mirror surface with idealized properties
    our_Mirror_opsurf = new G4OpticalSurface("Mirror_opsurf");
    our_Mirror_opsurf->SetFinish(polishedfrontpainted); // needed for mirror
    our_Mirror_opsurf->SetModel(glisur);
    our_Mirror_opsurf->SetType(dielectric_metal);
    our_Mirror_opsurf->SetPolish(0.999); // a guess -- FIXME
    G4MaterialPropertiesTable * propMirror = NULL;
    G4Material * matMirror = G4Material::GetMaterial("PMT_Mirror");
    if (matMirror) propMirror = matMirror->GetMaterialPropertiesTable();
    if (propMirror == NULL) {
      G4cerr << "Warning: setting PMT mirror reflectivity to 0.9999 because no "
                "PMT_Mirror material properties defined"
             << G4endl;
      propMirror = new G4MaterialPropertiesTable();
      propMirror->AddProperty("REFLECTIVITY", new G4MaterialPropertyVector());
      propMirror->AddEntry("REFLECTIVITY", twopi * hbarc / (800.0e-9 * m),
                           0.9999);
      propMirror->AddEntry("REFLECTIVITY", twopi * hbarc / (200.0e-9 * m),
                           0.9999);
    }
    our_Mirror_opsurf->SetMaterialPropertiesTable(propMirror);
  }
}

////////////////////////////////////////////////////////////////
// constants defining the (fixed by manufacturer) dimensions of the
// phototubes:
//
static const int R3600_n_edge = 9;
static const G4double R3600_z_edge[R3600_n_edge + 1] = {
    188.00,  116.71,  0.00,    -116.71, -136.05,
    -195.00, -282.00, -355.00, -370.00, -492.00};
static const G4double R3600_rho_edge[R3600_n_edge + 1] = {
    0.00, 198.55, 254.00, 198.55, 165.40, 127.00, 127.00, 53.00, 41.35, 41.35};
static const G4double R3600_z_o[R3600_n_edge] = {
    -127.00, 0.00, 0.00, 127.00, -195.00, -195.00, -280.00, -370.00, -370.00};

// the following constants were derived from an old drawing of a
// discontinued "R1408" model Hamamatsu phototube, but they
// give an inconsistant shape with a sharp corner at (rxy=95.91,z=44.91).
//
// static const int R1408_n_edge= 6;
// static const G4double R1408_z_edge[R1408_n_edge+1]= { 75.00, 44.91, 0.00,
// -44.91, -73.86, -85.00, -215.00}; static const G4double
// R1408_rho_edge[R1408_n_edge+1]= {0.00, 95.91,
// 102.00, 95.91, 44.32, 32.00, 32.00 }; static const G4double
// R1408_z_o[R1408_n_edge]= {     -51.00,  0.00,   0.00, 51.00, -85.00, -215.00
// };
//

static const int R7081_n_edge = 6;
static const G4double R7081_z_edge[R7081_n_edge + 1] = {
    96.8, 40.0, 0.0, -40.0, -90.0, -142.0, -223.3}; // [0]: 96.7 -> 96.8 by kmlee
static const G4double R7081_rho_edge[R7081_n_edge + 1] = {
    0.0, 111.0, 126.5, 111.0, 42.25, 42.25, 42.25};
static const G4double R7081_z_o[R7081_n_edge] = {-40.0, 0.0,    0.0,
                                                 40.0,  -142.0, -223.3};

static const int R5912_n_edge = 6;
static const G4double R5912_z_edge[R5912_n_edge + 1] = {
    75.00, 53.06, 0.00, -53.06, -73.86, -85.00, -215.00};
static const G4double R5912_rho_edge[R5912_n_edge + 1] = {
    0.00, 72.57, 101.00, 72.57, 44.32, 42.00, 42.00};
static const G4double R5912_z_o[R5912_n_edge] = {-56.00, 0.00,   0.00,
                                                 56.00,  -85.00, -215.00};

static const int ETI_9372A_n_edge = 5;
static const G4double ETI_9372A_z_edge[ETI_9372A_n_edge + 1] = {
    50.8, 35.56, 0.00, -35.56, -42.42, -165.1};
static const G4double ETI_9372A_rho_edge[ETI_9372A_n_edge + 1] = {
    0.00, 45.72, 63.5, 45.72, 42.42, 42.42};
static const G4double ETI_9372A_z_o[ETI_9372A_n_edge] = {-25.40, 0.00, 0.00,
                                                         -42.42, -165.1};

////////////////////////////////////////////////////////////////
// LSC_17inch_LogicalVolume
//
LSC_17inch_LogicalVolume::LSC_17inch_LogicalVolume(
    const G4String & plabel,  // label -- subvolume names are derived from this
    G4Material * ExteriorMat, // material which fills the bounding cylinder
    G4Material * GlassMat,    // glass material
    G4OpticalSurface * Photocathode_opsurf, // photocathode
    G4Material * PMT_Vacuum,                // vacuum inside tube
    G4Material * DynodeMat,                 // dynode material
    G4Material * MaskMat, // material for photocathode mask (e.g, blk acryl)
                          // OK to set MaskMat == NULL for no mask
    G4VSensitiveDetector * detector // sensitive detector hook
    )
  : LSC_PMT_LogicalVolume(
        plabel, 260. * millimeter,
        340. * millimeter, // hh_bound: half height of bounding cylinder
        ExteriorMat)
{
  ConstructPMT_UsingTorusStack(
      R3600_n_edge, R3600_z_edge, R3600_rho_edge, R3600_z_o,
      94. * millimeter,    // radius of dynode stack
      -117. * millimeter,  // z coordinate of top of dynode stack, equator=0
      4. * millimeter,     // thickness of the walls
      ExteriorMat,         // material outside tube
      GlassMat,            // glass material
      Photocathode_opsurf, // photocathode surface
      PMT_Vacuum,          // tube interior
      DynodeMat,           // dynode stack metal
      detector             // detector hook
  );

  if (MaskMat != NULL) {
    // make the mask -- use thin cylindrical disk for now
    G4double r_mask_inner = 254.0 * millimeter;
    G4double r_mask_outer = ((G4Tubs *)(this->GetSolid()))
                                ->GetOuterRadius(); // bounding cylinder size
    G4double hh_mask = 1.5 * millimeter;            // half height

    G4LogicalVolume * mask_log = new G4LogicalVolume(
        new G4Tubs(plabel + "_mask_solid",     // name of solid
                   r_mask_inner, r_mask_outer, // inner and outer radii
                   hh_mask,             // use flat disk, 2mm thick for now
                   0. * deg, 360. * deg // start and end span angle
                   ),
        MaskMat, plabel + "_mask_log");

    /**    G4PVPlacement* mask_phys = **/
    new G4PVPlacement(
        0,                                                    // no rotation
        G4ThreeVector(0., 0., 19.5 * millimeter + z_equator), // displacement
        mask_log,                                             // logical volume
        plabel + "_mask_phys",                                // name
        this,                                                 // mother volume
        false,                                                // no boolean ops
        0);                                                   // copy number

    // mask is black
    //    G4VisAttributes * visAtt= new
    //    G4VisAttributes(G4Color(0.0,0.0,0.0,1.0));
    G4VisAttributes * visAtt = new G4VisAttributes(G4Color(1.0, 1.0, 0.0, 0.3));
    visAtt->SetForceSolid(true);
    mask_log->SetVisAttributes(visAtt);

    // extra torus mask for 17-inch
    G4double z_edge[3];
    G4double rho_edge[3];
    G4double z_o[2];
    z_edge[0] = -26.0 * mm;
    z_edge[1] = 0.0;
    z_edge[2] = 99.0 * mm;
    z_o[0] = z_o[1] = 0.0;
    rho_edge[1] = 254.1 * mm;
    G4double side_curvature_radius = 150.1 * mm;
    rho_edge[0] = rho_edge[1] - side_curvature_radius +
                  sqrt(side_curvature_radius * side_curvature_radius -
                       z_edge[0] * z_edge[0]);
    rho_edge[2] = rho_edge[1] - side_curvature_radius +
                  sqrt(side_curvature_radius * side_curvature_radius -
                       z_edge[2] * z_edge[2]);

    GLG4TorusStack * facemask_inner_solid =
        new GLG4TorusStack(plabel + "_facemask_inner_solid");
    facemask_inner_solid->SetAllParameters(2, z_edge, rho_edge, z_o);

    rho_edge[0] += 2.0 * mm;
    rho_edge[1] += 2.0 * mm;
    rho_edge[2] += 2.0 * mm;
    GLG4TorusStack * facemask_solid =
        new GLG4TorusStack(plabel + "_facemask_solid");
    facemask_solid->SetAllParameters(2, z_edge, rho_edge, z_o,
                                     facemask_inner_solid);

    G4LogicalVolume * facemask_log =
        new G4LogicalVolume(facemask_solid, MaskMat, plabel + "_facemask_log");

    /**    G4PVPlacement* facemask_phys =  **/
    new G4PVPlacement(0,                                // no rotation
                      G4ThreeVector(0., 0., z_equator), // displacement
                      facemask_log,                     // logical volume
                      plabel + "_facemask_phys",        // name
                      this,                             // mother volume
                      false,                            // no boolean ops
                      0);                               // copy number

    // facemask is same color as annular mask
    facemask_log->SetVisAttributes(visAtt);
  }
}

////////////////////////////////////////////////////////////////
// LSC_20inch_LogicalVolume
//
LSC_20inch_LogicalVolume::LSC_20inch_LogicalVolume(
    const G4String & plabel,  // label -- subvolume names are derived from this
    G4Material * ExteriorMat, // material which fills the bounding cylinder
    G4Material * GlassMat,    // glass material
    G4OpticalSurface * Photocathode_opsurf, // photocathode
    G4Material * PMT_Vacuum,                // vacuum inside tube
    G4Material * DynodeMat,                 // dynode material
    G4Material * MaskMat, // material for photocathode mask (e.g, blk acryl)
                          // OK to set MaskMat == NULL for no mask
    G4VSensitiveDetector * detector // sensitive detector hook
    )
  : LSC_PMT_LogicalVolume(
        plabel, 260. * millimeter,
        340. * millimeter, // hh_bound: half height of bounding cylinder
        ExteriorMat)
{
  ConstructPMT_UsingTorusStack(
      R3600_n_edge, R3600_z_edge, R3600_rho_edge, R3600_z_o,
      94. * millimeter,    // radius of dynode stack
      -117. * millimeter,  // z coordinate of top of dynode stack, equator=0
      4. * millimeter,     // thickness of the walls
      ExteriorMat,         // material outside tube
      GlassMat,            // glass material
      Photocathode_opsurf, // photocathode surface
      PMT_Vacuum,          // tube interior
      DynodeMat,           // dynode stack metal
      detector             // detector hook
  );

  if (MaskMat != NULL) {
    // make the mask -- use thin cylindrical disk for now
    G4double r_mask_inner = 254. * millimeter;
    G4double r_mask_outer = ((G4Tubs *)(this->GetSolid()))
                                ->GetOuterRadius(); // bounding cylinder size
    G4double hh_mask = 1.5 * millimeter;            // half height

    G4LogicalVolume * mask_log = new G4LogicalVolume(
        new G4Tubs(plabel + "_mask_solid",     // name of solid
                   r_mask_inner, r_mask_outer, // inner and outer radii
                   hh_mask,             // use flat disk, 2mm thick for now
                   0. * deg, 360. * deg // start and end span angle
                   ),
        MaskMat, plabel + "_mask_log");

    /** G4PVPlacement* mask_phys =  **/
    new G4PVPlacement(
        0,                                                    // no rotation
        G4ThreeVector(0., 0., 19.5 * millimeter + z_equator), // displacement
        mask_log,                                             // logical volume
        plabel + "_mask_phys",                                // name
        this,                                                 // mother volume
        false,                                                // no boolean ops
        0);                                                   // copy number

    // mask is black
    //    G4VisAttributes * visAtt= new
    //    G4VisAttributes(G4Color(0.0,0.0,0.0,1.0));
    G4VisAttributes * visAtt = new G4VisAttributes(G4Color(0.8, 0.8, 0.8, 1.0));
    visAtt->SetForceSolid(true);
    mask_log->SetVisAttributes(visAtt);
  }
}

////////////////////////////////////////////////////////////////
// LSC_10inch_LogicalVolume
//
LSC_10inch_LogicalVolume::LSC_10inch_LogicalVolume(
    const G4String & plabel,  // label -- subvolume names are derived from this
    G4Material * ExteriorMat, // material which fills the bounding cylinder
    G4Material * GlassMat,    // glass material
    G4OpticalSurface * Photocathode_opsurf, // photocathode
    G4Material * PMT_Vacuum,                // vacuum inside tube
    G4Material * DynodeMat,                 // dynode material
    G4Material * MaskMat, // material for photocathode mask (e.g, blk acryl)
                          // OK to set MaskMat == NULL for no mask
    G4VSensitiveDetector * detector // sensitive detector hook
    )
  : LSC_PMT_LogicalVolume(plabel, 126.5 * millimeter, 160. * millimeter,
                          ExteriorMat)
{
  ConstructPMT_UsingTorusStack(
      R7081_n_edge, R7081_z_edge, R7081_rho_edge, R7081_z_o,
      27.5 * millimeter,   // radius of dynode stack
      -55.0 * millimeter,  // z coordinate of top of dynode stack, equator=0
      3. * millimeter,     // thickness of the walls
      ExteriorMat,         // material outside tube
      GlassMat,            // glass material
      Photocathode_opsurf, // photocathode surface
      PMT_Vacuum,          // tube interior
      DynodeMat,           // dynode stack metal
      detector             // detector hook
  );

  if (MaskMat != NULL) {
    // make the mask -- use thin cylindrical disk for now
    G4double r_mask_inner = 96. * millimeter;
    G4double r_mask_outer = ((G4Tubs *)(this->GetSolid()))
                                ->GetOuterRadius(); // bounding cylinder size
    G4double hh_mask = 1.0 * millimeter;            // half height

    G4LogicalVolume * mask_log = new G4LogicalVolume(
        new G4Tubs(plabel + "_mask_solid",     // name of solid
                   r_mask_inner, r_mask_outer, // inner and outer radii
                   hh_mask,             // use flat disk, 2mm thick for now
                   0. * deg, 360. * deg // start and end span angle
                   ),
        MaskMat, plabel + "_mask_log");

    /**    G4PVPlacement* mask_phys =  **/
    new G4PVPlacement(
        0, // no rotation
        G4ThreeVector(0., 0.,
                      R5912_z_edge[1] + hh_mask + z_equator), // displacement
        mask_log,                                             // logical volume
        plabel + "_mask_phys",                                // name
        this,                                                 // mother volume
        false,                                                // no boolean ops
        0);                                                   // copy number
  }
}

////////////////////////////////////////////////////////////////
// LSC_8inch_LogicalVolume
//
LSC_8inch_LogicalVolume::LSC_8inch_LogicalVolume(
    const G4String & plabel,  // label -- subvolume names are derived from this
    G4Material * ExteriorMat, // material which fills the bounding cylinder
    G4Material * GlassMat,    // glass material
    G4OpticalSurface * Photocathode_opsurf, // photocathode
    G4Material * PMT_Vacuum,                // vacuum inside tube
    G4Material * DynodeMat,                 // dynode material
    G4Material * MaskMat, // material for photocathode mask (e.g, blk acryl)
                          // OK to set MaskMat == NULL for no mask
    G4VSensitiveDetector * detector // sensitive detector hook
    )
  : LSC_PMT_LogicalVolume(plabel, 110. * millimeter, 150. * millimeter,
                          ExteriorMat)
{
  ConstructPMT_UsingTorusStack(
      R5912_n_edge, R5912_z_edge, R5912_rho_edge, R5912_z_o,
      27.5 * millimeter,   // radius of dynode stack
      -30.0 * millimeter,  // z coordinate of top of dynode stack, equator=0
      3. * millimeter,     // thickness of the walls
      ExteriorMat,         // material outside tube
      GlassMat,            // glass material
      Photocathode_opsurf, // photocathode surface
      PMT_Vacuum,          // tube interior
      DynodeMat,           // dynode stack metal
      detector             // detector hook
  );

  if (MaskMat != NULL) {
    // make the mask -- use thin cylindrical disk for now
    G4double r_mask_inner = 96. * millimeter;
    G4double r_mask_outer = ((G4Tubs *)(this->GetSolid()))
                                ->GetOuterRadius(); // bounding cylinder size
    G4double hh_mask = 1.0 * millimeter;            // half height

    G4LogicalVolume * mask_log = new G4LogicalVolume(
        new G4Tubs(plabel + "_mask_solid",     // name of solid
                   r_mask_inner, r_mask_outer, // inner and outer radii
                   hh_mask,             // use flat disk, 2mm thick for now
                   0. * deg, 360. * deg // start and end span angle
                   ),
        MaskMat, plabel + "_mask_log");

    /**    G4PVPlacement* mask_phys =  **/
    new G4PVPlacement(
        0, // no rotation
        G4ThreeVector(0., 0.,
                      R5912_z_edge[1] + hh_mask + z_equator), // displacement
        mask_log,                                             // logical volume
        plabel + "_mask_phys",                                // name
        this,                                                 // mother volume
        false,                                                // no boolean ops
        0);                                                   // copy number
  }
}

////////////////////////////////////////////////////////////////
// LSC_5inch_LogicalVolume
//
LSC_5inch_LogicalVolume::LSC_5inch_LogicalVolume(
    const G4String & plabel,  // label -- subvolume names are derived from this
    G4Material * ExteriorMat, // material which fills the bounding cylinder
    G4Material * GlassMat,    // glass material
    G4OpticalSurface * Photocathode_opsurf, // photocathode
    G4Material * PMT_Vacuum,                // vacuum inside tube
    G4Material * DynodeMat,                 // dynode material
    G4Material * MaskMat, // material for photocathode mask (e.g, blk acryl)
                          // OK to set MaskMat == NULL for no mask
    G4VSensitiveDetector * detector // sensitive detector hook
    )
  : LSC_PMT_LogicalVolume(plabel, 64. * millimeter, 108. * millimeter,
                          ExteriorMat)
{

  ConstructPMT_UsingTorusStack(
      ETI_9372A_n_edge, ETI_9372A_z_edge, ETI_9372A_rho_edge, ETI_9372A_z_o,
      40. * millimeter,    // radius of dynode stack (FIXME?)
      -35. * millimeter,   // z coordinate of top of dynode stack, equator=0
      2. * millimeter,     // thickness of the walls
      ExteriorMat,         // material outside tube
      GlassMat,            // glass material
      Photocathode_opsurf, // photocathode surface
      PMT_Vacuum,          // tube interior
      DynodeMat,           // dynode stack metal
      detector             // detector hook
  );

  if (MaskMat != NULL) {
    // make the mask -- use thin cylindrical disk for now
    G4double r_mask_inner = 45.72 * millimeter;
    G4double r_mask_outer = ((G4Tubs *)(this->GetSolid()))
                                ->GetOuterRadius(); // bounding cylinder size
    G4double hh_mask = 1.0 * millimeter;            // half height

    G4LogicalVolume * mask_log = new G4LogicalVolume(
        new G4Tubs(plabel + "_mask_solid",     // name of solid
                   r_mask_inner, r_mask_outer, // inner and outer radii
                   hh_mask,             // use flat disk, 2mm thick for now
                   0. * deg, 360. * deg // start and end span angle
                   ),
        MaskMat, plabel + "_mask_log");

    /**    G4PVPlacement* mask_phys =  **/
    new G4PVPlacement(
        0,                                                     // no rotation
        G4ThreeVector(0., 0., 36.56 * millimeter + z_equator), // displacement
        mask_log,                                              // logical volume
        plabel + "_mask_phys",                                 // name
        this,                                                  // mother volume
        false,                                                 // no boolean ops
        0);                                                    // copy number
  }
}

////////////////////////////////////////////////////////////////
// ConstructPMT_UsingTorusStack
//  -- makes PMT assembly using TorusStack shapes
//     This is the preferred PMT model.
//     A model built using "Ellipsoid"'s is provided for comparison
//
void LSC_PMT_LogicalVolume::ConstructPMT_UsingTorusStack(
    const G4int n_edge, const G4double outer_z_edge[],
    const G4double outer_rho_edge[], const G4double outer_z_o[],
    G4double r_dynode, // radius of dynode stack
    G4double z_dynode, // z coordinate of top of dynode stack, equator=0.
    G4double d_wall,   // thickness of the walls
    G4Material * /* Exterior */,    // material outside tube
    G4Material * Glass,             // glass material
    G4OpticalSurface * OpPCSurface, // photocathode surface
    G4Material * PMT_Vac,           // tube interior
    G4Material * Dynode_mat,        // dynode stack metal
    G4VSensitiveDetector * detector // detector hook
)
{
  ////////////////////////////////////////////////////////////////
  // MAKE SOLIDS
  ////

  // pmt-shaped volume (to be filled with glass)
  GLG4TorusStack * body_solid = new GLG4TorusStack(GetName() + "_body_solid");
  body_solid->SetAllParameters(n_edge, outer_z_edge, outer_rho_edge, outer_z_o);

  // inner volumes (to be filled with vacuum)
  GLG4TorusStack * inner1_solid =
      new GLG4TorusStack(GetName() + "_inner1_solid");
  GLG4TorusStack * inner2_solid =
      new GLG4TorusStack(GetName() + "_inner2_solid");

  // set shapes of inner volumes
  // also scan for lowest allowed point of dynode
  G4double z_lowest_dynode = z_dynode;
  {
    // We will have to calculate the inner dimensions of the PMT.
    // To allow this, we allocate some workspace.
    // Once upon a time, a very long time ago, g++ was inefficient
    // in allocating small arrays and would fail if the array size was < 4,
    // so here we make sure we allocate one array of sufficient size.
    G4double * dscratch = new G4double[4 * (n_edge + 1)];
    G4double * inner_z_edge = dscratch;
    G4double * inner_rho_edge = dscratch + n_edge + 1;
    G4ThreeVector norm;
    G4int iedge_equator = -1;
    // calculate inner surface edges, check dynode position, and find equator
    inner_z_edge[0] = outer_z_edge[0] - d_wall;
    inner_rho_edge[0] = 0.0;
    for (int i = 1; i < n_edge; i++) {
      norm = body_solid->SurfaceNormal(
          G4ThreeVector(0.0, outer_rho_edge[i], outer_z_edge[i]));
      inner_z_edge[i] = outer_z_edge[i] - d_wall * norm.z();
      inner_rho_edge[i] = outer_rho_edge[i] - d_wall * norm.y();
      if (inner_rho_edge[i] > r_dynode && inner_z_edge[i] < z_lowest_dynode)
        z_lowest_dynode = inner_z_edge[i];
      if (outer_z_edge[i] == 0.0 || inner_z_edge[i] == 0.0) iedge_equator = i;
    }
    inner_z_edge[n_edge] = outer_z_edge[n_edge] + d_wall;
    inner_rho_edge[n_edge] = outer_rho_edge[n_edge] - d_wall;
    // one final check on dynode allowed position
    if (inner_rho_edge[n_edge] > r_dynode &&
        inner_z_edge[n_edge] < z_lowest_dynode)
      z_lowest_dynode = inner_z_edge[n_edge];
    // sanity check equator index
    if (iedge_equator < 0) {
      iedge_equator = (1 + n_edge) / 2;
      G4cerr << "LSC_PMT_LogicalVolume::ConstructPMT_UsingTorusStack: "
                "Warning, pathological PMT shape, equator edge not found!"
             << G4endl;
    }
    // sanity check on dynode height
    if (z_dynode > inner_z_edge[iedge_equator]) {
      z_dynode = inner_z_edge[iedge_equator];
      G4cerr << "LSC_PMT_LogicalVolume::ConstructPMT_UsingTorusStack: "
                "Warning, dynode higher than equator, dynode truncated!"
             << G4endl;
    }
    // set inner surfaces
    inner1_solid->SetAllParameters(iedge_equator, inner_z_edge, inner_rho_edge,
                                   outer_z_o);
    inner2_solid->SetAllParameters(
        n_edge - iedge_equator, inner_z_edge + iedge_equator,
        inner_rho_edge + iedge_equator, outer_z_o + iedge_equator);
    // GLG4TorusStack keeps its own copy of edges, so we can delete our
    // workspace
    delete[] dscratch;
  }

  // dynode volume
  G4double hh_dynode = (z_dynode - z_lowest_dynode) / 2.0;
  G4Tubs * dynode_solid = new G4Tubs(GetName() + "_dynode_solid", 0.0,
                                     r_dynode,       // solid cylinder (fixme?)
                                     hh_dynode,      // half height of cylinder
                                     0., 2. * M_PI); // cylinder complete in phi

  ////////////////////////////////////////////////////////////////
  // MAKE LOGICAL VOLUMES (add materials)
  ////

  G4LogicalVolume * body_log =
      new G4LogicalVolume(body_solid, Glass, GetName() + "_body_log");
  body_log->SetSensitiveDetector(detector);
	G4cout << "PMT Glass mass:" << body_log->GetMass(true, false)/kg << " kg" << G4endl;

  G4LogicalVolume * inner1_log =
      new G4LogicalVolume(inner1_solid, PMT_Vac, GetName() + "_inner1_log");
  inner1_log->SetSensitiveDetector(detector);

  G4LogicalVolume * inner2_log =
      new G4LogicalVolume(inner2_solid, PMT_Vac, GetName() + "_inner2_log");

  G4LogicalVolume * dynode_log =
      new G4LogicalVolume(dynode_solid, Dynode_mat, GetName() + "_dynode_log");

  ////////////////////////////////////////////////////////////////
  // MAKE PHYSICAL VOLUMES (place logical volumes)
  ////

  // calculate z coordinate of equatorial plane in envelope
  z_equator = ((G4Tubs *)(this->GetSolid()))->GetZHalfLength() -
              outer_z_edge[0] -
              0.1 * mm; // face of tube 100 um from front of cylinder
  G4ThreeVector equatorTranslation(0., 0., z_equator);
  G4ThreeVector noTranslation(0., 0., 0.);

  // place outer solids in envelope
  G4PVPlacement * body_phys = new G4PVPlacement(
      0,                        // no rotation
      equatorTranslation,       // puts body equator in right place
      body_log,                 // the logical volume
      GetName() + "_body_phys", // a name for this physical volume
      this,                     // the mother volume
      false,                    // no boolean ops
      0);                       // copy number

  // place inner solids in outer solid (vacuum)
  G4PVPlacement * inner1_phys = new G4PVPlacement(
      0,                          // no rotation
      noTranslation,              // puts face equator in right place
      GetName() + "_inner1_phys", // a name for this physical volume
      inner1_log,                 // the logical volume
      body_phys,                  // the mother volume
      false,                      // no boolean ops
      0);                         // copy number
  G4PVPlacement * inner2_phys = new G4PVPlacement(
      0,                          // no rotation
      noTranslation,              // puts face equator in right place
      GetName() + "_inner2_phys", // a name for this physical volume
      inner2_log,                 // the logical volume
      body_phys,                  // the mother volume
      false,                      // no boolean ops
      0);                         // copy number

  // place dynode in stem/back
  /**  G4PVPlacement* dynode_phys =  **/
  new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, z_dynode - hh_dynode),
                    GetName() + "_dynode_phys", dynode_log, inner2_phys, false,
                    0);

  ////////////////////////////////////////////////////////////////
  // Attach optical surfaces to borders
  ////
  new G4LogicalBorderSurface(GetName() + "_photocathode_logsurf1", inner1_phys,
                             body_phys, OpPCSurface);
  new G4LogicalBorderSurface(GetName() + "_photocathode_logsurf2", body_phys,
                             inner1_phys, OpPCSurface);
  new G4LogicalBorderSurface(GetName() + "_mirror_logsurf1", inner2_phys,
                             body_phys, our_Mirror_opsurf);
  new G4LogicalBorderSurface(GetName() + "_mirror_logsurf2", body_phys,
                             inner2_phys, our_Mirror_opsurf);

  ////////////////////////////////////////////////////////////////
  // FastSimulationModel
  // setup optical model
  G4Region * body_region = new G4Region(GetName() + "_GLG4_PMTOpticalRegion");
  body_region->AddRootLogicalVolume(body_log);
  new LSCPMTOpticalModel(GetName() + "_optical_model", body_region, body_log,
                         OpPCSurface);

  ////////////////////////////////////////////////////////////////
  // Set colors and visibility
  ////
  G4VisAttributes * visAtt;
  this->SetVisAttributes(G4VisAttributes::GetInvisible());
  //  PMT glass
  visAtt = new G4VisAttributes(G4Color(0.0, 1.0, 1.0, 0.05));
  visAtt->SetForceSolid(true);
  body_log->SetVisAttributes(visAtt);
  // body_log->SetVisAttributes(G4VisAttributes::GetInvisible());
  //  dynode is medium gray
  visAtt = new G4VisAttributes(G4Color(0.5, 0.5, 0.5, 1.0));
  visAtt->SetForceSolid(true);
  dynode_log->SetVisAttributes(visAtt);
  // (surface of) interior vacuum is clear orangish gray on top (PC),
  // silvery blue on bottom (mirror)
  //visAtt = new G4VisAttributes(G4Color(0.7, 0.5, 0.3, 0.27));
  visAtt = new G4VisAttributes(G4Color(0.7, 0.5, 0.3, 0.47));
  visAtt->SetForceSolid(true);
  inner1_log->SetVisAttributes(visAtt);
  visAtt = new G4VisAttributes(G4Color(0.6, 0.7, 0.8, 0.67));
  visAtt->SetForceSolid(true);
  inner2_log->SetVisAttributes(visAtt);
}
