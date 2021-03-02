## Running the DAQ application
    daq_application --commandFacility rest://localhost:12345

## Sending a command

    send-restcmd.py --file ../test/test-init.json

## Sending a command sequence
The script checks if there are multiple command objects to send. (File contains a JSON array of objects.)
One can also specify a wait time between sending commands.

    send-restcmd.py --file ../test/fdpc-commands.json --wait 3

## Sending command in interactive mode
There is also an interactive mode. This requires typing the next command's ID from the set of commands that are available in the file:

    send-restcmd.py --file ../test/fdpc-commands.json --interactive

## Receiving command return

    recv-restcmd.py

starts a Flask server exposing a POST `/response` route on a configurable port (default 12333).

## Sending and receiving command replies

    send-recv-restcmd.py

The best of both scripts with a fancy "command reply queue".