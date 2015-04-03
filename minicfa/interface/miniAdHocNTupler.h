// ADHOCNTUPLER: Creates eventA in the cfA ntuples, the tree that requires
//               Ad hoc c++ code to be filled.

#include <cmath>

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include "DataFormats/Common/interface/ConditionsInEdm.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/TriggerPath.h"
#include "DataFormats/PatCandidates/interface/TriggerEvent.h"
#include "DataFormats/PatCandidates/interface/TriggerAlgorithm.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "RecoEgamma/EgammaTools/interface/ConversionTools.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h" 
#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/GhostedAreaSpec.hh>


using namespace std;
using namespace fastjet;

class miniAdHocNTupler : public NTupler {
 public:

  double getPFIsolation(edm::Handle<pat::PackedCandidateCollection> pfcands,
                        const reco::Candidate* ptcl,  
                        double r_iso_min, double r_iso_max, double kt_scale,
                        bool use_pfweight, bool charged_only) {

    if (ptcl->pt()<5.) return 99999.;

    double deadcone_nh(0.), deadcone_ch(0.), deadcone_ph(0.), deadcone_pu(0.);
    if(ptcl->isElectron()) {
      if (fabs(ptcl->eta())>1.479) {deadcone_ch = 0.015; deadcone_pu = 0.015; deadcone_ph = 0.08;}
    } else if(ptcl->isMuon()) {
      deadcone_ch = 0.0001; deadcone_pu = 0.01; deadcone_ph = 0.01;deadcone_nh = 0.01;  
    } else {
      //deadcone_ch = 0.0001; deadcone_pu = 0.01; deadcone_ph = 0.01;deadcone_nh = 0.01; // maybe use muon cones??
    }

    double iso_nh(0.); double iso_ch(0.); 
    double iso_ph(0.); double iso_pu(0.);
    double ptThresh(0.5);
    if(ptcl->isElectron()) ptThresh = 0;
    double r_iso = max(r_iso_min,min(r_iso_max, kt_scale/ptcl->pt()));
    for (const pat::PackedCandidate &pfc : *pfcands) {
      if (abs(pfc.pdgId())<7) continue;

      double dr = deltaR(pfc, *ptcl);
      if (dr > r_iso) continue;
      
      //////////////////  NEUTRALS  /////////////////////////
      if (pfc.charge()==0){
        if (pfc.pt()>ptThresh) {
          double wpf(1.);
          if (use_pfweight){
            double wpv(0.), wpu(0.);
            for (const pat::PackedCandidate &jpfc : *pfcands) {
              double jdr = deltaR(pfc, jpfc);
              if (pfc.charge()!=0 || jdr<0.00001) continue;
              double jpt = jpfc.pt();
              if (pfc.fromPV()>1) wpv *= jpt/jdr;
              else wpu *= jpt/jdr;
            }
            wpv = log(wpv);
            wpu = log(wpu);
            wpf = wpv/(wpv+wpu);
          }
          /////////// PHOTONS ////////////
          if (abs(pfc.pdgId())==22) {
            if(dr < deadcone_ph) continue;
            iso_ph += wpf*pfc.pt();
	    /////////// NEUTRAL HADRONS ////////////
          } else if (abs(pfc.pdgId())==130) {
            if(dr < deadcone_nh) continue;
            iso_nh += wpf*pfc.pt();
          }
        }
        //////////////////  CHARGED from PV  /////////////////////////
      } else if (pfc.fromPV()>1){
        if (abs(pfc.pdgId())==211) {
          if(dr < deadcone_ch) continue;
          iso_ch += pfc.pt();
        }
        //////////////////  CHARGED from PU  /////////////////////////
      } else {
        if (pfc.pt()>ptThresh){
          if(dr < deadcone_pu) continue;
          iso_pu += pfc.pt();
        }
      }
    }
    double iso(0.);
    if (charged_only){
      iso = iso_ch;
    } else {
      iso = iso_ph + iso_nh;
      if (!use_pfweight) iso -= 0.5*iso_pu;
      if (iso>0) iso += iso_ch;
      else iso = iso_ch;
    }
    iso = iso/ptcl->pt();

    return iso;
  }

