import socket
import sys
import struct

if __name__ == '__main__':
    sock = socket.socket(type=socket.SOCK_DGRAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))

    asock = socket.socket(type=socket.SOCK_DGRAM)
    asock.bind(('', 0))

    msg = b'\x01\x00' + struct.pack('>H', asock.getsockname()[1])
    print(asock.getsockname())
    response = bytearray(20)

    while True:
        sock.sendall(msg)
        print('Sent request for node')
        asock.recv_into(response, 20)
        print(response)
        print('Response is:')
        print('Type: ' + str(response[0]))
        print('Address: ' + response[1:-2].decode('ascii'))
        print('Port: ' + str(struct.unpack('>H', response[18:])[0]))
        input()
