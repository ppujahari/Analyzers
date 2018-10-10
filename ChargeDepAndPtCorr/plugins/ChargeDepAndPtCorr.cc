// -*- C++ -*-
//
// Package:    Analyzers/ChargeDepAndPtCorr
// Class:      ChargeDepAndPtCorr
//
/**\class ChargeDepAndPtCorr ChargeDepAndPtCorr.cc Analyzers/ChargeDepAndPtCorr/plugins/ChargeDepAndPtCorr.cc
 Description: [one line class summary]
 Implementation:
     [Notes on implementation]
*/
//
//          Author:  Prabhat Pujahari 
//         Created:  Thu, 01 Jun 2017 16:56:11 GMT
//
//This code is doing the calculation using DeltaVz method 2.

// system include files

// CMSSW include files
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

// user include files
#include "Analyzers/ChargeDepAndPtCorr/interface/ChargeDepAndPtCorr.h"

//
// constructors and destructor
//
ChargeDepAndPtCorr::ChargeDepAndPtCorr(const edm::ParameterSet& iConfig) :
  //tracks
  trackTags_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"))),
  //vertex
  vtxTags_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertex"))),
  //caloTower
  caloTowersTags_(consumes<CaloTowerCollection>(iConfig.getParameter<edm::InputTag>("caloTower"))),
  //centrality
  centralityTags_(consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralitySrc"))),
  centralityBinTags_(consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinSrc"))),
  //Event classifier
  evtclassifier_(iConfig.getUntrackedParameter<int>("evtclassifier")),
  centmin_(iConfig.getUntrackedParameter<int>("centmin")),
  centmax_(iConfig.getUntrackedParameter<int>("centmax")),
  noffmin_(iConfig.getUntrackedParameter<int>("noffmin")),
  noffmax_(iConfig.getUntrackedParameter<int>("noffmax")),
  //Eff/Fake correction
  cweight_(iConfig.getUntrackedParameter<bool>("cweight")),
  fname_(iConfig.getUntrackedParameter<edm::InputTag>("fname")),
  effCorrBinMin_(iConfig.getUntrackedParameter< std::vector< int > >("effCorrBinMin")),
  effCorrBinMax_(iConfig.getUntrackedParameter< std::vector< int > >("effCorrBinMax")),
  //vertex selection
  zminVtx_(iConfig.getUntrackedParameter<double>("zminVtx")),
  zmaxVtx_(iConfig.getUntrackedParameter<double>("zmaxVtx")),
  rhomaxVtx_(iConfig.getUntrackedParameter<double>("rhomaxVtx")),
  nTrkAssoToVtx_(iConfig.getUntrackedParameter<unsigned int>("nTrkAssoToVtx")),
  selectVtxByMult_(iConfig.getUntrackedParameter<bool>("selectVtxByMult")),
  //track selection
  dzdzerror_(iConfig.getUntrackedParameter<double>("dzdzerror")),
  d0dz0rror_(iConfig.getUntrackedParameter<double>("d0dz0rror")),
  pTerrorpT_(iConfig.getUntrackedParameter<double>("pTerrorpT")),
  pTmin_trg_(iConfig.getUntrackedParameter< std::vector< double > >("pTminTrk_trg")),
  pTmax_trg_(iConfig.getUntrackedParameter< std::vector< double > >("pTmaxTrk_trg")),
  pTmin_ass_(iConfig.getUntrackedParameter< std::vector< double > >("pTminTrk_ass")),
  pTmax_ass_(iConfig.getUntrackedParameter< std::vector< double > >("pTmaxTrk_ass")),
  etamin_trg_(iConfig.getUntrackedParameter<double>("etaminTrk_trg")),
  etamax_trg_(iConfig.getUntrackedParameter<double>("etamaxTrk_trg")),
  etamin_ass_(iConfig.getUntrackedParameter<double>("etaminTrk_ass")),
  etamax_ass_(iConfig.getUntrackedParameter<double>("etamaxTrk_ass")),
  isHI_(iConfig.getUntrackedParameter<bool>("isHI")),
  isPix_(iConfig.getUntrackedParameter<bool>("isPix")),
  pTmax_pix_(iConfig.getUntrackedParameter<double>("pTmax_pix")),
  nhitsmin_pix_(iConfig.getUntrackedParameter<int>("nhitsmin_pix")),
  nhitsmax_pix_(iConfig.getUntrackedParameter<int>("nhitsmax_pix")),
  chi2nmax_pix_(iConfig.getUntrackedParameter<double>("chi2nmax_pix")),
  dzdzerror_pix_(iConfig.getUntrackedParameter<double>("dzdzerror_pix")),
  nhitsmin_(iConfig.getUntrackedParameter<int>("nhitsmin")),
  algo_(iConfig.getUntrackedParameter< std::vector<int> >("algo")),
  chi2nmax_(iConfig.getUntrackedParameter<double>("chi2nmax")),
  //Mixing
  bkgFactor_(iConfig.getUntrackedParameter<unsigned int>("bkgFactor")),
  //Histogram binning
  nEtaBins_(iConfig.getUntrackedParameter<int>("nEtaBins")),
  nPhiBins_(iConfig.getUntrackedParameter<int>("nPhiBins"))
{
   //file acc & eff
   TString filename(fname_.label().c_str());
   feff_ = 0x0;
   //heff_ = 0x0;
   if(cweight_ && !filename.IsNull())
   {
      edm::FileInPath fip(Form("Analyzers/ChargeDepAndPtCorr/data/EFF/%s",filename.Data()));
      feff_ = new TFile(fip.fullPath().c_str(),"READ");
      heff_.resize(feff_->GetNkeys());
      for(unsigned int ik = 0; ik < heff_.size(); ++ik)
      {
          heff_[ik] = (TH2D*) feff_->Get(feff_->GetListOfKeys()->At(ik)->GetName());
      }
   }
   else
   {
      cweight_ = false;
      edm::LogWarning ("Cannot open file") <<"Invalid efficiency correction file!";
   }

   evt_ = new DiHadronCorrelationEvt(pTmin_trg_.size(), pTmin_ass_.size());

   TH1::SetDefaultSumw2();

   // Now do what ever initialization is needed
   usesResource("TFileService");
   edm::Service<TFileService> fs;
   // Histograms
   TFileDirectory fVtxHist  = fs->mkdir("Vertex");
   /*
   hXBestVtx_   = fVtxHist.make<TH1F>("hXvtx", "", 80,   -0.4, 0.4);
   hYBestVtx_   = fVtxHist.make<TH1F>("hYvtx", "", 80,   -0.4, 0.4);
   hRhoBestVtx_ = fVtxHist.make<TH1F>("hRvtx", "", 600,   0, 0.6);
   hZBestVtx_   = fVtxHist.make<TH1F>("hZvtx", "", 600, -30., 30.);
   hNVtx_       = fVtxHist.make<TH1I>("hnVtx", "", 100, 0, 100);
   */
   TFileDirectory fGlobalHist  = fs->mkdir("Global");
   //hCent_          = fGlobalHist.make<TH1I>("hCent", "",  200, 0, 200);
   hCounts_          = fGlobalHist.make<TH1I>("hCent", "",  2, 0, 2);

   //hNoff_          = fGlobalHist.make<TH1I>("hNoff", "p_{T} > 0.4 GeV/c", 1000, 0, 1000);*/
   hMult_trg_      = fGlobalHist.make<TH1I>("hMult_trg",
                                            Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[0], pTmax_trg_[pTmax_trg_.size()-1]),
                                            2000, 0, 2000);

   /*hMult_corr_trg_ = fGlobalHist.make<TH1F>("hMultcorr_trg",
                                            Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[0], pTmax_trg_[pTmax_trg_.size()-1]),
					    1000, 0, 50000);*/
   hMult_ass_      = fGlobalHist.make<TH1I>("hMult_ass",
                                            Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_ass_[0], pTmax_ass_[pTmax_ass_.size()-1]),
                                            2000, 0, 2000);/*
   hMult_corr_ass_ = fGlobalHist.make<TH1F>("hMultcorr_ass",
                                            Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_ass_[0], pTmax_ass_[pTmax_ass_.size()-1]),
                                            1000, 0, 50000);

                                            */
   hPhidist_ = fGlobalHist.make<TH1F>("hPhidist", "", 31, -TMath::Pi(),  TMath::Pi() );

   TFileDirectory fTrkTrgHist  = fs->mkdir("TrgTracksRaw");
   /*
   hEtaTrk_trg_.resize(pTmin_trg_.size());
   hPtTrk_trg_.resize(pTmin_trg_.size());
   hPhiTrk_trg_.resize(pTmin_trg_.size());
   hMultTrk_trg_.resize(pTmin_trg_.size());
   for(unsigned int ipt = 0; ipt < pTmin_trg_.size(); ++ipt)
   {
      //bin size for some plots
      int nbins_trg = static_cast<int>((pTmax_trg_[ipt]-pTmin_trg_[ipt])/0.01);

      hEtaTrk_trg_[ipt] = fTrkTrgHist.make<TH1F>(Form("hEtatrk_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 300, -3., 3.);
      hPtTrk_trg_[ipt]  = fTrkTrgHist.make<TH1F>(Form("hPttrk_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 nbins_trg, pTmin_trg_[ipt], pTmax_trg_[ipt]);
      hPhiTrk_trg_[ipt] = fTrkTrgHist.make<TH1F>(Form("hPhitrk_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 640, -3.2, 3.2);
      hMultTrk_trg_[ipt] = fTrkTrgHist.make<TH1I>(Form("hMulttrk_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 1000, 0, 50000);
   }
   */
   TFileDirectory fTrkCorrTrgHist  = fs->mkdir("TrgTracksCorr");
   /*
   hEtaTrk_corr_trg_.resize(pTmin_trg_.size());
   hPtTrk_corr_trg_.resize(pTmin_trg_.size());
   hPhiTrk_corr_trg_.resize(pTmin_trg_.size());
   hMultTrk_corr_trg_.resize(pTmin_trg_.size());
   for(unsigned int ipt = 0; ipt < pTmin_trg_.size(); ++ipt)
   {
      //bin size for some plots
      int nbins_trg = static_cast<int>((pTmax_trg_[ipt]-pTmin_trg_[ipt])/0.01);

      hEtaTrk_corr_trg_[ipt] = fTrkCorrTrgHist.make<TH1F>(Form("hEtatrk_corr_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 300, -3., 3.);
      hPtTrk_corr_trg_[ipt]  = fTrkCorrTrgHist.make<TH1F>(Form("hPttrk_corr_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 nbins_trg, pTmin_trg_[ipt], pTmax_trg_[ipt]);
      hPhiTrk_corr_trg_[ipt] = fTrkCorrTrgHist.make<TH1F>(Form("hPhitrk_corr_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 640, -3.2, 3.2);
      hMultTrk_corr_trg_[ipt] = fTrkCorrTrgHist.make<TH1F>(Form("hMulttrk_corr_trg_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_trg_[ipt], pTmax_trg_[ipt]),
                                                 1000, 0, 50000);
   }
   */
   TFileDirectory fTrkAssHist  = fs->mkdir("AssTracksRaw");
   /*
   hEtaTrk_ass_.resize(pTmin_ass_.size());
   hPtTrk_ass_.resize(pTmin_ass_.size());
   hPhiTrk_ass_.resize(pTmin_ass_.size());
   hMultTrk_ass_.resize(pTmin_ass_.size());
   for(unsigned int ipt = 0; ipt < pTmin_ass_.size(); ++ipt)
   {
      //bin size for some plots
      int nbins_ass = static_cast<int>((pTmax_ass_[ipt]-pTmin_ass_[ipt])/0.01);

      hEtaTrk_ass_[ipt] = fTrkAssHist.make<TH1F>(Form("hEtatrk_ass_%d",ipt),
                                                 Form("%1.1f<$p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 300, -3., 3.);
      hPtTrk_ass_[ipt]  = fTrkAssHist.make<TH1F>(Form("hPttrk_ass_%d",ipt),
                                                 Form("%1.1f<$p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 nbins_ass, pTmin_ass_[ipt], pTmax_ass_[ipt]);
      hPhiTrk_ass_[ipt] = fTrkAssHist.make<TH1F>(Form("hPhitrk_ass_%d",ipt),
                                                 Form("%1.1f<$p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 640, -3.2, 3.2);
      hMultTrk_ass_[ipt] = fTrkAssHist.make<TH1I>(Form("hMulttrk_ass_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 1000, 0, 50000);
   }*/
   TFileDirectory fTrkCorrAssHist  = fs->mkdir("AssTracksCorr");
   /*
   hEtaTrk_corr_ass_.resize(pTmin_ass_.size());
   hPtTrk_corr_ass_.resize(pTmin_ass_.size());
   hPhiTrk_corr_ass_.resize(pTmin_ass_.size());
   hMultTrk_corr_ass_.resize(pTmin_ass_.size());
   for(unsigned int ipt = 0; ipt < pTmin_ass_.size(); ++ipt)
   {
      //bin size for some plots
      int nbins_ass = static_cast<int>((pTmax_ass_[ipt]-pTmin_ass_[ipt])/0.01);

      hEtaTrk_corr_ass_[ipt] = fTrkCorrAssHist.make<TH1F>(Form("hEtatrk_corr_ass_%d",ipt),
                                                 Form("%1.1f<$p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 300, -3., 3.);
      hPtTrk_corr_ass_[ipt]  = fTrkCorrAssHist.make<TH1F>(Form("hPttrk_corr_ass_%d",ipt),
                                                 Form("%1.1f<$p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 nbins_ass, pTmin_ass_[ipt], pTmax_ass_[ipt]);
      hPhiTrk_corr_ass_[ipt] = fTrkCorrAssHist.make<TH1F>(Form("hPhitrk_corr_ass_%d",ipt),
                                                 Form("%1.1f<$p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 640, -3.2, 3.2);
      hMultTrk_corr_ass_[ipt] = fTrkCorrAssHist.make<TH1F>(Form("hMulttrk_corr_ass_%d",ipt),
                                                 Form("%1.1f<p_{T}<%1.1f GeV/c", pTmin_ass_[ipt], pTmax_ass_[ipt]),
                                                 1000, 0, 50000);
   }*/

   TFileDirectory fCTowHist = fs->mkdir("CaloTowers");
   /*
   hEtaCTow_ = fCTowHist.make<TH1F>("hEtatow", "", 120, -6.,   6.);
   hEtCTow_  = fCTowHist.make<TH1F>("hEttow",  "", 100,  0.,  10.);*/
   //hPhiCTow_ = fCTowHist.make<TH1F>("hPhitow", "", 640, -3.2,  3.2);

   double etaW = (etamax_trg_ - etamin_ass_ - etamin_trg_ + etamax_ass_) / nEtaBins_;
   //double etaW = 0.0;
   double phiW = 2.0*(TMath::Pi())/nPhiBins_;
   //double phiW = 0.0;

   TFileDirectory fSignalHist      = fs->mkdir("Signal");
   hSignal_.resize(pTmin_trg_.size());
   //hSignalPP_.resize(pTmin_trg_.size());
   //hSignalMM_.resize(pTmin_trg_.size());
   //hSignalPM_.resize(pTmin_trg_.size());
   //hSignalMP_.resize(pTmin_trg_.size());
   for(unsigned int itrg = 0; itrg < pTmin_trg_.size(); itrg++)
   {
       hSignal_[itrg].resize(pTmin_ass_.size());
       //hSignalPP_[itrg].resize(pTmin_ass_.size());
       //hSignalMM_[itrg].resize(pTmin_ass_.size());
       //hSignalPM_[itrg].resize(pTmin_ass_.size());
       //hSignalMP_[itrg].resize(pTmin_ass_.size());
       for(unsigned int jass = 0; jass < pTmin_ass_.size(); jass++)
       {
          if( itrg < jass ) continue;
          hSignal_[itrg][jass] = fSignalHist.make<TH2D>(Form("signal_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          /*
          hSignalPP_[itrg][jass] = fSignalHist.make<TH2D>(Form("signal_pp_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hSignalMM_[itrg][jass] = fSignalHist.make<TH2D>(Form("signal_mm_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hSignalMP_[itrg][jass] = fSignalHist.make<TH2D>(Form("signal_mp_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hSignalPM_[itrg][jass] = fSignalHist.make<TH2D>(Form("signal_pm_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);*/
       }
   }
   TFileDirectory fBackgroundHist  = fs->mkdir("Background");
   //hDeltaZvtx_ = fBackgroundHist.make<TH1D>("deltazvtx",";#Delta z_{vtx}",200,-10.0,10.0);
   hBackground_.resize(pTmin_trg_.size());
   //hBackgroundPP_.resize(pTmin_trg_.size());
   //hBackgroundMM_.resize(pTmin_trg_.size());
   //hBackgroundPM_.resize(pTmin_trg_.size());
   //hBackgroundMP_.resize(pTmin_trg_.size());
   for(unsigned int itrg = 0; itrg < pTmin_trg_.size(); itrg++)
   {
       hBackground_[itrg].resize(pTmin_ass_.size());
       //hBackgroundPP_[itrg].resize(pTmin_ass_.size());
       //hBackgroundMM_[itrg].resize(pTmin_ass_.size());
       //hBackgroundPM_[itrg].resize(pTmin_ass_.size());
       //hBackgroundMP_[itrg].resize(pTmin_ass_.size());
       for(unsigned int jass = 0; jass < pTmin_ass_.size(); jass++)
       {
          if( itrg < jass ) continue;
          hBackground_[itrg][jass] = fBackgroundHist.make<TH2D>(Form("background_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          /*
          hBackgroundPP_[itrg][jass] = fBackgroundHist.make<TH2D>(Form("background_pp_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hBackgroundMM_[itrg][jass] = fBackgroundHist.make<TH2D>(Form("background_mm_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hBackgroundPM_[itrg][jass] = fBackgroundHist.make<TH2D>(Form("background_pm_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hBackgroundMP_[itrg][jass] = fBackgroundHist.make<TH2D>(Form("background_mp_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
          pTmin_trg_[itrg], pTmax_trg_[itrg],
          pTmin_trg_[jass], pTmax_trg_[jass]),
          nEtaBins_ + 1,
          etamin_trg_ - etamax_ass_ - etaW/2.,
          etamax_trg_ - etamin_ass_ + etaW/2.,
          nPhiBins_ - 1,
          -( TMath::Pi() - phiW ) / 2.0,
           ( TMath::Pi() * 3.0 - phiW) / 2.0);*/
       }
   }
   TFileDirectory fCorrelationHist = fs->mkdir("Correlation");
   hCorrelation_.resize(pTmin_trg_.size());
   //hCorrelationPP_.resize(pTmin_trg_.size());
   //hCorrelationMM_.resize(pTmin_trg_.size());
   //hCorrelationPM_.resize(pTmin_trg_.size());
   //hCorrelationMP_.resize(pTmin_trg_.size());
   for(unsigned int itrg = 0; itrg < pTmin_trg_.size(); itrg++)
     {
       hCorrelation_[itrg].resize(pTmin_ass_.size());
       //hCorrelationPP_[itrg].resize(pTmin_ass_.size());
       //hCorrelationMM_[itrg].resize(pTmin_ass_.size());
       //hCorrelationPM_[itrg].resize(pTmin_ass_.size());
       //hCorrelationMP_[itrg].resize(pTmin_ass_.size());
       for(unsigned int jass = 0; jass < pTmin_ass_.size(); jass++)
	 {
	   if( itrg < jass ) continue;
	   
	   hCorrelation_[itrg][jass] = 
	     fCorrelationHist.make<TH2D>(Form("corr_trg%d_ass%d", itrg, jass),
					 Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
					      pTmin_trg_[itrg], pTmax_trg_[itrg],
					      pTmin_trg_[jass], pTmax_trg_[jass]),
					 nEtaBins_ + 1,
					 etamin_trg_ - etamax_ass_ - etaW/2.,
					 etamax_trg_ - etamin_ass_ + etaW/2.,
					 nPhiBins_ - 1,
					 -( TMath::Pi() - phiW ) / 2.0,
					 ( TMath::Pi() * 3.0 - phiW) / 2.0);
	   /*
	     hCorrelationPP_[itrg][jass] = fCorrelationHist.make<TH2D>(Form("corr_pp_trg%d_ass%d", itrg, jass),
	     Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
	     pTmin_trg_[itrg], pTmax_trg_[itrg],
	     pTmin_trg_[jass], pTmax_trg_[jass]),
	     nEtaBins_ + 1,
	     etamin_trg_ - etamax_ass_ - etaW/2.,
	     etamax_trg_ - etamin_ass_ + etaW/2.,
	     nPhiBins_ - 1,
	     -( TMath::Pi() - phiW ) / 2.0,
	     ( TMath::Pi() * 3.0 - phiW) / 2.0);
	     hCorrelationMM_[itrg][jass] = fCorrelationHist.make<TH2D>(Form("corr_mm_trg%d_ass%d", itrg, jass),
	     Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
	     pTmin_trg_[itrg], pTmax_trg_[itrg],
               pTmin_trg_[jass], pTmax_trg_[jass]),
               nEtaBins_ + 1,
               etamin_trg_ - etamax_ass_ - etaW/2.,
               etamax_trg_ - etamin_ass_ + etaW/2.,
               nPhiBins_ - 1,
               -( TMath::Pi() - phiW ) / 2.0,
                ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hCorrelationPM_[itrg][jass] = fCorrelationHist.make<TH2D>(Form("corr_pm_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
               pTmin_trg_[itrg], pTmax_trg_[itrg],
               pTmin_trg_[jass], pTmax_trg_[jass]),
               nEtaBins_ + 1,
               etamin_trg_ - etamax_ass_ - etaW/2.,
               etamax_trg_ - etamin_ass_ + etaW/2.,
               nPhiBins_ - 1,
               -( TMath::Pi() - phiW ) / 2.0,
                ( TMath::Pi() * 3.0 - phiW) / 2.0);
          hCorrelationMP_[itrg][jass] = fCorrelationHist.make<TH2D>(Form("corr_mp_trg%d_ass%d", itrg, jass),
          Form("%1.1f<p_{T}^{trg}<%1.1f GeV/c, %1.1f<p_{T}^{ass}<%1.1f GeV/c;#Delta#eta;#Delta#phi",
               pTmin_trg_[itrg], pTmax_trg_[itrg],
               pTmin_trg_[jass], pTmax_trg_[jass]),
               nEtaBins_ + 1,
               etamin_trg_ - etamax_ass_ - etaW/2.,
               etamax_trg_ - etamin_ass_ + etaW/2.,
               nPhiBins_ - 1,
               -( TMath::Pi() - phiW ) / 2.0,
                ( TMath::Pi() * 3.0 - phiW) / 2.0);*/
	 }
     }
}

ChargeDepAndPtCorr::~ChargeDepAndPtCorr()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
   delete evt_;
}


//
// member functions
//

// ------------ method called for each event  ------------
void
ChargeDepAndPtCorr::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   // ----- centrality selection -----
   // Get calo centrality collection by token
   edm::Handle< reco::Centrality > centrality;
   iEvent.getByToken(centralityTags_, centrality);
   // Get calo centrality bin by token
   edm::Handle< int > cbin;
   iEvent.getByToken(centralityBinTags_,cbin);
   int centBin = *cbin;
   if(centBin < 0)
   {
       edm::LogWarning ("Invalid value") <<"Invalid centrality value";
   }

   // ----- Vertex selection -----
   // Get vertex collection by token
   edm::Handle< reco::VertexCollection > vertices;
   iEvent.getByToken(vtxTags_, vertices);
   if( !vertices->size() )
   {
      edm::LogWarning ("Missing Collection") <<"Invalid or empty vertex collection!";
      return;
   }

   nVtx_ = 0; // N valid vertex in collection
   xBestVtx_   = -999.; //Best X vtx coordinates
   yBestVtx_   = -999.; //Best Y vtx coordinates
   zBestVtx_   = -999.; //Best Z vtx coordinates
   rhoBestVtx_ = -999.; //Best transverse vtx coordinates
   xBestVtxError_ = -999.; //Best X vtx error
   yBestVtxError_ = -999.; //Best Y vtx error
   zBestVtxError_ = -999.; //Best Z vtx error

   // Loop over vertices
   LoopVertices(iEvent, iSetup);
   if( zBestVtx_ < zminVtx_ || zBestVtx_ > zmaxVtx_ ) return; //cut on vertex Z position
   if( rhoBestVtx_ > rhomaxVtx_ ) return; //cut on vertex XY position

   // ----- Ntrk offline selection -----
   double noff = LoopNoff(iEvent, iSetup); //compute ntrk^offline

   // ----- Define event classification (either centrality or Ntrk^off) -----
   int evtclass = -1;
   switch(evtclassifier_)
   {
      case 0:
         evtclass = centBin/2;
         if(evtclass < centmin_ || evtclass >= centmax_)
           return;
         break;
      case 1:
         evtclass = noff;
         if(evtclass < noffmin_ || evtclass >= noffmax_)
           return;
      default:
         evtclass = -1;
   }
   //hCent_->Fill(centBin);
   //hNoff_->Fill(noff);

   nTrkTot_trg_      = 0;
   nTrkTot_corr_trg_ = 0;
   nTrkTot_ass_      = 0;
   nTrkTot_corr_ass_ = 0;
   nTrk_trg_      = std::vector<int>(pTmin_trg_.size(), 0.);
   nTrk_corr_trg_ = std::vector<double>(pTmin_trg_.size(), 0.);
   nTrk_ass_      = std::vector<int>(pTmin_ass_.size(), 0.);
   nTrk_corr_ass_ = std::vector<double>(pTmin_ass_.size(), 0.);


   // ----- Track selection -----
   LoopTracks(iEvent, iSetup, true,  evtclass); //trigger tracks
   LoopTracks(iEvent, iSetup, false, evtclass); //associated tracks

   // ----- Calotower selection -----
   LoopCaloTower(iEvent, iSetup);

   // ----- Fill and push evt containers -----
   evt_->run   = iEvent.id().run();
   evt_->event = iEvent.id().event();
   evt_->zvtx  = zBestVtx_;

   evtVec_.push_back(*evt_);

   // ----- Reset evt container -----
   evt_->reset();
}


// ------------ method called once each job just before starting event loop  ------------
void
ChargeDepAndPtCorr::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void
ChargeDepAndPtCorr::endJob()
{
   std::cout<< "Start sorting the events!" << std::endl;
   std::sort(evtVec_.begin(),evtVec_.end());
   std::cout<< "Finish sorting the events!" << std::endl;

   std::cout<< "Total of " << evtVec_.size() << " events are selected!" << std::endl;

   //int long Nmix=0; //Claude
   //int long NTrue=0; //Claude

   std::cout<< "Start running correlation analysis!" << std::endl;
   for( unsigned int i = 0; i < evtVec_.size(); i++ )
   {
      if( i % 100 == 0 ) std::cout << "Processing " << i << "th event" << std::endl;
      //std::cout<<"Signal event :  " << i << std::endl<<std::endl<<std::endl;
      FillHistsSignal( i );
      //NTrue++;
      hCounts_->AddBinContent(1);
      unsigned int mixstart = i - bkgFactor_/2;
      unsigned int mixend   = i + bkgFactor_/2 + 1;

      //unsigned int mixstart = i - bkgFactor_;
      //unsigned int mixend   = i + bkgFactor_ + 1;

      //int Nmix=0;

      if(i < bkgFactor_)
      {
         mixstart = 0;
         mixend   = 2*bkgFactor_ + 1;
      }
      else if(i > evtVec_.size() - bkgFactor_ - 1)
      {
         mixstart = evtVec_.size() - 2*bkgFactor_ - 1;
         mixend   = evtVec_.size();
      }

      if( mixend > evtVec_.size() )
         mixend = evtVec_.size();

      for( unsigned int j = mixstart; j < mixend; j++ )
      {
          if(i == j) continue;
          //if(Nmix >= 10) continue;
	  //if(Nmix >= 5) continue;
	  
          //std::cout << i << " : " << j << std::endl;
          double deltazvtx = evtVec_[i].zvtx-evtVec_[j].zvtx;
          if(fabs(deltazvtx) > 2.0) continue;
	  //if(fabs(deltazvtx) > 0.5) continue;

          FillHistsBackground( i, j );
	  //Nmix++;
	  hCounts_->AddBinContent(2);
      }
    }
   

    std::cout<< "Finish running correlation analysis!" << std::endl;

    NormalizeHists();
    //std::cout<< "Finish normalizing the histograms!" << std::endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
ChargeDepAndPtCorr::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//=========================================================================================

void
ChargeDepAndPtCorr::LoopVertices(const edm::Event& iEvent,
                                 const edm::EventSetup& iSetup)
{

   edm::Handle< reco::VertexCollection > vertices;
   iEvent.getByToken(vtxTags_, vertices);
   if(!vertices->size())
   {
      std::cout<<"Invalid or empty vertex collection!"<<std::endl;
      return;
   }

   reco::VertexCollection recoVertices = *vertices;

   if(selectVtxByMult_)
   {
       std::sort( recoVertices.begin(),
                  recoVertices.end(),
                  [](const reco::Vertex &a, const reco::Vertex &b)
                  {
                     if ( a.tracksSize() == b.tracksSize() ) return a.chi2() < b.chi2();
                          return a.tracksSize() > b.tracksSize();
                  }
                );
   }

   for( reco::VertexCollection::const_iterator itVtx = recoVertices.begin();
        itVtx != recoVertices.end();
        ++itVtx )
   {
        // Drop fake vertex and vertex with less than 2 tracks attached to it
        if( !itVtx->isFake() && itVtx->tracksSize() >= nTrkAssoToVtx_ )
        {
            // x,y,z vertex position
            double xVtx = itVtx->x();
            double yVtx = itVtx->y();
            double zVtx = itVtx->z();
            // x,y,z vertex position error
            double xVtxError = itVtx->xError();
            double yVtxError = itVtx->yError();
            double zVtxError = itVtx->zError();
            // Radial vertex position
            double rho = sqrt(xVtx*xVtx + yVtx*yVtx);
            // Increase N valid vertex in the collection
            ++nVtx_;

            //Get the first vertex as the best one (greatest sum p_{T}^{2})
            if( itVtx == recoVertices.begin() )
            {
                xBestVtx_ = xVtx;
                yBestVtx_ = yVtx;
                zBestVtx_ = zVtx;
                xBestVtxError_ = xVtxError;
                yBestVtxError_ = yVtxError;
                zBestVtxError_ = zVtxError;

                rhoBestVtx_ = rho;
            }
        }
   }
   // Fill vtx histograms
   //hXBestVtx_  ->Fill(xBestVtx_);
   //hYBestVtx_  ->Fill(yBestVtx_);
   //hRhoBestVtx_->Fill(rhoBestVtx_);
   //hZBestVtx_  ->Fill(zBestVtx_);
   //hNVtx_      ->Fill(nVtx_);
}

//=========================================================================================

double
ChargeDepAndPtCorr::LoopNoff(const edm::Event& iEvent,
                             const edm::EventSetup& iSetup)
{
   double noff = 0.;

   // Get track collection by token
   edm::Handle< reco::TrackCollection > tracks;
   iEvent.getByToken(trackTags_, tracks);
   if( !tracks->size() )
   {
       edm::LogWarning ("Missing Collection") <<"Invalid or empty track collection!";
       return -999.;
   }

   // Loop over tracks
   for( reco::TrackCollection::const_iterator itTrk = tracks->begin();
        itTrk != tracks->end();
        ++itTrk )
   {
       // Select tracks based on proximity to best vertex
       math::XYZPoint bestvtx(xBestVtx_,yBestVtx_,zBestVtx_);
       double dzvtx    = itTrk->dz(bestvtx);
       double dxyvtx   = itTrk->dxy(bestvtx);
       double dzerror  = sqrt(itTrk->dzError()*itTrk->dzError() + zBestVtxError_*zBestVtxError_);
       double dxyerror = sqrt(itTrk->d0Error()*itTrk->d0Error() + xBestVtxError_*yBestVtxError_);
       double pterror  = itTrk->ptError();
       // Get eta, pt and charge of the track
       double eta    = itTrk->eta();
       double pt     = itTrk->pt();
       int charge = itTrk->charge();


       // Select track based on quality
       if( !itTrk->quality(reco::TrackBase::highPurity) ) continue;
       if( fabs(dzvtx / dzerror)   > 2.0 ) continue;
       if( fabs(dxyvtx / dxyerror) > 2.0 ) continue;
       if( fabs(pterror) / pt      > 0.05 ) continue;
       if( pt <= 0.3 ) continue;
       if( eta < -2.0 || eta > 2.0 ) continue;
       if( charge == 0 ) continue;

       ++noff;
   }

   return noff;
}

//=========================================================================================

void
ChargeDepAndPtCorr::LoopTracks(const edm::Event& iEvent, const edm::EventSetup& iSetup,
                               bool istrg, int evtclass)
{
   // Get track collection by token
   edm::Handle< reco::TrackCollection > tracks;
   iEvent.getByToken(trackTags_, tracks);
   if( !tracks->size() )
   {
       edm::LogWarning ("Missing Collection") <<"Invalid or empty track collection!";
       return;
   }

   // Loop over tracks
   for( reco::TrackCollection::const_iterator itTrk = tracks->begin();
        itTrk != tracks->end();
        ++itTrk )
   {
       // Select tracks based on proximity to best vertex
       math::XYZPoint bestvtx(xBestVtx_,yBestVtx_,zBestVtx_);
       double dzvtx    = itTrk->dz(bestvtx);
       double dxyvtx   = itTrk->dxy(bestvtx);
       double dzerror  = sqrt(itTrk->dzError()*itTrk->dzError() + zBestVtxError_*zBestVtxError_);
       double dxyerror = sqrt(itTrk->d0Error()*itTrk->d0Error() + xBestVtxError_*yBestVtxError_);
       double pterror  = itTrk->ptError();
       // Get eta, pt, phi and charge of the track
       double eta    = itTrk->eta();
       double pt     = itTrk->pt();
       double phi    = itTrk->phi();
       int charge = itTrk->charge();
       // HI specific cuts
       double chi2n   = itTrk->normalizedChi2();
       double nlayers = itTrk->hitPattern().trackerLayersWithMeasurement();
       chi2n = chi2n/nlayers;
       int nHits = itTrk->numberOfValidHits();
       int algo  = itTrk->originalAlgo();

       // Select track based on quality
       if( !itTrk->quality(reco::TrackBase::highPurity) ) continue;
       if( pt < 0.0001 ) continue;
       if( charge == 0 ) continue;

       if(isHI_ && isPix_ )
       {
          bool goodpixtrk = false; // Pixel tracks specific cuts for
          if( pt <= pTmax_pix_         &&
              nHits >= nhitsmin_pix_   &&
              nHits <= nhitsmax_pix_   &&
              chi2n <= chi2nmax_pix_   &&
              fabs(dzvtx / dzerror) <  dzdzerror_pix_ )
              goodpixtrk = true;

          if( !goodpixtrk )
          {
             if( nHits < nhitsmin_ ) continue;
             if( pt > pTmax_pix_ )
             {
                 if( std::find(algo_.begin(), algo_.end(), algo) == algo_.end() )
                    continue;
             }
             if( chi2n > chi2nmax_ ) continue;
             if( fabs(pterror) / pt      > pTerrorpT_ ) continue;
             if( fabs(dzvtx / dzerror)   > dzdzerror_ ) continue;
             if( fabs(dxyvtx / dxyerror) > d0dz0rror_ ) continue;
          }
       }
       else if(isHI_)
       {
           if( nHits < nhitsmin_ ) continue;
           if( pt > pTmax_pix_ )
           {
               if( std::find(algo_.begin(), algo_.end(), algo) == algo_.end() )
                  continue;
           }
           if( chi2n > chi2nmax_ ) continue;
           if( fabs(pterror) / pt      > pTerrorpT_ ) continue;
           if( fabs(dzvtx / dzerror)   > dzdzerror_ ) continue;
           if( fabs(dxyvtx / dxyerror) > d0dz0rror_ ) continue;
       }
       else
       {
           if( fabs(pterror) / pt      > pTerrorpT_ ) continue;
           if( fabs(dzvtx / dzerror)   > dzdzerror_ ) continue;
           if( fabs(dxyvtx / dxyerror) > d0dz0rror_ ) continue;
       }

       // Track selection for analysis
       int index = GetpTbin(pt, istrg);
       if(index == -1) continue;

        if( istrg && ( eta < etamin_trg_ || eta > etamax_trg_ )) continue;
        else if( eta < etamin_ass_ || eta > etamax_ass_ )        continue;

       double eff = 1.0;
       if(cweight_) eff = GetEffWeight(eta, pt, evtclass);
       //if(cweight_) eff = GetEffWeight(trigger, evtclass);
       AssignpTbins(pt, eta, phi, charge, eff, istrg, index);

       // hPhidist_->Fill(phi);
   }

   // Fill trk histograms
   if(istrg)
   {
      hMult_trg_     ->Fill(nTrkTot_trg_);
      //hMult_corr_trg_->Fill(nTrkTot_corr_trg_);
      for(unsigned int itrg = 0; itrg < pTmin_trg_.size(); ++itrg)
      {
         //hMultTrk_trg_[itrg]     ->Fill(nTrk_trg_[itrg]);
         //hMultTrk_corr_trg_[itrg]->Fill(nTrk_corr_trg_[itrg]);
         //std::cout << nTrk_corr_trg_[itrg] <<std::endl;
         (evt_->nMultCorrVect_trg)[itrg] = nTrk_corr_trg_[itrg];
      }
   }
   else
   {
      hMult_ass_     ->Fill(nTrkTot_ass_);
      //hMult_corr_ass_->Fill(nTrkTot_corr_ass_);
      for(unsigned int iass = 0; iass < pTmin_ass_.size(); ++iass)
      {
         //hMultTrk_ass_[iass]     ->Fill(nTrk_ass_[iass]);
         //hMultTrk_corr_ass_[iass]->Fill(nTrk_corr_ass_[iass]);
         (evt_->nMultCorrVect_ass)[iass] = nTrk_corr_ass_[iass];
      }
   }
}

//=========================================================================================

void
ChargeDepAndPtCorr::LoopCaloTower(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   // Get calo tower collection by token
   edm::Handle< CaloTowerCollection > calotowers;
   iEvent.getByToken(caloTowersTags_, calotowers);
   if( !calotowers->size() )
   {
       edm::LogWarning ("Missing Collection") <<"Invalid or empty caloTower collection!";
       return;
   }
   // Loop over caloTowers
   for( CaloTowerCollection::const_iterator itCTow = calotowers->begin();
        itCTow != calotowers->end();
        ++itCTow )
   {
       /*
       // Get eta, pt and phi of the calo tower
       double eta  = itCTow->eta();*/
       double et   = itCTow->et();
       //double phi  = itCTow->phi();

       // Select calo tower based on quality
       if( et < 0.01 ) continue;
        /*
       // Fill trk histograms
       hEtaCTow_->Fill(eta);
       hEtCTow_ ->Fill(et);
       hPhiCTow_->Fill(phi);*/
   }
}

//=========================================================================================

double
ChargeDepAndPtCorr::GetEffWeight(double eta, double pt, int evtclass)
{
   double effweight = 1.0;
   if(evtclass == -1)
   {
      effweight = heff_[0]->GetBinContent(heff_[0]->FindBin(eta,pt));
   }
   else
   {
      int centIdx = 0;
      for(int icent = 0; icent < static_cast<int>(effCorrBinMin_.size()); ++icent)
      {
          if(evtclass >= effCorrBinMin_[icent] && evtclass < effCorrBinMax_[icent])
          {
             centIdx = icent;
             continue;
          }
      }
      //effweight = heff_[centIdx]->GetBinContent(heff_[centIdx]->FindBin(eta,pt));
      effweight = heff_[centIdx]->GetBinContent(heff_[centIdx]->GetXaxis()->FindBin(eta), heff_[centIdx]->GetXaxis()->FindBin(pt));
   }
   return effweight;
   }

//=========================================================================================
/*double
ChargeDepAndPtCorr::GetEffWeight(const TVector3 a, int evtclass)
{
   double effweight = 1.0;


   for(int i_cf=0; i_cf < static_cast<int>(effCorrBinMin_.size()); i_cf++) if(evtclass >= effCorrBinMin_[i_cf]*2 && evtclass < effCorrBinMax_[i_cf]*2) {
       effweight = heff_[i_cf]->GetBinContent(
					      heff_[i_cf]->GetXaxis()->FindBin(a.Eta()),
					      heff_[i_cf]->GetYaxis()->FindBin(a.Pt())
					      );
     }
   return effweight;
   }
*/
 //----------------------------------------------------
int
ChargeDepAndPtCorr::GetpTbin(double pt, bool istrg)
{
    int idx = -1;
    if(istrg)
    {
       for(unsigned int trgidx = 0; trgidx < pTmin_trg_.size(); ++trgidx)
       {
          if(pt >= pTmin_trg_[trgidx] && pt < pTmax_trg_[trgidx])
            idx = trgidx;
       }
    }
    else
    {
       for(unsigned int assidx = 0; assidx < pTmin_ass_.size(); ++assidx)
       {
          if(pt >= pTmin_ass_[assidx] && pt < pTmax_ass_[assidx])
            idx = assidx;
       }
    }

    return idx;
}
//=========================================================================================
void
ChargeDepAndPtCorr::AssignpTbins(double pt,  double eta,
                                 double phi, int charge,
                                 double eff, bool istrg,
                                 int idx)
{
  TLorentzVector pvector;
  pvector.SetPtEtaPhiM(pt, eta, phi, 0.140);
  //TVector3 pvector; 
  //pvector.SetPtEtaPhiM(pt, eta, phi);

    if(istrg)
    {
      if(charge < 0)
       {
           //hPtTrk_trg_[idx] ->Fill(pt);
           //hEtaTrk_trg_[idx]->Fill(eta);
           //hPhiTrk_trg_[idx]->Fill(phi);
           //hPtTrk_corr_trg_[idx] ->Fill(pt, 1.0/eff);
           //hEtaTrk_corr_trg_[idx]->Fill(eta,1.0/eff);
           //hPhiTrk_corr_trg_[idx]->Fill(phi,1.0/eff);
           (evt_->pVect_trg[idx]).push_back(pvector);
           (evt_->chgVect_trg[idx]).push_back(charge);
           (evt_->effVect_trg[idx]).push_back(eff);
           nTrkTot_trg_++;
           nTrkTot_corr_trg_ += 1.0/eff;
           nTrk_trg_[idx]++;
           nTrk_corr_trg_[idx] += 1.0/eff;

       }
    }
    else
    {
      if(charge > 0)
      {

           //hPtTrk_ass_[idx] ->Fill(pt);
           //hEtaTrk_ass_[idx]->Fill(eta);
           //hPhiTrk_ass_[idx]->Fill(phi);
           //hPtTrk_corr_ass_[idx] ->Fill(pt, 1.0/eff);
           //hEtaTrk_corr_ass_[idx]->Fill(eta,1.0/eff);
           //hPhiTrk_corr_ass_[idx]->Fill(phi,1.0/eff);
           (evt_->pVect_ass[idx]).push_back(pvector);
           (evt_->chgVect_ass[idx]).push_back(charge);
           (evt_->effVect_ass[idx]).push_back(eff);
           nTrkTot_ass_++;
           nTrkTot_corr_ass_ += 1.0/eff;
           nTrk_ass_[idx]++;
           nTrk_corr_ass_[idx] += 1.0/eff;
	   }
    }
}

double
ChargeDepAndPtCorr::GetDeltaEta(double eta_trg, double eta_ass)
{
  double deltaEta = eta_ass - eta_trg;
  //double deltaEta = eta_trg - eta_ass;
   return deltaEta;
}

double
ChargeDepAndPtCorr::GetDeltaPhi(double phi_trg, double phi_ass)
{
  //double deltaPhi = phi_trg - phi_ass;
  double deltaPhi = phi_ass - phi_trg;

   if(deltaPhi > 1.5*TMath::Pi())
    deltaPhi = deltaPhi - 2.0*TMath::Pi();

   else if(deltaPhi < -1.0*TMath::Pi() / 2.0)
   deltaPhi = deltaPhi + 2.0*TMath::Pi();

   return deltaPhi;

}

double  ChargeDepAndPtCorr::GetDPhiStar(double phi1, double pt1, int charge1, double phi2, double pt2, int charge2, double radius, double bSign)
{
  // calculates dphistar
  double dphistar = phi1 - phi2 - charge1*bSign*TMath::ASin(0.003*radius / pt1) + charge2*bSign*TMath::ASin(0.003*radius / pt2);

  static const double kPi = TMath::Pi();

  /*
  if (dphistar > 1.5 * kPi)
    dphistar = dphistar - 2.0*kPi;
  else if (dphistar < -0.5 * kPi)
    dphistar = dphistar + 2.0*kPi;*/

  if (dphistar > kPi)
    dphistar = kPi * 2.0 - dphistar;
  if (dphistar < -kPi)
    dphistar = -kPi * 2.0 - dphistar;
  if (dphistar > kPi) // might look funny but is needed
    dphistar = kPi * 2.0 - dphistar;

  return dphistar;
}

void
ChargeDepAndPtCorr::FillHistsSignal(int ievt)
{
  for( unsigned int itrg = 0; itrg < pTmin_trg_.size(); itrg++ )
  {
    for( unsigned int jass = 0; jass < pTmin_ass_.size(); jass++ )
    {
      if( itrg < jass ) continue;

      unsigned int ntrgsize = evtVec_[ievt].pVect_trg[itrg].size();
      unsigned int nasssize = evtVec_[ievt].pVect_ass[jass].size();
      //double nMult_corr_trg = evtVec_[ievt].nMultCorrVect_trg[itrg];

      for( unsigned int ntrg = 0; ntrg < ntrgsize; ntrg++ )
      {
	TLorentzVector pvector_trg = (evtVec_[ievt].pVect_trg[itrg])[ntrg];
	double effweight_trg = (evtVec_[ievt].effVect_trg[itrg])[ntrg];
	int chg_trg = (evtVec_[ievt].chgVect_trg[itrg])[ntrg];
	double eta_trg = pvector_trg.Eta();
	double phi_trg = pvector_trg.Phi();
	double pt_trg  = pvector_trg.Pt();
	//std::cout<<"Effweight trigger :   " << effweight_trg << std::endl;
	
	for( unsigned int nass = 0; nass < nasssize; nass++ )
          {
	    TLorentzVector pvector_ass = (evtVec_[ievt].pVect_ass[jass])[nass];
	    double effweight_ass = (evtVec_[ievt].effVect_ass[jass])[nass];
	    int chg_ass = (evtVec_[ievt].chgVect_ass[jass])[nass];
	    double eta_ass = pvector_ass.Eta();
	    double phi_ass = pvector_ass.Phi();
	    double pt_ass  = pvector_ass.Pt();
	    
	    double deltaPhi = GetDeltaPhi(phi_trg, phi_ass);
	    double deltaPhi2 = GetDeltaPhi(phi_ass, phi_trg);
	    double deltaEta = GetDeltaEta(eta_trg, eta_ass);
	    
	    //Skip the loop when trg, ass particles are the same
	    if( deltaEta == 0.0  &&  deltaPhi == 0.0 && deltaPhi2 ==0.0 && pt_trg==pt_ass) continue;
	    //if (pt_trg  < pt_ass) continue;
	    
	    //Total weight
	    double effweight = effweight_trg * effweight_ass;
	    //double effweight = 1.0;
	    
	    //Fill and symmetrize the distribution
	    if(chg_trg < 0 && chg_ass > 0)
      	      {
		
		hSignal_[itrg][jass]->Fill( fabs(deltaEta),
					    deltaPhi,
					    1.0/4.0/effweight );
		hSignal_[itrg][jass]->Fill(-fabs(deltaEta),
					   deltaPhi,
					   1.0/4.0/effweight );
		hSignal_[itrg][jass]->Fill( fabs(deltaEta),
					    deltaPhi2,
					    1.0/4.0/effweight );
		hSignal_[itrg][jass]->Fill(-fabs(deltaEta),
					   deltaPhi2,
					   1.0/4.0/effweight );
		
		//hSignal_[itrg][jass]->Fill( deltaEta, deltaPhi, 1.0/effweight );
      	      }
	  }
      }
    }
  }
}

void
ChargeDepAndPtCorr::FillHistsBackground(int ievt_trg, int jevt_ass)
{
  if( evtVec_[ievt_trg].run ==  evtVec_[jevt_ass].run &&
      evtVec_[ievt_trg].event == evtVec_[jevt_ass].event )
  {
      std::cout << "Event are the same. Skipping it" << std::endl;
      return;
  }

  for( unsigned int itrg = 0; itrg < pTmin_trg_.size(); itrg++ )
  {
      for( unsigned int jass = 0; jass < pTmin_ass_.size(); jass++ )
      {
	if( itrg < jass ) continue;

          unsigned int ntrgsize = evtVec_[ievt_trg].pVect_trg[itrg].size();
          unsigned int nasssize = evtVec_[jevt_ass].pVect_ass[jass].size();
          //double nMult_corr_trg = evtVec_[ievt_trg].nMultCorrVect_trg[itrg];

          for( unsigned int ntrg = 0; ntrg < ntrgsize; ntrg++ )
          {
	    TLorentzVector pvector_trg = (evtVec_[ievt_trg].pVect_trg[itrg])[ntrg];
	    double effweight_trg = (evtVec_[ievt_trg].effVect_trg[itrg])[ntrg];
	    int chg_trg = (evtVec_[ievt_trg].chgVect_trg[itrg])[ntrg];
	    double eta_trg = pvector_trg.Eta();
	    double phi_trg = pvector_trg.Phi();
	    double pt_trg  = pvector_trg.Pt();
	    //std::cout<<"Effweight trigger    " << effweight_trg<<std::endl;
	    for( unsigned int nass = 0; nass < nasssize; nass++ )
              {
		TLorentzVector pvector_ass = (evtVec_[jevt_ass].pVect_ass[jass])[nass];
		double effweight_ass = (evtVec_[jevt_ass].effVect_ass[jass])[nass];
		int chg_ass = (evtVec_[jevt_ass].chgVect_ass[jass])[nass];
		double eta_ass = pvector_ass.Eta();
		double phi_ass = pvector_ass.Phi();
		double pt_ass  = pvector_ass.Pt();
		
		double deltaPhi = GetDeltaPhi( phi_trg, phi_ass );
		double deltaPhi2 = GetDeltaPhi(phi_ass, phi_trg);
		double deltaEta = GetDeltaEta( eta_trg, eta_ass );
		
		if( deltaEta == 0.0  &&  deltaPhi == 0.0 && deltaPhi2 && pt_trg==pt_ass) continue;
		//if (pt_trg  < pt_ass) continue;
		
		//Total weight
		double effweight = effweight_trg * effweight_ass;
		//double effweight = 1.0;
		
		//`Fill and symmetrize the distribution
		if(chg_trg < 0 && chg_ass > 0)
		  {
		    hBackground_[itrg][jass]->Fill( fabs(deltaEta), deltaPhi, 1.0/4.0/effweight );
		    hBackground_[itrg][jass]->Fill(-fabs(deltaEta), deltaPhi, 1.0/4.0/effweight );
		    hBackground_[itrg][jass]->Fill( fabs(deltaEta), deltaPhi2, 1.0/4.0/effweight );
		    hBackground_[itrg][jass]->Fill(-fabs(deltaEta), deltaPhi2, 1.0/4.0/effweight ); 
		    //hBackground_[itrg][jass]->Fill( deltaEta, deltaPhi, 1.0/effweight );
		  }
	      }
          }
      }
  }
}

void
  ChargeDepAndPtCorr::NormalizeHists()
{
  for( unsigned int itrg = 0; itrg < pTmin_trg_.size(); itrg++ )
  {
    for(unsigned int jass = 0; jass < pTmin_ass_.size(); jass++ )
    {
      if(itrg < jass) continue;

      if( hSignal_[itrg][jass]->Integral() != 0 &&
          hBackground_[itrg][jass]->Integral() != 0 )
      {
         double  etabinwidth = hSignal_[itrg][jass]->GetXaxis()->GetBinWidth(1);
         double  phibinwidth = hSignal_[itrg][jass]->GetYaxis()->GetBinWidth(1);

         //hSignal_[itrg][jass]    ->Scale(1.0/etabinwidth/phibinwidth);
         //hBackground_[itrg][jass]->Scale(1.0/etabinwidth/phibinwidth);

         hCorrelation_[itrg][jass]->Add(hSignal_[itrg][jass]);
         hCorrelation_[itrg][jass]->Divide(hBackground_[itrg][jass]);
         hCorrelation_[itrg][jass]->Scale(hBackground_[itrg][jass]->GetBinContent(hBackground_[itrg][jass]->FindBin(0,0)));

	 hCorrelation_[itrg][jass]->Scale(1.0/etabinwidth/phibinwidth);
	 
      }
    }
  }
}

//define this as a plug-in
DEFINE_FWK_MODULE(ChargeDepAndPtCorr);
