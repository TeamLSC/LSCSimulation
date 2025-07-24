R__LOAD_LIBRARY(libHist)
R__LOAD_LIBRARY(libTrgSim)

void digi_example()
{
  auto pmt = new PMTSignal();
  pmt->SetTransitTime(100);
  pmt->SetTTS(1.2);

  pmt->AddHitTime(0);
  pmt->SetModel(PMTSignal::MOYAL);
  //pmt->Draw();

  auto fadc = new FADCWaveformGenerator();
  fadc->SetNBIT(12);
  fadc->SetVpp(200);
  fadc->SetSamplingRate(500);
  fadc->SetTimeWindow(250);
  fadc->SetPedOffset(100);
  fadc->SetPedRMS(0.3);

  fadc->SetSignal(pmt);
  fadc->Digitize();

  fadc->Draw();
}
