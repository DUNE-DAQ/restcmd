import argparse
import requests
import json

parser = argparse.ArgumentParser(description='POST command object from file to commanded endpoint.')
parser.add_argument('--host', type=str, default='localhost', help='target host/endpoint')
parser.add_argument('-p', '--port', type=int, default=12345, help='target port')
parser.add_argument('-a', '--answer-port', type=int, default=12333, help='answer to service listening on this port')
parser.add_argument('-r', '--route', type=str, default='command',help='target route on endpoint')
parser.add_argument('-f', '--file', type=str, help='file that contains command to be posted')

args = parser.parse_args()

url = 'http://'+args.host+':'+str(args.port)+'/'+args.route
print('Target url: ' + url)
headers = {'content-type': 'application/json', 'x-answer-port': args.answer_port}

cmd = {}
with open(args.file) as f:
  cmd = json.load(f)

response = requests.post(url, data=json.dumps(cmd), headers=headers)
print(response)
print(response.content)
