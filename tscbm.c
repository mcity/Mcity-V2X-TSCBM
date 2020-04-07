/**
 * TSCBM Message parser example
 * Tyler Worman (tworman@umich.edu)
 * 8/8/19 - Initial version of test script.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
    bin2int(const char *s)
    Takes a null terminated bit string and converst it to an integer
    11111111 = 256
    00000000 = 0
 */
int bin2int(const char *s) {
  return (int) strtol(s, NULL, 2);
}

char *parseTSCBM(char *in, char *id, char* in_time)
{
    char sub[25]; //Use this as temp storage for mem copy.
    sub[24] = '\0';
    char *buf = "{";
    

    //GNU C string builder
    //Get number of phases in this packet (should always be 16 but check..)
    memcpy(sub, &in[8], 8);
    sub[8] = '\0';
    int phase_cnt = bin2int(sub);
    if (phase_cnt != 16) {
        exit(1);
    }

    //Build the message format we expect.
    asprintf(&buf,"%s%s%s%s",buf,"\"id\": \"", id, "\", \"messageSet\": \"NTCIP\", ");
    asprintf(&buf,"%s%s%s%s",buf,"\"updated\": \"", in_time, "\" , ");

    // Byte 236-238: SystemSeconds (3 byte)	(sys-clock seconds in day 0-84600)     
    // Byte 239-240: SystemMilliSeconds (2 byte)	(sys-clock milliseconds 0-999)  
    memcpy(sub, &in[8*236], 24);
    asprintf(&buf,"%s%s%d%s",buf,"\"timeSystem\": \"", bin2int(sub), ".");
    memcpy(sub, &in[8*239], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%d%s",buf, bin2int(sub), "\", ");

    //Colors
    // bytes 210-215: PhaseStatusReds, Yellows, Greens	(2 bytes bit-mapped for phases 1-16)
    memcpy(sub, &in[8*214], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"green\": \"", sub, "\", ");

    memcpy(sub, &in[8*212], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"yellow\": \"", sub, "\", ");

    memcpy(sub, &in[8*210], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"red\": \"", sub, "\", ");

    //Peds
    //# bytes 216-221: PhaseStatusDontWalks, PhaseStatusPedClears, PhaseStatusWalks (2 bytes bit-mapped for phases 1-16)
    memcpy(sub, &in[8*220], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"walk\": \"", sub, "\", ");

    memcpy(sub, &in[8*216], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"walkDont\": \"", sub, "\", ");

    memcpy(sub, &in[8*218], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"pedestrianClear\": \"", sub, "\", ");
    
    //Flash
    //# bytes 228-229: FlashingOutputPhaseStatus	(2 bytes bit-mapped for phases 1-16)
    //# bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    memcpy(sub, &in[8*228], 16);
    sub[16] = '\0';
    asprintf(&buf,"%s%s%s%s", buf, "\"flash\": \"", sub, "\", ");

    //Loop phases
    //Get number of phases in this packet (should always be 16 but check..)
    //'phases': [
    asprintf(&buf,"%s%s", buf, "\"phases\": [");

    int i;
    for (i=0;i < phase_cnt; i++) {
        if (i >  0) {
            asprintf(&buf,"%s%s%d%s",buf, ",{\"phase\": ", i + 1, ", ");
        } else {
            asprintf(&buf,"%s%s%d%s",buf, "{\"phase\": ", i + 1, ", ");
        }
        //Each Phase is 13 bytes (8 bits)
        
        //Color is stored in multiple places.
        //If it's not Green/Yellow it's red, so must only check 2.
        memcpy(sub, &in[8*214+15-i], 1);
        sub[2] = '\0';
        if (sub[0] == '0') {
            //It's not Green, check yellow
            memcpy(sub, &in[8*212+15-i], 1);
            if (sub[0] == '0') {
                //It's not yellow or green
                asprintf(&buf,"%s%s", buf, "\"color\": \"RED\", ");
            } else {
                asprintf(&buf,"%s%s", buf, "\"color\": \"YELLOW\", ");
            }
        } else {
            asprintf(&buf,"%s%s", buf, "\"color\": \"GREEN\", ");
        }

        //Flash is stored at byte 228. 
        //Its format is 16 bit decending. So Phase 1 is stored at 228 + 15 bits
        memcpy(sub, &in[8*228+15-i], 1);
        sub[1] = '\0';
        if (sub[0] == '0') {
            asprintf(&buf,"%s%s", buf, "\"flash\": false, ");
        } else {
            asprintf(&buf,"%s%s", buf, "\"flash\": true, ");
        }
        
        memcpy(sub, &in[8*3+(i*13*8)], 16);
        sub[16] = '\0';
        asprintf(&buf,"%s%s%d%s", buf, "\"vehTimeMin\": ", bin2int(sub), ", ");
        
        memcpy(sub, &in[8*5+(i*13*8)], 16);
        sub[16] = '\0';
        asprintf(&buf,"%s%s%d%s", buf, "\"vehTimeMax\": ", bin2int(sub), ", ");
                
        memcpy(sub, &in[8*7+(i*13*8)], 16);
        sub[16] = '\0';
        asprintf(&buf,"%s%s%d%s", buf, "\"pedTimeMin\": ", bin2int(sub), ", ");
                
        memcpy(sub, &in[8*218], 16);
        sub[16] = '\0';
        asprintf(&buf,"%s%s%d%s", buf, "\"pedTimeMax\": ", bin2int(sub), "}");
        
    }
    asprintf(&buf,"%s]}", buf);
    return buf;
}

/**
 * Sample code building a string/char array buffer with 
 * resulting JSON formatted text resulting from the parsing of
 * a sample TSCBM formatted binary input string.
 * 
 * This is not the most efficiently written version, we attempted easy to read.
 * Easy improvements you could make:
 * - Combine buffer appends
 * - Move away from GNU apsrintf to a POSIX version with a fixed/non-dynamic buffer sizes
 * - Directly access the values from the char* input string through index instead of memcopy for each value.
 * - Return a native typed object to your calling code instead of a char * [].
 * -
 */
int main(int ac, char *av[])
{
    char* in = "1100110100010000000000010000000011011100000000101010101000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000001111101000000001101110000000010101010100000000000000000000000000000000000000011000000001101110000000001110110110000000000000000000000000000000000000000000000000000000000000000000001000000000000111111000000001011110000000000001111110000000010111100000000000000000000000000000000000000010100000000001111110000001011010100000000000000000000000000000000000000000000000000000000000000000000000110000000000000000000000000100100110000000000111111000000101101010000000000000000000000000000000000000001110000000000111111000000001101001000000000000000000000000000000000000000000000000000000000000000000000100000000000001111110000000110100001000000000011111100000001101000010000000000000000000000000000000000001001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000101100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011011101000000000000000000000000001000100000000011111111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100001011101000000000011111011001010000000111100111000000000000000000000000000000000";
    char *id = "10";
    char *time = "2019-08-12 10:41:55";
    char* out= parseTSCBM(in, id, time);

    printf("%s\n",out);
}