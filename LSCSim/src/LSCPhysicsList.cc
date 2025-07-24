#include "LSCSim/LSCPhysicsList.hh"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "G4DeexPrecoParameters.hh"
#include "G4EmParameters.hh"
#include "G4NuclearLevelData.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4ios.hh"
#include "GLG4Sim/GLG4DeferTrackProc.hh"
#include "GLG4Sim/GLG4param.hh"
#include "globals.hh"

using namespace std;

// Constructor /////////////////////////////////////////////////////////////
LSCPhysicsList::LSCPhysicsList()
  : G4VModularPhysicsList()
{
  defaultCutValue = 1.0 * micrometer; //
  cutForGamma = defaultCutValue;
  cutForElectron = 1.0 * nanometer;
  cutForPositron = defaultCutValue;

  VerboseLevel = 1;
  OpVerbLevel = 0;

  omitNeutHP = false;
  omitHadronicProc = false;

  // set a finer grid of the physic tables in order to improve precision
  // former LowEnergy models have 200 bins up to 100 GeV
  G4EmParameters * param = G4EmParameters::Instance();
  param->SetMaxEnergy(100 * GeV);
  param->SetNumberOfBinsPerDecade(20);
  param->SetMscStepLimitType(fMinimal);
  param->SetFluo(true);
  param->SetPixe(true);
  param->SetAuger(true);

  G4EmParameters::Instance()->AddPhysics("World", "G4RadioactiveDecay");
  G4DeexPrecoParameters * deex =
      G4NuclearLevelData::GetInstance()->GetParameters();
  deex->SetStoreICLevelData(true);
  deex->SetMaxLifeTime(G4NuclideTable::GetInstance()->GetThresholdOfHalfLife() /
                       std::log(2.));

  SetVerboseLevel(VerboseLevel);

  // for messenger
  physDir = new G4UIdirectory("/LSC/phys/");
  physDir->SetGuidance("PhysicsList control");

  gammaCutCmd = new G4UIcmdWithADoubleAndUnit("/LSC/phys/CutGamma", this);
  gammaCutCmd->SetGuidance("Set gamma cut.");
  gammaCutCmd->SetParameterName("Gcut", false);
  gammaCutCmd->SetUnitCategory("Length");
  gammaCutCmd->SetRange("Gcut>0.0");
  gammaCutCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  electCutCmd = new G4UIcmdWithADoubleAndUnit("/LSC/phys/CutEl", this);
  electCutCmd->SetGuidance("Set electron cut.");
  electCutCmd->SetParameterName("Ecut", false);
  electCutCmd->SetUnitCategory("Length");
  electCutCmd->SetRange("Ecut>0.0");
  electCutCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  posCutCmd = new G4UIcmdWithADoubleAndUnit("/LSC/phys/CutPos", this);
  posCutCmd->SetGuidance("Set positron cut.");
  posCutCmd->SetParameterName("Pcut", false);
  posCutCmd->SetUnitCategory("Length");
  posCutCmd->SetRange("Pcut>0.0");
  posCutCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  allCutCmd = new G4UIcmdWithADoubleAndUnit("/LSC/phys/CutsAll", this);
  allCutCmd->SetGuidance("Set cut for all.");
  allCutCmd->SetParameterName("cut", false);
  allCutCmd->SetUnitCategory("Length");
  allCutCmd->SetRange("cut>0.0");
  allCutCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  verboseCmd = new G4UIcmdWithAnInteger("/LSC/phys/verbose", this);
  verboseCmd->SetGuidance("set verbose for physics processes");
  verboseCmd->SetParameterName("verbose", true);
  verboseCmd->SetDefaultValue(1);
  verboseCmd->SetRange("verbose>=0");
  verboseCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

// Destructor //////////////////////////////////////////////////////////////
LSCPhysicsList::~LSCPhysicsList()
{
  delete physDir;
  delete gammaCutCmd;
  delete electCutCmd;
  delete posCutCmd;
  delete allCutCmd;
  delete verboseCmd;
}

// Construct Particles /////////////////////////////////////////////////////
void LSCPhysicsList::ConstructParticle()
{

  // In this method, static member functions should be called
  // for all particles which yodu want to use.
  // This ensures that objects of these particle types will be
  // created in the program.

  ConstructMyBosons();
  ConstructMyLeptons();
  ConstructMyHadrons();
  ConstructMyShortLiveds();
}

// construct Bosons://///////////////////////////////////////////////////
#include "G4ChargedGeantino.hh"
#include "G4Gamma.hh"
#include "G4Geantino.hh"
#include "G4OpticalPhoton.hh"

void LSCPhysicsList::ConstructMyBosons()
{
  // pseudo-particles
  G4Geantino::GeantinoDefinition();
  G4ChargedGeantino::ChargedGeantinoDefinition();

  // gamma
  G4Gamma::GammaDefinition();

  // OpticalPhotons
  G4OpticalPhoton::OpticalPhotonDefinition();
}

// construct Leptons://///////////////////////////////////////////////////
#include "G4AntiNeutrinoE.hh"
#include "G4AntiNeutrinoMu.hh"
#include "G4Electron.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4NeutrinoE.hh"
#include "G4NeutrinoMu.hh"
#include "G4Positron.hh"

void LSCPhysicsList::ConstructMyLeptons()
{
  // leptons
  G4Electron::ElectronDefinition();
  G4Positron::PositronDefinition();
  G4MuonPlus::MuonPlusDefinition();
  G4MuonMinus::MuonMinusDefinition();

  G4NeutrinoE::NeutrinoEDefinition();
  G4AntiNeutrinoE::AntiNeutrinoEDefinition();
  G4NeutrinoMu::NeutrinoMuDefinition();
  G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();
}

// construct Hadrons://///////////////////////////////////////////////////
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4MesonConstructor.hh"

void LSCPhysicsList::ConstructMyHadrons()
{
  //  mesons
  G4MesonConstructor mConstructor;
  mConstructor.ConstructParticle();

  //  baryons
  G4BaryonConstructor bConstructor;
  bConstructor.ConstructParticle();

  //  ions
  G4IonConstructor iConstructor;
  iConstructor.ConstructParticle();
}

// construct Shortliveds://///////////////////////////////////////////////////
#include "G4ShortLivedConstructor.hh"

void LSCPhysicsList::ConstructMyShortLiveds()
{
  // ShortLiveds
  G4ShortLivedConstructor slConstructor;
  slConstructor.ConstructParticle();
}

// Construct Processes //////////////////////////////////////////////////////
void LSCPhysicsList::ConstructProcess()
{

  AddTransportation();

  AddParameterisation();

  ConstructEM();

  ConstructOp();

  ConstructHad(); // JW: hadron check

  ConstructGeneral();
}

// G4FastSimulation Processes
// //////////////////////////////////////////////////////
#include "G4FastSimulationManagerProcess.hh"

void LSCPhysicsList::AddParameterisation()
{
  auto theParticleIterator = GetParticleIterator();

  G4FastSimulationManagerProcess * theFastSimulationManagerProcess =
      new G4FastSimulationManagerProcess();
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition * particle = theParticleIterator->value();
    G4ProcessManager * pmanager = particle->GetProcessManager();
    pmanager->AddProcess(theFastSimulationManagerProcess, -1, -1, 1);
  }
}

