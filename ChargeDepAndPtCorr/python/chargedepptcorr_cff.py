import FWCore.ParameterSet.Config as cms

from Analyzers.ChargeDepAndPtCorr.chargedepptcorr_cfi import *

#### Standard analysis for pixel Rereco PbPb 2015 ####

#### centrality 0-5% ####
CPDC05 = defaultCPDC.clone()
CPDC05.centmin = cms.untracked.int32(0)
CPDC05.centmax = cms.untracked.int32(5) ##5
#### centrality 5-10% ####
CPDC510 = defaultCPDC.clone()
CPDC510.centmin = cms.untracked.int32(5)
CPDC510.centmax = cms.untracked.int32(10)
#### centrality 10-15% ####
CPDC1015 = defaultCPDC.clone()
CPDC1015.centmin = cms.untracked.int32(10)
CPDC1015.centmax = cms.untracked.int32(15)
#### centrality 15-20% ####
CPDC1520 = defaultCPDC.clone()
CPDC1520.centmin = cms.untracked.int32(15)
CPDC1520.centmax = cms.untracked.int32(20)
#### centrality 20-25% ####
CPDC2025 = defaultCPDC.clone()
CPDC2025.centmin = cms.untracked.int32(20)
CPDC2025.centmax = cms.untracked.int32(25)
#### centrality 25-30% ####
CPDC2530 = defaultCPDC.clone()
CPDC2530.centmin = cms.untracked.int32(25)
CPDC2530.centmax = cms.untracked.int32(30)
#### centrality 30-35% ####
CPDC3035 = defaultCPDC.clone()
CPDC3035.centmin = cms.untracked.int32(30)
CPDC3035.centmax = cms.untracked.int32(35)
#### centrality 35-40% ####
CPDC3540 = defaultCPDC.clone()
CPDC3540.centmin = cms.untracked.int32(35)
CPDC3540.centmax = cms.untracked.int32(40)
#### centrality 40-45% ####
CPDC4045 = defaultCPDC.clone()
CPDC4045.centmin = cms.untracked.int32(40)
CPDC4045.centmax = cms.untracked.int32(45)
#### centrality 45-50% ####
CPDC4550 = defaultCPDC.clone()
CPDC4550.centmin = cms.untracked.int32(45)
CPDC4550.centmax = cms.untracked.int32(50)
#### centrality 50-55% ####
CPDC5055 = defaultCPDC.clone()
CPDC5055.centmin = cms.untracked.int32(50)
CPDC5055.centmax = cms.untracked.int32(55)
#### centrality 55-60% ####
CPDC5560 = defaultCPDC.clone()
CPDC5560.centmin = cms.untracked.int32(55)
CPDC5560.centmax = cms.untracked.int32(60)
#### centrality 60-65% ####
CPDC6065 = defaultCPDC.clone()
CPDC6065.centmin = cms.untracked.int32(60)
CPDC6065.centmax = cms.untracked.int32(65)
#### centrality 65-70% ####
CPDC6570 = defaultCPDC.clone()
CPDC6570.centmin = cms.untracked.int32(65)
CPDC6570.centmax = cms.untracked.int32(70)
#### centrality 70-75% ####
CPDC7075 = defaultCPDC.clone()
CPDC7075.centmin = cms.untracked.int32(70)
CPDC7075.centmax = cms.untracked.int32(75)
#### centrality 75-80% ####
CPDC7580 = defaultCPDC.clone()
CPDC7580.centmin = cms.untracked.int32(75)
CPDC7580.centmax = cms.untracked.int32(80)
#### centrality 80-85% ####
CPDC8085 = defaultCPDC.clone()
CPDC8085.centmin = cms.untracked.int32(80)
CPDC8085.centmax = cms.untracked.int32(85)
#### centrality 85-90% ####
CPDC8590 = defaultCPDC.clone()
CPDC8590.centmin = cms.untracked.int32(85)
CPDC8590.centmax = cms.untracked.int32(90)

#pT diff file
#CPDCptdiff = defaultCPDC.clone()
#CPDCptdiff.pTminTrk_trg = cms.untracked.int32(0.1,0.2,0.3,0.4,0.5,1.0,2.0)
#CPDCptdiff.pTminTrk_trg = cms.untracked.int32(0.2,0.3,0.4,0.5,1.0,2.0,3.0)
