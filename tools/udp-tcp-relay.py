#!/usr/bin/env python3
"""Carry UDP datagrams (cube's ENet traffic) over a TCP stream, e.g. an SSH tunnel.

Each datagram is sent on the stream with a 2 byte big-endian length in front. Every UDP
client gets its own TCP connection, so the server sees one UDP peer per client.

  near the clients:  udp-tcp-relay.py listen  0.0.0.0:28765 127.0.0.1:38765
  near the server:   udp-tcp-relay.py connect 127.0.0.1:38765 127.0.0.1:28765

with an SSH tunnel joining the two TCP ports, e.g. ssh -R 38765:127.0.0.1:38765 <client side>.
"""
import argparse
import asyncio
import struct
import sys

IDLE_TIMEOUT = 120


def addr(s):
    host, _, port = s.rpartition(':')
    return host or '0.0.0.0', int(port)


def log(*args):
    print(*args, file=sys.stderr, flush=True)


async def read_frames(reader, on_datagram):
    try:
        while True:
            n, = struct.unpack('!H', await reader.readexactly(2))
            on_datagram(await reader.readexactly(n))
    except (asyncio.IncompleteReadError, ConnectionError):
        pass


def write_frame(writer, data):
    writer.write(struct.pack('!H', len(data)) + data)


class Listen(asyncio.DatagramProtocol):
    """UDP clients -> one TCP connection each."""

    def __init__(self, tcp_addr):
        self.tcp_addr = tcp_addr
        self.sessions = {}

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data, peer):
        session = self.sessions.get(peer)
        if session is None:
            session = self.sessions[peer] = {'queue': [data], 'writer': None, 'last': 0}
            asyncio.ensure_future(self.run(peer, session))
        elif session['writer'] is None:
            session['queue'].append(data)
        else:
            write_frame(session['writer'], data)
        session['last'] = asyncio.get_running_loop().time()

    async def run(self, peer, session):
        log('client', peer)
        try:
            reader, writer = await asyncio.open_connection(*self.tcp_addr)
        except OSError as e:
            log('connect', self.tcp_addr, e)
            del self.sessions[peer]
            return
        for data in session['queue']:
            write_frame(writer, data)
        session['writer'] = writer
        reading = asyncio.ensure_future(read_frames(reader, lambda d: self.transport.sendto(d, peer)))
        loop = asyncio.get_running_loop()
        while not reading.done() and loop.time() - session['last'] < IDLE_TIMEOUT:
            await asyncio.wait([reading], timeout=5)
        reading.cancel()
        writer.close()
        del self.sessions[peer]
        log('client gone', peer)


class Upstream(asyncio.DatagramProtocol):
    def __init__(self, writer):
        self.writer = writer

    def datagram_received(self, data, peer):
        write_frame(self.writer, data)


async def connect_client(reader, writer, udp_addr):
    loop = asyncio.get_running_loop()
    log('tunnel connection')
    transport, _ = await loop.create_datagram_endpoint(lambda: Upstream(writer), remote_addr=udp_addr)
    await read_frames(reader, transport.sendto)
    transport.close()
    writer.close()
    log('tunnel connection closed')


async def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('mode', choices=['listen', 'connect'])
    p.add_argument('src', type=addr, help='listen: UDP address to listen on; connect: TCP address to listen on')
    p.add_argument('dst', type=addr, help='listen: TCP address to connect to; connect: UDP address to send to')
    args = p.parse_args()
    loop = asyncio.get_running_loop()
    if args.mode == 'listen':
        await loop.create_datagram_endpoint(lambda: Listen(args.dst), local_addr=args.src)
        log('udp', args.src, '-> tcp', args.dst)
        await asyncio.Event().wait()
    else:
        server = await asyncio.start_server(lambda r, w: connect_client(r, w, args.dst), *args.src)
        log('tcp', args.src, '-> udp', args.dst)
        await server.serve_forever()


if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