// Electromagnetic Processes ////////////////////////////////////////////////
// all charged particles

// gamma
#include "G4BetheHeitler5DModel.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4LivermoreComptonModel.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreRayleighModel.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4RayleighScattering.hh"

// e-
#include "G4LivermoreIonisationModel.hh"
#include "G4UniversalFluctuation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eIonisation.hh"
#include "G4eMultipleScattering.hh"

// e+
#include "G4eBremsstrahlung.hh"
#include "G4eIonisation.hh"
#include "G4eplusAnnihilation.hh"

// alpha and GenericIon and deuterons, triton, He3:
// muon:
#include "G4MuBremsstrahlung.hh"
#include "G4MuIonisation.hh"
#include "G4MuMultipleScattering.hh"
#include "G4MuPairProduction.hh"
#include "G4MuonMinusCapture.hh"

// OTHERS:
#include "G4IonParametrisedLossModel.hh"
#include "G4hBremsstrahlung.hh"
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"
#include "G4ionIonisation.hh"

// em process options to allow msc step-limitation to be switched off
#include "G4LossTableManager.hh"
#include "G4UAtomicDeexcitation.hh"
#include "G4VAtomDeexcitation.hh"

void LSCPhysicsList::ConstructEM()
{
  G4LossTableManager * man = G4LossTableManager::Instance();
  man->SetAtomDeexcitation(new G4UAtomicDeexcitation());

  auto particleIterator = GetParticleIterator();
  particleIterator->reset();
  while ((*particleIterator)()) {
    G4ParticleDefinition * particle = particleIterator->value();
    G4ProcessManager * pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
    G4String particleType = particle->GetParticleType();
    G4double charge = particle->GetPDGCharge();

    if (particleName == "gamma") {
      // gamma
      G4RayleighScattering * theRayleigh = new G4RayleighScattering();
      pmanager->AddDiscreteProcess(theRayleigh);

      G4PhotoElectricEffect * thePhotoElectricEffect =
          new G4PhotoElectricEffect();
      thePhotoElectricEffect->SetEmModel(new G4LivermorePhotoElectricModel());
      pmanager->AddDiscreteProcess(thePhotoElectricEffect);

      G4ComptonScattering * theComptonScattering = new G4ComptonScattering();
      theComptonScattering->SetEmModel(new G4LivermoreComptonModel());
      pmanager->AddDiscreteProcess(theComptonScattering);

      G4GammaConversion * theGammaConversion = new G4GammaConversion();
      theGammaConversion->SetEmModel(new G4BetheHeitler5DModel());
      pmanager->AddDiscreteProcess(theGammaConversion);
    }
    else if (particleName == "e-") {
      // electron
      //  process ordering: AddProcess(name, at rest, along step, post step)
      //  Multiple scattering
      G4eMultipleScattering * msc = new G4eMultipleScattering();
      msc->SetStepLimitType(fUseDistanceToBoundary);
      //pmanager->AddProcess(msc, -1, 1, -1);
      pmanager->AddProcess(msc, -1, 1, 1);

      // Ionisation
      G4eIonisation * eIonisation = new G4eIonisation();
      G4VEmModel * theIoniLiv = new G4LivermoreIonisationModel();
      theIoniLiv->SetHighEnergyLimit(0.1 * MeV);
      eIonisation->AddEmModel(0, theIoniLiv, new G4UniversalFluctuation());
      eIonisation->SetStepFunction(0.2,
                                   100 * um); // improved precision in tracking
      pmanager->AddProcess(eIonisation, -1, 2, 2);

      // Bremsstrahlung
      G4eBremsstrahlung * eBremsstrahlung = new G4eBremsstrahlung();
      pmanager->AddProcess(eBremsstrahlung, -1, -3, 3);
    }
    else if (particleName == "e+") {
      // positron
      G4eMultipleScattering * msc = new G4eMultipleScattering();
      msc->SetStepLimitType(fUseDistanceToBoundary);
      pmanager->AddProcess(msc, -1, 1, 1);

      // Ionisation
      G4eIonisation * eIonisation = new G4eIonisation();
      eIonisation->SetStepFunction(0.2, 100 * um); //
      pmanager->AddProcess(eIonisation, -1, 2, 2);

      // Bremsstrahlung (use default, no low-energy available)
      pmanager->AddProcess(new G4eBremsstrahlung(), -1, -1, 3);

      // Annihilation
      pmanager->AddProcess(new G4eplusAnnihilation(), 0, -1, 4);
    }
    else if (particleName == "mu+" || particleName == "mu-") {
      // muon
      //pmanager->AddProcess(new G4MuMultipleScattering, -1, 1, -1);
      //pmanager->AddProcess(new G4MuIonisation(), -1, 2, 1);
      //pmanager->AddProcess(new G4MuBremsstrahlung(), -1, -1, 2);
      //pmanager->AddProcess(new G4MuPairProduction(), -1, -1, 3);
      pmanager->AddProcess(new G4MuMultipleScattering, -1, 1, 1);
      pmanager->AddProcess(new G4MuIonisation(), -1, 2, 2);
      pmanager->AddProcess(new G4MuBremsstrahlung(), -1, -1, 3);
      pmanager->AddProcess(new G4MuPairProduction(), -1, -1, 4);
      if (particleName == "mu-")
        pmanager->AddProcess(new G4MuonMinusCapture(), 0, -1, -1);
    }
    else if (particleName == "proton" || particleName == "pi+" ||
             particleName == "pi-") {
      // multiple scattering
      pmanager->AddProcess(new G4hMultipleScattering, -1, 1, -1);

      // ionisation
      G4hIonisation * hIonisation = new G4hIonisation();
      hIonisation->SetStepFunction(0.2, 50 * um);
      //pmanager->AddProcess(hIonisation, -1, 2, 1);
      pmanager->AddProcess(hIonisation, -1, 2, 2);

      // bremmstrahlung
      //pmanager->AddProcess(new G4hBremsstrahlung, -1, -3, 2);
      pmanager->AddProcess(new G4hBremsstrahlung, -1, -3, 3);
    }
    else if (particleName == "alpha" || particleName == "deuteron" ||
             particleName == "triton" || particleName == "He3") {
      // multiple scattering
      // pmanager->AddProcess(new G4hMultipleScattering, -1, 1, -1);
      pmanager->AddProcess(new G4hMultipleScattering, -1, 1, 1);

      // ionisation
      G4ionIonisation * ionIoni = new G4ionIonisation();
      ionIoni->SetStepFunction(0.1, 20 * um);
      // pmanager->AddProcess(ionIoni, -1, 2, -2);
      pmanager->AddProcess(ionIoni, -1, 2, 2);
    }
    else if (particleName == "GenericIon") {
      // OBJECT may be dynamically created as either a GenericIon or nucleus
      // G4Nucleus exists and therefore has particle type nucleus
      // genericIon:

      // multiple scattering
      //pmanager->AddProcess(new G4hMultipleScattering, -1, 1, -1);
      pmanager->AddProcess(new G4hMultipleScattering, -1, 1, 1);

      // ionisation
      G4ionIonisation * ionIoni = new G4ionIonisation();
      ionIoni->SetEmModel(new G4IonParametrisedLossModel());
      ionIoni->SetStepFunction(0.1, 20 * um);
      //pmanager->AddProcess(ionIoni, -1, 2, 1);
      pmanager->AddProcess(ionIoni, -1, 2, 2);
    }

    else if ((!particle->IsShortLived()) && (charge != 0.0) &&
             (particle->GetParticleName() != "chargedgeantino")) {
      // all others charged particles except geantino
      G4hMultipleScattering * aMultipleScattering = new G4hMultipleScattering();
      G4hIonisation * ahadronIon = new G4hIonisation();

      // multiple scattering
      //pmanager->AddProcess(aMultipleScattering, -1, 1, -1);
      pmanager->AddProcess(aMultipleScattering, -1, 1, 1);

      // ionisation
      //pmanager->AddProcess(ahadronIon, -1, 2, 1);
      pmanager->AddProcess(ahadronIon, -1, 2, 2);
    }
  }
}

