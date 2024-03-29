#!/usr/bin/env python3

import argparse
import requests
import json
import time
import sys
from colorama import Fore, Back, Style

from flask import Flask, request, cli
from multiprocessing import Process, SimpleQueue
import logging

from rich.logging import RichHandler

logging.basicConfig(
    level="INFO",
    format="%(message)s",
    datefmt="[%X]",
    handlers=[RichHandler(rich_tracebacks=True)]
)


log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

OKGREEN = '\033[92m'
FAIL = '\033[91m'
ENDC = '\033[0m'

cli.show_server_banner = lambda *_: None

parser = argparse.ArgumentParser(description='POST command object from file to commanded endpoint.')
parser.add_argument('--host', type=str, default='localhost', help='target host/endpoint')
parser.add_argument('-p', '--port', type=int, default=12345, help='target port')
parser.add_argument('-a', '--answer-port', type=int, default=12333, help='answer to service listening on this port')
parser.add_argument('-o', '--answer-host', type=str, default=None, help='answer to service listening on this host')
parser.add_argument('-x', '--proxy', type=str, default=None, help='Use proxy for posting requests')

parser.add_argument('-r', '--route', type=str, default='command', help='target route on endpoint')
parser.add_argument('-f', '--file', type=str, required=True, help='file that contains command to be posted') # This should be an argument
group = parser.add_mutually_exclusive_group()
group.add_argument('-c', '--command', type=str, default=None, help='Only send command COMMAND, not all commands in file')
parser.add_argument('-w', '--wait', type=int, default=2, help='seconds to wait between sending commands')
group.add_argument('-i', '--interactive', dest='interactive', action='store_true', help='interactive mode')
parser.add_argument('--non-interactive', dest='interactive', action='store_false')
parser.set_defaults(interactive=False)

args = parser.parse_args()

reply_queue = SimpleQueue() # command reply queue


app = Flask(__name__)
@app.route('/response', methods = ['POST'])
def index():
  json = request.get_json(force=True)
  # enqueue command reply
  reply_queue.put(json)
  return 'Response received'

if __name__ == "__main__":
  flask_server = Process(target=app.run, kwargs={'port': args.answer_port})
  flask_server.start()

url = 'http://'+args.host+':'+str(args.port)+'/'+args.route
# print(f'Target url: {Fore.YELLOW+url+Style.RESET_ALL}')
logging.info(f'Target url: {url}')
headers = {'content-type': 'application/json', 'X-Answer-Port': str(args.answer_port)}
if not args.answer_host is None:
  headers['X-Answer-Host'] = args.answer_host

cmdstr = None
try:
  with open(args.file) as f:
    cmdstr = json.load(f)
except:
  # print(f"\nERROR: failed to open file '{str(args.file)}'.")
  logging.error(f"ERROR: failed to open file '{str(args.file)}'.")
  raise SystemExit(0)

if isinstance(cmdstr, dict):
  logging.info(f'Found single command in {args.file}.')
  try:
    response = requests.post(
      url, 
      data=json.dumps(cmdstr),
      headers=headers,
      proxies=({
        'http': f'socks5h://{args.proxy}',
        'https': f'socks5h://{args.proxy}'}
        if args.proxy else None
      )
    )
    logging.info(f'Response code: {str(response)} with content: {response.content.decode("utf-8")}')
  except Exception as e:
    logging.error(f'Failed to send due to: {e}')
elif isinstance(cmdstr, list):
  print(f'Found a list of commands in {args.file}')
  avacmds = [cdict['id'] for cdict in cmdstr if cdict["id"]]
  if args.command and not args.command in avacmds:
    raise SystemExit(f"\nERROR: No command '{args.command}' found in file '{str(args.file)}'.")
  if not args.interactive:
    for cmd in cmdstr:
      try:
        if args.command and cmd['id']!=args.command:
          continue
        response = requests.post(
          url, 
          data=json.dumps(cmd), 
          headers=headers,
          proxies=({
            'http': f'socks5h://{args.proxy}',
            'https': f'socks5h://{args.proxy}'}
            if args.proxy else None
          )
        )
        print(f'Response code: {str(response)} with content: {response.content.decode("utf-8")}')
        # get command reply from queue
        r = reply_queue.get()
        print("Reply:")
        print(f"  Application : {Fore.CYAN}{r['appname']}{Style.RESET_ALL}")
        print(f"  Command : {Fore.CYAN}{r['data']['cmdid']}{Style.RESET_ALL}")
        print(f"  Result  : {Fore.GREEN if r['success'] else Fore.RED}{r['result']}{Style.RESET_ALL}")
        time.sleep(args.wait)
      except:
        print('Failed to send due to: %s' % sys.exc_info()[0])
  else:
    print('\nInteractive mode. Type the ID of the next command to send, or type \'end\' to finish.')
    while True:
      try:
        print(f'\nAvailable commands: {", ".join([Fore.CYAN+c+Style.RESET_ALL for c in avacmds])}')
        nextcmd = input('command >> ')
        if nextcmd == "end":
          break
        cmdobj = [cdict for cdict in cmdstr if cdict["id"] == nextcmd]
        if not cmdobj:
          print('Unrecognized command %s. (Not present in the command list?)' % nextcmd)
        else:
          print(f'\nSending {Fore.CYAN+nextcmd+Style.RESET_ALL} command.')
          try:
            response = requests.post(
              url, 
              data=json.dumps(cmdobj[0]),
              headers=headers,
              proxies=({
                'http': f'socks5h://{args.proxy}',
                'https': f'socks5h://{args.proxy}'}
                if args.proxy else None
              )
            )
            print(f'Response code: {str(response)} with content: {response.content.decode("utf-8")}')
            # get command reply from queue
            r = reply_queue.get()
            print("Reply:")
            print(f"  Command : {Fore.CYAN}{r['data']['cmdid']}{Style.RESET_ALL}")
            print(f"  Result  : {Fore.GREEN if r['success'] else Fore.RED}{r['result']}{Style.RESET_ALL}")
          except:
            print('Failed to send due to: %s' % sys.exc_info()[0])
      except KeyboardInterrupt as ki:
        break
      except EOFError as e:
        break

print('Exiting...')
flask_server.terminate()
flask_server.join()
