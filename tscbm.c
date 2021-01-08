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
    char *out = NULL;
    char *tmp = NULL;
    char sub[25], sub2[25], sub3[25], sub4[25], sub5[25], sub6[25]; //Use these as temp storage for mem copy.
    sub[24] = sub2[24] = sub3[24] = sub4[24] = sub5[24] = sub6[24] ='\0';

    asprintf(&tmp,"%s%s%s%s%s","{\"id\": \"", id, "\", \"messageSet\": \"NTCIP\", \"updated\": \"", in_time, "\" , ");
    //free(out);
    out = tmp;

    //Get number of phases in this packet (should always be 16, but check...)
    memcpy(sub, &in[8], 8);
    sub[8] = '\0';
    int phase_cnt = bin2int(sub);
    if (phase_cnt != 16) {
        free(out);
        exit(1);
    }

    // Byte 236-238: SystemSeconds (3 byte)	(sys-clock seconds in day 0-84600)     
    // Byte 239-240: SystemMilliSeconds (2 byte)	(sys-clock milliseconds 0-999)  
    memcpy(sub, &in[8*236], 24);
    memcpy(sub2, &in[8*239], 16);
    sub2[16] = '\0';
    asprintf(&tmp,"%s%s%d%s%d%s",out,"\"timeSystem\": \"", bin2int(sub), ".", bin2int(sub2), "\", ");
    free(out);
    out = tmp;

    //Colors
    // bytes 210-215: PhaseStatusReds, Yellows, Greens	(2 bytes bit-mapped for phases 1-16)
    memcpy(sub, &in[8*214], 16);
    memcpy(sub2, &in[8*212], 16);
    memcpy(sub3, &in[8*210], 16);
    sub[16] = sub2[16] = sub3[16] = '\0';
    asprintf(&tmp,"%s%s%s%s%s%s%s%s", out, "\"green\": \"", sub, "\", \"yellow\": \"", sub2, "\", \"red\": \"", sub3, "\", ");
    free(out);
    out = tmp;

    //Peds
    //# bytes 216-221: PhaseStatusDontWalks, PhaseStatusPedClears, PhaseStatusWalks (2 bytes bit-mapped for phases 1-16)
    memcpy(sub, &in[8*220], 16); //walk
    memcpy(sub2, &in[8*216], 16); //Dont Walk
    memcpy(sub3, &in[8*218], 16); //PedClear
    sub[16] = sub2[16] = sub3[16] = '\0';
    asprintf(&tmp,"%s%s%s%s%s%s%s%s", out, "\"walk\": \"", sub, "\", \"walkDont\": \"", sub2, "\", \"pedestrianClear\": \"", sub3, "\", ");
    free(out);
    out = tmp;

    //Flash + Overlaps
    //# bytes 228-229: FlashingOutputPhaseStatus	(2 bytes bit-mapped for phases 1-16)
    //# bytes 230-231: FlashingOutputOverlapStatus	(2 bytes bit-mapped for overlaps 1-16)
    //# bytes 222-227: OverlapStatusReds, OverlapStatusYellows, OverlapStatusGreens (2 bytes bit-mapped for overlaps 1-16)
    memcpy(sub, &in[8*228], 16); //FL output
    memcpy(sub2, &in[8*230], 16); //FL Overlap
    memcpy(sub3, &in[8*222], 16); //Red Overlap
    memcpy(sub4, &in[8*224], 16); //Yellow Overlap
    memcpy(sub5, &in[8*226], 16); //Green Overlap
    sub[16] = sub2[16] = sub3[16] = sub4[16] = sub5[16] ='\0';
    asprintf(&tmp,"%s%s%s%s%s%s%s%s%s%s%s%s", out, "\"flash\": \"", sub, "\", \"overlap\": {\"green\": \"", sub5, "\", \"red\": \"", sub3, "\", \"yellow\": \"", sub4, "\", \"flash\": \"", sub2, "\"}, \"phases\": [");
    free(out);
    out = tmp;

    //Loop phases
    int i;
    for (i=0;i < phase_cnt; i++) {
        //Append the phase identifier
        if (i >  0) {
            strncpy(sub, ",{\"phase\": \0'", 24);
        } else {
            strncpy(sub, "{\"phase\": \0'", 24);
        }
        asprintf(&tmp,"%s%s%d%s",out, sub, i + 1, ", ");
        free(out);
        out = tmp;

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
                asprintf(&tmp,"%s%s", out, "\"color\": \"RED\", ");
            } else {
                asprintf(&tmp,"%s%s", out, "\"color\": \"YELLOW\", ");
            }
        } else {
            asprintf(&tmp,"%s%s", out, "\"color\": \"GREEN\", ");
        }
        free(out);
        out = tmp;

        //Flash + Ped by Phase. Flash is stored at byte 228. 
        //Its format is 16 bit decending. So Phase 1 is stored at 228 + 15 bits
        memcpy(sub, &in[8*228+15-i], 1); //FL
        memcpy(sub2, &in[8*220+15-i], 1); //Walk
        memcpy(sub3, &in[8*216+15-i], 1); //Don't Walk
        memcpy(sub4, &in[8*218+15-i], 1); //Ped Clear
        sub[1] = sub2[1] = sub3[1] = sub4[1] ='\0';
        asprintf(&tmp,"%s%s%s%s%s%s%s%s%s%s", out, "\"flash\": ", (sub[0] == '0' ? "false" : "true"), ", \"walkDont\": ", (sub3[0] == '0' ? "false" : "true"), ", \"walk\": ", (sub2[0] == '0' ? "false" : "true"), ", \"pedestrianClear\": ", (sub4[0] == '0' ? "false" : "true"), ", ");
        free(out);
        out = tmp;

        //Overlaps
        memcpy(sub, &in[8*230+15-i], 1); //FL OL
        memcpy(sub2, &in[8*222+15-i], 1); //Red Overlap
        memcpy(sub3, &in[8*224+15-i], 1); //Yellow Overlap
        memcpy(sub4, &in[8*226+15-i], 1); //Green Overlap
        sub[1] = sub2[1] = sub3[1] = sub4[1] ='\0';
        asprintf(&tmp,"%s%s%s%s%s%s%s%s%s%s", out, "\"overlap\": { \"flash\": ", (sub[0] == '0' ? "false" : "true"), ", \"green\": ", (sub4[0] == '0' ? "false" : "true"), ", \"red\": ", (sub2[0] == '0' ? "false" : "true"), ", \"yellow\": ", (sub3[0] == '0' ? "false" : "true"), "},");
        free(out);
        out = tmp;

        //Timings
        memcpy(sub, &in[8*3+(i*13*8)], 16); //Veh min
        memcpy(sub2, &in[8*5+(i*13*8)], 16); //Veh Max
        memcpy(sub3, &in[8*7+(i*13*8)], 16); //Ped min
        memcpy(sub4, &in[8*9+(i*13*8)], 16); //Ped Max 218?
        memcpy(sub5, &in[8*11+(i*13*8)], 16); //OL Min
        memcpy(sub6, &in[8*13+(i*13*8)], 16); //OL Max
        sub[16] = sub2[16] = sub3[16] = sub4[16] = sub5[16] = sub6[16] ='\0';
        asprintf(&tmp,"%s%s%.1f%s%.1f%s%.1f%s%.1f%s%.1f%s%.1f%s", out, "\"vehTimeMin\": ", bin2int(sub) * 0.10, ", \"vehTimeMax\": ", bin2int(sub2) * .10, ", \"pedTimeMin\": ", bin2int(sub3) * .10, ", \"pedTimeMax\": ", bin2int(sub4) * .10, ", \"overlapMin\": ", bin2int(sub5) * .10, ", \"overlapMax\": ", bin2int(sub6) * .10, "}");
        free(out);
        out = tmp;

    }
    asprintf(&tmp,"%s]}", out);
    free(out);
    out = tmp;
    return out;
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