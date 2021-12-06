// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// Executable to check functioning of stack
// Analyses kinematics and track references of a kinematics file

#include "SimulationDataFormat/MCTrack.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "SimulationDataFormat/Stack.h"
#include "SimulationDataFormat/TrackReference.h"
#include "Steer/MCKinematicsReader.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "FairLogger.h"
#include "DetectorsCommonDataFormats/NameConf.h"

int checkStackDQ(Bool_t midysim=kTRUE)
{
  Double_t rapMin = -5.; Double_t rapMax = -1.; Int_t leptonPDG = 13; 
  if(midysim) { rapMin = -3.; rapMax = 3.; leptonPDG = 11;}

  /// define histos prompt jpsi / psi2s
  TH1F *histPtPairPrompt = new TH1F("PtPairPrompt",";p_{T}(GeV/c);Entries",100,0.,20.);
  TH1F *histMassPairPrompt = new TH1F("MassPrompt",";m_{ll}(GeV/c^{2});Entries",5./0.02,0.,5.);
  TH1F *histYJpsiPrompt = new TH1F("YJpsiPrompt",";y;Entries",100,rapMin,rapMax);
  TH1F *histPtJpsiPrompt = new TH1F("PtJpsiPrompt",";p_{T}(GeV/c);Entries",100,0.,15.);
  TH1F *histYPsi2sPrompt = new TH1F("YPsi2sPrompt",";y;Entries",100,rapMin,rapMax);
  TH1F *histPtPsi2sPrompt = new TH1F("PtPsi2sPrompt",";p_{T}(GeV/c);Entries",100,0.,15.);
  /// define histos non-prompt jpsi / psi2s
  TH1F *histPtPairNonPrompt = new TH1F("PtPairNonPrompt",";p_{T}(GeV/c);Entries",100,0.,20.);
  TH1F *histMassPairNonPrompt = new TH1F("MassNonPrompt",";m_{ll}(GeV/c^{2});Entries",5./0.02,0.,5.);
  TH1F *histYJpsiNonPrompt = new TH1F("YJpsiNonPrompt",";y;Entries",100,rapMin,rapMax);
  TH1F *histPtJpsiNonPrompt = new TH1F("PtJpsiNonPrompt",";p_{T}(GeV/c);Entries",100,0.,15.);
  TH1F *histYPsi2sNonPrompt = new TH1F("YPsi2sNonPrompt",";y;Entries",100,rapMin,rapMax);
  TH1F *histPtPsi2sNonPrompt = new TH1F("PtPsi2sNonPrompt",";p_{T}(GeV/c);Entries",100,0.,15.);
  //
  TH1D *histIsPrompt = new TH1D("jpsiOrig","",4,0,4);
  histIsPrompt->GetXaxis()->SetBinLabel(1,"IsJpsiPrompt");
  histIsPrompt->GetXaxis()->SetBinLabel(2,"IsJpsiFromB");
  histIsPrompt->GetXaxis()->SetBinLabel(3,"IsPsi2sPrompt");
  histIsPrompt->GetXaxis()->SetBinLabel(4,"IsPsi2sFromB");

  /// 
  Double_t invMassPair, ptPair;
  Double_t ptMoth, yMoth;
  Int_t mothPDG[] = {443, 100443}; // jpsi / psi2s PDG 

  const char* nameprefix = "sgn";

  FairLogger::GetLogger()->SetLogScreenLevel("DEBUG");
  TFile f(o2::base::NameConf::getMCKinematicsFileName(nameprefix).c_str());

  LOG(DEBUG) << "Checking input file :" << f.GetPath();

  std::vector<o2::MCTrack>* mctracks = nullptr;
  auto tr = (TTree*)f.Get("o2sim"); 
  if(!tr) printf("attenzione! non esiste! \n");
  assert(tr);

  auto mcbr = tr->GetBranch("MCTrack");
  assert(mcbr);
  mcbr->SetAddress(&mctracks);

  std::vector<o2::TrackReference>* trackrefs = nullptr;
  auto refbr = tr->GetBranch("TrackRefs");
  assert(refbr);
  refbr->SetAddress(&trackrefs);

  o2::steer::MCKinematicsReader mcreader(nameprefix, o2::steer::MCKinematicsReader::Mode::kMCKine);

  for (int eventID = 0; eventID < mcbr->GetEntries(); ++eventID) {
    mcbr->GetEntry(eventID);
    refbr->GetEntry(eventID);
    LOG(DEBUG) << "-- Entry --" << eventID;
    LOG(DEBUG) << "Have " << mctracks->size() << " tracks";
    int ti = 0;
    int tiprimary = 0;
   
    // record tracks that left a hit in TPC
    // (we know that these tracks should then have a TrackRef)
    std::vector<int> trackidsinTPC;
    Int_t  bpdgs[] = {511, 521, 531, 5112, 5122, 5232, 5132};
    Int_t sizePdg = sizeof(bpdgs)/sizeof(Int_t);

   for (auto& t : *mctracks) {
      ti++;
      if(t.isPrimary()) tiprimary++; else continue;
      // check that mother indices are reasonable
      Int_t ifirst = t.getFirstDaughterTrackId();
      Int_t ilast = t.getLastDaughterTrackId();  
      Bool_t hasBeautyMoth = kFALSE;
      if( (TMath::Abs(t.GetPdgCode()) == mothPDG[0]) || (TMath::Abs(t.GetPdgCode()) == mothPDG[1] ) ){ 
      o2::MCTrack *lepton=0x0; o2::MCTrack *antilepton=0x0;
      Int_t idMoth = t.getMotherTrackId();
      Bool_t isPrompt = idMoth <= 0 ? kTRUE : kFALSE; 
      if(!isPrompt){ //  check beauty mother 
	auto tdM = mcreader.getTrack(eventID, idMoth);
	for(int i=0; i<sizePdg; i++){ if (TMath::Abs(tdM->GetPdgCode()) == bpdgs[i] ) hasBeautyMoth = kTRUE; }
      }
      ptMoth = t.GetPt();
      yMoth = t.GetRapidity();
      ifirst  = (mcreader.getTrack(eventID, ifirst))->isPrimary() ? ifirst : -1; 
      ilast  = (mcreader.getTrack(eventID, ilast))->isPrimary() ? ilast : -1; 
      //LOG(DEBUG) << " mother track - pdg " << t.GetPdgCode() << " first daughter "<<  ifirst <<" last daughter " << ilast << " position " << ti; 
      for(int idaugh=ifirst; idaugh<ilast+1; idaugh++ ){
      auto td = mcreader.getTrack(eventID, idaugh); 
      if(td->GetPdgCode() == -1*leptonPDG) lepton = (o2::MCTrack*)td;   
      if(td->GetPdgCode() == leptonPDG) antilepton = (o2::MCTrack*)td;   
      }
      if((!lepton) || (!antilepton)) continue;
      // evaluate inv mass, pt, y of pairs 
      Double_t m1 = TDatabasePDG::Instance()->GetParticle(leptonPDG)->Mass();
      Double_t m2 = TDatabasePDG::Instance()->GetParticle(leptonPDG)->Mass();
      invMassPair =  m1*m1+m2*m2 + 2.0*(TMath::Sqrt(m1*m1+lepton->GetP()*lepton->GetP())*TMath::Sqrt(m2*m2+antilepton->GetP()*antilepton->GetP()) - lepton->Px()*antilepton->Px() - lepton->Py()*antilepton->Py() - lepton->Pz()*antilepton->Pz()); 
      invMassPair = TMath::Sqrt(invMassPair);
      ////
      Double_t px = lepton->Px()+antilepton->Px();
      Double_t py = lepton->Py()+antilepton->Py();
      Double_t ptPair = TMath::Sqrt(px*px + py*py);
      ////
      //// fill prompt
      if(isPrompt){
      	histMassPairPrompt->Fill(invMassPair);
      	histPtPairPrompt->Fill(ptPair);
      	//
      	if(TMath::Abs(t.GetPdgCode()) == mothPDG[0]){
      	histPtJpsiPrompt->Fill(ptMoth);
      	histYJpsiPrompt->Fill(yMoth);
	histIsPrompt->Fill(0.5);
      	}else{
      	histPtPsi2sPrompt->Fill(ptMoth);
      	histYPsi2sPrompt->Fill(yMoth);
	histIsPrompt->Fill(2.5);
      	}
      }else if(!isPrompt && hasBeautyMoth){
      //// fill non prompt
        histMassPairNonPrompt->Fill(invMassPair);
        histPtPairNonPrompt->Fill(ptPair);
        //
        if(TMath::Abs(t.GetPdgCode()) == mothPDG[0]){
        histPtJpsiNonPrompt->Fill(ptMoth);
        histYJpsiNonPrompt->Fill(yMoth);
	histIsPrompt->Fill(1.5);
        }else{
        histPtPsi2sNonPrompt->Fill(ptMoth);
        histYPsi2sNonPrompt->Fill(yMoth);
	histIsPrompt->Fill(3.5);
        }
      }
      
     }

    } 

    LOG(DEBUG) << "MC tracks: " << ti << " Primaries: " << tiprimary;
    LOG(DEBUG) << "Have " << trackidsinTPC.size() << " tracks with hits in TPC";
    LOG(DEBUG) << "Have " << trackrefs->size() << " track refs";

    // check correct working of MCKinematicsReader
    bool havereferences = trackrefs->size();
    if (havereferences) {
      for (auto& trackID : trackidsinTPC) {
        auto trackrefs = mcreader.getTrackRefs(eventID, trackID);
        assert(trackrefs.size() > 0);
        LOG(DEBUG) << " Track " << trackID << " has " << trackrefs.size() << " TrackRefs";
        for (auto& ref : trackrefs) {
          assert(ref.getTrackID() == trackID);
        }
      }
    }
  }

  TFile foutput("histKine.root","RECREATE");
  histMassPairPrompt->Write();
  histPtPairPrompt->Write();
  histPtJpsiPrompt->Write();
  histYJpsiPrompt->Write();
  histPtPsi2sPrompt->Write();
  histYPsi2sPrompt->Write();

  histMassPairNonPrompt->Write();
  histPtPairNonPrompt->Write();
  histPtJpsiNonPrompt->Write();
  histYJpsiNonPrompt->Write();
  histPtPsi2sNonPrompt->Write();
  histYPsi2sNonPrompt->Write(); 

  histIsPrompt->Write();

  LOG(INFO) << "STACK TEST SUCCESSFULL\n";
  return 0;
}
