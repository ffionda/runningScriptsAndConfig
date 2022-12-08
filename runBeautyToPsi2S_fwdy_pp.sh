#!/usr/bin/env bash

# make sure O2DPG + O2 is loaded
[ ! "${O2DPG_ROOT}" ] && echo "Error: This needs O2DPG loaded" && exit 1
[ ! "${O2_ROOT}" ] && echo "Error: This needs O2 loaded" && exit 1


# ----------- LOAD UTILITY FUNCTIONS --------------------------
. ${O2_ROOT}/share/scripts/jobutils.sh 

RNDSEED=${RNDSEED:-0}
NSIGEVENTS=${NSIGEVENTS:-20}
NBKGEVENTS=${NBKGEVENTS:-20}
NWORKERS=${NWORKERS:-1}
NTIMEFRAMES=${NTIMEFRAMES:-5}

# ----------- SETUP LOCAL CCDB CACHE --------------------------
export ALICEO2_CCDB_LOCALCACHE=$PWD/.ccdb


${O2DPG_ROOT}/MC/bin/o2dpg_sim_workflow.py -eCM 13600 -gen external -j ${NWORKERS} -ns ${NSIGEVENTS} -tf ${NTIMEFRAMES} -e TGeant4 -mod "--skipModules ZDC" \
        -trigger "external" -ini $O2DPG_ROOT/MC/config/PWGDQ/ini/GeneratorHF_bbbar_Psi2S_fwdy.ini  \
    -genBkg pythia8 -procBkg inel -colBkg pp --embedding -nb ${NBKGEVENTS} --mft-reco-full \
    -interactionRate 500000  -confKeyBkg "Diamond.width[2]=6" -bcPatternFile ${PWD}/bcPattern_25ns_2556b_2544_2215_2332_144bpi_20injV3.root --mft-assessment-full --fwdmatching-assessment-full  \
    --fwdmatching-cut-4-param


export FAIRMQ_IPC_PREFIX=./
# run workflow (highly-parallel)
${O2DPG_ROOT}/MC/bin/o2_dpg_workflow_runner.py -f workflow.json -tt aod  -jmax 1  