  void fill(edm::Event& iEvent){


    nevents++;

    //////////////// Fat jets //////////////////
    edm::Handle<pat::JetCollection> jets;
    iEvent.getByLabel("slimmedJets", jets);

    JetDefinition jet_def_12(antikt_algorithm, 1.2);
    //    vector<vector<PseudoJet>> fjets_vvector(0);
    vector<PseudoJet> fjets_constituents(0), fjets(0);
    vector<float> ptThresholds;
    ptThresholds.push_back(30);
    const float etaThreshold(5);

    for(unsigned int ipt(0); ipt < ptThresholds.size(); ipt++){
      fjets_constituents.resize(0);
      //cout<<endl<<"SKINNY JETS"<<endl;
      for (unsigned int ijet(0); ijet < jets->size(); ijet++) {
        const pat::Jet &jet = (*jets)[ijet];
        if(fabs(jet.eta()) > etaThreshold) continue;
        if(jet.pt() < ptThresholds[ipt]) continue;
        fjets_constituents.push_back(PseudoJet(jet.px(),jet.py(),jet.pz(),jet.energy()));
        //cout<<"pt "<<jet.pt()<<", eta "<<jet.eta()<<", phi "<<jet.phi()<<endl;
      }
      ClusterSequence cs_fjets(fjets_constituents, jet_def_12);
      fjets = sorted_by_pt(cs_fjets.inclusive_jets());
      for (unsigned int ifjet(0); ifjet < fjets.size(); ifjet++) {
        fjets30_pt->push_back(fjets[ifjet].pt());
        fjets30_eta->push_back(fjets[ifjet].eta());
        fjets30_phi->push_back(fjets[ifjet].phi());
        fjets30_energy->push_back(fjets[ifjet].E());
        fjets30_m->push_back(fjets[ifjet].m());
      }

      //      cout<<endl<<"FAT JETS"<<endl;
      //      for (unsigned int ifjet(0); ifjet < fjets.size(); ifjet++) {
      //	cout<<"pt "<<fjets[ifjet].pt()<<", eta "<<fjets[ifjet].eta()<<", phi "<<fjets[ifjet].phi()<<endl;
      //      }
      //      fjets_vvector.push_back(fjets);
    }


    //////////////// pfcands shenanigans //////////////////
    edm::Handle<pat::PackedCandidateCollection> pfcands;
    iEvent.getByLabel("packedPFCandidates", pfcands);
    edm::Handle<pat::MuonCollection> muons;
    iEvent.getByLabel("slimmedMuons", muons);
    edm::Handle<pat::ElectronCollection> electrons;
    iEvent.getByLabel("slimmedElectrons", electrons);
    edm::Handle<pat::TauCollection> taus;
    iEvent.getByLabel("slimmedTaus", taus);
    edm::Handle<pat::METCollection> mets;
    iEvent.getByLabel("slimmedMETs", mets);

    vector<const pat::PackedCandidate*> el_pfmatch, mu_pfmatch; 
    for (const pat::PackedCandidate &pfc : *pfcands) {
      for (unsigned int ilep(0); ilep < electrons->size(); ilep++) {
        const pat::Electron &lep = (*electrons)[ilep];
        if(el_pfmatch.size() <= ilep) el_pfmatch.push_back(&pfc);
        else if(lep.pdgId()==pfc.pdgId() && deltaR(pfc, lep) < deltaR(*(el_pfmatch[ilep]), lep)) el_pfmatch[ilep] = &pfc;
      }
      for (unsigned int ilep(0); ilep < muons->size(); ilep++) {
        const pat::Muon &lep = (*muons)[ilep];
        if(mu_pfmatch.size() <= ilep) mu_pfmatch.push_back(&pfc);
        else if(lep.pdgId()==pfc.pdgId() && deltaR(pfc, lep) < deltaR(*(mu_pfmatch[ilep]), lep)) mu_pfmatch[ilep] = &pfc;
      }
    } // Loop over pfcands

    // Finding electron PF match
    for (unsigned int ilep(0); ilep < electrons->size(); ilep++) {
      const pat::Electron &lep = (*electrons)[ilep];
      els_isPF->push_back(deltaR(lep, *el_pfmatch[ilep]) < 0.1 && abs(lep.p()-el_pfmatch[ilep]->p())/lep.p()<0.05 &&
			  lep.pdgId() == el_pfmatch[ilep]->pdgId());
      els_jet_ind->push_back(-1);

      els_miniso->push_back(getPFIsolation(pfcands, dynamic_cast<const reco::Candidate *>(&lep), 0.05, 0.2, 10., false, false));
    }

    // Finding muon PF match
    for (unsigned int ilep(0); ilep < muons->size(); ilep++) {
      const pat::Muon &lep = (*muons)[ilep];
      mus_isPF->push_back(lep.numberOfSourceCandidatePtrs()==1 && lep.sourceCandidatePtr(0)->pdgId()==lep.pdgId());
      mus_jet_ind->push_back(-1);

      mus_miniso->push_back(getPFIsolation(pfcands, dynamic_cast<const reco::Candidate *>(&lep), 0.05, 0.2, 10., false, false));
    }

    // Finding leptons in jets
    for (unsigned int ijet(0); ijet < jets->size(); ijet++) {
      const pat::Jet &jet = (*jets)[ijet];
      jets_AK4_mu_ind->push_back(-1);
      jets_AK4_el_ind->push_back(-1);

      float maxp(-99.), maxp_mu(-99.), maxp_el(-99.);
      int maxid(0);
      for (unsigned int i = 0, n = jet.numberOfSourceCandidatePtrs(); i < n; ++i) {
        const pat::PackedCandidate &pfc = dynamic_cast<const pat::PackedCandidate &>(*jet.sourceCandidatePtr(i));
        int pf_id = pfc.pdgId();
        float pf_p = pfc.p();
        if(pf_p > maxp){
          maxp = pf_p;
          maxid = pf_id;
        }

        if(abs(pf_id) == 11){
          for (unsigned int ilep(0); ilep < electrons->size(); ilep++) {
            if(&pfc == el_pfmatch[ilep]){
              els_jet_ind->at(ilep) = ijet;
              if(pf_p > maxp_el){
                maxp_el = pf_p;
                jets_AK4_el_ind->at(ijet) = ilep; // Storing the index of the highest pt electron in jet
              }
              break;
            }
          } // Loop over electrons
        } // If pfc is an electron

        if(abs(pf_id) == 13){
          for (unsigned int ilep(0); ilep < muons->size(); ilep++) {
            if(&pfc == mu_pfmatch[ilep]){
              mus_jet_ind->at(ilep) = ijet;
              if(pf_p > maxp_mu){
                maxp_mu = pf_p;
                jets_AK4_mu_ind->at(ijet) = ilep; // Storing the index of the highest pt muon in jet
              }
              break;
            }
          } // Loop over muons
        } // If pfc is an muon
      } // Loop over jet constituents
      jets_AK4_maxpt_id->push_back(maxid);
    } // Loop over jets

    // Finding leptons in taus
    for (unsigned int itau(0); itau < taus->size(); itau++) {
      const pat::Tau &tau = (*taus)[itau];
      taus_mu_ind->push_back(-1);
      taus_el_ind->push_back(-1);

      if(tau.numberOfSourceCandidatePtrs() == 1){
        const pat::PackedCandidate &pfc = dynamic_cast<const pat::PackedCandidate &> (*tau.sourceCandidatePtr(0));
        if(abs(pfc.pdgId())==11){
          for (unsigned int ilep(0); ilep < electrons->size(); ilep++) {
            if(&pfc == el_pfmatch[ilep]){
              taus_el_ind->at(itau) = ilep;
              break;
            } 
          } // Loop over electrons
        }
        if(abs(pfc.pdgId())==13){
          for (unsigned int ilep(0); ilep < muons->size(); ilep++) {
            if(&pfc == mu_pfmatch[ilep]){
              taus_mu_ind->at(itau) = ilep;
              break;
            } 
          } // Loop over electrons
        }
      } // If tau has one constituent
    } // Loop over taus

    //////////////// Pile up and generator information //////////////////
    double htEvent = 0.0;
    if(!iEvent.isRealData()) { //Access PU info in MC
      edm::Handle<std::vector< PileupSummaryInfo > >  PupInfo;
      iEvent.getByLabel("addPileupInfo", PupInfo);
      std::vector<PileupSummaryInfo>::const_iterator PVI;

      for(PVI = PupInfo->begin(); PVI != PupInfo->end(); ++PVI) {
        // cout << " PU Information: bunch crossing " << PVI->getBunchCrossing() 
        //      << ", NumInteractions " << PVI->getPU_NumInteractions() 
        //      << ", TrueNumInteractions " << PVI->getTrueNumInteractions() 
        //      <<", evend ID   "<< iEvent.id().event() << std::endl;
        (*PU_NumInteractions_).push_back(PVI->getPU_NumInteractions());
        (*PU_bunchCrossing_).push_back(PVI->getBunchCrossing());
        (*PU_TrueNumInteractions_).push_back(PVI->getTrueNumInteractions());
        (*PU_zpositions_).push_back(PVI->getPU_zpositions());
        (*PU_sumpT_lowpT_).push_back(PVI->getPU_sumpT_lowpT());
        (*PU_sumpT_highpT_).push_back(PVI->getPU_sumpT_highpT());
        (*PU_ntrks_lowpT_).push_back(PVI->getPU_ntrks_lowpT());
        (*PU_ntrks_highpT_).push_back(PVI->getPU_ntrks_highpT());
      }

      edm::Handle<LHEEventProduct> product;
      if(iEvent.getByLabel("externalLHEProducer", product)){
        iEvent.getByLabel("externalLHEProducer", product);
        const lhef::HEPEUP hepeup_ = product->hepeup();
        const std::vector<lhef::HEPEUP::FiveVector> pup_ = hepeup_.PUP;

        size_t iMax = hepeup_.NUP;
        for(size_t i = 2; i < iMax; ++i) {
          if( hepeup_.ISTUP[i] != 1 ) continue;
          int idabs = abs( hepeup_.IDUP[i] );
          if( idabs != 21 && (idabs<1 || idabs>6) ) continue;
          double ptPart = sqrt( pow(hepeup_.PUP[i][0],2) + pow(hepeup_.PUP[i][1],2) );
          htEvent += ptPart;
        } 
      }
      *genHT_ = htEvent;
    } // if it's not real data


    //////////////// Filter decisions and names //////////////////
    edm::Handle<edm::TriggerResults> filterBits;
    edm::InputTag labfilterBits("TriggerResults","","PAT");
    iEvent.getByLabel(labfilterBits,filterBits);  
    int trackingfailurefilterResult(1);			    
    int goodVerticesfilterResult(1);				    
    int cschalofilterResult(1);						    
    int trkPOGfilterResult(1);						    
    int trkPOG_logErrorTooManyClustersfilterResult(1);	
    int EcalDeadCellTriggerPrimitivefilterResult(1);	
    int ecallaserfilterResult(1);						    
    int trkPOG_manystripclus53XfilterResult(1);		    
    int eebadscfilterResult(1);						    
    int METFiltersfilterResult(1);						    
    int HBHENoisefilterResult(1);						    
    int trkPOG_toomanystripclus53XfilterResult(1);		    
    int hcallaserfilterResult(1);
			               
    const edm::TriggerNames &fnames = iEvent.triggerNames(*filterBits);
    for (unsigned int i = 0, n = filterBits->size(); i < n; ++i) {
      string filterName = fnames.triggerName(i);
      int filterdecision = filterBits->accept(i);
      if (filterName=="Flag_trackingFailureFilter")		 trackingfailurefilterResult = filterdecision;
      if (filterName=="Flag_goodVertices")			 goodVerticesfilterResult = filterdecision;
      if (filterName=="Flag_CSCTightHaloFilter")		 cschalofilterResult = filterdecision;
      if (filterName=="Flag_trkPOGFilters")			 trkPOGfilterResult = filterdecision;
      if (filterName=="Flag_trkPOG_logErrorTooManyClusters")	 trkPOG_logErrorTooManyClustersfilterResult = filterdecision;
      if (filterName=="Flag_EcalDeadCellTriggerPrimitiveFilter") EcalDeadCellTriggerPrimitivefilterResult = filterdecision;
      if (filterName=="Flag_ecalLaserCorrFilter")		 ecallaserfilterResult = filterdecision;
      if (filterName=="Flag_trkPOG_manystripclus53X")		 trkPOG_manystripclus53XfilterResult = filterdecision;
      if (filterName=="Flag_eeBadScFilter")			 eebadscfilterResult = filterdecision;
      if (filterName=="Flag_METFilters")			 METFiltersfilterResult = filterdecision;
      if (filterName=="Flag_HBHENoiseFilter")			 HBHENoisefilterResult = filterdecision;
      if (filterName=="Flag_trkPOG_toomanystripclus53X")	 trkPOG_toomanystripclus53XfilterResult = filterdecision;
      if (filterName=="Flag_hcalLaserEventFilter")		 hcallaserfilterResult = filterdecision;
    }

    *trackingfailurefilter_decision_			=                trackingfailurefilterResult;	   
    *goodVerticesfilter_decision_			=		    goodVerticesfilterResult;	   
    *cschalofilter_decision_				=			    cschalofilterResult;   	    
    *trkPOGfilter_decision_				=			    trkPOGfilterResult;	   
    *trkPOG_logErrorTooManyClustersfilter_decision_	=  trkPOG_logErrorTooManyClustersfilterResult;  
    *EcalDeadCellTriggerPrimitivefilter_decision_	=    EcalDeadCellTriggerPrimitivefilterResult;    
    *ecallaserfilter_decision_				=			    ecallaserfilterResult; 	    
    *trkPOG_manystripclus53Xfilter_decision_		=	    trkPOG_manystripclus53XfilterResult;   
    *eebadscfilter_decision_				=			    eebadscfilterResult;   	    
    *METFiltersfilter_decision_				=			    METFiltersfilterResult;	    
    *HBHENoisefilter_decision_				=			    HBHENoisefilterResult; 	    
    *trkPOG_toomanystripclus53Xfilter_decision_		=	    trkPOG_toomanystripclus53XfilterResult;
    *hcallaserfilter_decision_				=                       hcallaserfilterResult;     

    //////////////// Trigger decisions and names //////////////////
    edm::Handle<edm::TriggerResults> triggerBits;
    edm::InputTag labtriggerBits("TriggerResults","","HLT");
    iEvent.getByLabel(labtriggerBits,triggerBits);  

    //////////////// Trigger prescales //////////////////
    edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;
    edm::InputTag labtriggerPrescales("patTrigger");
    iEvent.getByLabel(labtriggerPrescales,triggerPrescales);  

    const edm::TriggerNames &names = iEvent.triggerNames(*triggerBits);
    for (unsigned int i = 0, n = triggerBits->size(); i < n; ++i) {
      (*trigger_decision).push_back(triggerBits->accept(i));
      (*trigger_name).push_back(names.triggerName(i));
      (*trigger_prescalevalue).push_back(triggerPrescales->getPrescaleForIndex(i));
    }
   
    //////////////// HLT trigger objects //////////////////
    edm::Handle<pat::TriggerObjectStandAloneCollection> triggerObjects;
    edm::InputTag labtriggerObjects("selectedPatTrigger");
    iEvent.getByLabel(labtriggerObjects,triggerObjects);  

    for (pat::TriggerObjectStandAlone obj : *triggerObjects) { // note: not "const &" since we want to call unpackPathNames
      obj.unpackPathNames(names);
      (*standalone_triggerobject_collectionname).push_back(obj.collection()); 
      (*standalone_triggerobject_pt).push_back(obj.pt());
      (*standalone_triggerobject_px).push_back(obj.px());
      (*standalone_triggerobject_py).push_back(obj.py());
      (*standalone_triggerobject_pz).push_back(obj.pz());
      (*standalone_triggerobject_et).push_back(obj.et());
      (*standalone_triggerobject_energy).push_back(obj.energy());
      (*standalone_triggerobject_phi).push_back(obj.phi());
      (*standalone_triggerobject_eta).push_back(obj.eta());
    }

    //////////////// L1 trigger objects --- TO BE UNDERSTOOD ---
    edm::Handle<L1GlobalTriggerReadoutRecord> L1trigger_h;
    edm::InputTag labL1trigger("gtDigis","","HLT");
    iEvent.getByLabel(labL1trigger, L1trigger_h);  

    std::vector<bool> gtbits;
    int ngtbits = 128;
    gtbits.reserve(ngtbits); for(int i=0; i<ngtbits; i++) gtbits[i]=false;
    if(L1trigger_h.isValid()) 
      gtbits = L1trigger_h->decisionWord();
    for(int i=0; i<ngtbits; i++) if(gtbits[i]) cout<<"Bit "<<i<<" is true"<<endl;

    const L1GlobalTriggerReadoutRecord* L1trigger = L1trigger_h.failedToGet () ? 0 : &*L1trigger_h;
    if(L1trigger) cout<<"Level 1 decision: "<<L1trigger->decision()<<endl;


    //isolated pf candidates as found by TrackIsolationMaker                                                                               
    /* edm::Handle< vector<float> > pfcand_dzpv; */
    /* iEvent.getByLabel("trackIsolationMaker","pfcandsdzpv", pfcand_dzpv); */
    /* edm::Handle< vector<float> > pfcand_pt; */
    /* iEvent.getByLabel("trackIsolationMaker","pfcandspt", pfcand_pt); */
    /* edm::Handle< vector<float> > pfcand_eta; */
    /* iEvent.getByLabel("trackIsolationMaker","pfcandseta", pfcand_eta); */
    /* edm::Handle< vector<float> > pfcand_phi; */
    /* iEvent.getByLabel("trackIsolationMaker","pfcandsphi", pfcand_phi); */
    /* edm::Handle< vector<float> > pfcand_iso; */
    /* iEvent.getByLabel("trackIsolationMaker","pfcandstrkiso", pfcand_iso); */
    /* edm::Handle< vector<int> > pfcand_charge; */
    /* iEvent.getByLabel("trackIsolationMaker","pfcandschg", pfcand_charge); */

    /* for (size_t it=0; it<pfcand_pt->size(); ++it ) { */
    /*   isotk_pt_->push_back( pfcand_pt->at(it)); */
    /*   isotk_phi_ -> push_back( pfcand_phi->at(it)); */
    /*   isotk_eta_ -> push_back( pfcand_eta->at(it)); */
    /*   isotk_iso_ -> push_back( pfcand_iso->at(it)); */
    /*   isotk_dzpv_ -> push_back( pfcand_dzpv->at(it)); */
    /*   isotk_charge_ -> push_back( pfcand_charge->at(it)); */
    /* } */

    // tauID -- see https://indico.cern.ch/event/359233/contribution/4/material/slides/0.pdf
    for (unsigned int itau(0); itau < taus->size(); itau++) {
      const pat::Tau &tau = (*taus)[itau];
      taus_n_pfcands_->push_back( tau.numberOfSourceCandidatePtrs() );
      taus_decayMode_->push_back( tau.pfEssential().decayMode_ );
      taus_CombinedIsolationDeltaBetaCorrRaw3Hits_->push_back( tau.tauID("byCombinedIsolationDeltaBetaCorrRaw3Hits") );
      taus_byLooseCombinedIsolationDeltaBetaCorr3Hits_->push_back( tau.tauID("byLooseCombinedIsolationDeltaBetaCorr3Hits") );
      taus_byMediumCombinedIsolationDeltaBetaCorr3Hits_->push_back( tau.tauID("byMediumCombinedIsolationDeltaBetaCorr3Hits") );
      taus_byTightCombinedIsolationDeltaBetaCorr3Hits_->push_back( tau.tauID("byTightCombinedIsolationDeltaBetaCorr3Hits") );
      taus_byDecayModeFinding_->push_back( tau.tauID("decayModeFinding") );
      taus_byDecayModeFindingNewDMs_->push_back( tau.tauID("decayModeFindingNewDMs") );
      taus_chargedIsoPtSum_->push_back( tau.tauID("chargedIsoPtSum") );
      taus_neutralIsoPtSum_->push_back( tau.tauID("neutralIsoPtSum") );
      taus_puCorrPtSum_->push_back( tau.tauID("puCorrPtSum") );
      taus_againstMuonLoose3_->push_back( tau.tauID("againstMuonLoose3") );
      taus_againstElectronLooseMVA5_->push_back( tau.tauID("againstElectronLooseMVA5") );
    } // Loop over taus

    // MET
    const pat::MET &met = mets->front();
    *pfType1mets_uncert_JetEnUp_dpx_ = met.shiftedPx(pat::MET::JetEnUp)-met.px();
    *pfType1mets_uncert_JetEnUp_dpy_ = met.shiftedPy(pat::MET::JetEnUp)-met.py();
    *pfType1mets_uncert_JetEnUp_sumEt_ = met.shiftedSumEt(pat::MET::JetEnUp)-met.sumEt();
    *pfType1mets_uncert_JetEnDown_dpx_ = met.shiftedPx(pat::MET::JetEnDown)-met.px();
    *pfType1mets_uncert_JetEnDown_dpy_ = met.shiftedPy(pat::MET::JetEnDown)-met.py();
    *pfType1mets_uncert_JetEnDown_sumEt_ = met.shiftedSumEt(pat::MET::JetEnDown)-met.sumEt();
    *pfType1mets_uncert_JetResUp_dpx_ = met.shiftedPx(pat::MET::JetResUp)-met.px();
    *pfType1mets_uncert_JetResUp_dpy_ = met.shiftedPy(pat::MET::JetResUp)-met.py();
    *pfType1mets_uncert_JetResUp_sumEt_ = met.shiftedSumEt(pat::MET::JetResUp)-met.sumEt();
    *pfType1mets_uncert_JetResDown_dpx_ = met.shiftedPx(pat::MET::JetResDown)-met.px();
    *pfType1mets_uncert_JetResDown_dpy_ = met.shiftedPy(pat::MET::JetResDown)-met.py();
    *pfType1mets_uncert_JetResDown_sumEt_ = met.shiftedSumEt(pat::MET::JetResDown)-met.sumEt();

    *raw_met_et_ = met.shiftedPt(pat::MET::METUncertainty::NoShift, pat::MET::METUncertaintyLevel::Raw);
    *raw_met_phi_ = met.shiftedPhi(pat::MET::METUncertainty::NoShift, pat::MET::METUncertaintyLevel::Raw);
    *raw_met_sumEt_ = met.shiftedSumEt(pat::MET::METUncertainty::NoShift, pat::MET::METUncertaintyLevel::Raw);

    // electron ID
    /* edm::Handle< vector<int> > els_passVetoId; */
    /* iEvent.getByLabel("electronProducer","els_passVetoId", els_passVetoId); */
    /* edm::Handle< vector<int> > els_passLooseId; */
    /* iEvent.getByLabel("electronProducer","els_passLooseId", els_passLooseId); */
    /* edm::Handle< vector<int> > els_passMediumId; */
    /* iEvent.getByLabel("electronProducer","els_passMediumId", els_passMediumId); */
    /* edm::Handle< vector<int> > els_passTightId; */
    /* iEvent.getByLabel("electronProducer","els_passTightId", els_passTightId); */
 
    /* for (unsigned int ilep(0); ilep < els_passVetoId->size(); ilep++) { */
    /*   els_passPhys14VetoId_->push_back( els_passVetoId->at(ilep) ); */
    /*   els_passPhys14LooseId_->push_back( els_passLooseId->at(ilep) ); */
    /*   els_passPhys14MediumId_->push_back( els_passMediumId->at(ilep) ); */
    /*   els_passPhys14TightId_->push_back( els_passTightId->at(ilep) ); */
    /* } */

    // photons
    edm::Handle< vector<float> > ph_sieie;
    iEvent.getByLabel("photonProducer","photonsfull5x5sigmaIEtaIEta", ph_sieie);
    edm::Handle< vector<bool> > ph_elVeto;
    iEvent.getByLabel("photonProducer","photonspasselveto", ph_elVeto);
    for (unsigned int iphoton(0); iphoton < ph_sieie->size(); iphoton++) {
      photons_full5x5sigmaIEtaIEta_->push_back(ph_sieie->at(iphoton));
      photons_pass_el_veto_->push_back(ph_elVeto->at(iphoton));
    } 

    // energy density
    edm::Handle<double> rhoH_all;
    iEvent.getByLabel( "fixedGridRhoFastjetAll", rhoH_all );
    if(rhoH_all.isValid()){
      *fixedGridRhoFastjetAll_ = *rhoH_all;
    }

    // JEC
    edm::Handle< vector<float> > corL1Fast;
    iEvent.getByLabel("jecCorL1Fast","JEC", corL1Fast);
    edm::Handle< vector<float> > corL2L3;
    iEvent.getByLabel("jecCorL2L3","JEC", corL2L3);
    edm::Handle< vector<float> > corL1FastL2L3;
    iEvent.getByLabel("jecCorL1FastL2L3","JEC", corL1FastL2L3);

    for (size_t it=0; it<corL1Fast->size(); ++it ) {
      jets_AK4_corL1Fast_->push_back( corL1Fast->at(it));
      jets_AK4_corL2L3_->push_back( corL2L3->at(it));
      jets_AK4_corL1FastL2L3_->push_back( corL1FastL2L3->at(it));
    }
   

    //fill the tree    
    if (ownTheTree_){ tree_->Fill(); }
    (*trigger_name).clear();
    (*trigger_decision).clear();
    (*trigger_prescalevalue).clear();
    (*standalone_triggerobject_pt).clear();
    (*standalone_triggerobject_px).clear();
    (*standalone_triggerobject_py).clear();
    (*standalone_triggerobject_pz).clear();
    (*standalone_triggerobject_et).clear();
    (*standalone_triggerobject_energy).clear();
    (*standalone_triggerobject_phi).clear();
    (*standalone_triggerobject_eta).clear();
    (*standalone_triggerobject_collectionname).clear();

    (*PU_zpositions_).clear();
    (*PU_sumpT_lowpT_).clear();
    (*PU_sumpT_highpT_).clear();
    (*PU_ntrks_lowpT_).clear();
    (*PU_ntrks_highpT_).clear();
    (*PU_NumInteractions_).clear();
    (*PU_bunchCrossing_).clear();
    (*PU_TrueNumInteractions_).clear();

    (*els_isPF).clear();
    (*mus_isPF).clear();

    (*els_miniso).clear();
    (*mus_miniso).clear();

    (*jets_AK4_maxpt_id).clear();
    (*jets_AK4_mu_ind).clear();
    (*jets_AK4_el_ind).clear();
    (*jets_AK4_corL1Fast_).clear();
    (*jets_AK4_corL2L3_).clear();
    (*jets_AK4_corL1FastL2L3_).clear();
    
    (*taus_el_ind).clear();
    (*taus_mu_ind).clear();
    (*els_jet_ind).clear();
    (*mus_jet_ind).clear();

    /* (*els_passPhys14VetoId_).clear(); */
    /* (*els_passPhys14LooseId_).clear(); */
    /* (*els_passPhys14MediumId_).clear(); */
    /* (*els_passPhys14TightId_).clear(); */

    /* (*isotk_pt_).clear(); */
    /* (*isotk_phi_).clear(); */
    /* (*isotk_eta_).clear(); */
    /* (*isotk_iso_).clear(); */
    /* (*isotk_dzpv_).clear(); */
    /* (*isotk_charge_).clear(); */
    
    (*taus_CombinedIsolationDeltaBetaCorrRaw3Hits_).clear();
    (*taus_byLooseCombinedIsolationDeltaBetaCorr3Hits_).clear();
    (*taus_byMediumCombinedIsolationDeltaBetaCorr3Hits_).clear();
    (*taus_byTightCombinedIsolationDeltaBetaCorr3Hits_).clear();
    (*taus_n_pfcands_).clear();
    (*taus_decayMode_).clear();
    (*taus_byDecayModeFinding_).clear();
    (*taus_byDecayModeFindingNewDMs_).clear();
    (*taus_chargedIsoPtSum_).clear();
    (*taus_neutralIsoPtSum_).clear();
    (*taus_puCorrPtSum_).clear();
    (*taus_againstMuonLoose3_).clear();
    (*taus_againstElectronLooseMVA5_).clear();

    (*fjets30_pt).clear();
    (*fjets30_eta).clear();
    (*fjets30_phi).clear();
    (*fjets30_energy).clear();
    (*fjets30_m).clear();

    (*photons_full5x5sigmaIEtaIEta_).clear();
    (*photons_pass_el_veto_).clear();


  }

