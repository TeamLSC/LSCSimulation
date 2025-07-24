#include "MCObjs/MCCerenkov.hh"

#include <iostream>

#include "MCObjs/MCCerenkovStep.hh"

using namespace std;

ClassImp(MCCerenkov)

MCCerenkov::MCCerenkov()
  : TClonesArray("MCCerenkovStep")
{
  fVolumeId = 0;

  // Cerenkov
  // fEdep = 0;
  // fEdepQuenched = 0;
  fNCerenkovPhoton = 0;

  fNStep = 0;
}

MCCerenkov::MCCerenkov(int id)
  : TClonesArray("MCCerenkovStep")
{
  fVolumeId = id;

  // Cerenkov
  // fEdep = 0;
  // fEdepQuenched = 0;
  fNCerenkovPhoton = 0;

  fNStep = 0;
}

MCCerenkov::MCCerenkov(const MCCerenkov & cerenkov)
  : TClonesArray(cerenkov)
{
  fVolumeId = cerenkov.GetVolumeId();

  // Cerenkov
  // fEdep = cerenkov.GetEnergyDeposit();
  // fEdepQuenched = cerenkov.GetEnergyVisible();
  fNCerenkovPhoton = cerenkov.GetNCerenkovPhoton();
}

MCCerenkov::~MCCerenkov() {}

MCCerenkovStep * MCCerenkov::AddStep()
{
  return new ((*this)[fNStep++]) MCCerenkovStep();
}

void MCCerenkov::Clear(const Option_t * opt)
{
  // Cerenkov
  // fEdep = 0;
  // fEdepQuenched = 0;
  fNCerenkovPhoton = 0;

  fNStep = 0;
  Delete();
}

void MCCerenkov::Print(const Option_t * opt) const
{
  cout << Form("Volume: %d ", fVolumeId) << endl;
  // cout << Form("   Deposit Energy    = %.6f [MeV]", GetEnergyDeposit())
  //      << endl;
  // cout << Form("   Visible Energy    = %.6f [MeV]", GetEnergyVisible())
  //      << endl;
  cout << Form("   # of Cerenkov Photon = %d         ", GetNCerenkovPhoton()) << endl;

  if (GetNStep() > 0) {
    cout << Form("   # of Cerenkov Step   = %d         ", GetNStep()) << endl;
  }
}