// Optical Processes ////////////////////////////////////////////////////////
// #include "G4Cerenkov.hh"
#include "G4EmSaturation.hh"
#include "G4OpBoundaryProcess.hh"
#include "LSCSim/LSCCerenkov.hh"
#include "LSCSim/LSCOpAttenuation.hh"
#include "LSCSim/LSCScintillation.hh"

void LSCPhysicsList::ConstructOp()
{
  G4ProcessManager * pManager =
      G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
  if (!pManager) {
    G4ExceptionDescription ed;
    ed << "Optical Photon without a Process Manager";
    G4Exception("G4OpticalPhysics::ConstructProcess()", "", FatalException, ed);
    return;
  }

  LSCOpAttenuation * attenuation = new LSCOpAttenuation();
  attenuation->UseTimeProfile("exponential");
  attenuation->SetVerboseLevel(OpVerbLevel);
  pManager->AddDiscreteProcess(attenuation);

  G4OpBoundaryProcess * boundary = new G4OpBoundaryProcess();
  boundary->SetVerboseLevel(OpVerbLevel);
  pManager->AddDiscreteProcess(boundary);

  // scintillation process
  LSCScintillation * scint = new LSCScintillation("Scintillation");
  scint->SetTrackSecondariesFirst(true);
  scint->SetScintillationYieldFactor(1.0);
  scint->SetScintillationExcitationRatio(0.0);
  scint->SetVerboseLevel(OpVerbLevel);

  G4EmSaturation * emSaturation =
      G4LossTableManager::Instance()->EmSaturation();
  scint->AddSaturation(emSaturation);

  // Cerenkov
  auto cerenkov = new LSCCerenkov("Cerenkov");
  cerenkov->SetTrackSecondariesFirst(true);

  auto myParticleIterator = GetParticleIterator();
  myParticleIterator->reset();

  while ((*myParticleIterator)()) {
    G4ParticleDefinition * particle = myParticleIterator->value();
    G4String particleName = particle->GetParticleName();

    pManager = particle->GetProcessManager();
    if (!pManager) {
      G4ExceptionDescription ed;
      ed << "Particle " << particleName << "without a Process Manager";
      G4Exception("G4OpticalPhysics::ConstructProcess()", "", FatalException,
                  ed);
      return; // else coverity complains for pManager use below
    }

    if (cerenkov->IsApplicable(*particle)) {
      pManager->AddProcess(cerenkov);
      pManager->SetProcessOrdering(cerenkov, idxPostStep);
    }
    if (scint->IsApplicable(*particle)) {
      pManager->AddProcess(scint);
      pManager->SetProcessOrderingToLast(scint, idxAtRest);
      pManager->SetProcessOrderingToLast(scint, idxPostStep);
    }
    if (boundary->IsApplicable(*particle)) {
      pManager->SetProcessOrderingToLast(boundary, idxPostStep);
    }
  }
}

