#!/usr/bin/env python3

import argparse
from flask import Flask, request

parser = argparse.ArgumentParser(description='')
parser.add_argument('-a', '--answer-port', type=int, default=12333, help='listening port for command return')

args = parser.parse_args()

app = Flask(__name__)

@app.route('/response', methods = ['POST'])
def index():
  json = request.get_json(force=True)
  print("Command: ", json["data"]["cmdid"])
  print("Success:", json["success"])
  print("Result:", json["result"])
  return 'Response received'

if __name__ == "__main__":
  app.run(port=args.answer_port)

