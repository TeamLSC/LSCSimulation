#include "LSCSim/LSCRootManager.hh"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "G4HCofThisEvent.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4Neutron.hh"
#include "G4OpticalPhoton.hh"
#include "G4ParticleDefinition.hh"
#include "G4PrimaryVertex.hh"
#include "G4ProcessType.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4TouchableHandle.hh"
#include "G4TrajectoryContainer.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4VPhysicalVolume.hh"
#include "LSCSim/LSCScintillation.hh"
#include "LSCSim/LSCCerenkov.hh"
#include "LSCSim/PMTHit.hh"
#include "MCObjs/MCPMT.hh"
#include "MCObjs/MCPhotonHit.hh"
#include "MCObjs/MCPrimary.hh"
#include "MCObjs/MCScint.hh"
#include "MCObjs/MCScintStep.hh"
#include "MCObjs/MCCerenkov.hh"
#include "MCObjs/MCCerenkovStep.hh"
#include "MCObjs/MCTrack.hh"

using namespace std;
using namespace CLHEP;

LSCRootManager::LSCRootManager()
  : G4UImessenger(),
    fPMTHitCollId(-1)
{
  fTrackSaveOption = 0;
  fStepSaveOption = 0;
  fHitPhotonSave = 0;
  fScintStepSave = 0;
  fCerenkovStepSave = 0;

  fROOTDir = new G4UIdirectory("/LSC/ROOT/");

  fTrackSaveOptCmd = new G4UIcmdWithAString("/LSC/ROOT/savetrackopt", this);
  fStepSaveOptCmd = new G4UIcmdWithAString("/LSC/ROOT/savestepopt", this);
  fHitPhotonSaveCmd = new G4UIcmdWithAString("/LSC/ROOT/savehitphoton", this);
  fScintStepSaveCmd = new G4UIcmdWithAString("/LSC/ROOT/savescintstep", this);
  fCerenkovStepSaveCmd = new G4UIcmdWithAString("/LSC/ROOT/savecerenkovstep", this);

  G4cout << "LSCRootManager::LSCRootManager() created" << G4endl;
}

LSCRootManager::~LSCRootManager()
{
  delete fTrackSaveOptCmd;
  delete fStepSaveOptCmd;
  delete fHitPhotonSaveCmd;
  delete fScintStepSaveCmd;
  delete fCerenkovStepSaveCmd;

  G4cout << "LSCRootManager::LSCRootManager() destroyed" << G4endl;
}

void LSCRootManager::SetNewValue(G4UIcommand * command, G4String newValues)
{
  if (command == fTrackSaveOptCmd) {
    istringstream is((const char *)newValues);
    is >> fTrackSaveOption;
  }
  else if (command == fStepSaveOptCmd) {
    istringstream is((const char *)newValues);
    is >> fStepSaveOption;
  }
  else if (command == fHitPhotonSaveCmd) {
    istringstream is((const char *)newValues);
    is >> fHitPhotonSave;
  }
  else if (command == fScintStepSaveCmd) {
    istringstream is((const char *)newValues);
    is >> fScintStepSave;
  }
  else if (command == fCerenkovStepSaveCmd) {
    istringstream is((const char *)newValues);
    is >> fCerenkovStepSave;
  }
}

void LSCRootManager::BeginOfRun(const G4Run * aRun)
{
  OpenRootFile();
  Booking();

  G4int runId = aRun->GetRunID();
  G4int nEventToBeProcessed = aRun->GetNumberOfEventToBeProcessed();

  G4cout << G4endl;
  G4cout << "++++++++++++++++++ Run Initialized ++++++++++++++++++" << G4endl;
  G4cout << "   RunID              : " << runId << G4endl;
  G4cout << "   NEventToBeProcessed: " << nEventToBeProcessed << G4endl;
  G4cout << "++++++++++++++++++ Run Initialized ++++++++++++++++++" << G4endl;
  G4cout << G4endl;
}

void LSCRootManager::EndOfRun(const G4Run * aRun)
{
  G4int runId = aRun->GetRunID();
  G4int nEventProcessed = aRun->GetNumberOfEvent();

  CloseRootFile();

  delete fPrimaryData;
  delete fTrackData;
  delete fScintData;
  delete fCerenkovData;
  delete fPMTData;

  G4cout << G4endl;
  G4cout << "++++++++++++++++++  Run Finalized  ++++++++++++++++++" << G4endl;
  G4cout << "   RunID          : " << runId << G4endl;
  G4cout << "   NEventProcessed: " << nEventProcessed << G4endl;
  G4cout << "++++++++++++++++++  Run Finalized  ++++++++++++++++++" << G4endl;
  G4cout << G4endl;
}

