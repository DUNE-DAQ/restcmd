## Running the DAQ application
    daq_application --commandFacility rest://localhost:12345

## Sending a command

    send-restcmd.py --file ../test/test-init.json

## Sending a command sequence
The script checks if there are multiple command objects to send. (File contains a JSON array of objects.)
One can also specify a wait time between sending commands.

    send-restcmd.py --file ../test/fdpc-commands.json --wait 3

## Sending command in interactive mode

    send-restcmd.py --file ../test/fdpc-commands.json --interactive
