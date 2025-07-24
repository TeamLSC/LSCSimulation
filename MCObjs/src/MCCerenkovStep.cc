#include "MCObjs/MCCerenkovStep.hh"

ClassImp(MCCerenkovStep)

MCCerenkovStep::MCCerenkovStep()
    : MCStep()
{
  // fEnergyVisible = 0;
  fNCerenkovPhoton = 0;
}

MCCerenkovStep::MCCerenkovStep(const MCCerenkovStep & step)
    : MCStep(step)
{
  // fEnergyVisible = step.GetEnergyVisible();
  fNCerenkovPhoton = step.GetNCerenkovPhoton();
}

MCCerenkovStep::~MCCerenkovStep() {}