  uint registerleaves(edm::ProducerBase * producer){
    uint nLeaves=0;
    if (useTFileService_){
      edm::Service<TFileService> fs;      
      if (ownTheTree_){
        ownTheTree_=true;
        tree_=fs->make<TTree>(treeName_.c_str(),"miniStringBasedNTupler tree");
      }else{
        TObject * object = fs->file().Get(treeName_.c_str());
        if (!object){
          ownTheTree_=true;
          tree_=fs->make<TTree>(treeName_.c_str(),"miniStringBasedNTupler tree");
        }
        tree_=dynamic_cast<TTree*>(object);
        if (!tree_){
          ownTheTree_=true;
          tree_=fs->make<TTree>(treeName_.c_str(),"miniStringBasedNTupler tree");
        }
      }
      
      //register the leaves by hand
      tree_->Branch("trigger_decision",&trigger_decision);
      tree_->Branch("trigger_name",&trigger_name);
      tree_->Branch("trigger_prescalevalue",&trigger_prescalevalue);
      tree_->Branch("standalone_triggerobject_pt",&standalone_triggerobject_pt);
      tree_->Branch("standalone_triggerobject_px",&standalone_triggerobject_px);
      tree_->Branch("standalone_triggerobject_py",&standalone_triggerobject_py);
      tree_->Branch("standalone_triggerobject_pz",&standalone_triggerobject_pz);
      tree_->Branch("standalone_triggerobject_et",&standalone_triggerobject_et);
      tree_->Branch("standalone_triggerobject_energy",&standalone_triggerobject_energy);
      tree_->Branch("standalone_triggerobject_phi",&standalone_triggerobject_phi);
      tree_->Branch("standalone_triggerobject_eta",&standalone_triggerobject_eta);
      tree_->Branch("standalone_triggerobject_collectionname",&standalone_triggerobject_collectionname);

      tree_->Branch("PU_zpositions",&PU_zpositions_);
      tree_->Branch("PU_sumpT_lowpT",&PU_sumpT_lowpT_);
      tree_->Branch("PU_sumpT_highpT",&PU_sumpT_highpT_);
      tree_->Branch("PU_ntrks_lowpT",&PU_ntrks_lowpT_);
      tree_->Branch("PU_ntrks_highpT",&PU_ntrks_highpT_);
      tree_->Branch("PU_NumInteractions",&PU_NumInteractions_);
      tree_->Branch("PU_bunchCrossing",&PU_bunchCrossing_);
      tree_->Branch("PU_TrueNumInteractions",&PU_TrueNumInteractions_);
 
      tree_->Branch("genHT",genHT_,"genHT/F");

      tree_->Branch("trackingfailurefilter_decision", trackingfailurefilter_decision_ ,"trackingfailurefilter_decision/I");    
      tree_->Branch("goodVerticesfilter_decision", goodVerticesfilter_decision_	 ,"goodVerticesfilter_decision/I");
      tree_->Branch("cschalofilter_decision", cschalofilter_decision_,"cschalofilter_decision/I");			  
      tree_->Branch("trkPOGfilter_decision",  trkPOGfilter_decision_ ,"trkPOGfilter_decision/I");			  	 
      tree_->Branch("trkPOG_logErrorTooManyClustersfilter_decision", trkPOG_logErrorTooManyClustersfilter_decision_ ,"trkPOG_logErrorTooManyClustersfilter_decision/I");  	 
      tree_->Branch("EcalDeadCellTriggerPrimitivefilter_decision",	  EcalDeadCellTriggerPrimitivefilter_decision_	 ,"ecalDeadCellTriggerPrimitivefilter_decision/I");	  
      tree_->Branch("ecallaserfilter_decision",	ecallaserfilter_decision_ ,"ecallaserfilter_decision/I");			  
      tree_->Branch("trkPOG_manystripclus53Xfilter_decision",	  trkPOG_manystripclus53Xfilter_decision_	 ,"trkPOG_manystripclus53Xfilter_decision/I");	  
      tree_->Branch("eebadscfilter_decision",  eebadscfilter_decision_		 ,"eebadscfilter_decision/I");			  
      tree_->Branch("METFiltersfilter_decision", METFiltersfilter_decision_	 ,"METFiltersfilter_decision/I");
      tree_->Branch("HBHENoisefilter_decision",	 HBHENoisefilter_decision_ ,"HBHENoisefilter_decision/I");			  
      tree_->Branch("trkPOG_toomanystripclus53Xfilter_decision",	  trkPOG_toomanystripclus53Xfilter_decision_	 ,"trkPOG_toomanystripclus53Xfilter_decision/I");	  
      tree_->Branch("hcallaserfilter_decision",    hcallaserfilter_decision_,"hcallaserfilter_decision/I");   

      tree_->Branch("mus_isPF",&mus_isPF);
      tree_->Branch("mus_miniso",&mus_miniso);
      tree_->Branch("mus_jet_ind",	&mus_jet_ind);


      

      tree_->Branch("jets_AK4_maxpt_id", &jets_AK4_maxpt_id);
      tree_->Branch("jets_AK4_mu_ind",	&jets_AK4_mu_ind);  
      tree_->Branch("jets_AK4_el_ind",	&jets_AK4_el_ind);
      tree_->Branch("jets_AK4_corL1Fast", &jets_AK4_corL1Fast_);
      tree_->Branch("jets_AK4_corL2L3", &jets_AK4_corL2L3_);
      tree_->Branch("jets_AK4_corL1FastL2L3", &jets_AK4_corL1FastL2L3_);
      

      tree_->Branch("els_isPF",&els_isPF);
      tree_->Branch("els_miniso",&els_miniso);
      tree_->Branch("els_jet_ind",	&els_jet_ind);      
      /* tree_->Branch("els_passPhys14VetoId",	&els_passPhys14VetoId_);       */
      /* tree_->Branch("els_passPhys14LooseId",	&els_passPhys14LooseId_);       */
      /* tree_->Branch("els_passPhys14MediumId",	&els_passPhys14MediumId_);       */
      /* tree_->Branch("els_passPhys14TightId",	&els_passPhys14TightId_);      */ 

      
      /* tree_->Branch("isotk_pt",&isotk_pt_); */
      /* tree_->Branch("isotk_phi",&isotk_phi_); */
      /* tree_->Branch("isotk_eta",&isotk_eta_); */
      /* tree_->Branch("isotk_iso",&isotk_iso_); */
      /* tree_->Branch("isotk_dzpv",&isotk_dzpv_); */
      /* tree_->Branch("isotk_charge",&isotk_charge_); */

      tree_->Branch("taus_n_pfcands",&taus_n_pfcands_);
      tree_->Branch("taus_decayMode",&taus_decayMode_);
      tree_->Branch("taus_CombinedIsolationDeltaBetaCorrRaw3Hits",&taus_CombinedIsolationDeltaBetaCorrRaw3Hits_);
      tree_->Branch("taus_byLooseCombinedIsolationDeltaBetaCorr3Hits",&taus_byLooseCombinedIsolationDeltaBetaCorr3Hits_);
      tree_->Branch("taus_byMediumCombinedIsolationDeltaBetaCorr3Hits",&taus_byMediumCombinedIsolationDeltaBetaCorr3Hits_);
      tree_->Branch("taus_byTightCombinedIsolationDeltaBetaCorr3Hits",&taus_byTightCombinedIsolationDeltaBetaCorr3Hits_);
      tree_->Branch("taus_byDecayModeFinding", &taus_byDecayModeFinding_);
      tree_->Branch("taus_byDecayModeFindingNewDMs", &taus_byDecayModeFindingNewDMs_);
      tree_->Branch("taus_chargedIsoPtSum", &taus_chargedIsoPtSum_);
      tree_->Branch("taus_neutralIsoPtSum", &taus_neutralIsoPtSum_);
      tree_->Branch("taus_puCorrPtSum", &taus_puCorrPtSum_);
      tree_->Branch("taus_againstMuonLoose3", &taus_againstMuonLoose3_);
      tree_->Branch("taus_againstElectronLooseMVA5", &taus_againstElectronLooseMVA5_);
      tree_->Branch("taus_el_ind",	&taus_el_ind);      
      tree_->Branch("taus_mu_ind",	&taus_mu_ind); 

      tree_->Branch("fjets30_pt", &fjets30_pt);  
      tree_->Branch("fjets30_eta", &fjets30_eta);
      tree_->Branch("fjets30_phi", &fjets30_phi);
      tree_->Branch("fjets30_energy",&fjets30_energy);
      tree_->Branch("fjets30_m",&fjets30_m);

      tree_->Branch("pfType1mets_uncert_JetEnUp_dpx", pfType1mets_uncert_JetEnUp_dpx_, "pfType1mets_uncert_JetEnUp_dpx/F");
      tree_->Branch("pfType1mets_uncert_JetEnUp_dpy", pfType1mets_uncert_JetEnUp_dpy_, "pfType1mets_uncert_JetEnUp_dpy/F");
      tree_->Branch("pfType1mets_uncert_JetEnUp_sumEt", pfType1mets_uncert_JetEnUp_sumEt_, "pfType1mets_uncert_JetEnUp_sumEt/F");
      tree_->Branch("pfType1mets_uncert_JetEnDown_dpx", pfType1mets_uncert_JetEnDown_dpx_, "pfType1mets_uncert_JetEnDown_dpx/F");
      tree_->Branch("pfType1mets_uncert_JetEnDown_dpy", pfType1mets_uncert_JetEnDown_dpy_, "pfType1mets_uncert_JetEnDown_dpy/F");
      tree_->Branch("pfType1mets_uncert_JetEnDown_sumEt", pfType1mets_uncert_JetEnDown_sumEt_, "pfType1mets_uncert_JetEnDown_sumEt/F");
      tree_->Branch("pfType1mets_uncert_JetResUp_dpx", pfType1mets_uncert_JetResUp_dpx_, "pfType1mets_uncert_JetResUp_dpx/F");
      tree_->Branch("pfType1mets_uncert_JetResUp_dpy", pfType1mets_uncert_JetResUp_dpy_, "pfType1mets_uncert_JetResUp_dpy/F");
      tree_->Branch("pfType1mets_uncert_JetResUp_sumEt", pfType1mets_uncert_JetResUp_sumEt_, "pfType1mets_uncert_JetResUp_sumEt/F");
      tree_->Branch("pfType1mets_uncert_JetResDown_dpx", pfType1mets_uncert_JetResDown_dpx_, "pfType1mets_uncert_JetResDown_dpx/F");
      tree_->Branch("pfType1mets_uncert_JetResDown_dpy", pfType1mets_uncert_JetResDown_dpy_, "pfType1mets_uncert_JetResDown_dpy/F");
      tree_->Branch("pfType1mets_uncert_JetResDown_sumEt", pfType1mets_uncert_JetResDown_sumEt_, "pfType1mets_uncert_JetResDown_sumEt/F");
      tree_->Branch("raw_met_et",raw_met_et_, "raw_met_et/F");
      tree_->Branch("raw_met_phi",raw_met_phi_, "raw_met_phi/F");
      tree_->Branch("raw_met_sumEt",raw_met_sumEt_, "raw_met_sumEt/F");


      tree_->Branch("photons_full5x5sigmaIEtaIEta", photons_full5x5sigmaIEtaIEta_);
      tree_->Branch("photons_pass_el_veto", photons_pass_el_veto_);
  
      tree_->Branch("fixedGridRhoFastjetAll", fixedGridRhoFastjetAll_, "fixedGridRhoFastjetAll/F");


    }

    else{
      //EDM COMPLIANT PART
      //      producer->produce<ACertainCollection>(ACertainInstanceName);
    }


    return nLeaves;
  }

