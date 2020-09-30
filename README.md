# ccm-ctrl
appfwk CommandFacility implementation based on DUNE CCM interfaces and libraries

## Running the application
    daq_application --commandFacility rest://localhost:12345

## Sending commands
    curl --header "Content-Type: application/json" --header "X-Answer-Port: 12333" --request POST --data @only-conf.json http://localhost:12345/command
