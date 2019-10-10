import socket
import sys
import struct

if __name__ == '__main__':
    sock = socket.socket(type=socket.SOCK_DGRAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))

    asock = socket.socket(type=socket.SOCK_DGRAM)
    asock.bind(('', 0))

    msg = b'\xc8\x00' + struct.pack('>H', asock.getsockname()[1])
    print(asock.getsockname())
    response = bytearray(18)

    while True:
        sock.sendall(msg)
        print('Sent stun request')
        asock.recv_into(response, 16)
        print('Address is: ' + response[1:].decode('ascii'))
        input()