  void callBack(){
    //clean up whatever memory was allocated
  }

  miniAdHocNTupler (const edm::ParameterSet& iConfig){
    edm::ParameterSet adHocPSet = iConfig.getParameter<edm::ParameterSet>("AdHocNPSet");
    nevents = 0;

    if (adHocPSet.exists("useTFileService"))
      useTFileService_=adHocPSet.getParameter<bool>("useTFileService");         
    else
      useTFileService_=iConfig.getParameter<bool>("useTFileService");

    if (useTFileService_){
      if (adHocPSet.exists("treeName")){
        treeName_=adHocPSet.getParameter<std::string>("treeName");
        ownTheTree_=true;
      }
      else{
        treeName_=iConfig.getParameter<std::string>("treeName");
        ownTheTree_=false;
      }
    }
    

    trigger_decision = new std::vector<bool>;
    trigger_name = new std::vector<std::string>;
    trigger_prescalevalue = new std::vector<float>;
    standalone_triggerobject_pt = new std::vector<float>;
    standalone_triggerobject_px = new std::vector<float>;
    standalone_triggerobject_py = new std::vector<float>;
    standalone_triggerobject_pz = new std::vector<float>;
    standalone_triggerobject_et = new std::vector<float>;
    standalone_triggerobject_energy = new std::vector<float>;
    standalone_triggerobject_phi = new std::vector<float>;
    standalone_triggerobject_eta = new std::vector<float>;
    standalone_triggerobject_collectionname = new std::vector<std::string>;

    PU_zpositions_ = new std::vector<std::vector<float> >;
    PU_sumpT_lowpT_ = new std::vector<std::vector<float> >;
    PU_sumpT_highpT_ = new std::vector<std::vector<float> >;
    PU_ntrks_lowpT_ = new std::vector<std::vector<int> >;
    PU_ntrks_highpT_ = new std::vector<std::vector<int> >;
    PU_NumInteractions_ = new std::vector<int>;
    PU_bunchCrossing_ = new std::vector<int>;
    PU_TrueNumInteractions_ = new std::vector<float>;

    genHT_ = new float;

    trackingfailurefilter_decision_			= new int;   
    goodVerticesfilter_decision_			= new int;
    cschalofilter_decision_				= new int;    
    trkPOGfilter_decision_				= new int;
    trkPOG_logErrorTooManyClustersfilter_decision_	= new int;
    EcalDeadCellTriggerPrimitivefilter_decision_	= new int;
    ecallaserfilter_decision_				= new int;    
    trkPOG_manystripclus53Xfilter_decision_		= new int;
    eebadscfilter_decision_				= new int;    
    METFiltersfilter_decision_				= new int;    
    HBHENoisefilter_decision_				= new int;    
    trkPOG_toomanystripclus53Xfilter_decision_		= new int;
    hcallaserfilter_decision_				= new int;

    els_isPF = new std::vector<bool>;
    mus_isPF = new std::vector<bool>;

    els_miniso = new std::vector<float>;
    mus_miniso = new std::vector<float>;

    jets_AK4_maxpt_id = new std::vector<int>;
    jets_AK4_mu_ind = new std::vector<int>;
    jets_AK4_el_ind = new std::vector<int>;
    jets_AK4_corL1Fast_ = new std::vector<float>;
    jets_AK4_corL2L3_ = new std::vector<float>;
    jets_AK4_corL1FastL2L3_ = new std::vector<float>;
  
    taus_el_ind = new std::vector<int>;
    taus_mu_ind = new std::vector<int>;
    els_jet_ind = new std::vector<int>;
    mus_jet_ind = new std::vector<int>;

    /* els_passPhys14VetoId_ = new std::vector<int>; */
    /* els_passPhys14LooseId_ = new std::vector<int>; */
    /* els_passPhys14MediumId_ = new std::vector<int>; */
    /* els_passPhys14TightId_ = new std::vector<int>; */

    //isolated tracks (charged pf candidates)                                                                                                               
    /* isotk_pt_ = new std::vector<float>; */
    /* isotk_phi_ = new std::vector<float>; */
    /* isotk_eta_ = new std::vector<float>; */
    /* isotk_iso_ = new std::vector<float>; */
    /* isotk_dzpv_ = new std::vector<float>; */
    /* isotk_charge_ = new std::vector<int>; */

    taus_CombinedIsolationDeltaBetaCorrRaw3Hits_ = new std::vector<float>;
    taus_byLooseCombinedIsolationDeltaBetaCorr3Hits_ = new std::vector<bool>;
    taus_byMediumCombinedIsolationDeltaBetaCorr3Hits_ = new std::vector<bool>;
    taus_byTightCombinedIsolationDeltaBetaCorr3Hits_ = new std::vector<bool>;
    taus_n_pfcands_ = new std::vector<int>;
    taus_decayMode_ = new std::vector<int>;
    taus_byDecayModeFinding_ = new std::vector<bool>;
    taus_byDecayModeFindingNewDMs_ = new std::vector<bool>;
    taus_chargedIsoPtSum_ = new std::vector<float>;
    taus_neutralIsoPtSum_= new std::vector<float>;
    taus_puCorrPtSum_ = new std::vector<float>;
    taus_againstMuonLoose3_ = new std::vector<bool>;
    taus_againstElectronLooseMVA5_ = new std::vector<bool>;
  
    fjets30_pt =     new std::vector<float>;
    fjets30_eta =    new std::vector<float>;
    fjets30_phi =    new std::vector<float>;
    fjets30_energy = new std::vector<float>;
    fjets30_m =     new std::vector<float>;

    pfType1mets_uncert_JetEnUp_dpx_ =     new float;
    pfType1mets_uncert_JetEnUp_dpy_ =    new float;
    pfType1mets_uncert_JetEnUp_sumEt_ =    new float;
    pfType1mets_uncert_JetEnDown_dpx_ =     new float;
    pfType1mets_uncert_JetEnDown_dpy_ =    new float;
    pfType1mets_uncert_JetEnDown_sumEt_ =    new float;
    pfType1mets_uncert_JetResUp_dpx_ =     new float;
    pfType1mets_uncert_JetResUp_dpy_ =    new float;
    pfType1mets_uncert_JetResUp_sumEt_ =    new float;
    pfType1mets_uncert_JetResDown_dpx_ =     new float;
    pfType1mets_uncert_JetResDown_dpy_ =    new float;
    pfType1mets_uncert_JetResDown_sumEt_ =    new float;
    raw_met_et_ = new float;
    raw_met_phi_ = new float;
    raw_met_sumEt_ = new float;

    photons_full5x5sigmaIEtaIEta_ =     new std::vector<float>;
    photons_pass_el_veto_ =     new std::vector<bool>;
  
    fixedGridRhoFastjetAll_ = new float;
 
  }

