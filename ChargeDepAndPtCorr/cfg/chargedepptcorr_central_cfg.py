import FWCore.ParameterSet.Config as cms

process = cms.Process("ChargeDepAndPtCorr")


# __________________ General _________________

# Configure the logger
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 10

# Configure the number of maximum event the analyser run on in interactive mode
# -1 == ALL
process.maxEvents = cms.untracked.PSet( 
    #input = cms.untracked.int32(-1) 
    input = cms.untracked.int32(1000) 
)


# __________________ I/O files _________________

# Define the input file to run on in interactive mode
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'root://cms-xrd-global.cern.ch//store/hidata/HIRun2015/HIMinimumBias2/AOD/25Aug2016-v1/90000/34CD034C-ED6F-E611-A55F-44A842124E15.root'
    )
)

#----- Testing one One file -------------------
#process.source = cms.Source("PoolSource",
#    fileNames = cms.untracked.vstring(
#        'file:/afs/cern.ch/work/q/qwang/public/PbPb2015_tracking/pixeltracking_1.root'
#    )
#)
#----------------------------------------------

#process.source = cms.Source("PoolSource",
#    fileNames = cms.untracked.vstring(
#        'root://cms-xrd-global.cern.ch//store/himc/HINPbPbWinter16DR/Hydjet_Quenched_MinBias_5020GeV_750/AODSIM/NoPU_75X_mcRun2_HeavyIon_v13_75X_mcRun2_HeavyIon_v13-v1/80000/001E4607-5BBA-E611-9A99-0CC47A7E6A2C.root'
#    )
#)

# Define output file name
import os
process.TFileService = cms.Service("TFileService",
     #fileName = cms.string(os.getenv('CMSSW_BASE') + '/src/Analyzers/ChargeDepAndPtCorr/test/chargeptdepcorr.root')
     fileName = cms.string('chargeptdepcorr_central.root')
)


# __________________ Detector conditions _________________

# Configure the Global Tag
# Global tag contains information about detector geometry, calibration, alignement, ...
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '75X_dataRun2_v13', '')


# __________________ Event selection _________________

# Define the trigger selection
# TO DO: To be moved in a separate config file
process.load("HLTrigger.HLTfilters.hltHighLevel_cfi")
process.hltMB = process.hltHighLevel.clone()
process.hltMB.HLTPaths = [
               "HLT_HIL1MinimumBiasHF1AND*",
               "HLT_HIL1MinimumBiasHF2AND*"
              ]
process.hltMB.andOr = cms.bool(True)
process.hltMB.throw = cms.bool(False)

# Load centrality producer for centrality calculation
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.newCentralityBin = process.centralityBin.clone()

# Load HI event selection modules
process.load('HeavyIonsAnalysis.Configuration.collisionEventSelection_cff')
process.load('HeavyIonsAnalysis.EventAnalysis.HIClusterCompatibilityFilter_cfi')
process.clusterCompatibilityFilter.clusterPars = cms.vdouble(0.0,0.006)
process.clusterCompatibilityFilter.clusterTrunc = cms.double(2.0)

# Pixel ReRECO CC Tune (New addition for Pileup rejection Prabhat---- Jun29)
#The following lines of code for the provide the parameters for the respective tunes:

#process.clusterCompatibilityFilter.pixelTune  = cms.untracked.bool(True)

# 0.5%

#process.clusterCompatibilityFilter.pixelTunePolyClusterPars = cms.untracked.vdouble(2.80464, 1.31997e-05, -6.65202e-10, 3.39093e-14, -9.93328e-19, 1.01562e-23)
#process.clusterCompatibilityFilter.pixelTuneLineClusterPars = cms.untracked.vdouble(1.82726, 0.000989945)
#process.clusterCompatibilityFilter.clusterTrunc             = cms.double(3.64)
#process.clusterCompatibilityFilter.nhitsLineTrunc           = cms.untracked.int32(1000)

# 3%

#process.clusterCompatibilityFilter.pixelTunePolyClusterPars = cms.untracked.vdouble(3.12211, 5.04532e-05, -2.4#6064e-09, 7.08956e-14, -1.22693e-18, 9.17866e-24)
#process.clusterCompatibilityFilter.pixelTuneLineClusterPars = cms.untracked.vdouble(2.24092, 0.000929259)
#process.clusterCompatibilityFilter.clusterTrunc             = cms.double(3.79)
#process.clusterCompatibilityFilter.nhitsLineTrunc           = cms.untracked.int32(1000)

# __________________ Analyse Sequence _________________

# Load you analyzer with initial configuration
process.load("Analyzers.ChargeDepAndPtCorr.chargedepptcorr_cff")
process.defaultAnalysis_05   = process.CPDC05.clone()
#process.defaultAnalysis_510   = process.CPDC510.clone()
process.p = cms.Path(process.hfCoincFilter3 *             # Requier HF coincidence with 3 GeV  
                     process.primaryVertexFilter *        # Clean up on vertices
                     process.clusterCompatibilityFilter * # Clean up on pileup
                     process.centralityBin *              # Compute centrality
                     process.hltMB *                      # Select MB events
                     process.defaultAnalysis_05)
                     
                     

