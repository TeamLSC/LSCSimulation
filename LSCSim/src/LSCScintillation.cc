#include "LSCSim/LSCScintillation.hh"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "G4EmProcessSubType.hh"
#include "G4ParticleTypes.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4ios.hh"
#include "globals.hh"

using namespace std;

LSCScintillation::LSCScintillation(const G4String & processName,
                                   G4ProcessType type)
  : G4VRestDiscreteProcess(processName, type)
{
  SetProcessSubType(fScintillation); // 22

  fTrackSecondariesFirst = false;

  YieldFactor = 1.0;
  ExcitationRatio = 0.0;

  scintillationByParticleType = false;

  theEmitIntegralTable = NULL;

  if (verboseLevel > 0) {
    G4cout << GetProcessName() << " is created " << G4endl;
  }

  BuildThePhysicsTable();

  emSaturation = NULL;

  fScintillationDir = new G4UIdirectory("/LSC/Scintillation/");
  fScintOnCmd = new G4UIcmdWithAString("/LSC/Scintillation/scinton", this);

  fIsScintOn = 1;
}

////////////////
// Destructors
////////////////

LSCScintillation::~LSCScintillation()
{
  if (theEmitIntegralTable != NULL) {
    theEmitIntegralTable->clearAndDestroy();
    delete theEmitIntegralTable;
  }
}

////////////
// Methods
////////////

void LSCScintillation::SetNewValue(G4UIcommand * command, G4String newValues)
{
  if (command == fScintOnCmd) {
    istringstream is((const char *)newValues);
    is >> fIsScintOn;
    if (fIsScintOn == 0) {
      G4cout << "LSCScintillation::Scintillation will be turned off" << G4endl;
    }
  }
}

// AtRestDoIt
// ----------
//
G4VParticleChange * LSCScintillation::AtRestDoIt(const G4Track & aTrack,
                                                 const G4Step & aStep)

// This routine simply calls the equivalent PostStepDoIt since all the
// necessary information resides in aStep.GetTotalEnergyDeposit()

{
  return LSCScintillation::PostStepDoIt(aTrack, aStep);
}

// PostStepDoIt
// -------------
//
G4VParticleChange * LSCScintillation::PostStepDoIt(const G4Track & aTrack,
                                                   const G4Step & aStep)

// This routine is called for each tracking step of a charged particle
// in a scintillator. A Poisson/Gauss-distributed number of photons is
// generated according to the scintillation yield formula, distributed
// evenly along the track segment and uniformly into 4pi.

