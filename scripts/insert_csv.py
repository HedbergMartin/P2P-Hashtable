import socket
import sys
import struct
import time

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Expected arguments are node_address, node_port, csv_file!")
        sys.exit(1)

    sock = socket.socket(type=socket.SOCK_DGRAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))

    with open(sys.argv[3]) as f:
        data = [l.strip().split(',') for l in f]
    for e in data:
        ssn, name, email = e
        msg = b'\x64' + struct.pack('13sB1s{0}sB7s{1}s'.format(len(name) + 1, len(email) + 1),
                                    str.encode(ssn, 'ascii'),
                                    len(name) + 1, b'\x00', str.encode(name, 'ascii'),
                                    len(email) + 1, b'\x00'*7,
                                    str.encode(email, 'ascii'))

        print("Inserting {} {} {}".format(ssn, email, name))
        sock.sendall(msg)
        print('Sent insert!')
        #time.sleep(0.5)