/*
void LSCPhysicsList::ConstructOp()
{
  // EJ: start
  // scintillation process
  LSCScintillation * scint = new LSCScintillation("Scintillation");
  scint->SetTrackSecondariesFirst(true);
  scint->SetScintillationYieldFactor(1.0);     //
  scint->SetScintillationExcitationRatio(0.0); //
  scint->SetVerboseLevel(OpVerbLevel);

  G4EmSaturation * emSaturation =
      G4LossTableManager::Instance()->EmSaturation();
  scint->AddSaturation(emSaturation);

  // optical processes
  LSCOpAttenuation * attenuation = new LSCOpAttenuation();
  attenuation->UseTimeProfile("exponential");
  attenuation->SetVerboseLevel(OpVerbLevel);

  G4OpBoundaryProcess * boundary = new G4OpBoundaryProcess();
  boundary->SetVerboseLevel(OpVerbLevel);

  // Cerenkov
  // G4Cerenkov * cerenkov = new G4Cerenkov();
  auto cerenkov = new LSCCerenkov();
  cerenkov->SetTrackSecondariesFirst(true);

  auto theParticleIterator = GetParticleIterator();
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition * particle = theParticleIterator->value();
    G4ProcessManager * pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
    if (scint->IsApplicable(*particle)) {
      pmanager->AddProcess(scint);
      pmanager->SetProcessOrderingToLast(scint, idxAtRest);
      pmanager->SetProcessOrderingToLast(scint, idxPostStep);
    }
    if (cerenkov->IsApplicable(*particle)) {
      pmanager->AddProcess(cerenkov);
      pmanager->SetProcessOrdering(cerenkov, idxPostStep);
    }

    if (particleName == "opticalphoton") {
      pmanager->AddDiscreteProcess(attenuation);
      pmanager->AddDiscreteProcess(boundary);
      pmanager->SetProcessOrderingToLast(boundary, idxPostStep);
    }
  }
}
*/

// Hadronic processes ////////////////////////////////////////////////////////
// Elastic processes:
#include "G4ChipsElasticModel.hh"
#include "G4ElasticHadrNucleusHE.hh"
#include "G4HadronElasticProcess.hh"

// Inelastic processes:
#include "G4HadronInelasticProcess.hh"

// High energy FTFP model and Bertini cascade
#include "G4CascadeInterface.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4FTFModel.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4LundStringFragmentation.hh"
#include "G4PreCompoundModel.hh"
#include "G4TheoFSGenerator.hh"

// Cross sections
#include "G4VCrossSectionDataSet.hh"
#include "G4CrossSectionDataSetRegistry.hh"

#include "G4CrossSectionElastic.hh"
#include "G4CrossSectionInelastic.hh"
#include "G4BGGPionElasticXS.hh"
#include "G4BGGPionInelasticXS.hh"
#include "G4AntiNuclElastic.hh"

