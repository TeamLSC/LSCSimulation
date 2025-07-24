#ifndef MCCerenkovData_HH
#define MCCerenkovData_HH

#include "TClonesArray.h"

class MCCerenkov;
class MCCerenkovData : public TClonesArray {
public:
  MCCerenkovData();
  MCCerenkovData(const MCCerenkovData & data);
  virtual ~MCCerenkovData();

  virtual void Clear(const Option_t * opt = "");

  MCCerenkov * Add();
  MCCerenkov * Add(int id);

  int GetN() const;
  MCCerenkov * Get(int i) const;
  MCCerenkov * FindCerenkov(int id);

  virtual void Print(const Option_t * opt = "") const;

private:
  int fN; //!

  ClassDef(MCCerenkovData, 1)
};

inline int MCCerenkovData::GetN() const { return GetEntriesFast(); }
inline MCCerenkov * MCCerenkovData::Get(int n) const { return (MCCerenkov *)At(n); }

#endif
