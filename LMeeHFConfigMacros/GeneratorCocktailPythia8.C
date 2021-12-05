//
//  class for defining a cocktail of GeneratorPythia8 generators
//  
// 
// R__ADD_INCLUDE_PATH($O2DPG_ROOT/MC/config/PWGHF/external/generator)
R__ADD_INCLUDE_PATH($O2DPG_ROOT/MC/config/PWGHF/pythia8/hooks)
#include "pythia8_userhooks_qqbar.C"
#include <vector>
#include "GeneratorHF.C"

using namespace o2::eventgen;

class GeneratorCocktailPythia8_class : public Generator
{
enum confParam {kConfig=0, kHooksFunc, kDecayTab, kAll}; // index of string parameters
	
public:
  GeneratorCocktailPythia8_class(int ndim) { mEntries = new  std::vector<GeneratorPythia8*>(ndim); };
  ~GeneratorCocktailPythia8_class() = default;

  // at init we init all generators
  bool Init() override {
    for(int igen=0; igen<mEntries->size(); igen++) {
    mEntries->at(igen)->Init(); 
    }
    // normalize the probabilities for random generation
    float sum = 0.;
      for(int igen=0; igen<mProb.size(); igen++) {   
        sum += mProb.at(igen);
      } 
    
    float psum = 0.;
    for(int i=0; i<mProb.size(); i++) {
      psum +=  mProb.at(i) / sum;
      mProb.at(i) = psum;
      }
    return true;
  };

  // call generate method for all generators
  bool generateEvent() override { 
    
    // Select a generator of the cocktail randomly
    int igen=-1;
    float p0 =  gRandom->Rndm();
    for (igen = 0; igen < mProb.size(); igen++) {
      		if (p0 < mProb.at(igen)) break;
    	        }
     mEntries->at(igen)->generateEvent();  
     return true; 
  };

  // at importParticles we add particles to the output particle vector
  bool importParticles() override { 
  for(int igen=0; igen<mEntries->size(); igen++){ 
	mEntries->at(igen)->importParticles();
        for (auto& p : mEntries->at(igen)->getParticles()) mParticles.push_back(p); 
        mEntries->at(igen)->clearParticles(); 
        } 
	return true;
  };

  void SetConfigParameters(Int_t indGen, Int_t pdg, string configParams[kAll], Bool_t ispp=kTRUE, double prob=1.){
	mEntries->at(indGen) = new o2::eventgen::GeneratorHF();
        mEntries->at(indGen)->setConfig(configParams[kConfig]);
        mEntries->at(indGen)->setHooksFuncName(configParams[kHooksFunc]);
        if(configParams[kDecayTab].size()) mEntries->at(indGen)->readFile(configParams[kDecayTab]);
        ((o2::eventgen::GeneratorHF*)mEntries->at(indGen))->setPDG(pdg);
	if(ispp) ((o2::eventgen::GeneratorHF*)mEntries->at(indGen))->setFormula("1");
	else ((o2::eventgen::GeneratorHF*)mEntries->at(indGen))->setFormula("max(1.,120.*(x<5.)+80.*(1.-x/20.)*(x>5.)*(x<11.)+240.*(1.-x/13.)*(x>11.))");
        mProb.push_back(prob);
       	return; 
  }


private:
  std::vector<GeneratorPythia8*> *mEntries; // vector of GeneratorPythia8 generators 
  std::vector<double> mProb; // vector of probabilities for random selection 

};