#include "G4CrossSectionInelastic.hh"
#include "G4BGGNucleonInelasticXS.hh"
#include "G4BGGNucleonElasticXS.hh"
#include "G4NeutronInelasticXS.hh"
#include "G4NeutronElasticXS.hh"
#include "G4ComponentAntiNuclNuclearXS.hh"
#include "G4ComponentGGNuclNuclXsc.hh"
#include "G4ComponentGGHadronNucleusXsc.hh"

#include "G4HadronElastic.hh"
#include "G4NeutronCaptureProcess.hh"
#include "G4NeutronFissionProcess.hh"

// Neutron high-precision models: <20 MeV
#include "G4ParticleHPElastic.hh"
#include "G4ParticleHPElasticData.hh"
#include "G4ParticleHPCapture.hh"
#include "G4ParticleHPCaptureData.hh"
#include "G4ParticleHPFissionData.hh"
#include "G4ParticleHPInelastic.hh"
#include "G4ParticleHPInelasticData.hh"
#include "G4ParticleHPFission.hh"

// Stopping processes
#include "G4HadronStoppingProcess.hh"
#include "G4HadronicAbsorptionBertini.hh"
#include "G4HadronicAbsorptionFritiof.hh"

#include "G4HadronicParameters.hh"

#include "G4Decay.hh"
#include "G4RadioactiveDecay.hh"
#include "G4PhysicsListHelper.hh"
#include "G4NuclideTable.hh"
#include "G4NuclearLevelData.hh"

// Muon Nuclear processes
#include "G4MuonNuclearProcess.hh"
#include "G4MuonVDNuclearModel.hh"

