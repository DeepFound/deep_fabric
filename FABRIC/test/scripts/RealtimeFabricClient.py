#!/usr/bin/env python

__author__ = "Deep Grant"

import socket
import select
import sys
import time
import argparse
import string
import uuid
import json
import pdb

MAXLEN = 64*1024

def main():
    _parser    = argparse.ArgumentParser(description='Deep Realtime Fabric Client.')
    _parser.add_argument('--server',
                         action='store',
                         dest='serverId',
                         help='FABRIC ID.',
                         type=int)

    _parser.add_argument('--api-ip',
                         action='store',
                         dest='apiIp',
                         help='The IP Address of the Fabrics API.')

    _parser.add_argument('--get-topo',
                         action='store_true',
                         default=False,
                         dest='getTopo',
                         help='Get the discovered FABRIC topology')

    _addPeerGrp = _parser.add_argument_group('peer')
    _addPeerGrp.add_argument('--add-peer',
                             action='store',
                             dest='addPeer',
                             help='Address for the peer. e.g. tcp://2.2.2.2:10000')

    _addPeerGrp.add_argument('--del-peer',
                             action='store',
                             dest='delPeer',
                             help='UUID of the Peer to delete.')

    _args       = _parser.parse_args()
    _commandStr = None

    if None == _args.serverId:
        print >>sys.stderr, 'No Server/Fabric ID set. Use --server to specify.'
        sys.exit(-1)

    if None == _args.apiIp:
        print >>sys.stderr, 'No IP Address specified for the API. uses: --apiIp'
        sys.exit(-2)

    if None != _args.addPeer:
        _command = {
            'cmd'  : 'POST',
            'path' : '/fabric/peer',
            'data' : {
                'type'       : 'peer',
                'id'         : str(uuid.uuid1()),
                'attributes' : {
                    'address' : _args.addPeer
                }
            }
        }

    elif None != _args.delPeer:
        _command = {
            'cmd'  : 'DELETE',
            'path' : '/fabric/peer/%s' % (_args.delPeer,),
        }

    elif True == _args.getTopo:
        _command = {
            'cmd'  : 'GET',
            'path' : '/fabric/topology'
        }

    else:
        sys.exit(0)

    _commandStr = json.dumps(_command)
    print _commandStr

    # Domain socket
    #_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    # TCP AF_INET socket
    _sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    #_serverAddress = '/tmp/RealtimeFabric.%d' % (_args.serverId,)

    _data = None

    try:
        #_sock.connect(_serverAddress)
        _sock.connect((_args.apiIp, 8080+_args.serverId));
    except socket.error, msg:
        print >>sys.stderr, msg
        sys.exit(-2)

    try:
        _sock.sendall(_commandStr)

        _epoll = select.epoll(1)
        _epoll.register(_sock.fileno(), select.EPOLLIN | select.EPOLLET)

        _retVal = False
        while False == _retVal:
            _events = _epoll.poll(timeout=1.0)
            for _sd, _event in _events:
                if _event & select.EPOLLIN:
                    _data = _sock.recv(MAXLEN)
                    _retVal = True
                elif _event & select.EPOLLHUP:
                    _retVal = True
                    break

    finally:
        _sock.close()

    _actualData = json.loads(_data[:-1])
    print json.dumps(_actualData,
                     sort_keys=True,
                     indent=4,
                     separators=(',', ' : '))

if __name__ == "__main__":
    main()
