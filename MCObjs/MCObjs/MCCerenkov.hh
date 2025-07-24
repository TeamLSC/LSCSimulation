#ifndef MCCerenkov_hh
#define MCCerenkov_hh

#include "TClonesArray.h"

class MCCerenkovStep;
class MCCerenkov : public TClonesArray {
public:
  MCCerenkov();
  MCCerenkov(int id);
  MCCerenkov(const MCCerenkov & cerenkov);
  virtual ~MCCerenkov();

  virtual void Clear(const Option_t * opt = "");

  void SetVolumeId(int val);
  // void AddEnergyDeposit(float val);
  // void AddEnergyVisible(float val);
  void AddCerenkovPhotons(int val);

  int GetVolumeId() const;
  // float GetEnergyDeposit() const;
  // float GetEnergyVisible() const;
  int GetNCerenkovPhoton() const;

  MCCerenkovStep * AddStep();
  int GetNStep() const;
  MCCerenkovStep * GetStep(int i) const;

  virtual void Print(const Option_t * opt = "") const;

private:
  int fVolumeId;
  int fNCerenkovPhoton;
  // float fEdep;
  // float fEdepQuenched;

  // Step
  int fNStep; //!

  ClassDef(MCCerenkov, 1)
};

inline void MCCerenkov::SetVolumeId(int val) { fVolumeId = val; }
// inline void MCCerenkov::AddEnergyDeposit(float val) { fEdep += val; }
// inline void MCCerenkov::AddEnergyVisible(float val) { fEdepQuenched += val; }
inline void MCCerenkov::AddCerenkovPhotons(int val) { fNCerenkovPhoton += val; }
inline int MCCerenkov::GetVolumeId() const { return fVolumeId; }
// inline float MCCerenkov::GetEnergyDeposit() const { return fEdep; }
// inline float MCCerenkov::GetEnergyVisible() const { return fEdepQuenched; }
inline int MCCerenkov::GetNCerenkovPhoton() const { return fNCerenkovPhoton; }
inline int MCCerenkov::GetNStep() const { return GetEntriesFast(); }
inline MCCerenkovStep * MCCerenkov::GetStep(int n) const { return (MCCerenkovStep *)At(n); }

#endif