void LSCPhysicsList::ConstructHad() {
  //Elastic models
  G4HadronElastic* elastic_lhep0 = new G4HadronElastic();
  G4ChipsElasticModel* elastic_chip = new G4ChipsElasticModel();
  G4ElasticHadrNucleusHE* elastic_he = new G4ElasticHadrNucleusHE();

  // Inelastic scattering
  const G4double theFTFMin0 =    0.0*GeV;
  const G4double theFTFMin1 =    3.0*GeV;
  const G4double theFTFMax = G4HadronicParameters::Instance()->GetMaxEnergy();
  const G4double theBERTMin0 =   0.0*GeV;
  const G4double theBERTMin1 =  19.0*MeV;
  const G4double theBERTMax =    6.0*GeV;
  const G4double theHPMin =      0.0*GeV;
  const G4double theHPMax =     20.0*MeV;

  G4FTFModel *theStringModel           = new G4FTFModel;
	G4ExcitedStringDecay *theStringDecay = new G4ExcitedStringDecay(new G4LundStringFragmentation);
	theStringModel->SetFragmentationModel(theStringDecay);
	G4PreCompoundModel *thePreEquilib = new G4PreCompoundModel(new G4ExcitationHandler);
	G4GeneratorPrecompoundInterface *theCascade =
	new G4GeneratorPrecompoundInterface(thePreEquilib);

	G4TheoFSGenerator *theFTFModel0 = new G4TheoFSGenerator("FTFP");
	theFTFModel0->SetHighEnergyGenerator(theStringModel);
	theFTFModel0->SetTransport(theCascade);
	theFTFModel0->SetMinEnergy(theFTFMin0);
	theFTFModel0->SetMaxEnergy(theFTFMax);

	G4TheoFSGenerator *theFTFModel1 = new G4TheoFSGenerator("FTFP");
	theFTFModel1->SetHighEnergyGenerator(theStringModel);
	theFTFModel1->SetTransport(theCascade);
	theFTFModel1->SetMinEnergy(theFTFMin1);
	theFTFModel1->SetMaxEnergy(theFTFMax);

	G4CascadeInterface *theBERTModel0 = new G4CascadeInterface;
	theBERTModel0->SetMinEnergy(theBERTMin0);
	theBERTModel0->SetMaxEnergy(theBERTMax);

	G4CascadeInterface *theBERTModel1 = new G4CascadeInterface;
	theBERTModel1->SetMinEnergy(theBERTMin1);
	theBERTModel1->SetMaxEnergy(theBERTMax);

  G4VCrossSectionDataSet * theAntiNucleonData = new G4CrossSectionInelastic( new G4ComponentAntiNuclNuclearXS );
  G4ComponentGGNuclNuclXsc * ggNuclNuclXsec = new G4ComponentGGNuclNuclXsc();
  G4VCrossSectionDataSet * theGGNuclNuclData = new G4CrossSectionInelastic(ggNuclNuclXsec);
  G4VCrossSectionDataSet * theGGNNEl = new G4CrossSectionElastic(ggNuclNuclXsec);
  G4ComponentGGHadronNucleusXsc * ggHNXsec = new G4ComponentGGHadronNucleusXsc();
  G4VCrossSectionDataSet * theGGHNEl = new G4CrossSectionElastic(ggHNXsec);
  G4VCrossSectionDataSet * theGGHNInel = new G4CrossSectionInelastic(ggHNXsec);

  auto theParticleIterator = GetParticleIterator();

	theParticleIterator->reset();
	while ((*theParticleIterator)()) {
		G4ParticleDefinition *particle = theParticleIterator->value();
		G4ProcessManager *pmanager     = particle->GetProcessManager();
		G4String particleName          = particle->GetParticleName();

		if (particleName == "pi+") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( new G4BGGPionElasticXS( particle ) );
      theElasticProcess->RegisterMe( elastic_he );
      pmanager->AddDiscreteProcess( theElasticProcess );
      //Inelastic scattering
      G4HadronInelasticProcess* theInelasticProcess = 
          new G4HadronInelasticProcess( "inelastic", G4PionPlus::Definition() );
      theInelasticProcess->AddDataSet( new G4BGGPionElasticXS( particle ) );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 ); 
		  pmanager->AddDiscreteProcess( theInelasticProcess );
		}
    else if (particleName == "pi-") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( new G4BGGPionElasticXS( particle ) );
      theElasticProcess->RegisterMe( elastic_he );
      pmanager->AddDiscreteProcess( theElasticProcess );

      //Inelastic scattering
      G4HadronInelasticProcess* theInelasticProcess =
          new G4HadronInelasticProcess( "inelastic", G4PionMinus::Definition() );
      theInelasticProcess->AddDataSet( new G4BGGPionInelasticXS( particle ) );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
          
      //Absorption
      pmanager->AddRestProcess(new G4HadronicAbsorptionBertini(G4PionMinus::Definition()), ordDefault);
		}
		else if (particleName == "kaon+") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( theGGHNEl );
      theElasticProcess->RegisterMe( elastic_lhep0 );
      pmanager->AddDiscreteProcess( theElasticProcess );
      // Inelastic scattering       
      G4HadronInelasticProcess* theInelasticProcess =
          new G4HadronInelasticProcess( "inelastic", G4KaonPlus::Definition() );
      theInelasticProcess->AddDataSet( theGGHNInel );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
		}
    else if (particleName == "kaon0S") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( theGGHNEl );
      theElasticProcess->RegisterMe( elastic_lhep0 );
      pmanager->AddDiscreteProcess( theElasticProcess );
      // Inelastic scattering        
      G4HadronInelasticProcess* theInelasticProcess =
        new G4HadronInelasticProcess( "inelastic", G4KaonZeroShort::Definition() );
      theInelasticProcess->AddDataSet( theGGHNInel );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
		}
		else if (particleName == "kaon0L") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( theGGHNEl );
      theElasticProcess->RegisterMe( elastic_lhep0 );
      pmanager->AddDiscreteProcess( theElasticProcess );
      // Inelastic scattering
      G4HadronInelasticProcess* theInelasticProcess =
        new G4HadronInelasticProcess( "inelastic", G4KaonZeroLong::Definition() );
      theInelasticProcess->AddDataSet( theGGHNInel );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
		}
    else if (particleName == "kaon-") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( theGGHNEl );
      theElasticProcess->RegisterMe( elastic_lhep0 );
      pmanager->AddDiscreteProcess( theElasticProcess );
      // Inelastic scattering
      G4HadronInelasticProcess* theInelasticProcess =
        new G4HadronInelasticProcess( "inelastic", G4KaonMinus::Definition() );
      theInelasticProcess->AddDataSet( theGGHNInel );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
      pmanager->AddRestProcess(new G4HadronicAbsorptionBertini(G4KaonMinus::Definition()), ordDefault);
		}
		else if (particleName == "proton") {
      // Elastic scattering
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( new G4BGGNucleonElasticXS( G4Proton::Proton() ) );
      theElasticProcess->RegisterMe( elastic_chip );
      pmanager->AddDiscreteProcess( theElasticProcess );
      // Inelastic scattering
      G4HadronInelasticProcess* theInelasticProcess =
        new G4HadronInelasticProcess( "inelastic", G4Proton::Definition() );
      theInelasticProcess->AddDataSet( new G4BGGNucleonInelasticXS( G4Proton::Proton() ) );
      theInelasticProcess->RegisterMe( theFTFModel1 );
      theInelasticProcess->RegisterMe( theBERTModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
		}
    else if (particleName == "anti_proton") {
      // Elastic scattering
      const G4double elastic_elimitAntiNuc = 100.0*MeV;
      G4AntiNuclElastic* elastic_anuc = new G4AntiNuclElastic();
      elastic_anuc->SetMinEnergy( elastic_elimitAntiNuc );
      G4CrossSectionElastic* elastic_anucxs = new G4CrossSectionElastic( elastic_anuc->GetComponentCrossSection() );    
      G4HadronElastic* elastic_lhep2 = new G4HadronElastic();
      elastic_lhep2->SetMaxEnergy( elastic_elimitAntiNuc );
      G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet( elastic_anucxs );
      theElasticProcess->RegisterMe( elastic_lhep2 );
      theElasticProcess->RegisterMe( elastic_anuc );
      pmanager->AddDiscreteProcess( theElasticProcess );
      // Inelastic scattering
      G4HadronInelasticProcess* theInelasticProcess = 
        new G4HadronInelasticProcess( "inelastic", G4AntiProton::Definition() );
      theInelasticProcess->AddDataSet( theAntiNucleonData );
      theInelasticProcess->RegisterMe( theFTFModel0 );
      pmanager->AddDiscreteProcess( theInelasticProcess );
      // Absorption
      pmanager->AddRestProcess(new G4HadronicAbsorptionFritiof(G4AntiProton::Definition()), ordDefault);
		}
    else if (particleName == "neutron") {
			// elastic scattering
			G4HadronElasticProcess* theElasticProcess = new G4HadronElasticProcess;
      theElasticProcess->AddDataSet(new G4NeutronElasticXS());
      G4HadronElastic* elastic_neutronChipsModel = new G4ChipsElasticModel();
			if (omitNeutHP) {
				elastic_neutronChipsModel->SetMinEnergy( 19.0*MeV );
        theElasticProcess->RegisterMe( elastic_neutronChipsModel );
			} else {
				elastic_neutronChipsModel->SetMinEnergy( 19.0*MeV );
        theElasticProcess->RegisterMe( elastic_neutronChipsModel );
				G4ParticleHPElastic * theElasticNeutronHP = new G4ParticleHPElastic;
        theElasticNeutronHP->SetMinEnergy( theHPMin );
        theElasticNeutronHP->SetMaxEnergy( theHPMax );
				theElasticProcess->RegisterMe( theElasticNeutronHP );
				theElasticProcess->AddDataSet( new G4ParticleHPElasticData );
			}
			pmanager->AddDiscreteProcess( theElasticProcess );

      // inelastic scattering
			G4HadronInelasticProcess* theInelasticProcess =
	  		  new G4HadronInelasticProcess( "inelastic", G4Neutron::Definition() );
			theInelasticProcess->AddDataSet( new G4NeutronInelasticXS() );
			if (omitNeutHP) {
				theInelasticProcess->RegisterMe(theFTFModel1);
				theInelasticProcess->RegisterMe(theBERTModel1);
			} else {
				theInelasticProcess->RegisterMe(theFTFModel1);
				theInelasticProcess->RegisterMe(theBERTModel1);
				G4ParticleHPInelastic * theNeutronInelasticHPModel = new G4ParticleHPInelastic;
				theNeutronInelasticHPModel->SetMinEnergy(theHPMin);
				theNeutronInelasticHPModel->SetMaxEnergy(theHPMax);
				theInelasticProcess->RegisterMe(theNeutronInelasticHPModel);
				theInelasticProcess->AddDataSet(new G4ParticleHPInelasticData);
			}
			pmanager->AddDiscreteProcess(theInelasticProcess);

      // capture
			G4NeutronCaptureProcess* theCaptureProcess =
	  		  new G4NeutronCaptureProcess;
			G4ParticleHPCapture * theLENeutronCaptureModel = new G4ParticleHPCapture;
			if (omitNeutHP) {
				theCaptureProcess->RegisterMe(theLENeutronCaptureModel);
			} else {
				theLENeutronCaptureModel->SetMinEnergy(theHPMin);
				theLENeutronCaptureModel->SetMaxEnergy(theHPMax);
				theCaptureProcess->RegisterMe(theLENeutronCaptureModel);
				theCaptureProcess->AddDataSet(new G4ParticleHPCaptureData);
			}
			pmanager->AddDiscreteProcess(theCaptureProcess);
			// fission
			G4NeutronFissionProcess *theNeutronFissionProcess = new G4NeutronFissionProcess();
			G4ParticleHPFission *theNeutronLFission          = new G4ParticleHPFission();
			if (omitNeutHP) {
				theNeutronFissionProcess->RegisterMe(theNeutronLFission);
			} else {
				G4ParticleHPFission *theNeutronHPFission = new G4ParticleHPFission();
				theNeutronHPFission->SetMaxEnergy(20. * MeV);
				theNeutronHPFission->SetMinEnergy(20. * MeV);
				theNeutronFissionProcess->RegisterMe(theNeutronHPFission);
				theNeutronFissionProcess->RegisterMe(theNeutronLFission);
				theNeutronFissionProcess->AddDataSet(new G4ParticleHPFissionData);
			}
			pmanager->AddDiscreteProcess(theNeutronFissionProcess);

    } else if (particleName == "anti_neutron") {
			// Elastic scattering
			G4HadronElasticProcess *theElasticProcess = new G4HadronElasticProcess;
			theElasticProcess->AddDataSet(theAntiNucleonData);	
			theElasticProcess->RegisterMe(elastic_lhep0);
			pmanager->AddDiscreteProcess(theElasticProcess);
			// Inelastic scattering (include annihilation on-fly)
			G4HadronInelasticProcess* theInelasticProcess = 
	    	  new G4HadronInelasticProcess( "inelastic", G4AntiNeutron::Definition() );
			theInelasticProcess->AddDataSet(theAntiNucleonData);
			theInelasticProcess->RegisterMe(theFTFModel0);
			pmanager->AddDiscreteProcess(theInelasticProcess);
		}
    else if (particleName == "deuteron") {
			// Elastic scattering
			G4HadronElasticProcess *theElasticProcess = new G4HadronElasticProcess;
			theElasticProcess->AddDataSet(theGGNuclNuclData);
			theElasticProcess->RegisterMe(elastic_lhep0);
			pmanager->AddDiscreteProcess(theElasticProcess);
			// Inelastic scattering
			G4HadronInelasticProcess* theInelasticProcess = 
	    	  new G4HadronInelasticProcess( "inelastic", G4Deuteron::Definition() );
			theInelasticProcess->AddDataSet(theGGNuclNuclData);
			theInelasticProcess->RegisterMe(theFTFModel1);
			theInelasticProcess->RegisterMe(theBERTModel0);
			pmanager->AddDiscreteProcess(theInelasticProcess);
		}
		else if (particleName == "triton") {
			// Elastic scattering
			G4HadronElasticProcess *theElasticProcess = new G4HadronElasticProcess;
			theElasticProcess->AddDataSet(theGGNuclNuclData);
			theElasticProcess->RegisterMe(elastic_lhep0);
			pmanager->AddDiscreteProcess(theElasticProcess);
			// Inelastic scattering
			G4HadronInelasticProcess* theInelasticProcess = 
	    	  new G4HadronInelasticProcess( "inelastic", G4Triton::Definition() );
			theInelasticProcess->AddDataSet(theGGNuclNuclData);
			theInelasticProcess->RegisterMe(theFTFModel1);
			theInelasticProcess->RegisterMe(theBERTModel0);
			pmanager->AddDiscreteProcess(theInelasticProcess);
		}
    else if (particleName == "alpha") {
			// Elastic scattering
			G4HadronElasticProcess *theElasticProcess = new G4HadronElasticProcess;
			theElasticProcess->AddDataSet(theGGNuclNuclData);
			theElasticProcess->RegisterMe(elastic_lhep0);
			pmanager->AddDiscreteProcess(theElasticProcess);
			// Inelastic scattering
			G4HadronInelasticProcess* theInelasticProcess = 
	    	  new G4HadronInelasticProcess( "inelastic", G4Alpha::Definition() );	
			theInelasticProcess->AddDataSet(theGGNuclNuclData);
			theInelasticProcess->RegisterMe(theFTFModel1);
			theInelasticProcess->RegisterMe(theBERTModel0);
			pmanager->AddDiscreteProcess(theInelasticProcess);
		}
		//for muon nuclear processes
		else if (particleName == "mu-") {
			G4MuonNuclearProcess *muNuclearProcess = new G4MuonNuclearProcess();
			G4MuonVDNuclearModel *muNuclearModel   = new G4MuonVDNuclearModel();
			muNuclearProcess->RegisterMe(muNuclearModel);
			pmanager->AddDiscreteProcess(muNuclearProcess);
		}
	}
}

