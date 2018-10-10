import FWCore.ParameterSet.Config as cms

process = cms.Process("ChargeDepAndPtCorr")


# __________________ General _________________

# Configure the logger
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 10

# Configure the number of maximum event the analyser run on in interactive mode
# -1 == ALL
process.maxEvents = cms.untracked.PSet( 
    input = cms.untracked.int32(-1) 
    #input = cms.untracked.int32(1) 
)


# __________________ I/O files _________________

#Simulation file
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'root://cms-xrd-global.cern.ch//store/himc/HINPbPbWinter16DR/Hydjet_Quenched_MinBias_5020GeV_750/AODSIM/NoPU_75X_mcRun2_HeavyIon_v13_75X_mcRun2_HeavyIon_v13-v1/80000/001E4607-5BBA-E611-9A99-0CC47A7E6A2C.root'
    )
)

# Define output file name
import os
process.TFileService = cms.Service("TFileService",
     #fileName = cms.string(os.getenv('CMSSW_BASE') + '/src/Analyzers/ChargeDepAndPtCorr/test/chargeptdepcorr.root')
     fileName = cms.string('chargeptdepcorr_midperipheral.root')
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


# __________________ Event selection _________________

# Load you analyzer with initial configuration
process.load("Analyzers.ChargeDepAndPtCorr.chargedepptcorr_cff")
process.defaultAnalysis_3540 = process.CPDC3540.clone()
#process.defaultAnalysis_4045 = process.CPDC4045.clone()
#process.defaultAnalysis_4550 = process.CPDC4550.clone()
process.p = cms.Path(process.hfCoincFilter3 *             # Requier HF coincidence with 3 GeV  
                     process.primaryVertexFilter *        # Clean up on vertices
                     process.clusterCompatibilityFilter * # Clean up on pileup
                     process.centralityBin *              # Compute centrality
                     process.hltMB *                      # Select MB events
                     process.defaultAnalysis_3540)