  ~miniAdHocNTupler(){
    delete trigger_decision;
    delete trigger_name;
    delete trigger_prescalevalue;
    delete standalone_triggerobject_pt;
    delete standalone_triggerobject_px;
    delete standalone_triggerobject_py;
    delete standalone_triggerobject_pz;
    delete standalone_triggerobject_et;
    delete standalone_triggerobject_energy;
    delete standalone_triggerobject_phi;
    delete standalone_triggerobject_eta;
    delete standalone_triggerobject_collectionname;

    delete PU_zpositions_;
    delete PU_sumpT_lowpT_;
    delete PU_sumpT_highpT_;
    delete PU_ntrks_lowpT_;
    delete PU_ntrks_highpT_;
    delete PU_NumInteractions_;
    delete PU_bunchCrossing_;
    delete PU_TrueNumInteractions_;

    delete genHT_;

    delete trackingfailurefilter_decision_		     ;
    delete goodVerticesfilter_decision_		     ;
    delete cschalofilter_decision_			     ;
    delete trkPOGfilter_decision_			     ;
    delete trkPOG_logErrorTooManyClustersfilter_decision_;
    delete EcalDeadCellTriggerPrimitivefilter_decision_  ;
    delete ecallaserfilter_decision_		     ;    
    delete trkPOG_manystripclus53Xfilter_decision_	     ;
    delete eebadscfilter_decision_			     ;
    delete METFiltersfilter_decision_		     ;    
    delete HBHENoisefilter_decision_		     ;    
    delete trkPOG_toomanystripclus53Xfilter_decision_    ;
    delete hcallaserfilter_decision_		     ;

    delete els_isPF;
    delete mus_isPF;

    delete els_miniso;
    delete mus_miniso;

    delete jets_AK4_maxpt_id;
    delete jets_AK4_mu_ind;
    delete jets_AK4_el_ind;
    delete jets_AK4_corL1Fast_;
    delete jets_AK4_corL2L3_;
    delete jets_AK4_corL1FastL2L3_;
  
    delete taus_el_ind;
    delete taus_mu_ind;
    delete els_jet_ind;
    delete mus_jet_ind;

    /* delete els_passPhys14VetoId_; */
    /* delete els_passPhys14LooseId_; */
    /* delete els_passPhys14MediumId_; */
    /* delete els_passPhys14TightId_; */

    /* delete isotk_pt_; */
    /* delete isotk_phi_; */
    /* delete isotk_eta_; */
    /* delete isotk_iso_; */
    /* delete isotk_dzpv_; */
    /* delete isotk_charge_; */

    delete taus_CombinedIsolationDeltaBetaCorrRaw3Hits_;
    delete taus_byLooseCombinedIsolationDeltaBetaCorr3Hits_;
    delete taus_byMediumCombinedIsolationDeltaBetaCorr3Hits_;
    delete taus_byTightCombinedIsolationDeltaBetaCorr3Hits_;
    delete taus_n_pfcands_;
    delete taus_decayMode_;
    delete taus_byDecayModeFinding_;
    delete taus_byDecayModeFindingNewDMs_;
    delete taus_chargedIsoPtSum_;
    delete taus_neutralIsoPtSum_;
    delete taus_puCorrPtSum_;
    delete taus_againstMuonLoose3_;
    delete taus_againstElectronLooseMVA5_;
  
    delete fjets30_pt;
    delete fjets30_eta;
    delete fjets30_phi;
    delete fjets30_energy;
    delete fjets30_m;

    delete pfType1mets_uncert_JetEnUp_dpx_;
    delete pfType1mets_uncert_JetEnUp_dpy_;
    delete pfType1mets_uncert_JetEnUp_sumEt_;
    delete pfType1mets_uncert_JetEnDown_dpx_;
    delete pfType1mets_uncert_JetEnDown_dpy_;
    delete pfType1mets_uncert_JetEnDown_sumEt_;
    delete pfType1mets_uncert_JetResUp_dpx_;
    delete pfType1mets_uncert_JetResUp_dpy_;
    delete pfType1mets_uncert_JetResUp_sumEt_;
    delete pfType1mets_uncert_JetResDown_dpx_;
    delete pfType1mets_uncert_JetResDown_dpy_;
    delete pfType1mets_uncert_JetResDown_sumEt_;
    delete raw_met_et_;
    delete raw_met_phi_;
    delete raw_met_sumEt_;

    delete photons_full5x5sigmaIEtaIEta_;
    delete photons_pass_el_veto_;

    delete fixedGridRhoFastjetAll_;


    // delete clusterTools_;

  }

