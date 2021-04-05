import datetime
import logging

import asyncio

import aiocoap.resource as resource
import aiocoap
import csv

def newEntry():
	return  {
		"measurement time" : "",
		"receive time" : datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
		"dev" : "",
		"tmpRaw" : "",
		"tmpKlm" : "",
		"lux" : "",
		"hal" : "",
		"cpu0" : "",
		"cpu1" : "",
	}

class RootResource(resource.Resource):
	"""Resource supports PUT method for data collecting"""

	def __init__(self, writer):
		self.writer = writer

	async def render_post(self, request):
		data = request.payload.decode().split(';')
		print('POST payload:', data)

		entry = newEntry()

		for keyVal in data:
			try:
				key, val = keyVal.split('=')
				if key == "time":
					entry["measurement time"] = val
				elif key != "receive time" and key in entry:
					entry[key] = val
				else:
					print('ERROR: POST payload ignoring unknown key: ', keyVal)
			except:
				print('ERROR: POST payload has invalid entry: ', keyVal)

		self.writer.writerow(entry.values());

		return aiocoap.Message(mtype=aiocoap.ACK, code=aiocoap.CHANGED)

logging.basicConfig(level=logging.INFO)
logging.getLogger("coap-server").setLevel(logging.DEBUG)

def main():
	# Open output file, truncate prev content
	with open('data.csv', 'w', newline='') as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(newEntry().keys())
		# Resource creation
		root = resource.Site()
		root.add_resource([], RootResource(writer))
		asyncio.Task(aiocoap.Context.create_server_context(root))
		asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
	main()
