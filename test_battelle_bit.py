from bitstring import ConstBitStream
## 494 is length (247 normal for parsing)
spat_data = 'cd100100dc02aa0000000000000000020000007d00dc02aa000000000300dc01db000000000000000004003f00bc003f00bc0000000005003f02d400000000000000000600000093003f02d40000000007003f00d2000000000000000008003f01a1003f01a100000000090000000000000000000000000a0000000000000000000000000b0000000000000000000000000c0000000000000000000000000d0000000000000000000000000e0000000000000000000000000f0000000000000000000000001000000000000000000000000000dd0000002200ff00000000000000000000000000000000085d003eca03ce00000000'

stream = ConstBitStream(bytes.fromhex(spat_data))

pre = stream.read('hex:8') #'CD' DynObj13 response byte (0xcd)
block = stream.read('int:8') #The number of blocks of phase/overlap below 16.
phases=[]
for i in range(block):
    outP = stream.read('int:8') #2
    outVMin, outVMax = stream.readlist('int:16, 16') #3,4 5,6
    outPMin, outPMax = stream.readlist('int:16, 16') #7, 8 9, 10
    outOMin, outOMax = stream.readlist('int:16, 16') #11, 12 Overlap min #13, 14 Overlap Max
    phase = {
        "phase": outP,
        "color": 'RED',
        "flash": False,
        "vehTimeMin": outVMin,
        "vehTimeMax": outVMax,
        "pedTimeMin": outPMin,
        "pedTimeMax": outPMax,
    }
    phases.append(phase)

# bytes 210-215: PhaseStatusReds, Yellows, Greens	(2 bytes bit-mapped for phases 1-16)
outR, outY, outG = stream.readlist('bin:16, 16, 16')
# # bytes 216-221: PhaseStatusDontWalks, PhaseStatusPedClears, PhaseStatusWalks (2 bytes bit-mapped for phases 1-16)
outDW, outPC, outW = stream.readlist('bin:16, 16, 16')

# bytes 222-227: OverlapStatusReds, OverlapStatusYellows, OverlapStatusGreens (2 bytes bit-mapped for overlaps 1-16)
# bytes 228-229: FlashingOutputPhaseStatus	(2 bytes bit-mapped for phases 1-16)
# bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
outOLR, outOLY, outOLG = stream.readlist('int:16, 16, 16')
outFL, outFLO = stream.readlist('int:16, 16')

# byte 232: IntersectionStatus (1 byte) (bit-coded byte) 
outInt = stream.read('int:8')

# Byte 233: TimebaseAscActionStatus (1 byte)  	(current action plan)                       
# byte 234: DiscontinuousChangeFlag (1 byte)          (upper 5 bits are msg version #2, 0b00010XXX)     
# byte 235: MessageSequenceCounter (1 byte)           (lower byte of up-time deci-seconds) 
outTAS, outDCF, outMSC = stream.readlist('int:8, 8, 8')

# Byte 236-238: SystemSeconds (3 byte)	(sys-clock seconds in day 0-84600)     
# Byte 239-240: SystemMilliSeconds (2 byte)	(sys-clock milliseconds 0-999) 
outSS = stream.read('int:24')
outSSSS = stream.read('int:16')
time = '{}.{}'.format(outSS, outSSSS)

# Byte 241-242: PedestrianDirectCallStatus (2 byte)	(bit-mapped phases 1-16)             
# Byte 243-244: PedestrianLatchedCallStatus (2 byte)	(bit-mapped phases 1-16)  
outPDC, outPLC = stream.readlist('int:16, 16')

greens = outG if outG == '0' else '0000000000000000'
yellows = outY if outY == '0' else '0000000000000000'
reds = outR if outR == '0' else '0000000000000000'
flashing = outFL if outFL == '0' else '0000000000000000'
walk = outW if outW == '0' else '0000000000000000'
walkDont = outDW if outDW == '0' else '0000000000000000'
pedClear = outPC if outPC == '0' else '0000000000000000'

for phase in phases:
    index = phase['phase']
    if yellows[16-index] == 1:
        phase['color'] = "YELLOW"
    elif greens[16-index] == 1:
        phase['color'] = "GREEN"
    if flashing[16-index] == 1:
        phase['flash'] = True

payload = {
        'id': 10, 
        'messageSet': 'BATTELLE',
        'updated': None,
        'timeSystem': time,
        "green": greens,
        "yellow": yellows,
        "red": reds,
        "walk": walk,
        "walkDont": walkDont,
        "pedestrianClear": pedClear,
        "flash": flashing,
        'phases': phases
}

print (payload)