 private:
  bool ownTheTree_;
  std::string treeName_;
  bool useTFileService_;
  long nevents;


  std::vector<bool> * trigger_decision;
  std::vector<std::string> * trigger_name;
  std::vector<float> * trigger_prescalevalue;
  std::vector<float> * standalone_triggerobject_pt;
  std::vector<float> * standalone_triggerobject_px;
  std::vector<float> * standalone_triggerobject_py;
  std::vector<float> * standalone_triggerobject_pz;
  std::vector<float> * standalone_triggerobject_et;
  std::vector<float> * standalone_triggerobject_energy;
  std::vector<float> * standalone_triggerobject_phi;
  std::vector<float> * standalone_triggerobject_eta;
  std::vector<std::string> * standalone_triggerobject_collectionname;

  std::vector<std::vector<float> > * PU_zpositions_;
  std::vector<std::vector<float> > * PU_sumpT_lowpT_;
  std::vector<std::vector<float> > * PU_sumpT_highpT_;
  std::vector<std::vector<int> > * PU_ntrks_lowpT_;
  std::vector<std::vector<int> > * PU_ntrks_highpT_;
  std::vector<int> * PU_NumInteractions_;
  std::vector<int> * PU_bunchCrossing_;
  std::vector<float> * PU_TrueNumInteractions_;

