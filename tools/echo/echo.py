# -*- coding: utf-8 -*
from socket import * 
import sys
import re
import ssl
import pprint
import traceback
import argparse
import signal, os
import thread
import select
import Queue

######################################
# Global params
######################################
HOST = "0.0.0.0"
PORT = "443"
FILE = "index.html"
#ssl_version = None
ssl_version = "tlsv1.2"
certfile = "./mosquitto_client_unauth.crt"
keyfile = "./mosquitto_client.key"
ciphers = None
hostname = "localhost"
option_test_switch = 1 # to test, change to 1

version_dict = {
    "tlsv1.0" : ssl.PROTOCOL_TLSv1,
    "tlsv1.1" : ssl.PROTOCOL_TLSv1_1,
    "tlsv1.2" : ssl.PROTOCOL_TLSv1_2,
    "sslv23"  : ssl.PROTOCOL_SSLv23,
    "sslv3"   : ssl.PROTOCOL_SSLv3,
}


###########################################################
# Param handler: get sslContext options through user input
###########################################################
for i in range(1, len(sys.argv)):
    arg = sys.argv[i]
    if re.match("[-]{,2}(tlsv|sslv)[0-9.]{,3}", arg, re.I):
        ssl_version = re.sub("-", "", arg)
    if re.match("[-]{,2}ciphers", arg, re.I):
        ciphers = sys.argv[i + 1]
    if re.match("[-]{,2}cacert", arg, re.I):
        certfile = sys.argv[i + 1]
    if re.match("^[0-9]{,3}\.[0-9]{,3}\.[0-9]{,3}\.[0-9]{,3}|localhost$", arg, re.I):
        HOST = arg
    if re.match("^[0-9]{,5}$", arg):
        PORT = arg
    if re.match("^[0-9a-zA-Z_/]+\.[0-9a-zA-Z-_/]+$", arg, re.I):
        FILE = arg

if option_test_switch == 1:
    print "ver=", ssl_version, "ciphers=",ciphers, "certfile=", certfile, \
            "keyfile=", keyfile, "HOST=", HOST, "PORT=", PORT, "FILE=", FILE


def handler(signum, frame):
    print('exit...')
    exit()

################################################################################
# Init and configure SSLContext, then Wrap socket in context
# Params: socket sock
#         str ssl_version
#         str keyfile
#         str certificate
#         str ciphers
# Note: For client-side sockets, the context construction is lazy 
#       if the underlying socket isn't connected yet, the context 
#       construction will be performed after connect() is called on the socket.
# Exception: SSLError
###############################################################################
def ssl_wrap_socket_server(ssl_version=None, keyfile=None, certfile=None, ciphers=None):

    #1. init a context with given version(if any)
    if ssl_version is not None and ssl_version in version_dict:
        #create a new SSL context with specified TLS version
        sslContext = ssl.SSLContext(version_dict[ssl_version])
        if option_test_switch == 1:
            print "ssl_version loaded!! =", ssl_version
    else:
        #if not specified, default
        sslContext = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        
    if ciphers is not None:
        #if specified, set certain ciphersuite
        sslContext.set_ciphers(ciphers)
        if option_test_switch == 1:
            print "ciphers loaded!! =", ciphers
    
    #server-side must load certfile and keyfile, so no if-else
    sslContext.load_cert_chain(certfile, keyfile)
    print "ssl loaded!! certfile=", certfile, "keyfile=", keyfile
    return sslContext

def ssl_wrap_socket_server_v1(sock, ssl_version=None, keyfile=None, certfile=None, ciphers=None):
    try:
        return ssl.wrap_socket(sock, keyfile, certfile, server_side=True, ssl_version = ssl.PROTOCOL_TLSv1_2)
    except ssl.SSLError as e:
        print "wrap socket failed!"
        print traceback.format_exc()