// Decays ///////////////////////////////////////////////////////////////////
#include "G4Decay.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"
#include "G4PhysicsListHelper.hh"
#include "G4RadioactiveDecay.hh"
#include "GLG4Sim/GLG4DeferTrackProc.hh"

void LSCPhysicsList::ConstructGeneral()
{
  auto theParticleIterator = GetParticleIterator();

  // Add Decay Process
  G4Decay * theDecayProcess = new G4Decay();
  GLG4DeferTrackProc * theDeferProcess = new GLG4DeferTrackProc();
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition * particle = theParticleIterator->value();
    G4ProcessManager * pmanager = particle->GetProcessManager();

    if (theDecayProcess->IsApplicable(*particle) && !particle->IsShortLived()) {
      pmanager->AddProcess(theDecayProcess);
      // set ordering for PostStepDoIt and AtRestDoIt
      pmanager->SetProcessOrdering(theDecayProcess, idxPostStep);
      pmanager->SetProcessOrdering(theDecayProcess, idxAtRest);
    }
    if (!particle->IsShortLived())
      pmanager->AddDiscreteProcess(theDeferProcess);
  }

  // Declare radioactive decay to the GenericIon in the IonTable.
  // Declare radioactive decay to the GenericIon in the IonTable.
  G4LossTableManager * man = G4LossTableManager::Instance();
  G4VAtomDeexcitation * ad = man->AtomDeexcitation();
  if (!ad) {
    G4EmParameters::Instance()->SetAugerCascade(true);
    ad = new G4UAtomicDeexcitation();
    man->SetAtomDeexcitation(ad);
    ad->InitialiseAtomicDeexcitation();
  }

  G4PhysicsListHelper::GetPhysicsListHelper()->RegisterProcess(
      new G4RadioactiveDecay(), G4GenericIon::GenericIon());
}

