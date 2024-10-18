# Written 2024-05-28 for my thesis

import struct
from scapy.all import *
import json
import sys
import datetime

class Bin:
    def __init__(self, bin: bytes):
        self.bin = bin

    def __bool__(self) -> bool:
        return bool(self.bin)

    def consume_literal(self, expected: bytes):
        assert len(expected) <= len(self.bin)
        actual = self.bin[:len(expected)]
        assert actual == expected, f"Expected {expected}, got {actual}"
        self.bin = self.bin[len(expected):]

    def consume_uint(self, size) -> int:
        assert size//8 <= len(self.bin)
        map = { 8: "B", 16: ">H", 32: ">I", 64: ">Q" }
        assert size in map.keys()
        value, *_ = struct.unpack(map[size], self.bin[:size//8])
        self.bin = self.bin[size//8:]
        return value

    def consume_bytes(self, size) -> bytes:
        assert size <= len(self.bin)
        result, self.bin = self.bin[:size], self.bin[size:]
        return result

    def consume_str(self, size) -> str:
        assert size <= len(self.bin)
        result, self.bin = self.bin[:size], self.bin[size:]
        return result.decode()

    def split(self, pos):
        assert pos <= len(self.bin), f"{pos}"
        return self.__class__(self.bin[:pos]), self.__class__(self.bin[pos:])

    def __str__(self):
        return str(self.bin)

    def __len__(self):
        return len(self.bin)

def decode_timeline(payload: Bin) -> dict:
    timeline = {}
    timeline["tempo"] = payload.consume_uint(64)
    timeline["beat_origin"] = payload.consume_uint(64)
    timeline["time_origin"] = payload.consume_uint(64)
    assert not payload
    return { "timeline": timeline }

def decode_session(payload: Bin) -> dict:
    result = { "session": { "session_id": payload.consume_str(8) } }
    assert not payload
    return result

def decode_start_stop(payload: Bin) -> dict:
    result = {}
    result["is_playing"] = bool(payload.consume_uint(8))
    result["time"] = payload.consume_uint(64)
    result["timestamp"] = payload.consume_uint(64)
    assert not payload
    return result

def decode_measurement_endpoint_v4(payload: Bin) -> dict:
    result = {}
    result["address"] = '.'.join(str(c) for c in payload.consume_bytes(4))
    result["port"] = payload.consume_uint(16)
    assert not payload
    return result


PAYLOAD_DECODER = {
    b'tmln': decode_timeline,
    b'sess': decode_session,
    b'stst': decode_start_stop,
    b'mep4': decode_measurement_endpoint_v4,
}

def decode_link(message: Bin):
    result = {}

    message.consume_literal(b'_asdp_v')
    result['version'] = message.consume_uint(8)

    MESSAGE_TYPES = {0: "Invalid", 1: "Alive", 2: "Response", 3: "ByeBye"}
    message_type = message.consume_uint(8)
    assert message_type in MESSAGE_TYPES.keys()
    result['message_type'] = MESSAGE_TYPES[message_type]

    result['ttl']              = message.consume_uint(8)
    result['session_group_id'] = message.consume_uint(16)
    result['client_id']        = message.consume_str(8)

    while message:
        payload_type = message.consume_bytes(4)
        assert payload_type in PAYLOAD_DECODER.keys(), f"Unkown payload_type: {payload_type}"
        payload_length = message.consume_uint(32)
        payload, message = message.split(payload_length)
        result.update(PAYLOAD_DECODER[payload_type](payload))
    return result

EPOCH = datetime.datetime(1970, 1, 1)

if __name__ == "__main__":
    cap = rdpcap('link.pcap')
    for packet in cap:
        try:
            if packet[IP].dst == '224.76.78.75':
                link = decode_link(Bin(bytes(packet[UDP].payload)))
                info = { "link": link, "time": str(EPOCH + datetime.timedelta(seconds=float(packet.time))), "src": str(packet[IP].src) }
                json.dump(info, fp=sys.stdout)
        except IndexError:
            pass