def ssl_wrap_socket_client(sock, ssl_version=None, keyfile=None, certfile=None, ciphers=None):
    try:
        #2.init a sslContext with given version (if any)
        if ssl_version is not None and ssl_version in version_dict:
            #create a new SSL context with specified TLS version
            sslContext = ssl.SSLContext(version_dict[ssl_version])
            if option_test_switch == 1:
                print "ssl_version loaded!! =", ssl_version
        else:
            #default
            sslContext = ssl.create_default_context()
        
        if ciphers is not None:
            #if specified, set certain ciphersuite
            sslContext.set_ciphers(ciphers)
            if option_test_switch == 1:
                print "ciphers loaded!! =", ciphers
        
        #3. set root certificate path
        if certfile is not None and keyfile is not None:
            #if specified, load speficied certificate file and private key file
            sslContext.verify_mode = ssl.CERT_REQUIRED
            sslContext.check_hostname = True
            sslContext.load_verify_locations(certfile, keyfile)
            if option_test_switch == 1:
                print "ssl loaded!! certfile=", certfile, "keyfile=", keyfile 
            return sslContext.wrap_socket(sock, server_hostname = hostname)
        else:
            #default certs
            sslContext.check_hostname = False
            sslContext.verify_mode = ssl.CERT_NONE
            sslContext.load_default_certs()
            return sslContext.wrap_socket(sock)
        
    except ssl.SSLError:
        print "wrap socket failed!"
        print traceback.format_exc()
        sock.close()
        sys.exit(-1)


######################################
# Connection related (from hw1)
######################################
def start_tls_client():
    #4.Prepare a client socket
    clientSocket = socket(AF_INET, SOCK_STREAM)

    #5.Wrapping the TCP socket with the SSL/TLS context
    sslSocket = ssl_wrap_socket_client(clientSocket, ssl_version, keyfile, certfile, ciphers)

    #6.connect to server
    sslSocket.connect((HOST, eval(PORT)))

    while True:
        try:
            message = raw_input(">")
            if message == "exit":
                print 'DONE: exit'
                break

            #Send the whole string
            sslSocket.sendall(message)

            #receive data
            reply = sslSocket.recv(1024)
            print reply
            continue
            while sslSocket.recv(1024):
                reply = sslSocket.recv(1024)
                if(certfile is None):
                    print reply
                else:
                    #part 3-print certificate
                    pprint.pprint(sslSocket.getpeercert())

        except socket.error:
            #Send failed
            print 'ERROR: Send failed'
            sslSocket.shutdown(SHUT_RDWR)
            break

    #7.close the socket
    sslSocket.close()
    sys.exit(0)

def start_client():
    #4.Prepare a client socket
    clientSocket = socket(AF_INET, SOCK_STREAM)

    #6.connect to server
    clientSocket.connect((HOST, eval(PORT)))

    while True:
        try:
            message = raw_input(">")
            if message == "exit":
                print 'DONE: exit'
                break

            #Send the whole string
            clientSocket.sendall(message)

            #receive data
            reply = clientSocket.recv(1024)
            print reply
            continue
            while clientSocket.recv(1024):
                reply = clientSocket.recv(1024)
                if(certfile is None):
                    print reply
                else:
                    #part 3-print certificate
                    pprint.pprint(clientSocket.getpeercert())

        except socket.error:
            #Send failed
            print 'ERROR: Send failed'
            clientSocket.shutdown(SHUT_RDWR)
            break

    #7.close the socket
    clientSocket.close()
    sys.exit(0)

def tls_server_async_worker(sslContext, newSocket):
    connectionSocket = None
    try:
        newSocket.setblocking(0)
        connectionSocket = sslContext.wrap_socket(newSocket, do_handshake_on_connect = False)
    except ssl.SSLError as e:
        print "wrap socket SSLError!"
        print traceback.format_exc()
    except:
        print "wrap socket error!"
        print traceback.format_exc()

    if not connectionSocket:
        return

    id = thread.get_ident()
    sendQueue = Queue.Queue()
    while True:
        try:
            connectionSocket.do_handshake()
        except ssl.SSLWantReadError:
            select.select([connectionSocket], [], [])
            message = connectionSocket.recv(1024)
            if len(message) == 0:
                raise IOError
            print "[", id, "]", "message=", message
            sendQueue.put(message)
        except ssl.SSLWantWriteError:
            select.select([], [connectionSocket], [])
            #Send the content of the requested file to the client 
            message = sendQueue.get()
            if not message:
                connectionSocket.send(message)
        except IOError:
            #Close client socket
            connectionSocket.shutdown(SHUT_RDWR)
            connectionSocket.close()
            break

