
'''
Example parsing TSCBM formatted method with Python.
'''

def b2i(byte):
    '''
    Convert bytes to an integer.
    '''
    return int.from_bytes(byte, byteorder='big')

def readB(byte, offset, size):
    '''
    Split an array of bytes, and return the next location after this is removed.
    '''
    return byte[offset:offset+size], offset+size

def hextobin(hexval):
    '''
    Takes a string representation of hex data with
    arbitrary length and converts to string representation
    of binary.  Includes padding 0s
    '''
    thelen = len(hexval)*4
    binval = bin(int(hexval, 16))[2:]
    while ((len(binval)) < thelen):
        binval = '0' + binval
    return binval

def parse_TSCBM(id, bytes, time_now):
    '''
    Takes apart a TSCBM formatted SPaT message and makes a JSON object
    '''
    #byte 0: DynObj13 response byte (0xcd)
    #This is always CD

    #byte 1: number of phase/overlap blocks below (16) 
    block = b2i(bytes[1:2]) #The number of blocks of phase/overlap below 16.
    off = 2 #Start at offset 2 and loop until Range(blocks)
    phases=[]
    for i in range(block):
        #0x01 (phase#)	(1 byte) 
        outP, off = readB(bytes, off, 1) #2
        outVMin, off = readB(bytes, off, 2) #3,4
        outVMax, off = readB(bytes, off, 2) #5, 6
        outPMin, off = readB(bytes, off, 2) #7, 8
        outPMax, off = readB(bytes, off, 2) #9, 10
        outOMin, off = readB(bytes, off, 2) #11, 12 Overlap min
        outOMax, off = readB(bytes, off, 2) #13, 14 Overlap Max

        phase = {
            "phase": b2i(outP),
            "color": 'RED',
            "flash": False,
            "vehTimeMin": b2i(outVMin),
            "vehTimeMax": b2i(outVMax),
            "pedTimeMin": b2i(outPMin),
            "pedTimeMax": b2i(outPMax),
        }
        phases.append(phase)

    # bytes 210-215: PhaseStatusReds, Yellows, Greens	(2 bytes bit-mapped for phases 1-16)
    outR, off = readB(bytes, off, 2)
    outY, off = readB(bytes, off, 2)
    outG, off = readB(bytes, off, 2)

    # # bytes 216-221: PhaseStatusDontWalks, PhaseStatusPedClears, PhaseStatusWalks (2 bytes bit-mapped for phases 1-16)
    outDW, off = readB(bytes, 216, 2)
    outPC, off = readB(bytes, 218, 2)
    outW, off = readB(bytes, 220, 2)

    # bytes 222-227: OverlapStatusReds, OverlapStatusYellows, OverlapStatusGreens (2 bytes bit-mapped for overlaps 1-16)
    # bytes 228-229: FlashingOutputPhaseStatus	(2 bytes bit-mapped for phases 1-16)
    outFl, off = readB(bytes, off, 2)

    # bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    # byte 232: IntersectionStatus (1 byte) (bit-coded byte) 
    # bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    # byte 232: IntersectionStatus (1 byte) (bit-coded byte) 

    outInt, off = readB(bytes, 232, 1)
    # Byte 233: TimebaseAscActionStatus (1 byte)  	(current action plan)                       
    # byte 234: DiscontinuousChangeFlag (1 byte)          (upper 5 bits are msg version #2, 0b00010XXX)     
    # byte 235: MessageSequenceCounter (1 byte)           (lower byte of up-time deci-seconds) 
    # Byte 236-238: SystemSeconds (3 byte)	(sys-clock seconds in day 0-84600)     
    #  
    outSS, off = readB(bytes, 236, 3)          
    # Byte 239-240: SystemMilliSeconds (2 byte)	(sys-clock milliseconds 0-999)  

    outSSSS, off = readB(bytes, 239, 2)       
    # Byte 241-242: PedestrianDirectCallStatus (2 byte)	(bit-mapped phases 1-16)             
    # Byte 243-244: PedestrianLatchedCallStatus (2 byte)	(bit-mapped phases 1-16)  
    #            
    time = '{}.{}'.format(b2i(outSS), b2i(outSSSS))
    #Set lights to Green/Yellow/Flash by phase.
    greens = hextobin(outG.hex())
    yellows = hextobin(outY.hex())
    reds = hextobin(outR.hex())
    flashing = hextobin(outFl.hex())
    for phase in phases:
        index = phase['phase']
        if yellows[16-index] == 1:
            phase['color'] = "YELLOW"
        if greens[16-index] == 1:
            phase['color'] = "GREEN"
        if flashing[16-index] == 1:
            phase['flash'] = True

    payload = {
            'id': id, 
            'messageSet': 'BATTELLE',
            'updated': time_now,
            'timeSystem': time,
            "green": greens,
            "yellow": yellows,
            "red": reds,
            "walk": hextobin(outW.hex()),
            "walkDont": hextobin(outDW.hex()),
            "pedestrianClear": hextobin(outPC.hex()),
            "flash": flashing,
            'phases': phases
    }

    return payload

## 494 is length (247 rmal for parsing)
spat_data = 'cd100100dc02aa0000000000000000020000007d00dc02aa000000000300dc01db000000000000000004003f00bc003f00bc0000000005003f02d400000000000000000600000093003f02d40000000007003f00d2000000000000000008003f01a1003f01a100000000090000000000000000000000000a0000000000000000000000000b0000000000000000000000000c0000000000000000000000000d0000000000000000000000000e0000000000000000000000000f0000000000000000000000001000000000000000000000000000dd0000002200ff00000000000000000000000000000000085d003eca03ce00000000'
payload = parse_TSCBM('10', bytes.fromhex(spat_data), 'Time')
print (payload)
