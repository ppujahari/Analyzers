// -*- Header -*-
//
// Package:    Analyzers/ChargeDepAndPtCorr
// Class:      ChargeDepAndPtCorr
//
/**\class ChargeDepAndPtCorr ChargeDepAndPtCorr.h Analyzers/ChargeDepAndPtCorr/interface/ChargeDepAndPtCorr.h

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Maxime Guilbaud
//         Created:  Thu, 01 Jun 2017 16:56:11 GMT
//
//


// system include files
#include <memory>

// CMSSW include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"

#include "DataFormats/HeavyIonEvent/interface/Centrality.h"

#include "FWCore/Utilities/interface/InputTag.h"

// user include files
#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TVector3.h"

#include "Analyzers/ChargeDepAndPtCorr/interface/DiHadronCorrelationEvt.h"
//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class ChargeDepAndPtCorr : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit ChargeDepAndPtCorr(const edm::ParameterSet&);
      ~ChargeDepAndPtCorr();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

      void   LoopVertices(const edm::Event& iEvent, const edm::EventSetup& iSetup);
      double LoopNoff(const edm::Event& iEvent, const edm::EventSetup& iSetup);
      void   LoopTracks(const edm::Event& iEvent, const edm::EventSetup& iSetup,
                        bool istrg, int evtclass);
      void   LoopCaloTower(const edm::Event& iEvent, const edm::EventSetup& iSetup);

      double GetEffWeight(double eta, double pt, int evtclass);
      //double GetEffWeight(const TVector3 a, int evtclass);
      int    GetpTbin(double pt, bool istrig);
      void   AssignpTbins(double pt,  double eta,
                          double phi, int charge,
                          double eff, bool istrg,
                          int idx);


      double deltaeta(const TVector3 a, const TVector3 b);
      double deltaphi(const TVector3 a, const TVector3 b);


      double GetDeltaEta(double eta_trg, double eta_ass);
      double GetDeltaPhi(double phi_trg, double phi_ass);
      double GetDPhiStar(double phi1, double pt1, int charge1, double phi2, double pt2, int charge2, double radius, double bSign);

      void FillHistsSignal(int ievt);
      void FillHistsBackground(int ievt_trg, int jevt_ass);
      void NormalizeHists();


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;


      TVector3 trigger;
      TVector3 associated;

      // ----------member data ---------------------------
      // ## tracks ##
      // used to select what tracks to read from configuration file
      edm::EDGetTokenT<reco::TrackCollection> trackTags_;

      // ## vertex ##
      // used to select what vertex to read from configuration file
      edm::EDGetTokenT<reco::VertexCollection> vtxTags_;

      // ## calotower ##
      // used to select what calo tower to read from configuration file
      edm::EDGetTokenT<CaloTowerCollection> caloTowersTags_;

      // ## centrality ##
      // used to select what centrality collection to read from configuration file
      edm::EDGetTokenT<reco::Centrality> centralityTags_;
      // used to access centrality bins
      edm::EDGetTokenT<int> centralityBinTags_;

      // ## event classifier ##
      int evtclassifier_;//Switch to centrality dependent correction instead of mult
      int centmin_;
      int centmax_;
      int noffmin_;
      int noffmax_;

      // ## Eff/Fake coorection
      bool cweight_; //Apply (True) or not (False) the eff/fake correction
      edm::InputTag fname_; //Name of the file that contains the eff corrections
      std::vector<int> effCorrBinMin_; //Mult/Cent binning of the eff/coor tables
      std::vector<int> effCorrBinMax_; //Mult/Cent binning of the eff/coor tables
      TFile* feff_; //ROOT File that contains histograms used for corrections
      std::vector<TH2D*> heff_; //Histograms used for corrections (eta, pT)

      // ## Vertex variables
      int nVtx_;
      double xBestVtx_; //Best vertex X position
      double yBestVtx_; //Best vertex Y position
      double zBestVtx_; //Best vertex Z position
      double rhoBestVtx_; //Best vertex XY position
      double xBestVtxError_; //Best vertex X error
      double yBestVtxError_; //Best vertex Y error
      double zBestVtxError_; //Best vertex Z error
      double zminVtx_; //min value for Z cut on vtx position
      double zmaxVtx_; //max value for Z cut on vtx position
      double rhomaxVtx_; //max value for XY cut on vtx position
      unsigned int nTrkAssoToVtx_; //number of track requiered to be associated to a vertex
      bool selectVtxByMult_;       //False: sel best vtx by sum pT^2 True: sel best vtx with highest multiplicity

      // ## track selection ##
      int    nTrkTot_trg_;
      float  nTrkTot_corr_trg_;
      int    nTrkTot_ass_;
      float  nTrkTot_corr_ass_;
      std::vector<int>    nTrk_trg_;
      std::vector<double> nTrk_corr_trg_;
      std::vector<int>    nTrk_ass_;
      std::vector<double> nTrk_corr_ass_;
      double dzdzerror_; //DCA - z  significance
      double d0dz0rror_; //DCA - xy significance
      double pTerrorpT_; //DCA - pT resolution
      std::vector< double > pTmin_trg_; //min pt of the trigger tracks
      std::vector< double > pTmax_trg_; //max pt of the trigger tracks
      std::vector< double > pTmin_ass_; //min pt of the associated tracks
      std::vector< double > pTmax_ass_; //max pt of the associated tracks
      double etamin_trg_; //min eta of the trigger tracks
      double etamax_trg_; //max eta of the trigger tracks
      double etamin_ass_; //min eta of the associated tracks
      double etamax_ass_; //max eta of the associated tracks
      bool isHI_; //Specific set of cuts PbPb data
      bool isPix_;//Specific set of cuts for pixel tracks in PbPb data
      double pTmax_pix_; //maximum pT to be a pixel track
      int nhitsmin_pix_; //min number of hits to be a pix trk
      int nhitsmax_pix_; //max number of hits to be a pix trk
      double chi2nmax_pix_;  //maximum chi2n/nlayer to be a pixel track
      double dzdzerror_pix_; //maximum DCA - z significance to be a pixel track
      int nhitsmin_; //min number of hits for general tracks
      std::vector<int> algo_; //algo for general tracks
      double chi2nmax_; //maximum chi2 for general track

      // ## Dihardon corr events ##
      DiHadronCorrelationEvt* evt_;
      std::vector< DiHadronCorrelationEvt > evtVec_;
      unsigned int bkgFactor_;



      // ## histograms ##
      //~~~> Vertex
      TH1F* hXBestVtx_;
      TH1F* hYBestVtx_;
      TH1F* hRhoBestVtx_;
      TH1F* hZBestVtx_;
      TH1I* hNVtx_;

      TH1F* hPhidist_;

      //~~~> Global
      TH1I* hCent_;
      TH1I* hNoff_;
      TH1I* hMult_trg_;
      TH1F* hMult_corr_trg_;
      TH1I* hMult_ass_;
      TH1F* hMult_corr_ass_;

      TH1I* hCounts_; //claude

      //~~~> Trigger tracks RAW
      std::vector<TH1F*> hEtaTrk_trg_;
      std::vector<TH1F*> hPtTrk_trg_;
      std::vector<TH1F*> hPhiTrk_trg_;
      std::vector<TH1I*> hMultTrk_trg_;
      //~~~> Trigger tracks CORR
      std::vector<TH1F*> hEtaTrk_corr_trg_;
      std::vector<TH1F*> hPtTrk_corr_trg_;
      std::vector<TH1F*> hPhiTrk_corr_trg_;
      std::vector<TH1F*> hMultTrk_corr_trg_;
      //~~~> Associated tracks RAW
      std::vector<TH1F*> hEtaTrk_ass_;
      std::vector<TH1F*> hPtTrk_ass_;
      std::vector<TH1F*> hPhiTrk_ass_;
      std::vector<TH1I*> hMultTrk_ass_;
      //~~~> Associated tracks CORR
      std::vector<TH1F*> hEtaTrk_corr_ass_;
      std::vector<TH1F*> hPtTrk_corr_ass_;
      std::vector<TH1F*> hPhiTrk_corr_ass_;
      std::vector<TH1F*> hMultTrk_corr_ass_;
      //~~~> Calo towers
      TH1F* hEtaCTow_;
      TH1F* hEtCTow_;
      TH1F* hPhiCTow_;
      //~~~> Signal, background, correlation
      std::vector< std::vector<TH2D*> > hSignal_;
      std::vector< std::vector<TH2D*> > hSignalPP_;
      std::vector< std::vector<TH2D*> > hSignalMM_;
      std::vector< std::vector<TH2D*> > hSignalPM_;
      std::vector< std::vector<TH2D*> > hSignalMP_;

      TH1D* hDeltaZvtx_;

      std::vector< std::vector<TH2D*> > hBackground_;
      std::vector< std::vector<TH2D*> > hBackgroundPP_;
      std::vector< std::vector<TH2D*> > hBackgroundMM_;
      std::vector< std::vector<TH2D*> > hBackgroundPM_;
      std::vector< std::vector<TH2D*> > hBackgroundMP_;

      std::vector< std::vector<TH2D*> > hCorrelation_;
      std::vector< std::vector<TH2D*> > hCorrelationPP_;
      std::vector< std::vector<TH2D*> > hCorrelationMM_;
      std::vector< std::vector<TH2D*> > hCorrelationPM_;
      std::vector< std::vector<TH2D*> > hCorrelationMP_;
      int nEtaBins_;
      int nPhiBins_;




};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