// Cuts /////////////////////////////////////////////////////////////////////
void LSCPhysicsList::SetCuts()
{

  if (verboseLevel > 1) G4cout << "LSCPhysicsList::SetCuts:";
  if (verboseLevel > 0) {
    G4cout << "LSCPhysicsList::SetCuts:";
    G4cout << "CutLength : " << G4BestUnit(defaultCutValue, "Length") << G4endl;
  }

  // special for low energy physics
  // G4double lowlimit=250*eV;
  // G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(lowlimit,100.*GeV);

  // set cut values for gamma at first and for e- second and next for e+,
  // because some processes for e+/e- need cut values for gamma
  SetCutValue(cutForGamma, "gamma");
  SetCutValue(cutForElectron, "e-");
  SetCutValue(cutForPositron, "e+");

  if (verboseLevel > 0) DumpCutValuesTable();
}

void LSCPhysicsList::SetNewValue(G4UIcommand * command, G4String newValue)
{
  if (command == gammaCutCmd)
    cutForGamma = gammaCutCmd->GetNewDoubleValue(newValue);
  else if (command == electCutCmd)
    cutForElectron = electCutCmd->GetNewDoubleValue(newValue);
  else if (command == posCutCmd)
    cutForPositron = posCutCmd->GetNewDoubleValue(newValue);
  else if (command == allCutCmd) {
    G4double cut = allCutCmd->GetNewDoubleValue(newValue);
    cutForGamma = cut;
    cutForElectron = cut;
    cutForPositron = cut;
  }
  else if (command == verboseCmd)
    VerboseLevel = verboseCmd->GetNewIntValue(newValue);
}
