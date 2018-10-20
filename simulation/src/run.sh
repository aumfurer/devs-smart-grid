#!/bin/bash
# TODO: Put flags in order to choose .ma and .ev

SIMULATOR=./bin/cd++
OUTPUT_FOLDER=out

if [[ $# -eq 0 ]]; then
    echo "$0 modelFile eventsFile simulationName endingTime=7days"
    exit 0
fi

ENDING_TIME=$4
if [[ $4 -eq "" ]]; then
    ENDING_TIME=168:00:00:00
fi

SIMULATION_NAME=$3
MODEL_FILE=$1
EVENTS_FILE=$2

echo "Removing prevously used log files"
# TODO
echo "Running simulation"
echo -n "Command being run: "
echo "$SIMULATOR -e$EVENTS_FILE -m$MODEL_FILE -o$OUTPUT_FOLDER/$SIMULATION_NAME -l$OUTPUT_FOLDER/$SIMULATION_NAME.log -t$ENDING_TIME"

$SIMULATOR -e$EVENTS_FILE -m$MODEL_FILE -o$OUTPUT_FOLDER/$SIMULATION_NAME -l$OUTPUT_FOLDER/$SIMULATION_NAME.log -t$ENDING_TIME &> /tmp/cliOutput
echo "Filtering battery charge changes"
cat /tmp/cliOutput|grep NewCharge|less > $OUTPUT_FOLDER/batteryChargeChanges_$SIMULATION_NAME
