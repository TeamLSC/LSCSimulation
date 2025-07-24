#ifndef LSCRootManager_hh
#define LSCRootManager_hh

#include "globals.hh"

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4VProcess.hh"
#include "G4UImessenger.hh"

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "MCObjs/MCPrimaryData.hh"
#include "MCObjs/MCTrackData.hh"
#include "MCObjs/MCScintData.hh"
#include "MCObjs/MCCerenkovData.hh"
#include "MCObjs/MCPMTData.hh"
#include "MCObjs/MCEventInfo.hh"

#include "LSCSim/LSCRecorderBase.hh"

class G4UIdirectory;
class G4UIcmdWithAString;

class LSCRootManager : public LSCRecorderBase, public G4UImessenger {
public:
  LSCRootManager();
  virtual ~LSCRootManager();

  virtual void SetNewValue(G4UIcommand *, G4String);

  virtual void BeginOfRun(const G4Run *);
  virtual void EndOfRun(const G4Run *);
  virtual void BeginOfEvent(const G4Event *);
  virtual void EndOfEvent(const G4Event *);
  virtual void RecordTrack(const G4Track *);
  virtual void RecordStep(const G4Step *, const G4VProcess *);

  struct RunInfo {
    G4int fRunNumber;
    G4double fTargetRadius;
    G4double fTargetHeight;
  };

  void SetRootFile(const char * fname)
  { fRootFileName = fname; }
  void OpenRootFile();
  void CloseRootFile();

  void Booking();

private:
  G4UIdirectory * fROOTDir;

  G4UIcmdWithAString * fTrackSaveOptCmd;
  G4UIcmdWithAString * fStepSaveOptCmd;
  G4UIcmdWithAString * fHitPhotonSaveCmd;
  G4UIcmdWithAString * fScintStepSaveCmd;
  G4UIcmdWithAString * fCerenkovStepSaveCmd;

  G4int fTrackSaveOption;
  G4int fStepSaveOption;
  G4int fHitPhotonSave;
  G4int fScintStepSave;
  G4int fCerenkovStepSave;

  G4int fPMTHitCollId;

  MCPrimaryData * fPrimaryData;
  MCTrackData * fTrackData;
  MCScintData * fScintData;
  MCCerenkovData* fCerenkovData;
  MCPMTData * fPMTData;
  MCEventInfo * fEventInfo;

  TString fRootFileName;
  TFile * fRootFile;
  TTree * fRunTree;
  TTree * fEventTree;
};

#endif
