#include "LSCSim/LSCSteppingAction.hh"
#include "LSCSim/LSCRecorderBase.hh"
#include "LSCSim/LSCUserTrackInformation.hh"

#include "G4VProcess.hh"
#include "G4SteppingManager.hh"

LSCSteppingAction::LSCSteppingAction(LSCRecorderBase * r)
    : fRecorder(r)
{
}

LSCSteppingAction::~LSCSteppingAction() {}

void LSCSteppingAction::UserSteppingAction(const G4Step * theStep)
{
  G4SteppingManager * steppingManager = fpSteppingManager;
  G4VProcess * proc = steppingManager->GetfCurrentProcess();

  /*
  G4StepPoint* postStepPoint = theStep->GetPostStepPoint();
  G4Track* theTrack = theStep->GetTrack();

  if (theStep->GetTrack()->GetDefinition()->GetParticleName() == "opticalphoton") {
    // If the particle is not an optical photon, we do not need to record the step
    if (postStepPoint->GetPhysicalVolume()->GetName() == "InnerPMT_body_phys") {
      LSCUserTrackInformation * trackInformation =
        (LSCUserTrackInformation *)theTrack->GetUserInformation();
      trackInformation->AddTrackStatusFlag(hitPMT);
      //trackInformation->SetTrackStatusFlags(inactive);
      //G4cout << "Optical photon hit PMT: " << theTrack->GetTrackID() << G4endl;
    }
  }
  */
  if (fRecorder) fRecorder->RecordStep(theStep, proc);
}
