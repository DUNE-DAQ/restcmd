#! python3

import argparse
import requests
import json
import time
import sys

parser = argparse.ArgumentParser(description='POST command object from file to commanded endpoint.')
parser.add_argument('--host', type=str, default='localhost', help='target host/endpoint')
parser.add_argument('-p', '--port', type=int, default=12345, help='target port')
parser.add_argument('-a', '--answer-port', type=int, default=12333, help='answer to service listening on this port')
parser.add_argument('-r', '--route', type=str, default='command', help='target route on endpoint')
parser.add_argument('-f', '--file', type=str, help='file that contains command to be posted')
parser.add_argument('-w', '--wait', type=int, default=2, help='seconds to wait between sending commands')
parser.add_argument('-i', '--interactive', dest='interactive', action='store_true', help='interactive mode')
parser.add_argument('--non-interactive', dest='interactive', action='store_false')
parser.set_defaults(interactive=False)

args = parser.parse_args()

url = 'http://'+args.host+':'+str(args.port)+'/'+args.route
print('Target url: ' + url)
headers = {'content-type': 'application/json', 'x-answer-port': bytes(args.answer_port)}

cmdstr = None
with open(args.file) as f:
  cmdstr = json.load(f)

if isinstance(cmdstr, dict):
  print('This is single command.')
  try:
    response = requests.post(url, data=json.dumps(cmdstr), headers=headers)
    print('Response code: %s with content: %s' % (str(response), str(response.content)))
  except:
    print('Failed to send due to: %s' % sys.exc_info()[0])
elif isinstance(cmdstr, list):
  print('This is a list of commands.')
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
        nextcmd = input('Press enter a command to send next: ')
        if nextcmd == "end":
          break
        print('Command to send: %s' % nextcmd)
        cmdobj = [cdict for cdict in cmdstr if cdict["id"] == nextcmd]
        if not cmdobj:
          print('Unrecognized command. (Not present int the command list?)')
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

print('Exiting...')
