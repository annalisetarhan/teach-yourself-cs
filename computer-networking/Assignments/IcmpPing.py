from socket import *
import os
import sys
import struct
import time
import select
import binascii
ICMP_ECHO_REQUEST = 8

def checksum(string):
    csum = 0
    countTo = (len(string) / 2) * 2
    count = 0
    while count < countTo:
        thisVal = ord(string[count+1])*256 + ord(string[count])
        csum = csum + thisVal
        csum = csum & 0xffffffff
        count = count + 2
        
    if countTo < len(string):
        csum = csum + ord(string[len(string)-1])
        csum = csum & 0xffffffff
    
    csum = (csum >> 16) + (csum & 0xffff)
    csum = csum + (csum >> 16)
    answer = ~csum
    answer = answer & 0xffff
    answer = answer >> 8 | (answer << 8 & 0xff00)
    return answer
    
def receiveOnePing(mySocket, ID, timeout, destAddr):
    timeLeft = timeout
    while True:
        startedSelect = time.time()
        whatReady = select.select([mySocket], [], [], timeLeft)
        howLongInSelect = (time.time() - startedSelect)
        if whatReady[0] == []:
            return "Request timed out"
        timeReceived = time.time()
        recPacket, addr = mySocket.recvfrom(1024)
        
        # MY CODE STARTS HERE
        
        # ICMP header (no options in IP header, so 20 bytes)
        icmpHeader = recPacket[20:28]
        type, code, checksum, identifier, sequenceNum = struct.unpack("bbHHh", icmpHeader)
        # Normally, this data would be verified against the checksum

        # ICMP data
        timeSent = struct.unpack("d", recPacket[28:])[0]
        
        roundTripTime = timeReceived - timeSent
                
        print "Time sent: ", timeSent
        print "Time received: ", timeReceived
        print "Round trip time: ", roundTripTime
        
        return roundTripTime
        
        # MY CODE ENDS HERE
        
        timeLeft = timeLeft - howLongInSelect
        if timeLeft <= 0:
            return "Request timed out"
            
def sendOnePing(mySocket, destAddr, ID):
    # Header is type(8), code(8), checksum(16), id(16), sequence(16)
    myChecksum = 0  # Dummy header with checksum = 0
    # struct -- Interpret strings as packed binary data
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, myChecksum, ID, 1)
    data = struct.pack("d", time.time())
    # Calculate the checksum on the data and the dummy header
    myChecksum = checksum(str(header+data))
    
    # Get the right checksum, and put in the header
    if sys.platform =='darwin':
        myChecksum = htons(myChecksum)&0xffff
    else:
        myChecksum = htons(myChecksum)
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, myChecksum, ID, 1)
    packet = header + data
    
    mySocket.sendto(packet, (destAddr,1)) # AF_INET address must be tuple, not str
    
def doOnePing(destAddr, timeout):
    icmp = getprotobyname("icmp")
    mySocket = socket(AF_INET, SOCK_RAW, icmp)
    myID = os.getpid() & 0xffff
    sendOnePing(mySocket, destAddr, myID)
    delay = receiveOnePing(mySocket, myID, timeout, destAddr)
    mySocket.close()
    return delay

def ping(host, timeout=1):
    # timeout=1 means that if one second passes without a replay, assume ping or pong is lost
    dest = gethostbyname(host)
    print("Pinging " + dest + " using Python:")
    print("")
    # Send ping requests to a server separated by approximately one second
    while True:
        delay = doOnePing(dest, timeout)
        print(delay)
        time.sleep(1)
    return delay

ping("127.0.0.1")