{
  fEvis = 0.;
  fTotalEdep = 0.;
  fNScintillationPhoton = 0;

  aParticleChange.Initialize(aTrack);

  const G4DynamicParticle * aParticle = aTrack.GetDynamicParticle();
  const G4Material * aMaterial = aTrack.GetMaterial();

  G4StepPoint * pPreStepPoint = aStep.GetPreStepPoint();
  G4StepPoint * pPostStepPoint = aStep.GetPostStepPoint();

  G4ThreeVector x0 = pPreStepPoint->GetPosition();
  G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
  G4double t0 = pPreStepPoint->GetGlobalTime();

  fTotalEdep = aStep.GetTotalEnergyDeposit();
  if (fTotalEdep <= 0.) {
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  G4MaterialPropertiesTable * aMaterialPropertiesTable =
      aMaterial->GetMaterialPropertiesTable();
  if (!aMaterialPropertiesTable) {
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  G4MaterialPropertyVector * Emit_Intensity =
      aMaterialPropertiesTable->GetProperty("EMITCOMPONENT");

  if (!Emit_Intensity) { // not scintillator
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  G4double ScintillationYield = 0.;

  if (scintillationByParticleType) {
    // The scintillation response is a function of the energy
    // deposited by particle types.

    // Get the definition of the current particle
    G4ParticleDefinition * pDef = aParticle->GetDefinition();
    G4MaterialPropertyVector * Scint_Yield_Vector = NULL;

    // Obtain the G4MaterialPropertyVectory containing the
    // scintillation light yield as a function of the deposited
    // energy for the current particle type

    // Protons
    if (pDef == G4Proton::ProtonDefinition()) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("PROTONSCINTILLATIONYIELD");

      // Deuterons
    }
    else if (pDef == G4Deuteron::DeuteronDefinition()) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("DEUTERONSCINTILLATIONYIELD");

      // Tritons
    }
    else if (pDef == G4Triton::TritonDefinition()) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("TRITONSCINTILLATIONYIELD");

      // Alphas
    }
    else if (pDef == G4Alpha::AlphaDefinition()) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("ALPHASCINTILLATIONYIELD");

      // Ions (particles derived from G4VIon and G4Ions)
      // and recoil ions below tracking cut from neutrons after hElastic
    }
    else if (pDef->GetParticleType() == "nucleus" ||
             pDef == G4Neutron::NeutronDefinition()) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("IONSCINTILLATIONYIELD");

      // Electrons (must also account for shell-binding energy
      // attributed to gamma from standard PhotoElectricEffect)
    }
    else if (pDef == G4Electron::ElectronDefinition() ||
             pDef == G4Gamma::GammaDefinition()) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");

      // Default for particles not enumerated/listed above
    }
    else {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");
    }

    // If the user has not specified yields for (p,d,t,a,carbon)
    // then these unspecified particles will default to the
    // electron's scintillation yield
    if (!Scint_Yield_Vector) {
      Scint_Yield_Vector =
          aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");
    }

    // Throw an exception if no scintillation yield is found
    if (!Scint_Yield_Vector) {
      G4ExceptionDescription ed;
      ed << "\nLSCScintillation::PostStepDoIt(): "
         << "Request for scintillation yield for energy deposit and particle "
            "type without correct entry in MaterialPropertiesTable\n"
         << "ScintillationByParticleType requires at minimum that "
            "ELECTRONSCINTILLATIONYIELD is set by the user\n"
         << G4endl;
      G4String comments = "Missing MaterialPropertiesTable entry - No correct "
                          "entry in MaterialPropertiesTable";
      G4Exception("LSCScintillation::PostStepDoIt", "Scint01", FatalException,
                  ed, comments);
      return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
    }

    if (verboseLevel > 1) {
      G4cout << "\n"
             << "Particle = " << pDef->GetParticleName() << "\n"
             << "Energy Dep. = " << fTotalEdep / MeV << "\n"
             << "Yield = " << Scint_Yield_Vector->Value(fTotalEdep) << "\n"
             << G4endl;
    }

    // Obtain the scintillation yield using the total energy
    // deposited by the particle in this step.

    // Units: [# scintillation photons]
    ScintillationYield = Scint_Yield_Vector->Value(fTotalEdep);
  }
  else {
    // The default linear scintillation process
    ScintillationYield =
        aMaterialPropertiesTable->GetConstProperty("SCINTILLATIONYIELD");

    // Units: [# scintillation photons / MeV]
    ScintillationYield *= YieldFactor;
  }

  G4double ResolutionScale =
      aMaterialPropertiesTable->GetConstProperty("RESOLUTIONSCALE");

  // Birks law saturation:
  // G4double constBirks = 0.0;
  // constBirks = aMaterial->GetIonisation()->GetBirksConstant();

  G4double MeanNumberOfPhotons;

  // Birk's correction via emSaturation and specifying scintillation by
  // by particle type are physically mutually exclusive

  if (scintillationByParticleType) { MeanNumberOfPhotons = ScintillationYield; }
  else if (emSaturation) {
    fEvis = emSaturation->VisibleEnergyDepositionAtAStep(&aStep);
    MeanNumberOfPhotons = ScintillationYield * fEvis;
  }
  else {
    MeanNumberOfPhotons = ScintillationYield * fTotalEdep;
  }

  G4int NumPhotons;

  if (MeanNumberOfPhotons > 10.) {
    G4double sigma = ResolutionScale * std::sqrt(MeanNumberOfPhotons);
    NumPhotons = G4int(G4RandGauss::shoot(MeanNumberOfPhotons, sigma) + 0.5);
  }
  else {
    NumPhotons = G4int(G4Poisson(MeanNumberOfPhotons));
  }

  /*
  G4ParticleDefinition * pDef = aParticle->GetDefinition();
  G4cout << "\n"
         << "Particle = " << pDef->GetParticleName() << " " <<
  aTrack.GetTrackID() << "\n"
         << "Energy Dep. = " << fTotalEdep / MeV << "\n"
         << "Energy Vis. = " << fEvis / MeV << "\n"
         << "Yield = " << NumPhotons << "\n"
         << G4endl;
  */
  if (NumPhotons <= 0) {
    // return unchanged particle and no secondaries

    aParticleChange.SetNumberOfSecondaries(0);
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  fNScintillationPhoton = NumPhotons;

  if (fIsScintOn == 0) {
    aParticleChange.SetNumberOfSecondaries(0);
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  ////////////////////////////////////////////////////////////////

  aParticleChange.SetNumberOfSecondaries(NumPhotons);

  if (fTrackSecondariesFirst) {
    if (aTrack.GetTrackStatus() == fAlive) {
      aParticleChange.ProposeTrackStatus(fSuspend);
    }
  }

  ////////////////////////////////////////////////////////////////

  G4int materialIndex = aMaterial->GetIndex();

  // Retrieve the Scintillation Integral for this material
  // new G4PhysicsOrderedFreeVector allocated to hold CII's

  G4int Num = NumPhotons;
  G4PhysicsOrderedFreeVector * ScintillationIntegral = nullptr;

  ScintillationIntegral =
      (G4PhysicsOrderedFreeVector *)((*theEmitIntegralTable)(materialIndex));

  if (!ScintillationIntegral)
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);

  G4double risetime = 0.;
  G4double decaytime1 = 0.;
  G4double decaytime2 = 0.;
  G4double decayfrac1 = 0.;
  G4double decayfrac2 = 0.;

  if (aMaterialPropertiesTable->ConstPropertyExists("RISETIME")) {
    risetime = aMaterialPropertiesTable->GetConstProperty("RISETIME");
  }
  if (aMaterialPropertiesTable->ConstPropertyExists("DECAYTIME1")) {
    decaytime1 = aMaterialPropertiesTable->GetConstProperty("DECAYTIME1");
  }
  if (aMaterialPropertiesTable->ConstPropertyExists("DECAYTIME2")) {
    decaytime2 = aMaterialPropertiesTable->GetConstProperty("DECAYTIME2");
  }
  if (aMaterialPropertiesTable->ConstPropertyExists("DECAYFRACTION1")) {
    decayfrac1 = aMaterialPropertiesTable->GetConstProperty("DECAYFRACTION1");
  }
  if (aMaterialPropertiesTable->ConstPropertyExists("DECAYFRACTION2")) {
    decayfrac2 = aMaterialPropertiesTable->GetConstProperty("DECAYFRACTION2");
  }


  Num = NumPhotons;

  // Max Scintillation Integral
  G4double CIImax = ScintillationIntegral->GetMaxValue();

  for (G4int i = 0; i < Num; i++) {

    // Determine photon energy
    G4double CIIvalue = G4UniformRand() * CIImax;
    G4double sampledEnergy = ScintillationIntegral->GetEnergy(CIIvalue);

    if (verboseLevel > 1) {
      G4cout << "sampledEnergy = " << sampledEnergy << G4endl;
      G4cout << "CIIvalue =        " << CIIvalue << G4endl;
    }

    // Generate random photon direction
    G4double cost = 1. - 2. * G4UniformRand();
    G4double sint = std::sqrt((1. - cost) * (1. + cost));

    G4double phi = twopi * G4UniformRand();
    G4double sinp = std::sin(phi);
    G4double cosp = std::cos(phi);

    G4double px = sint * cosp;
    G4double py = sint * sinp;
    G4double pz = cost;

    // Create photon momentum direction vector
    G4ParticleMomentum photonMomentum(px, py, pz);

    // Determine polarization of new photon
    G4double sx = cost * cosp;
    G4double sy = cost * sinp;
    G4double sz = -sint;

    G4ThreeVector photonPolarization(sx, sy, sz);
    G4ThreeVector perp = photonMomentum.cross(photonPolarization);
    phi = twopi * G4UniformRand();
    sinp = std::sin(phi);
    cosp = std::cos(phi);

    photonPolarization = cosp * photonPolarization + sinp * perp;
    photonPolarization = photonPolarization.unit();

    // Generate a new photon:
    // Generate a new photon:
    auto scintPhoton =
        new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(), photonMomentum);
    scintPhoton->SetPolarization(photonPolarization);
    scintPhoton->SetKineticEnergy(sampledEnergy);

    // Generate new G4Track object:
    G4double rand = G4UniformRand();
    if (aParticle->GetDefinition()->GetPDGCharge() == 0) { rand = 1.0; }

    // emission time distribution
    G4double delta = rand * aStep.GetStepLength();
    G4double deltaTime =
        delta /
        (pPreStepPoint->GetVelocity() +
         rand * (pPostStepPoint->GetVelocity() - pPreStepPoint->GetVelocity()) /
             2.);

    double scintTime = decaytime1;
    double frac = G4UniformRand();
    if (frac > decayfrac1) { scintTime = decaytime2; }

    if (risetime == 0.0) { deltaTime -= scintTime * std::log(G4UniformRand()); }
    else {
      deltaTime += sample_time(risetime, scintTime);
    }

    G4double secTime = t0 + deltaTime;
    G4ThreeVector secPosition = x0 + rand * aStep.GetDeltaPosition();

    G4Track * secTrack = new G4Track(scintPhoton, secTime, secPosition);
    secTrack->SetTouchableHandle(aStep.GetPreStepPoint()->GetTouchableHandle());
    secTrack->SetParentID(aTrack.GetTrackID());
    aParticleChange.AddSecondary(secTrack);
  }

  if (verboseLevel > 0) {
    G4cout << "\n Exiting from LSCScintillation::DoIt -- NumberOfSecondaries = "
           << aParticleChange.GetNumberOfSecondaries() << G4endl;
  }

  return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
}

