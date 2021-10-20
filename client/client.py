import socket
import sys
import time
from datetime import datetime

name = str(sys.argv[1])
port = int(sys.argv[2])
period = int(sys.argv[3])

s = socket.socket()
s.connect(("localhost", port))

while True:
    now = datetime.now().isoformat(sep=' ', timespec='milliseconds') 
    s.send(bytes(f"[{now}] {name}\n", encoding="raw_unicode_escape"))
    time.sleep(period)
