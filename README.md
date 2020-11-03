## restcmd - HTTP REST backend based CommandFacility
This implementation of the CommandFacility interface from cmdlib, is providing a way to send
commands to applications via HTTP. This is carried out by the small web-server that this plugin
carries. The server answers to HTTP POST requests, where the request body is the content of the
command itself. The package ships a really lightweight Python script to send commands from the
JSON files, that is located under the scripts directory.

This package is compatible with DUNE DAQ v1.2.1.

### Dependencies
The Pistache library is needed to build and run this plugin. At the moment, it's located under products_dev. 
The dependency is available under devevlopment products. Please pay attention to the build steps that include how to setup pistache from cvmfs.

### Building and running examples:

* create a software work area
  * see https://github.com/DUNE-DAQ/appfwk/wiki/Compiling-and-running-under-v1.2.1
* clone this repo into your work/development area
  * `cd <your_work_area>/sourcecode`
  * `git clone https://github.com/DUNE-DAQ/restcmd.git`)
* build the software
  * `. ./setup_build_environment`
  * `export PRODUCTS=/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev:$PRODUCTS`
  * `setup pistache v2020_10_07 -q e19:prof`
  * `./build_daq_software.sh --clean --install` 
* run the demos in another shell
  * `. ./setup_runtime_environment`
  * `restcmd_test_rest_app`
    * the application will terminate in 20 seconds
    * from another terminal, send commands via [curl](#sendcom)

## Running DAQ applications
    daq_application --commandFacility rest://localhost:12345

## <a name="sendcom"></a> Sending commands
Example of sending only a configuration command:

    curl --header "Content-Type: application/json" --header "X-Answer-Port: 12333" --request POST --data @only-conf.json http://localhost:12345/command

The command facility enforces the content type. The following will fail:

    curl --header "Content-Type: application/xml" --header "X-Answer-Port: 12333" --request POST --data @only-conf.json http://epdtdi103:12345/command

The scripts directory also contains a command sender application based on Python's Requests.