void LSCRootManager::BeginOfEvent(const G4Event *)
{
  fPrimaryData->Clear();
  fTrackData->Clear();
  fScintData->Clear();
  fCerenkovData->Clear();
  fPMTData->Clear();

  G4SDManager * SDman = G4SDManager::GetSDMpointer();
  if (fPMTHitCollId < 0) {
    fPMTHitCollId = SDman->GetCollectionID("pmtHitCollection");
  }
}

void LSCRootManager::EndOfEvent(const G4Event * anEvent)
{
  G4RunManager * runManager = G4RunManager::GetRunManager();
  G4int eventId = anEvent->GetEventID() + 1; // event number starts from 1

  fEventInfo->SetEventNumber(eventId);

  // primary vertex
  G4int npvx = anEvent->GetNumberOfPrimaryVertex();
  for (int i = 0; i < npvx; i++) {
    G4PrimaryVertex * pvx = anEvent->GetPrimaryVertex(i);
    double x0 = pvx->GetX0();
    double y0 = pvx->GetY0();
    double z0 = pvx->GetZ0();
    double t0 = pvx->GetT0();

    int nptl = pvx->GetNumberOfParticle();
    for (int j = 0; j < nptl; j++) {
      G4PrimaryParticle * ptl = pvx->GetPrimary(j);

      MCPrimary * prim = fPrimaryData->Add();
      prim->SetT0(t0);
      prim->SetVertex(x0, y0, z0);
      prim->SetMomentum(ptl->GetPx(), ptl->GetPy(), ptl->GetPz());
      prim->SetKineticEnergy(ptl->GetKineticEnergy());
      prim->SetTrackId(ptl->GetTrackID());
      prim->SetParticleName(ptl->GetG4code()->GetParticleName().c_str());
    }
  }

  // PMT Hits
  G4HCofThisEvent * hits = anEvent->GetHCofThisEvent();
  PMTHitsCollection * pmtHC = nullptr;

  if (hits && (fPMTHitCollId >= 0)) {
    pmtHC = (PMTHitsCollection *)(hits->GetHC(fPMTHitCollId));
  }

  if (pmtHC) {
    G4int nPMT = pmtHC->entries();

    for (int i = 0; i < nPMT; i++) {
      const PMTHit * pmt = (*pmtHC)[i];
      int pmtId = pmt->GetPMTId();

      MCPMT * mcpmt = fPMTData->Add();
      mcpmt->SetId(pmtId);

      int npe = 0;

      int nph = pmt->GetNHit();
      if (fHitPhotonSave) {
        for (int j = 0; j < nph; j++) {
          MCPhotonHit * ph = pmt->GetHit(j);
          mcpmt->AddHit(ph);
        }
      }
    }
  }

  fEventTree->Fill();

  G4cout << std::setw(12) << eventId << " events processed ..." << G4endl;
}

void LSCRootManager::RecordTrack(const G4Track * gtrack)
{
  //G4cout << "RecordTrack is called for track ID: "
  //       << gtrack->GetTrackID() << G4endl;
  const G4VProcess * proc = gtrack->GetCreatorProcess();
  G4String particleName = gtrack->GetParticleDefinition()->GetParticleName();
  G4String processName = proc ? proc->GetProcessName() : "";

  /* Cerenkov (added by kmlee) */
  if (particleName == "opticalphoton" && processName == "Cerenkov") {
    LSCCerenkov * cerenkovproc = (LSCCerenkov *) proc; 
    
    G4VPhysicalVolume * volume = gtrack->GetVolume();
    G4String volumeName = volume->GetName();
    int volumeId = 0;

    if (G4StrUtil::contains(volumeName, "LSPhys")) {
      volumeId = G4StrUtil::contains(volumeName, "Target") ? 0 : 1;

      auto aCerenkov = fCerenkovData->FindCerenkov(volumeId);
      if (!aCerenkov) { aCerenkov = fCerenkovData->Add(volumeId); }

      aCerenkov->AddCerenkovPhotons(cerenkovproc->GetNumPhotons());
    }
    cerenkovproc->InitializeCerenkov();
  }

  if (fTrackSaveOption == 0) return;

  G4ProcessType processType = proc ? proc->GetProcessType() : fNotDefined;

  //if (fTrackSaveOption == 1 && particleName == "opticalphoton") return; 
  if (fTrackSaveOption == 1 && processName != "Cerenkov") return;

  if (fTrackSaveOption > 1 && particleName == "opticalphoton") return;

  if (fTrackSaveOption > 2) {
    // low energy electrons
    if (particleName == "e-") {
      if (processName == "compt") return;
      if (processName == "phot") return;
      if (processName == "conv") return;
      if (processName == "annihil") return;
      if (G4StrUtil::contains(processName, "Ioni")) return;
      if (G4StrUtil::contains(processName, "Brem")) return;
    }
  }
  if (fTrackSaveOption == 4 && particleName == "e-") return;

  MCTrack * mtrack = fTrackData->FindTrack(gtrack->GetTrackID());
  if (!mtrack) {
    mtrack = fTrackData->Add();
    mtrack->SetParticleName(
        gtrack->GetParticleDefinition()->GetParticleName().data());
    mtrack->SetPDGCode(gtrack->GetParticleDefinition()->GetPDGEncoding());
    mtrack->SetTrackId(gtrack->GetTrackID());
    mtrack->SetParentId(gtrack->GetParentID());
    mtrack->SetVertex(gtrack->GetVertexPosition().x(),
                      gtrack->GetVertexPosition().y(),
                      gtrack->GetVertexPosition().z());
    mtrack->SetKineticEnergy(gtrack->GetVertexKineticEnergy());
    mtrack->SetGlobalTime(gtrack->GetGlobalTime());

    if (proc) { mtrack->SetProcessName(proc->GetProcessName().data()); }
  }
}