// BuildThePhysicsTable for the scintillation process
// --------------------------------------------------
//
void LSCScintillation::BuildThePhysicsTable()
{
  if (theEmitIntegralTable) return;

  const G4MaterialTable * theMaterialTable = G4Material::GetMaterialTable();
  G4int numOfMaterials = G4Material::GetNumberOfMaterials();

  // create new physics table
  if (!theEmitIntegralTable)
    theEmitIntegralTable = new G4PhysicsTable(numOfMaterials);

  // loop for materials
  for (G4int i = 0; i < numOfMaterials; i++) {
    G4PhysicsOrderedFreeVector * aPhysicsOrderedFreeVector =
        new G4PhysicsOrderedFreeVector();

    // Retrieve vector of scintillation wavelength intensity for
    // the material from the material's optical properties table.
    G4Material * aMaterial = (*theMaterialTable)[i];
    G4MaterialPropertiesTable * aMaterialPropertiesTable =
        aMaterial->GetMaterialPropertiesTable();

    if (aMaterialPropertiesTable) {
      G4MaterialPropertyVector * theEmitLightVector =
          aMaterialPropertiesTable->GetProperty("EMITCOMPONENT");

      if (theEmitLightVector) {
        // Retrieve the first intensity point in vector
        // of (photon energy, intensity) pairs
        G4double currentIN = (*theEmitLightVector)[0];
        if (currentIN >= 0.0) {
          // Create first (photon energy, Scintillation
          // Integral pair
          G4double currentPM = theEmitLightVector->Energy(0);
          G4double currentCII = 0.0;

          aPhysicsOrderedFreeVector->InsertValues(currentPM, currentCII);

          // Set previous values to current ones prior to loop
          G4double prevPM = currentPM;
          G4double prevCII = currentCII;
          G4double prevIN = currentIN;

          // loop over all (photon energy, intensity)
          // pairs stored for this material
          for (size_t ii = 1; ii < theEmitLightVector->GetVectorLength();
               ++ii) {
            currentPM = theEmitLightVector->Energy(ii);
            currentIN = (*theEmitLightVector)[ii];

            currentCII = 0.5 * (prevIN + currentIN);
            currentCII = prevCII + (currentPM - prevPM) * currentCII;

            aPhysicsOrderedFreeVector->InsertValues(currentPM, currentCII);

            prevPM = currentPM;
            prevCII = currentCII;
            prevIN = currentIN;
          }
        }
      }
    }

    // The scintillation integral(s) for a given material
    // will be inserted in the table(s) according to the
    // position of the material in the material table.
    theEmitIntegralTable->insertAt(i, aPhysicsOrderedFreeVector);
  }
}