  float * genHT_;
 
  int *trackingfailurefilter_decision_		     ;
  int *goodVerticesfilter_decision_		     ;
  int *cschalofilter_decision_			     ;
  int *trkPOGfilter_decision_			     ;
  int *trkPOG_logErrorTooManyClustersfilter_decision_;
  int *EcalDeadCellTriggerPrimitivefilter_decision_  ;
  int *ecallaserfilter_decision_		     ;    
  int *trkPOG_manystripclus53Xfilter_decision_	     ;
  int *eebadscfilter_decision_			     ;
  int *METFiltersfilter_decision_		     ;    
  int *HBHENoisefilter_decision_		     ;    
  int *trkPOG_toomanystripclus53Xfilter_decision_    ;
  int *hcallaserfilter_decision_		     ;

  std::vector<bool> * els_isPF;
  std::vector<bool> * mus_isPF;

  std::vector<float> * els_miniso;
  std::vector<float> * mus_miniso;

  std::vector<int> * jets_AK4_maxpt_id;
  std::vector<int> * jets_AK4_mu_ind;
  std::vector<int> * jets_AK4_el_ind;
  std::vector<float> * jets_AK4_corL1Fast_;
  std::vector<float> * jets_AK4_corL2L3_;
  std::vector<float> * jets_AK4_corL1FastL2L3_;

