import logging
import asyncio
import subprocess

import aiocoap
import time

DEVICE_NAME = "rpiXKUDLA15"

from time import gmtime, strftime

logging.basicConfig(level=logging.INFO)

async def main():
	temp = subprocess.check_output('vcgencmd measure_temp', shell=True).decode()
	if len(temp) <= 7:
		print("ERROR: temperature string is invalid:", temp)
		return

	temp = temp[5:-3]
	print("INFO: Temp =", temp)

	data = "time=" + strftime("%Y-%m-%d %H:%M:%S", gmtime()) + ";tmpRaw=" + temp + ";dev=" + DEVICE_NAME
	print("INFO: Collected data to send:", data)

	protocol = await aiocoap.Context.create_client_context()
	request = aiocoap.Message(
		mtype=aiocoap.CON,
		code=aiocoap.POST,
		uri='coap://localhost',
		payload=data.encode()
	)

	try:
		response = await protocol.request(request).response
	except Exception as e:
		print('ERROR: Failed to fetch resource:', e)
	else:
		print('INFO: Sent ok, code:', response.code)

if __name__ == "__main__":
	while (True):
		asyncio.get_event_loop().run_until_complete(main())
		time.sleep(5)