// Called by the user to set the scintillation yield as a function
// of energy deposited by particle type

void LSCScintillation::SetScintillationByParticleType(const G4bool scintType)
{
  if (emSaturation) {
    G4Exception("LSCScintillation::SetScintillationByParticleType", "Scint02",
                JustWarning,
                "Redefinition: Birks Saturation is replaced by "
                "ScintillationByParticleType!");
    RemoveSaturation();
  }
  scintillationByParticleType = scintType;
}

// GetMeanFreePath
// ---------------
//

G4double LSCScintillation::GetMeanFreePath(const G4Track &, G4double,
                                           G4ForceCondition * condition)
{
  *condition = StronglyForced;
  return DBL_MAX;
}

// GetMeanLifeTime
// ---------------
//

G4double LSCScintillation::GetMeanLifeTime(const G4Track &,
                                           G4ForceCondition * condition)
{
  *condition = Forced;
  return DBL_MAX;
}

G4double LSCScintillation::sample_time(G4double tau1, G4double tau2)
{
  // tau1: rise time and tau2: decay time
  while (1) {
    // two random numbers
    G4double ran1 = G4UniformRand();
    G4double ran2 = G4UniformRand();
    //
    // exponential distribution as envelope function: very efficient
    //
    G4double d = (tau1 + tau2) / tau2;
    // make sure the envelope function is
    // always larger than the bi-exponential
    G4double t = -1.0 * tau2 * std::log(1 - ran1);
    G4double gg = d * single_exp(t, tau2);
    if (ran2 <= bi_exp(t, tau1, tau2) / gg) return t;
  }
  return -1.0;
}