  std::vector<int> * taus_el_ind;
  std::vector<int> * taus_mu_ind;
  std::vector<int> * els_jet_ind;
  std::vector<int> * mus_jet_ind;

  /* std::vector<int> * els_passPhys14VetoId_; */
  /* std::vector<int> * els_passPhys14LooseId_; */
  /* std::vector<int> * els_passPhys14MediumId_; */
  /* std::vector<int> * els_passPhys14TightId_; */
  
  /* std::vector<float> * isotk_pt_; */
  /* std::vector<float> * isotk_phi_; */
  /* std::vector<float> * isotk_eta_; */
  /* std::vector<float> * isotk_iso_; */
  /* std::vector<float> * isotk_dzpv_; */
  /* std::vector<int> *   isotk_charge_; */

  std::vector<float> *  taus_CombinedIsolationDeltaBetaCorrRaw3Hits_;
  std::vector<bool> *  taus_byLooseCombinedIsolationDeltaBetaCorr3Hits_;
  std::vector<bool> *  taus_byMediumCombinedIsolationDeltaBetaCorr3Hits_;
  std::vector<bool> *  taus_byTightCombinedIsolationDeltaBetaCorr3Hits_;
  std::vector<int> *  taus_n_pfcands_;
  std::vector<int> *  taus_decayMode_;
  std::vector<bool> *  taus_byDecayModeFinding_;
  std::vector<bool> *  taus_byDecayModeFindingNewDMs_;
  std::vector<float> *  taus_chargedIsoPtSum_;
  std::vector<float> *  taus_neutralIsoPtSum_;
  std::vector<float> *  taus_puCorrPtSum_;
  std::vector<bool> *  taus_againstMuonLoose3_;
  std::vector<bool> *  taus_againstElectronLooseMVA5_;
 
  std::vector<float> * fjets30_pt;
  std::vector<float> * fjets30_eta;
  std::vector<float> * fjets30_phi;
  std::vector<float> * fjets30_energy;
  std::vector<float> * fjets30_m;

  float *pfType1mets_uncert_JetEnUp_dpx_;
  float *pfType1mets_uncert_JetEnUp_dpy_;
  float *pfType1mets_uncert_JetEnUp_sumEt_;
  float *pfType1mets_uncert_JetEnDown_dpx_;
  float *pfType1mets_uncert_JetEnDown_dpy_;
  float *pfType1mets_uncert_JetEnDown_sumEt_;
  float *pfType1mets_uncert_JetResUp_dpx_;
  float *pfType1mets_uncert_JetResUp_dpy_;
  float *pfType1mets_uncert_JetResUp_sumEt_;
  float *pfType1mets_uncert_JetResDown_dpx_;
  float *pfType1mets_uncert_JetResDown_dpy_;
  float *pfType1mets_uncert_JetResDown_sumEt_;

  float *raw_met_et_;
  float *raw_met_phi_;
  float *raw_met_sumEt_;

  std::vector<float> *photons_full5x5sigmaIEtaIEta_;
  std::vector<bool> *photons_pass_el_veto_;

  float *fixedGridRhoFastjetAll_; 

};
