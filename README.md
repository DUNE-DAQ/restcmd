## restcmd - HTTP REST backend based cmdlib CommandFacility

### Building and running examples:

* create a software work area
  * see https://github.com/DUNE-DAQ/appfwk/wiki/Compiling-and-running
* clone this repo
  * `cd <MyTopDir>`
  * `git clone https://github.com/roland-sipos/restcmd.git`)
* build the software
  * `. ./setup_build_environment`
  * `./build_daq_software.sh --clean --install --pkgname restcmd`
* run the demos in another shell
  * `. ./setup_runtime_environment`
  * `restcmd_test_rest_app`
    * the application will terminate in 20 seconds
    * from another terminal, send commands via [curl](#sendcom)

## Running DAQ applications
    daq_application --commandFacility rest://localhost:12345

## <a name="sendcom"></a> Sending commands
Example good command:

    curl --header "Content-Type: application/json" --header "X-Answer-Port: 12333" --request POST --data @only-conf.json http://localhost:12345/command

The command facility enforces the content type. The following will fail:

    curl --header "Content-Type: application/xml" --header "X-Answer-Port: 12333" --request POST --data @only-conf.json http://epdtdi103:12345/command
