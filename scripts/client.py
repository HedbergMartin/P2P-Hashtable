import socket
import sys
import time
import struct

if __name__ == '__main__':
    sock = socket.socket(type=socket.SOCK_DGRAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))
    asock = socket.socket(type=socket.SOCK_DGRAM)
    asock.bind(('', 0))
    print('Listening on :{0}'.format(asock.getsockname()[1]))
    msg = b'\x00\x00' + struct.pack(">H", asock.getsockname()[1])
    while True:
        sock.send(msg)
        print('pong')
        time.sleep(1)
