#########################################################
### Branches for cfA
#########################################################

import FWCore.ParameterSet.Config as cms

basicKinematicLeaves = cms.PSet(
    pt = cms.string('pt'),
    eta = cms.string('eta'),
    phi = cms.string('phi'),
    energy = cms.string('energy'),
    px = cms.string('px'),
    py = cms.string('py'),
    pz = cms.string('pz'),
    theta = cms.string('theta'),
    et = cms.string('et'),
    status = cms.string('status')
)

genMatchingLeaves = cms.PSet(
    gen_particle_id = cms.string('genParticle.pdgId'),
    gen_particle_status = cms.string('genParticle.status'),
    gen_particle_pt = cms.string('genParticle.pt'),
    gen_particle_eta = cms.string('genParticle.eta'),
    gen_particle_phi = cms.string('genParticle.phi'),
    gen_particle_energy = cms.string('genParticle.energy'),
    ## gen_px = cms.string('genParticle.px'),
    ## gen_py = cms.string('genParticle.py'),
    ## gen_pz = cms.string('genParticle.pz'),
    gen_mother_id = cms.string('genParticle.mother.pdgId'),
    gen_mother_status = cms.string('genParticle.mother.status'),
    ## gen_mother_pt = cms.string('genParticle.mother.pt'),
    ## gen_mother_eta = cms.string('genParticle.mother.eta'),
    ## gen_mother_phi = cms.string('genParticle.mother.phi'),
    ## gen_mother_energy = cms.string('genParticle.mother.energy'),
    ## gen_mother_px = cms.string('genParticle.mother.px'),
    ## gen_mother_py = cms.string('genParticle.mother.py'),
    ## gen_mother_pz = cms.string('genParticle.mother.pz'),
    gen_grandmother_id = cms.string('genParticle.mother.mother.pdgId'),
    gen_grandmother_status = cms.string('genParticle.mother.mother.status'),
    ## gen_grandmother_pt = cms.string('genParticle.mother.mother.pt'),
    ## gen_grandmother_eta = cms.string('genParticle.mother.mother.eta'),
    ## gen_grandmother_phi = cms.string('genParticle.mother.mother.phi'),
    ## gen_grandmother_energy = cms.string('genParticle.mother.mother.energy'),
    ## gen_grandmother_px = cms.string('genParticle.mother.mother.px'),
    ## gen_grandmother_py = cms.string('genParticle.mother.mother.py'),
    ## gen_grandmother_pz = cms.string('genParticle.mother.mother.pz'),
    gen_ggrandmother_id = cms.string('genParticle.mother.mother.mother.pdgId'),
    gen_ggrandmother_status = cms.string('genParticle.mother.mother.mother.status'),
    ## gen_ggrandmother_pt = cms.string('genParticle.mother.mother.mother.pt'),
    ## gen_ggrandmother_eta = cms.string('genParticle.mother.mother.mother.eta'),
    ## gen_ggrandmother_phi = cms.string('genParticle.mother.mother.mother.phi'),
    ## gen_ggrandmother_energy = cms.string('genParticle.mother.mother.mother.energy'),
    ## gen_ggrandmother_px = cms.string('genParticle.mother.mother.mother.px'),
    ## gen_ggrandmother_py = cms.string('genParticle.mother.mother.mother.py'),
    ## gen_ggrandmother_pz = cms.string('genParticle.mother.mother.mother.pz')
)

