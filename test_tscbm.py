
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

def parse_TSCBM(RxId, bytes, time_now):
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
        out_P, off = readB(bytes, off, 1) #2
        out_VMin, off = readB(bytes, off, 2) #3,4
        out_VMax, off = readB(bytes, off, 2) #5, 6
        out_PMin, off = readB(bytes, off, 2) #7, 8
        out_PMax, off = readB(bytes, off, 2) #9, 10
        out_OMin, off = readB(bytes, off, 2) #11, 12 Overlap min
        out_OMax, off = readB(bytes, off, 2) #13, 14 Overlap Max

        phase = {
            "phase": b2i(out_P),
            "color": 'RED',
            "flash": 0,
            "walkDont": 0,
            "walk": 0,
            "pedestrianClear": 0,
            "overlap": {
                "green": 0,
                "red": 0,
                "yellow": 0,
                "flash": 0
            },
            "vehTimeMin": round((b2i(out_VMin) or 0) * .10, 1), #self.__b2i(out_VMin), 
            "vehTimeMax": round((b2i(out_VMax) or 0) * .10, 1), #self.__b2i(out_VMax), 
            "pedTimeMin": round((b2i(out_PMin) or 0) * .10, 1), #self.__b2i(out_PMin), # round(self.__b2i(out_PMin) * Decimal(.10), 1),
            "pedTimeMax": round((b2i(out_PMax) or 0) * .10, 1), #self.__b2i(out_PMax), # round(self.__b2i(out_PMax) * Decimal(.10), 1),
            "overlapMin": round((b2i(out_OMin) or 0) * .10, 1),
            "overlapMax": round((b2i(out_OMax) or 0) * .10, 1)
        }
        phases.append(phase)

    # bytes 210-215: PhaseStatusReds, Yellows, Greens	(2 bytes bit-mapped for phases 1-16)
    out_R, off = readB(bytes, off, 2)
    out_Y, off = readB(bytes, off, 2)
    out_G, off = readB(bytes, off, 2)

    # # bytes 216-221: PhaseStatusDontWalks, PhaseStatusPedClears, PhaseStatusWalks (2 bytes bit-mapped for phases 1-16)
    out_DW, off = readB(bytes, 216, 2)
    out_PC, off = readB(bytes, 218, 2)
    out_W, off = readB(bytes, 220, 2)

    # bytes 222-227: OverlapStatusReds, OverlapStatusYellows, OverlapStatusGreens (2 bytes bit-mapped for overlaps 1-16)
    out_RO, off = readB(bytes, 222, 2)
    out_YO, off = readB(bytes, 224, 2)
    out_GO, off = readB(bytes, 226, 2)

    # bytes 228-229: FlashingOutputPhaseStatus	(2 bytes bit-mapped for phases 1-16)
    out_Fl, off = readB(bytes, 228, 2)

    # bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    out_Flo, off = readB(bytes, 230, 2)

    # bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    # byte 232: IntersectionStatus (1 byte) (bit-coded byte) 
    # bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    # byte 232: IntersectionStatus (1 byte) (bit-coded byte) 
    outInt, off = readB(bytes, 232, 1)
    
    #   Added by J. Parikh (sept. 2023) Convert hex byte to bin 
    intxSO = hextobin(outInt.hex())

    # Byte 233: TimebaseAscActionStatus (1 byte)  	(current action plan)                       
    # byte 234: DiscontinuousChangeFlag (1 byte)    (upper 5 bits are msg version #2, 0b00010XXX)     
    # byte 235: MessageSequenceCounter (1 byte)     (lower byte of up-time deci-seconds) 
    #   Added by JP to get message counter
    msgCt, off = readB(bytes, 235, 1)
    msgCount = hextobin(msgCt.hex())
    
    # Byte 236-238: SystemSeconds (3 byte)	(sys-clock seconds in day 0-84600)     
    #  
    outSS, off = readB(bytes, 236, 3)          
    # Byte 239-240: SystemMilliSeconds (2 byte)	(sys-clock milliseconds 0-999)  

    outSSSS, off = readB(bytes, 239, 2)       
    # Byte 241-242: PedestrianDirectCallStatus (2 byte)	(bit-mapped phases 1-16)             
    # Byte 243-244: PedestrianLatchedCallStatus (2 byte)	(bit-mapped phases 1-16)  
    #            
    time = '%5u.%03u' % (b2i(outSS), b2i(outSSSS))
    #Set lights to Green/Yellow/Flash by phase.

    greens = hextobin(out_G.hex())
    greens_overlap = hextobin(out_GO.hex())
    yellows = hextobin(out_Y.hex())
    yellows_overlap = hextobin(out_YO.hex())
    reds = hextobin(out_R.hex())
    reds_overlap = hextobin(out_RO.hex())

    flashing = hextobin(out_Fl.hex())
    flashing_overlap = hextobin(out_Flo.hex())
    walkDont = hextobin(out_DW.hex())
    walk = hextobin(out_W.hex())
    pedClear = hextobin(out_PC.hex())

    for phase in phases:
        index = phase['phase']
        if yellows[16-index] == '1':
            phase['color'] = "YELLOW"
        if greens[16-index] == '1':
            phase['color'] = "GREEN"
        if greens_overlap[16-index] == '1':
            phase['overlap']['green'] = 1
        if yellows_overlap[16-index] == '1':
            phase['overlap']['yellow'] = 1
        if reds_overlap[16-index] == '1':
            phase['overlap']['red'] = 1
        if flashing[16-index] == '1':
            phase['flash'] = 1
        if flashing[16-index] == '1':
            phase['overlap']['flash'] = 1
        if walkDont[16-index] == '1':
            phase['walkDont'] = 1
        if walk[16-index] == '1':
            phase['walk'] = 1
        if pedClear[16-index] == '1':
            phase['pedestrianClear'] = 1

    payload = {
        'id': RxId,
        'messageSet': 'NTCIP',
        'updated': time_now,
        'timeSystem': time,
        'intx_SO': intxSO,
        'msgCount': int(msgCount,2),           #Upper 7 bits
        "green": greens,
        "yellow": yellows,
        "red": reds,
        "walk": walk,
        "walkDont": walkDont,
        "pedestrianClear": pedClear,
        "flash": flashing,
        "overlap": {
            "green": greens_overlap,
            "red": reds_overlap,
            "yellow": yellows_overlap,
            "flash": flashing_overlap
        },
        'phases': phases
    }

    return payload

