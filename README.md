# restcmd
cmdlib CommandFacility implementation based on Pistache HTTP REST server and client

## Running the application
    daq_application --commandFacility rest://localhost:12345

## Sending commands
    curl --header "Content-Type: application/json" --header "X-Answer-Port: 12333" --request POST --data @only-conf.json http://localhost:12345/command
