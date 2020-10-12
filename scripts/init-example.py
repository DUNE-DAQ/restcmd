import requests
import json

url = 'http://localhost:12345/command'

initCmd = {"data":{"addrdats":[{"data":{"addrdats":[{"data":{"capacity":10,"kind":"StdDeQueue"},"tn":{"name":"queue1","type":"StdDeQueue"}}]},"tn":{"name":"","type":"queue"}},{"data":{"data":{"output":"queue1"},"plugin":"FakeDataProducerDAQModule"},"tn":{"name":"source1","type":"module"}},{"data":{"data":{"input":"queue1"},"plugin":"FakeDataConsumerDAQModule"},"tn":{"name":"sink1","type":"module"}}]},"id":"init"}

headers = {'content-type': 'application/json', 'x-answer-port': 12333}

response = requests.post(url, data=json.dumps(initCmd), headers=headers)
print(response)
print(response.content)