cfA = cms.EDFilter("minicfa",
    Selections = cms.PSet(
        filters = cms.PSet(),
        selections = cms.PSet(
            minSelection = cms.PSet(
                filterOrder = cms.vstring(''),
                makeFinalPlots = cms.bool(True),
                makeSummaryTable = cms.bool(True),
                makeContentPlots = cms.bool(True),
                makeAllButOnePlots = cms.bool(True),
                ntuplize = cms.bool(True),
                nMonitor = cms.uint32(1000),
                makeCumulativePlots = cms.bool(True)
            )
        )    ),
    Plotter = cms.PSet(
        TH1s = cms.PSet(),
        ComponentName = cms.string('VariablePlotter'),
        TProfiles = cms.PSet(),
        TH2s = cms.PSet()
    ),
    Variables = cms.PSet(
        L1Bit = cms.PSet(
            src = cms.InputTag("gtDigis"),
            method = cms.string('ComputedVariable'),
            computer = cms.string('L1BitComputer')
        )
    ),
    workAsASelector = cms.bool(True),
    flows = cms.vstring('minSelection'),
    Ntupler = cms.PSet(
        branchesPSet = cms.PSet(
            treeName = cms.string('eventB'),
            pv = cms.PSet(
                src = cms.InputTag("offlineSlimmedPrimaryVertices"),
                 leaves = cms.PSet(
                       vars = cms.vstring(
                          'x:x',
                          'y:y',
                          'z:z',
                          'xErr:xError',
                          'yErr:yError',
                          'zErr:zError',
                          'chi2:chi2',
                          'ndof:ndof',
                          'isFake:isFake',               
                          'isValid:isValid'											 
               		),
               ),
               Class = cms.string('reco::Vertex'),
            ),
            beamSpot = cms.PSet(  
           	src = cms.InputTag("offlineBeamSpot"),
            leaves = cms.PSet(
                vars = cms.vstring(
                     'x:position.x',
                     'y:position.y',
                     'z:position.z',
                     'x0Error:x0Error',
                     'y0Error:y0Error',
                     'z0Error:z0Error',
                     'sigmaZ:sigmaZ',
                     'sigmaZ0Error:sigmaZ0Error',
                     'dxdz:dxdz',
                     'dxdzError:dxdzError',
                     'dydz:dydz',
                     'dydzError:dydzError',
                     'beamWidthX:BeamWidthX',#addedFB
                     'beamWidthY:BeamWidthY',#addedFB
                     #'beamWidth:BeamWidth',
                     'beamWidthXError:BeamWidthXError',#addedFB
                     'beamWidthYError:BeamWidthYError'#addedFB
                     #'beamWidthError:BeamWidthError'
                       )
                ),
            Class = cms.string('reco::BeamSpot')
            ),

            mus = cms.PSet(
                src = cms.InputTag('slimmedMuons'),
                leaves = cms.PSet(
                    basicKinematicLeaves,
                  #  genMatchingLeaves,
                    vars = cms.vstring(
                        # variables for proposed new medium ID -- https://indico.cern.ch/event/357213/contribution/2/material/slides/0.pdf
                        'globalTrack_normalizedChi2:globalTrack.normalizedChi2',
                        'trkPositionMatch:combinedQuality.chi2LocalPosition',
                        'trkKink:combinedQuality.trkKink',
                        'tkHits:track.hitPattern.numberOfValidHits',
                        'tkHitsFrac:innerTrack.validFraction',
                        'segmentCompatibility:segmentCompatibility',
                        'caloCompatibility:caloCompatibility',
                        'cIso:caloIso', 
                        'tIso:trackIso',
                        'ecalIso:ecalIso',
                        'hcalIso:hcalIso',
                        'ecalvetoDep:ecalIsoDeposit.candEnergy',
                        'hcalvetoDep:hcalIsoDeposit.candEnergy',
#                        'id:leptonID', 
                        'calEnergyEm:calEnergy.em',
                        'calEnergyHad:calEnergy.had',
                        'calEnergyHo:calEnergy.ho',
                        'calEnergyEmS9:calEnergy.emS9',
                        'calEnergyHadS9:calEnergy.hadS9',
                        'calEnergyHoS9:calEnergy.hoS9', 
                        'iso03_emVetoEt:isolationR03.emVetoEt', 	 
                        'iso03_hadVetoEt:isolationR03.hadVetoEt',
                        'iso03_sumPt:isolationR03.sumPt',
                        'iso03_emEt:isolationR03.emEt',
                        'iso03_hadEt:isolationR03.hadEt',
                        'iso03_hoEt:isolationR03.hoEt',
                        'iso03_nTracks:isolationR03.nTracks',
                        'iso05_sumPt:isolationR05.sumPt',
                        'iso05_emEt:isolationR05.emEt',
                        'iso05_hadEt:isolationR05.hadEt',
                        'iso05_hoEt:isolationR05.hoEt',
                        'iso05_nTracks:isolationR05.nTracks',
                        'pfIsolationR03_sumChargedHadronPt:pfIsolationR03.sumChargedHadronPt',
                        'pfIsolationR03_sumChargedParticlePt:pfIsolationR03.sumChargedParticlePt',
                        'pfIsolationR03_sumNeutralHadronEt:pfIsolationR03.sumNeutralHadronEt',
                        'pfIsolationR03_sumNeutralHadronEtHighThreshold:pfIsolationR03.sumNeutralHadronEtHighThreshold',
                        'pfIsolationR03_sumPhotonEt:pfIsolationR03.sumPhotonEt',
                        'pfIsolationR03_sumPhotonEtHighThreshold:pfIsolationR03.sumPhotonEtHighThreshold',
                        'pfIsolationR03_sumPUPt:pfIsolationR03.sumPUPt',
                        'pfIsolationR04_sumChargedHadronPt:pfIsolationR04.sumChargedHadronPt',
                        'pfIsolationR04_sumChargedParticlePt:pfIsolationR04.sumChargedParticlePt',
                        'pfIsolationR04_sumNeutralHadronEt:pfIsolationR04.sumNeutralHadronEt',
                        'pfIsolationR04_sumNeutralHadronEtHighThreshold:pfIsolationR04.sumNeutralHadronEtHighThreshold',
                        'pfIsolationR04_sumPhotonEt:pfIsolationR04.sumPhotonEt',
                        'pfIsolationR04_sumPhotonEtHighThreshold:pfIsolationR04.sumPhotonEtHighThreshold',
                        'pfIsolationR04_sumPUPt:pfIsolationR04.sumPUPt',
                        'charge:charge', 
                        'cm_chi2:combinedMuon.chi2', 
                        'cm_ndof:combinedMuon.ndof', 
                        'cm_chg:combinedMuon.charge', 
                        'cm_pt:combinedMuon.pt', 
                        'cm_px:combinedMuon.px', 
                        'cm_py:combinedMuon.py', 
                        'cm_pz:combinedMuon.pz', 
                        'cm_eta:combinedMuon.eta', 
                        'cm_phi:combinedMuon.phi', 
                        'cm_theta:combinedMuon.theta', 
                        'cm_d0dum:combinedMuon.d0', 
                        'cm_dz:combinedMuon.dz', 
                        'cm_vx:combinedMuon.vx', 
                        'cm_vy:combinedMuon.vy', 
                        'cm_vz:combinedMuon.vz', 
                        'cm_numvalhits:combinedMuon.numberOfValidHits', 
                        'cm_numlosthits:combinedMuon.numberOfLostHits', 
                        'cm_numvalMuonhits:combinedMuon.hitPattern.numberOfValidMuonHits',
                        'cm_d0dumErr:combinedMuon.d0Error', 
                        'cm_dzErr:combinedMuon.dzError', 
                        'cm_ptErr:combinedMuon.ptError', 
                        'cm_etaErr:combinedMuon.etaError', 
                        'cm_phiErr:combinedMuon.phiError', 
                        'tk_id:track.key',
                        'tk_chi2:track.chi2',
                        'tk_ndof:track.ndof', 
                        'tk_chg:track.charge', 
                        'tk_pt:track.pt', 
                        'tk_px:track.px', 
                        'tk_py:track.py', 
                        'tk_pz:track.pz', 
                        'tk_eta:track.eta', 
                        'tk_phi:track.phi', 
                        'tk_theta:track.theta', 
                        'tk_d0dum:track.d0', 
                        'tk_dz:track.dz', 
                        'tk_vx:track.vx', 
                        'tk_vy:track.vy', 
                        'tk_vz:track.vz', 
                        'tk_numvalhits:track.numberOfValidHits', 
                        'tk_numlosthits:track.numberOfLostHits', 
                        'tk_d0dumErr:track.d0Error', 
                        'tk_dzErr:track.dzError', 
                        'tk_ptErr:track.ptError', 
                        'tk_etaErr:track.etaError', 
                        'tk_phiErr:track.phiError', 
                        'tk_numvalPixelhits:track.hitPattern.numberOfValidPixelHits',
                        'tk_numpixelWthMeasr:track.hitPattern.pixelLayersWithMeasurement',
                        'stamu_chi2:standAloneMuon.chi2', 
                        'stamu_ndof:standAloneMuon.ndof', 
                        'stamu_chg:standAloneMuon.charge', 
                        'stamu_pt:standAloneMuon.pt', 
                        'stamu_px:standAloneMuon.px', 
                        'stamu_py:standAloneMuon.py', 
                        'stamu_pz:standAloneMuon.pz', 
                        'stamu_eta:standAloneMuon.eta', 
                        'stamu_phi:standAloneMuon.phi', 
                        'stamu_theta:standAloneMuon.theta', 
                        'stamu_d0dum:standAloneMuon.d0',
                        'stamu_dz:standAloneMuon.dz', 
                        'stamu_vx:standAloneMuon.vx', 
                        'stamu_vy:standAloneMuon.vy', 
                        'stamu_vz:standAloneMuon.vz', 
                        'stamu_numvalhits:standAloneMuon.numberOfValidHits', 
                        'stamu_numlosthits:standAloneMuon.numberOfLostHits', 
                        'stamu_d0dumErr:standAloneMuon.d0Error', 
                        'stamu_dzErr:standAloneMuon.dzError', 
                        'stamu_ptErr:standAloneMuon.ptError', 
                        'stamu_etaErr:standAloneMuon.etaError', 
                        'stamu_phiErr:standAloneMuon.phiError', 
                        'num_matches:numberOfMatches',
                        'isPFMuon:isPFMuon',
                        'isTrackerMuon:isTrackerMuon',
                        'isStandAloneMuon:isStandAloneMuon',
                        'isGlobalMuon:isGlobalMuon',
                        'id_All:isGood("All")',
                        'id_AllGlobalMuons:isGood("AllGlobalMuons")',
                        'id_AllStandAloneMuons:isGood("AllStandAloneMuons")',
                        'id_AllTrackerMuons:isGood("AllTrackerMuons")',
                        'id_TrackerMuonArbitrated:isGood("TrackerMuonArbitrated")',
                        'id_AllArbitrated:isGood("AllArbitrated")',
                        'id_GlobalMuonPromptTight:isGood("GlobalMuonPromptTight")',
                        'id_TMLastStationLoose:isGood("TMLastStationLoose")',                        
                        'id_TMLastStationTight:isGood("TMLastStationTight")',
                        'id_TM2DCompatibilityLoose:isGood("TM2DCompatibilityLoose")',
                        'id_TM2DCompatibilityTight:isGood("TM2DCompatibilityTight")',                        
                        'id_TMOneStationLoose:isGood("TMOneStationLoose")',
                        'id_TMOneStationTight:isGood("TMOneStationTight")',
                        'id_TMLastStationOptimizedLowPtLoose:isGood("TMLastStationOptimizedLowPtLoose")',
                        'id_TMLastStationOptimizedLowPtTight:isGood("TMLastStationOptimizedLowPtTight")',

                        'tk_LayersWithMeasurement:track.hitPattern.trackerLayersWithMeasurement',
                       'tk_PixelLayersWithMeasurement:track.hitPattern.pixelLayersWithMeasurement',
                       'tk_ValidStripLayersWithMonoAndStereoHit:track.hitPattern.numberOfValidStripLayersWithMonoAndStereo',

                        'tk_LayersWithoutMeasurement:track.hitPattern.trackerLayersWithoutMeasurement(\'TRACK_HITS\')',
#                        'tk_ExpectedHitsInner:track.trackerExpectedHitsInner.numberOfHits',
#                        'tk_ExpectedHitsOuter:track.trackerExpectedHitsOuter.numberOfHits',
                       'cm_LayersWithMeasurement:combinedMuon.hitPattern.trackerLayersWithMeasurement',

                       'cm_PixelLayersWithMeasurement:combinedMuon.hitPattern.pixelLayersWithMeasurement',
                       'cm_ValidStripLayersWithMonoAndStereoHit:combinedMuon.hitPattern.numberOfValidStripLayersWithMonoAndStereo',
                       'cm_LayersWithoutMeasurement:combinedMuon.hitPattern.trackerLayersWithoutMeasurement(\'TRACK_HITS\')',
#                        'cm_ExpectedHitsInner:combinedMuon.trackerExpectedHitsInner.numberOfHits',
#                        'cm_ExpectedHitsOuter:combinedMuon.trackerExpectedHitsOuter.numberOfHits',
                        'dB:dB',
                        'numberOfMatchedStations:numberOfMatchedStations',
                    )
                ),
                Class = cms.string('pat::Muon')
                ),

                 pfType1mets = cms.PSet(
                  src = cms.InputTag("slimmedMETs"),
                   leaves = cms.PSet(
                    vars = cms.vstring('et:et', 
                        'phi:phi', 
                        'ex:px', 
                        'ey:py',
                        'NeutralEMFraction:NeutralEMFraction',
                        'NeutralHadEtFraction:NeutralHadEtFraction',
                        'ChargedEMEtFraction:ChargedEMEtFraction',
                        'ChargedHadEtFraction:ChargedHadEtFraction',
                        'MuonEtFraction:MuonEtFraction',
                        'Type6EtFraction:Type6EtFraction',
                        'Type7EtFraction:Type7EtFraction',
                        'sumEt:sumEt', 
                        ## 'unCPhi:uncorrectedPhi', 
                        ## 'unCPt:uncorrectedPt',
                        'gen_et:genMET.pt',
                        'gen_phi:genMET.phi',
                        'Sig:mEtSig'
                        ## 'Significance:significance'
                        )
                ),
                Class = cms.string('pat::MET')
            ),

            photons = cms.PSet(
                src = cms.InputTag("slimmedPhotons"),
                leaves = cms.PSet(
                    basicKinematicLeaves,
                   # genMatchingLeaves,
                    vars= cms.vstring('isConv:hasConversionTracks',
                                      'pf_ch_iso:chargedHadronIso',
                                      'pf_nh_iso:neutralHadronIso',
                                      'pf_ph_iso:photonIso',
                                      #X'full5x5_sigmaIetaIeta:full5x5_sigmaIetaIeta',
                                      'passElectronVeto:passElectronVeto',
                                      'hadOverEM:hadronicOverEm',
                                      'hadTowOverEM:hadTowOverEm',
                                      'hasPixelSeed:hasPixelSeed',
                                      'scEnergy:superCluster.energy',
                                      'scRawEnergy:superCluster.rawEnergy',
                                      'scEta:superCluster.position.eta',
                                      'scPhi:superCluster.position.phi',
                                      'scEtaWidth:superCluster.etaWidth',
                                      'scPhiWidth:superCluster.phiWidth',
                                      'tIso:trackIso',
                                      'ecalIso:ecalIso',
                                      'hcalIso:hcalIso',                                      
                                      'isoEcalRecHitDR04:ecalRecHitSumEtConeDR04',
                                      'isoHcalRecHitDR04:hcalTowerSumEtConeDR04',
                                      'isoSolidTrkConeDR04:trkSumPtSolidConeDR04',
                                      'isoHollowTrkConeDR04:trkSumPtHollowConeDR04',
                                      'nTrkSolidConeDR04:nTrkSolidConeDR04',
                                      'nTrkHollowConeDR04:nTrkSolidConeDR04',
                                      'isoEcalRecHitDR03:ecalRecHitSumEtConeDR03',
                                      'isoHcalRecHitDR03:hcalTowerSumEtConeDR03',
                                      'isoSolidTrkConeDR03:trkSumPtSolidConeDR03',
                                      'isoHollowTrkConeDR03:trkSumPtHollowConeDR03',
                                      'nTrkSolidConeDR03:nTrkSolidConeDR03',
                                      'nTrkHollowConeDR03:nTrkSolidConeDR03',
                                      'isEBGap:isEBGap',
                                      'isEEGap:isEEGap',
                                      'isEBEEGap:isEBEEGap',
                                      'isEBPho:isEB',#changed from isEBPho
                                      'isEEPho:isEE',#changed from isEEPho
                                      'isLoosePhoton:photonID("PhotonCutBasedIDLoose")',
                                      'isTightPhoton:photonID("PhotonCutBasedIDTight")',
                                      'maxEnergyXtal:maxEnergyXtal',
                                      'e1x5:e1x5',
                                      'e2x5:e2x5',
                                      'e3x3:e3x3',
                                      'e5x5:e5x5',
                                      'sigmaEtaEta:sigmaEtaEta',
                                      'sigmaIetaIeta:sigmaIetaIeta',
                                      'r9:r9'
                    )
                ),
                Class = cms.string('pat::Photon')
            ),


            mc_doc = cms.PSet(
                src = cms.InputTag("prunedGenParticles"),
                leaves = cms.PSet(
                    vars = cms.vstring(
                        'id:pdgId',
                        'pt:pt',
                        'px:px',
                        'py:py',
                        'pz:pz',
                        'eta:eta',
                        'phi:phi',
                        #'theta:theta',
                        'energy:energy',
                        'status:status',
                        'charge:charge',
                        'mother_id:mother.pdgId',
                        'grandmother_id:mother.mother.pdgId',
                        'ggrandmother_id:mother.mother.mother.pdgId',
                        'vertex_x:vertex.x',
                        'vertex_y:vertex.y',
                        'vertex_z:vertex.z',
                        'mass:mass',
                        'numOfDaughters:numberOfDaughters',
                        'numOfMothers:numberOfMothers',
                        'isPromptFinalState:isPromptFinalState',
                        'isDirectPromptTauDecayProductFinalState:isDirectPromptTauDecayProductFinalState'
                    )
                ),
                #selection = cms.string('status=3'),  ## We'll cut on something later
                Class = cms.string('reco::GenParticle')
            ),

            mc_final = cms.PSet(
                src = cms.InputTag("packedGenParticles"),
                leaves = cms.PSet(
                    vars = cms.vstring(
                        'id:pdgId',
                        'pt:pt',
                        'eta:eta',
                        'phi:phi',
                        'energy:energy',
                        'charge:charge',
                        'mother_id:mother.pdgId',
                        'grandmother_id:mother.mother.pdgId',
                        'ggrandmother_id:mother.mother.mother.pdgId',
                        #'vertex_x:vertex.x',
                        #'vertex_y:vertex.y',
                        #'vertex_z:vertex.z',
                        'numOfMothers:numberOfMothers',
                        'isPromptFinalState:isPromptFinalState',
                        'isDirectPromptTauDecayProductFinalState:isDirectPromptTauDecayProductFinalState'
                    )
                ),
                Class = cms.string('pat::PackedGenParticle')
            ),

             taus = cms.PSet(
                 src = cms.InputTag("slimmedTaus"),
                 leaves = cms.PSet(
                     basicKinematicLeaves,
                    # genMatchingLeaves,
                     vars = cms.vstring(
                         'charge:charge',
                         'leadChargedHadrCand_pt:leadChargedHadrCand.pt',
                         'leadChargedHadrCand_charge:leadChargedHadrCand.charge',
                         'leadChargedHadrCand_eta:leadChargedHadrCand.eta',
                         'leadChargedHadrCand_phi:leadChargedHadrCand.phi'
                     )
                 ),
                 Class = cms.string('pat::Tau')
             ),

            els = cms.PSet(
                src = cms.InputTag('slimmedElectrons'),
                leaves = cms.PSet(
                    basicKinematicLeaves,
                   # genMatchingLeaves,
                    vars = cms.vstring(#'id:leptonID', 
                        'pfIsolationR03_sumChargedHadronPt:pfIsolationVariables.sumChargedHadronPt',
                        'pfIsolationR03_sumNeutralHadronEt:pfIsolationVariables.sumNeutralHadronEt',
                        'pfIsolationR03_sumPhotonEt:pfIsolationVariables.sumPhotonEt',
                        'pfIsolationR03_sumPUPt:pfIsolationVariables.sumPUPt',
                        'full5x5_sigmaIetaIeta:full5x5_sigmaIetaIeta',
                        'expectedMissingInnerHits:gsfTrack.hitPattern.numberOfLostHits(\'MISSING_INNER_HITS\')',
#                        'expectedMissingInnerHits:gsfTrack.trackerExpectedHitsInner.numberOfLostHits',
                        'tightId:electronID("eidTight")',
                        'looseId:electronID("eidLoose")',
                        'robustTightId:electronID("eidRobustTight")',
                        'robustLooseId:electronID("eidRobustLoose")',
                        'robustHighEnergyId:electronID("eidRobustHighEnergy")',
                        'cIso:caloIso', 
                        'tIso:trackIso', 
                        'ecalIso:ecalIso',
                        'hcalIso:hcalIso',
                        'chi2:gsfTrack.chi2', 
#                        'class:classification', 
                        'charge:charge', 
                        'caloEnergy:caloEnergy',
                        'hadOverEm:hadronicOverEm',
                        'hcalOverEcalBc:hcalOverEcalBc', 
                        'eOverPIn:eSuperClusterOverP', 
                        'eSeedOverPOut:eSeedClusterOverPout', 
                        'sigmaEtaEta:scSigmaEtaEta',
                        'sigmaIEtaIEta:scSigmaIEtaIEta',
                        'scEnergy:superCluster.energy',
                        'scRawEnergy:superCluster.rawEnergy',
                        'scSeedEnergy:superCluster.seed.energy',
                        'scEta:superCluster.position.eta',
                        'scPhi:superCluster.position.phi',
                        'scEtaWidth:superCluster.etaWidth',
                        'scPhiWidth:superCluster.phiWidth',
                        'scE1x5:scE1x5',
                        'scE2x5Max:scE2x5Max',
                        'scE5x5:scE5x5',
                        'isEB:isEB',
                        'isEE:isEE',
                        'dEtaIn:deltaEtaSuperClusterTrackAtVtx', 
                        'dPhiIn:deltaPhiSuperClusterTrackAtVtx', 
                        'dEtaOut:deltaEtaSeedClusterTrackAtCalo', 
                        'dPhiOut:deltaPhiSeedClusterTrackAtCalo', 
                        'numvalhits:gsfTrack.numberOfValidHits', 
                        'numlosthits:gsfTrack.numberOfLostHits', 
                        #'numCluster:numberOfClusters', 
                        'basicClustersSize:basicClustersSize',
                        'tk_pz:gsfTrack.pz',
                        'tk_pt:gsfTrack.pt', 
                        'tk_phi:gsfTrack.phi', 
                        'tk_eta:gsfTrack.eta', 
                        'd0dum:gsfTrack.d0', 
                        'dz:gsfTrack.dz', 
                        'vx:gsfTrack.vx', 
                        'vy:gsfTrack.vy', 
                        'vz:gsfTrack.vz', 
                        'ndof:gsfTrack.ndof', 
                        'ptError:gsfTrack.ptError', 
                        'd0dumError:gsfTrack.d0Error', 
                        'dzError:gsfTrack.dzError', 
                        'etaError:gsfTrack.etaError', 
                        'phiError:gsfTrack.phiError', 
                        'tk_charge:gsfTrack.charge',
                        'core_ecalDrivenSeed:core.ecalDrivenSeed',
#                        'n_inner_layer:gsfTrack.trackerExpectedHitsInner.numberOfHits',
#                        'n_outer_layer:gsfTrack.trackerExpectedHitsOuter.numberOfHits',
                        'ctf_tk_id:closestCtfTrackRef.key',
                        'ctf_tk_charge:closestCtfTrackRef.charge',
                        'ctf_tk_eta:closestCtfTrackRef.eta',
                        'ctf_tk_phi:closestCtfTrackRef.phi',
                        'fbrem:fbrem',
                        'shFracInnerHits:shFracInnerHits',                        
                        'dr03EcalRecHitSumEt:dr03EcalRecHitSumEt',
                        'dr03HcalTowerSumEt:dr03HcalTowerSumEt',
                        'dr03HcalDepth1TowerSumEt:dr03HcalDepth1TowerSumEt',
                        'dr03HcalDepth2TowerSumEt:dr03HcalDepth2TowerSumEt',
                        'dr03TkSumPt:dr03TkSumPt',
                        'dr04EcalRecHitSumEt:dr03EcalRecHitSumEt',
                        'dr04HcalTowerSumEt:dr03HcalTowerSumEt',
                        'dr04HcalDepth1TowerSumEt:dr04HcalDepth1TowerSumEt',
                        'dr04HcalDepth2TowerSumEt:dr04HcalDepth2TowerSumEt',
                        'dr04TkSumPt:dr04TkSumPt',
                        'cpx:trackMomentumAtCalo.x', 
                        'cpy:trackMomentumAtCalo.y', 
                        'cpz:trackMomentumAtCalo.z', 
                        'vpx:trackMomentumAtVtx.x', 
                        'vpy:trackMomentumAtVtx.y', 
                        'vpz:trackMomentumAtVtx.z',
                        'cx:TrackPositionAtCalo.x', 
                        'cy:TrackPositionAtCalo.y', 
                        'cz:TrackPositionAtCalo.z', 
                        #'IDRobust:electronIDRobust'
                        'PATpassConversionVeto:passConversionVeto'
                        )
                        
                ),
                Class = cms.string('pat::Electron')
            ),

            jets_AK4 = cms.PSet(
                src = cms.InputTag("slimmedJets"),
                leaves = cms.PSet(
                    basicKinematicLeaves,
                    vars = cms.vstring(
                        'caloJetMap_pt:userFloat("caloJetMap:pt")',
                        'caloJetMap_emEnergyFraction:userFloat("caloJetMap:emEnergyFraction")',
                        'btag_jetBProb:bDiscriminator("pfJetBProbabilityBJetTags")',
                        'btag_jetProb:bDiscriminator("pfJetProbabilityBJetTags")',
                        'btag_TC_highPur:bDiscriminator("pfTrackCountingHighPurBJetTags")',
                        'btag_TC_highEff:bDiscriminator("pfTrackCountingHighEffBJetTags")',
                        'btag_secVertexHighEff:bDiscriminator("pfSimpleSecondaryVertexHighEffBJetTags")',
                        'btag_secVertexHighPur:bDiscriminator("pfSimpleSecondaryVertexHighPurBJetTags")',
                        'btag_inc_secVertexCombined:bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags")',
                        'btag_pf_secVertexCombined:bDiscriminator("pfCombinedSecondaryVertexV2BJetTags")',
                        'btag_MVA:bDiscriminator("pfCombinedMVABJetTags")',
                        'btag_csv_soft_lepton:bDiscriminator("pfCombinedSecondaryVertexSoftLeptonBJetTags")',
                        'pileupID_MVA:userFloat("pileupJetId:fullDiscriminant")',
                        #'pileupID_MVA:userFloat("pileupJetIdEvaluator:fullDiscriminant")',
                        #'pileupID_cutbased:userInt("pileupJetId:cutbasedId")',
                        #'pileupID_full:userInt("pileupJetId:fullId")',
                        #'qgLikelihood:userFloat("QGTagger:qgLikelihood")',
                        'parton_Id:genParton.pdgId',
                        'parton_motherId:genParton.mother.pdgId',
                        'parton_grandmotherID:genParton.mother.mother.pdgId',
                        'parton_pt:genParton.pt',
                        'parton_phi:genParton.phi',
                        'parton_eta:genParton.eta',
                        'parton_Energy:genParton.energy',
                        'parton_mass:genParton.mass',                        
                        'partonFlavour:partonFlavour',  #TL add
                        'gen_pt:genJet.pt',
                        'jetCharge:jetCharge',
                        'chgEmE:chargedEmEnergy',
                        'chgHadE:chargedHadronEnergy',
                        'photonEnergy:photonEnergy',
                        'chgMuE:chargedMuEnergy',
                        'chg_Mult:chargedMultiplicity',
                        'neutralEmE:neutralEmEnergy',
                        'neutralHadE:neutralHadronEnergy',
                        'neutral_Mult:neutralMultiplicity',
                        'mu_Mult:muonMultiplicity',
# These two variables are only defined for jets made from CaloJets
#                        'emf:emEnergyFraction',
#                        'ehf:energyFractionHadronic',
                        'n60:n60',
                        'n90:n90',
#                        'etaetaMoment:etaetaMoment',
#                        'etaphiMoment:etaphiMoment',
#                        'phiphiMoment:phiphiMoment',
                        'area:jetArea',
                        'corrFactorRaw:jecFactor("Uncorrected")',
                        'rawPt:jecFactor("Uncorrected")*pt',
                        'mass:mass'
                    )
        	),
               # selection = cms.string('pt*jecFactor(0)>10.0 | pt>20.0'),
                Class = cms.string('pat::Jet')
            ),
            
            mc_jets = cms.PSet(
                src = cms.InputTag("slimmedGenJets"),
                leaves = cms.PSet(
                    vars = cms.vstring(
                        'pt:pt',
                        'eta:eta',
                        'phi:phi',
                        #'theta:theta',
                        'et:et',
                        'energy:energy',
                        'emEnergy:emEnergy',
                        'hadEnergy:hadEnergy',
                        'invisibleEnergy:invisibleEnergy',
                        'auxiliaryEnergy:auxiliaryEnergy',
#                        'etaetaMoment:etaetaMoment',
#                        'etaphiMoment:etaphiMoment',
#                        'phiphiMoment:phiphiMoment',
                        'mass:mass'
                    )
        	),
                Class = cms.string('reco::GenJet')
            ),
            
          pfcand = cms.PSet(
                src = cms.InputTag('packedPFCandidates'),
                leaves = cms.PSet(
                    vars = cms.vstring(
                        'pdgId:pdgId',
                        'pt:pt',
                        #'pz:pz',
                        #'px:px',
                        #'py:py',
                        'eta:eta',
                        'phi:phi',
                        'energy:energy',
                        'charge:charge',
                        'dz:dz',
                        'dxy:dxy',
                        'fromPV:fromPV'
                     )
                ),
                #selection = cms.string('fromPV>1&&pt>5'),
                Class = cms.string('pat::PackedCandidate')
            ),
        ),
        ComponentName = cms.string('miniCompleteNTupler'),
        AdHocNPSet = cms.PSet(treeName = cms.string('eventA')),
        useTFileService = cms.bool(True), ## false for EDM; true for non EDM
    )
)



