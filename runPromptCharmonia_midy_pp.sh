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


${O2DPG_ROOT}/MC/bin/o2dpg_sim_workflow.py --noIPC 1 -eCM 900 -gen external -j ${NWORKERS} -ns ${NSIGEVENTS} -tf ${NTIMEFRAMES} -e TGeant4 \
    -mod "--skipModules ZDC" \
    -confKey "GeneratorExternal.fileName=${O2DPG_ROOT}/MC/config/PWGDQ/external/generator/GeneratorCocktailPromptCharmoniaToElectronEvtGen_pp13TeV.C;GeneratorExternal.funcName=GeneratorCocktailPromptCharmoniaToElectronEvtGen_pp13TeV();Diamond.width[2]=6"         \
    -genBkg pythia8 -procBkg cdiff -colBkg pp --embedding -nb ${NBKGEVENTS} \
    -confKeyBkg "Diamond.width[2]=6" -interactionRate 2000

export FAIRMQ_IPC_PREFIX=./
# run workflow (highly-parallel)
${O2DPG_ROOT}/MC/bin/o2_dpg_workflow_runner.py -f workflow.json -tt aod  -jmax 1