void LSCRootManager::RecordStep(const G4Step * aStep, const G4VProcess * proc)
{
  if (!aStep) return;
  if (!proc) return;

  G4StepPoint * preStepPoint = aStep->GetPreStepPoint();
  G4StepPoint * postStepPoint = aStep->GetPostStepPoint();

  G4VPhysicalVolume * volume = postStepPoint->GetPhysicalVolume();
  if (!volume) return;

  G4Track * track = aStep->GetTrack();
  const G4ParticleDefinition * particleDef = track->GetParticleDefinition();
  
  G4String procName = proc->GetProcessName();

  /* Scintillation photon */
  if (proc->GetProcessName() == "Scintillation") {
    LSCScintillation * scintproc = (LSCScintillation *)proc;
    G4String volumeName = volume->GetName();
    if (G4StrUtil::contains(volumeName, "LSPhys")) {
      int volumeId;
      if (G4StrUtil::contains(volumeName, "Target")) { volumeId = 0; }
      else {
        volumeId = 1;
      }

      auto aScint = fScintData->FindScint(volumeId);
      if (!aScint) { aScint = fScintData->Add(volumeId); }

      aScint->AddEnergyDeposit(scintproc->GetEnergyDeposit());
      aScint->AddEnergyVisible(scintproc->GetEnergyVisible());
      aScint->AddScintPhotons(scintproc->GetNScintillationPhoton());

      if (fScintStepSave) {
        MCScintStep * step = aScint->AddStep();
        step->SetStepLength(aStep->GetStepLength());
        step->SetEnergyDeposit(scintproc->GetEnergyDeposit());
        step->SetEnergyVisible(scintproc->GetEnergyVisible());
        step->SetGlobalTime(postStepPoint->GetGlobalTime());
        step->SetVolumeName(volume->GetName().data());
        step->SetNScintPhoton(scintproc->GetNScintillationPhoton());
      }
    }
    scintproc->InitializeScint();
  }

  if (fTrackSaveOption > 0 && fStepSaveOption > 0) {
    int trackId = track->GetTrackID();
    MCTrack * mcTrack = fTrackData->FindTrack(trackId);
    if (mcTrack) {
      MCStep * mcStep = mcTrack->AddStep();
      mcStep->SetStepLength(aStep->GetStepLength());
      mcStep->SetEnergyDeposit(aStep->GetTotalEnergyDeposit());
      mcStep->SetEnergyDepositNonIonizing(aStep->GetNonIonizingEnergyDeposit());
      mcStep->SetGlobalTime(postStepPoint->GetGlobalTime());
      mcStep->SetVolumeName(volume->GetName().data());
    }
  }
}

void LSCRootManager::OpenRootFile()
{
  fRootFile = new TFile(fRootFileName.Data(), "recreate");
  G4cout << "LSCRootManager::OpenRootFile(): output file " << fRootFileName
         << " opened ..." << G4endl;
}

void LSCRootManager::CloseRootFile()
{
  fRootFile->Write();
  fRootFile->Close();
  delete fRootFile;
  fRootFile = NULL;
}

void LSCRootManager::Booking()
{
  fEventInfo = new MCEventInfo();
  fPrimaryData = new MCPrimaryData();
  fTrackData = new MCTrackData();
  fScintData = new MCScintData();
  fCerenkovData = new MCCerenkovData();
  fPMTData = new MCPMTData();

  fEventTree = new TTree("Event", "Event");
  fEventTree->Branch("MCEventInfo", &fEventInfo);
  fEventTree->Branch("MCPrimaryData", &fPrimaryData);
  fEventTree->Branch("MCTrackData", &fTrackData);
  fEventTree->Branch("MCScintData", &fScintData);
  fEventTree->Branch("MCCerenkovData", &fCerenkovData);
  fEventTree->Branch("MCPMTData", &fPMTData);
}
