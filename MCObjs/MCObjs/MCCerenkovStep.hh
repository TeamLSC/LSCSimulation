#ifndef MCCerenkovStep_hh
#define MCCerenkovStep_hh

#include "MCObjs/MCStep.hh"

class MCCerenkovStep : public MCStep {
public:
  MCCerenkovStep();
  MCCerenkovStep(const MCCerenkovStep & step);
  virtual ~MCCerenkovStep();

  // inline void SetEnergyVisible(double val) { fEnergyVisible = val; }
  inline void SetNCerenkovPhoton(int val) { fNCerenkovPhoton = val; }

  // inline float GetEnergyVisible() const { return fEnergyVisible; }
  inline int GetNCerenkovPhoton() const { return fNCerenkovPhoton; }

private:
  int fNCerenkovPhoton;
  // float fEnergyVisible;

  ClassDef(MCCerenkovStep, 1)
};
#endif
