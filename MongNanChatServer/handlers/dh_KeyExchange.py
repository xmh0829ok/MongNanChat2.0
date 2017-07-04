#-*- coding: utf-8 -*-
'''
 # author: 李贇
 # description: MongNanChat Project
'''
import random
from Crypto import Random
from Crypto.Hash import SHA
from Crypto.Cipher import PKCS1_v1_5 as Cipher_pkcs1_v1_5
from Crypto.Signature import PKCS1_v1_5 as Signature_pkcs1_v1_5
from Crypto.PublicKey import RSA
import base64
import string
import random

from ctypes import *
from binascii import b2a_hex,a2b_hex
dll = cdll.LoadLibrary('dh_KeyExchange.dll');
DH_KEY=c_ubyte * 16

class DH_KEY_Send(object):
    def __init__(self):
        self.public=DH_KEY()
        self.private=DH_KEY()
        dll.DH_generate_key_pair(self.public,self.private)

    def encrypt(self):
        data=b2a_hex(self.public)
        pubkey_path="private.pem"
        with open(pubkey_path) as f:
            key=f.read()
            rsakey=RSA.importKey(key)
            cipher=Cipher_pkcs1_v1_5.new(rsakey)
            cipher_data=base64.b64encode(cipher.encrypt(data))
        return cipher_data

class DH_KEY_Recv(object):
    def __init__(self,data):
        self.cipher_data=data

    def decrypt(self):
        prikey_path="private.pem"
        with open(prikey_path) as f:
            key=f.read()
            rsakey=RSA.importKey(key)
            cipher=Cipher_pkcs1_v1_5.new(rsakey)
            plain_data=cipher.decrypt(base64.b64decode(self.cipher_data), random.randint(0,999))
        self.public=DH_KEY()
        self.public=a2b_hex(plain_data)

class DH_SessionKey_Gen(object):
    def __init__(self,my_private,fri_public):
        sk=DH_KEY()
        dll.DH_generate_key_secret(sk,my_private,fri_public)
        self.sk=b2a_hex(sk)
'''
sk=sessionKey_En()
cipher_sk=sk.encrypt(afriendname)
//send
_sk=sessionKey_De(data)
sk=_sk.decrypt()
'''