def tls_server_worker(sslContext, newSocket):
    connectionSocket = None
    try:
        connectionSocket = sslContext.wrap_socket(newSocket, server_side = True)
    except ssl.SSLError as e:
        print "wrap socket SSLError!"
        print traceback.format_exc()
    except:
        print "wrap socket error!"
        print traceback.format_exc()

    if not connectionSocket:
        return

    id = thread.get_ident()
    length = 0
    totalLength = 0
    while True:
        try:
            message = connectionSocket.recv(1460)
            length = len(message)
            if length == 0:
                raise IOError
            totalLength += length
            print "[", id, ":", totalLength, "]", "message=", message
            connectionSocket.send(message)
        except IOError:
            #Close client socket
            connectionSocket.shutdown(SHUT_RDWR)
            connectionSocket.close()
            break

def start_tls_server():
    #4. Prepare a sever socket 
    serverSocket = socket(AF_INET, SOCK_STREAM) 
    serverSocket.bind((HOST, eval(PORT)))
    serverSocket.listen(10)

    sslContext = ssl_wrap_socket_server(ssl_version, keyfile, certfile, ciphers)

    #######################################################
    # Init socket and start connection (from hw1)
    #######################################################
    while True:
        #Establish the connection
        print 'Ready to serve...'
        try:
            newSocket, addr = serverSocket.accept()
            #thread.start_new_thread(tls_server_async_worker, (sslContext, newSocket,))
            thread.start_new_thread(tls_server_worker, (sslContext, newSocket,))
        except KeyboardInterrupt:
            break

    serverSocket.close()
    sys.exit(0)

def server_worker(connectionSocket):
    while True:
        try:
            message = connectionSocket.recv(1460)
            if len(message) == 0:
                raise IOError
            print "message=", message

            #Send the content of the requested file to the client 
            connectionSocket.send(message)
        except IOError:
            #Close client socket
            connectionSocket.shutdown(SHUT_RDWR)
            connectionSocket.close()
            break


def start_server():
    #4. Prepare a sever socket 
    serverSocket = socket(AF_INET, SOCK_STREAM) 
    serverSocket.bind((HOST, eval(PORT)))
    serverSocket.listen(10)

    #######################################################
    # Init socket and start connection (from hw1)
    #######################################################
    while True:
        #Establish the connection
        print 'Ready to serve...'
        try:
            connectionSocket, addr = serverSocket.accept()
            if not connectionSocket:
                continue
            thread.start_new_thread(server_worker, (connectionSocket,))
        except KeyboardInterrupt:
            break

    serverSocket.close()
    sys.exit(0)

parser = argparse.ArgumentParser(description="your script description")
parser.add_argument('--tls', '-t', action='store_true', help='tls mode')
parser.add_argument('--server', '-s', action='store_true', help='server mode')
parser.add_argument('--port', '-p', help='port number')
parser.add_argument('--address', '-a', help='address name')
parser.add_argument('--cert', '-c', help='cert name')

args = parser.parse_args()
signal.signal(signal.SIGINT, handler)
signal.signal(signal.SIGTERM, handler)
if args.server:
    if args.tls:
        print "tls server mode!"
        start_tls_server()
    else:
        print "normal server mode!"
        start_server()
else:
    if args.tls:
        print "tls client mode!"
        start_tls_client()
    else:
        print "normal client mode!"
        start_client()

#python echo.py -a 174.129.224.73 -c ./websocket.cer
#python echo.py -a 174.129.224.73 -c ./websocket.cert.cer
#python echo.py -a 174.129.224.73 -c ./websocket.rootca.cer
#python echo.py -a 192.168.44.7 -p 8883
#python echo.py -a 34.218.25.197 -p 8883
#python echo.py -s -p 8883
#python echo.py -a 127.0.0.1 -t -p 443
#python echo.py -s -t -p 443