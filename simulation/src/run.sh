#!/bin/bash
# TODO: Put flags in order to choose .ma and .ev
echo "Removing prevously used log files"
rm -f /tmp/cliOutput
rm -f /tmp/batteryChargeChanges
echo "Running simulation"
./bin/cd++ -e../eventGeneration/mergedData.ev -mmodels/controllerAndSolar.ma -oout/realData -lout/realData.log -t48:00:00:00 &> /tmp/cliOutput
echo "Filtering battery charge changes"
cat /tmp/cliOutput|grep NewCharge|less > /tmp/batteryChargeChanges
