## Running the DAQ application
    daq_application --commandFacility rest://localhost:12345

## Sending a command
    python send-cmd.py --file only-init.json

## Sending a command sequence
The script checks if there are multiple command objects to send. (File contains a JSON array of objects.)
One can also specify a wait time between sending commands.

    python send-cmd.py --file fdpc-job.json -w 3

