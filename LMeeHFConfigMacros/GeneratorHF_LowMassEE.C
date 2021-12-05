#include "GeneratorCocktailPythia8.C"

FairGenerator*
GeneratorCocktailLMee()
{
  enum confParam {kConfig=0, kHooksFunc, kDecayTab, kAll};

  auto gen = new GeneratorCocktailPythia8_class(3);
  bool ispp = kTRUE;

  TString pathO2 = gSystem->ExpandPathName("$O2DPG_ROOT/MC/config");
  TString pathPWD = gSystem->ExpandPathName("$PWD");

  //config generator 1
  string confParamsG1[kAll] ={Form("%s/common/pythia8/generator/pythia8_hf.cfg",pathO2.Data()),
                                   Form("pythia8_userhooks_bbbar(%f,%f)",-1.5,1.5),
                                   Form("%s/force_semiElectronicB.cfg",pathPWD.Data())};
  gen->SetConfigParameters(0,5,confParamsG1,ispp,0.10);

  //config generator 2
  string confParamsG2[kAll] ={Form("%s/common/pythia8/generator/pythia8_hf.cfg",pathO2.Data()),
                                   Form("pythia8_userhooks_ccbar(%f,%f)",-1.5,1.5),
                                   Form("%s/force_semiElectronicC.cfg",pathPWD.Data())};
  gen->SetConfigParameters(1,4,confParamsG2,ispp,0.10);

  //config generator 3
  string confParamsG3[kAll] ={Form("%s/common/pythia8/generator/pythia8_hf.cfg",pathO2.Data()),
                                   Form("pythia8_userhooks_bbbar(%f,%f)",-1.5,1.5),""};
  gen->SetConfigParameters(2,5,confParamsG3,ispp,0.80);

  return gen;
}
