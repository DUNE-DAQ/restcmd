import argparse
import requests
import json
import time

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

cmdobj = None
with open(args.file) as f:
  cmdobj = json.load(f)

if isinstance(cmdobj, dict):
  print("This is single command.")
  response = requests.post(url, data=json.dumps(cmdobj), headers=headers)
  print('Response code: %s with content: %s' % (str(response), str(response.content)))
elif isinstance(cmdobj, list):
  print("This is a command stream.")
  for cmd in cmdobj:
    response = requests.post(url, data=json.dumps(cmd), headers=headers)
    print('Response code: %s with content: %s' % (str(response), str(response.content)))
    if args.interactive:
      try:
        input("Press Enter to send the next command...")
      except (SyntaxError, NameError) as e:
        pass
    else:
      time.sleep(args.wait)

