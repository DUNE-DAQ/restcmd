#!/usr/bin/env python3

import argparse
import requests
import json
import time
import sys

from flask import Flask, request
from multiprocessing import Process

parser = argparse.ArgumentParser(description='POST command object from file to commanded endpoint.')
parser.add_argument('--host', type=str, default='localhost', help='target host/endpoint')
parser.add_argument('-p', '--port', type=int, default=12345, help='target port')
parser.add_argument('-a', '--answer-port', type=int, default=12333, help='answer to service listening on this port')
parser.add_argument('-r', '--route', type=str, default='command', help='target route on endpoint')
parser.add_argument('-f', '--file', type=str, required=True, help='file that contains command to be posted') # This should be an argument
parser.add_argument('-w', '--wait', type=int, default=2, help='seconds to wait between sending commands')
parser.add_argument('-i', '--interactive', dest='interactive', action='store_true', help='interactive mode')
parser.add_argument('--non-interactive', dest='interactive', action='store_false')
parser.set_defaults(interactive=False)

args = parser.parse_args()

app = Flask(__name__)

@app.route('/response', methods = ['POST'])
def index():
  json = request.get_json(force=True)
  print("Result: ", json["result"])
  print("of command: ", json["command"])
  return 'Response received'

def FlaskApp():
  app.run(port=args.answer_port)

if __name__ == "__main__":
  flask_server = Process(target=FlaskApp)
  flask_server.start()

url = 'http://'+args.host+':'+str(args.port)+'/'+args.route
print('Target url: ' + url)
headers = {'content-type': 'application/json', 'X-Answer-Port': str(args.answer_port)}

cmdstr = None
try:
  with open(args.file) as f:
    cmdstr = json.load(f)
except:
  print(f"\nERROR: failed to open file '{str(args.file)}'.")
  raise SystemExit(0)

if isinstance(cmdstr, dict):
  print('This is single command.')
  try:
    response = requests.post(url, data=json.dumps(cmdstr), headers=headers)
    print('Response code: %s with content: %s' % (str(response), str(response.content)))
  except:
    print('Failed to send due to: %s' % sys.exc_info()[0])
elif isinstance(cmdstr, list):
  print('This is a list of commands.')
  avacmds = [cdict['id'] for cdict in cmdstr if cdict["id"]]
  if not args.interactive:
    for cmd in cmdstr:
      try:
        response = requests.post(url, data=json.dumps(cmd), headers=headers)
        print('Response code: %s with content: %s' % (str(response), str(response.content)))
        time.sleep(args.wait)
      except:
        print('Failed to send due to: %s' % sys.exc_info()[0])
  else:
    print('Interactive mode. Type the ID of the next command to send, or type \'end\' to finish.')
    while True:
      try:
        print('\nAvailable commands: %s' % avacmds)
        nextcmd = input('Press enter a command to send next: ')
        if nextcmd == "end":
          break
        cmdobj = [cdict for cdict in cmdstr if cdict["id"] == nextcmd]
        if not cmdobj:
          print('Unrecognized command %s. (Not present in the command list?)' % nextcmd)
        else:
          print('Sending %s command.' % nextcmd)
          try: 
            response = requests.post(url, data=json.dumps(cmdobj[0]), headers=headers)
            print('Response code: %s with content: %s' % (str(response), str(response.content))) 
          except:
            print('Failed to send due to: %s' % sys.exc_info()[0])
      except KeyboardInterrupt as ki:
        break
      except EOFError as e:
        break

print("Wait for response...")
time.sleep(0.5)
print('Exiting...')
flask_server.terminate()
flask_server.join()

