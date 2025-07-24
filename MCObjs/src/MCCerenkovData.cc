#include "MCObjs/MCCerenkovData.hh"

#include <iostream>

#include "MCObjs/MCCerenkov.hh"

using namespace std;

ClassImp(MCCerenkovData)

MCCerenkovData::MCCerenkovData()
  : TClonesArray("MCCerenkov")
{
  fN = 0;
}

MCCerenkovData::MCCerenkovData(const MCCerenkovData & data)
  : TClonesArray(data)
{
}

MCCerenkovData::~MCCerenkovData() {}

MCCerenkov * MCCerenkovData::Add() { return new ((*this)[fN++]) MCCerenkov(); }
MCCerenkov * MCCerenkovData::Add(int id) { return new ((*this)[fN++]) MCCerenkov(id); }

void MCCerenkovData::Clear(const Option_t * opt)
{
  fN = 0;
  Delete();
}

MCCerenkov * MCCerenkovData::FindCerenkov(int id)
{
  MCCerenkov * cerenkov = nullptr;

  int n = GetN();
  for (int i = 0; i < n; i++) {
    MCCerenkov * rcerenkov = Get(i);
    if (rcerenkov->GetVolumeId() == id) {
      cerenkov = rcerenkov;
      break;
    }
  }

  return cerenkov;
}

void MCCerenkovData::Print(const Option_t * opt) const
{
  int n = GetN();
  cout << Form("===> MCCerenkovData: number of cerenkov volume: %d", n)
       << endl;
  for (int i = 0; i < n; i++) {
    MCCerenkov * cerenkov = Get(i);
    cerenkov->Print();
  }
}
