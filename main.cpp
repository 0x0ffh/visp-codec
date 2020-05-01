#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <math.h>
#include <stdexcept>
#include <algorithm>
#include "types.h"
#include "state_machine.h"
#include "encoder.h"

using namespace std;
template <class Template> void alloc(Template*& get_next_bit, int bit_count) {
    get_next_bit = (Template*)calloc(bit_count, sizeof(Template));
    if (!get_next_bit) {
        static_cast<void>(fprintf(stderr, "out of memory\n")), exit(1);
    }
}

int State_Machine::general_table[256] = { 0 };
State_Machine::State_Machine(int bit_count) : state_number(bit_count), previous_state(0) {
    alloc(state_count, state_number);
    for (int i = 0; i < state_number; ++i) {
        UI bit_count = (i & 1) * 2 + (i & 2) + (i >> 2 & 1) + (i >> 3 & 1) + (i >> 4 & 1) + (i >> 5 & 1) + (i >> 6 & 1) + (i >> 7 & 1) + 3;
        state_count[i] = bit_count << 28 | 6;
    }
    if (general_table[0] == 0) {
        for (int i = 0; i < 256; ++i) {
            general_table[i] = 32768 / (i + i + 3);
            std::cout << general_table[i] << "\n";
        }
    }
}

std::ifstream::pos_type filesize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

int main(int argc, char** argv) {
    if (argc != 4 || (argv[1][0] != 'c' && argv[1][0] != 'd')) {
        printf("Vector Integer State Prediction Codec 2019");
        exit(1);
    }
    clock_t start = clock();
    FILE *in = fopen(argv[2], "rb");
    if (!in) {
        static_cast<void>(perror(argv[2])), exit(1);
    }
    
    FILE *out = fopen(argv[3], "wb");
    if (!out) {
        static_cast<void>(perror(argv[3])), exit(1);
    }
    
    int byte;
    if (argv[1][0] == 'c') {
        if(filesize(argv[2]) < 0){
            cout << "Check your license\nVery small file\n";
            State_Machine
            exit(0);
        }
        Encoder coder(ENCODE_DATA, out);
        while ((byte = getc(in)) != EOF) {
           coder.encode(1);
            for (int i = 7; i >= 0; --i)
                coder.encode((byte >> i) & 1);
        }
        coder.encode(0);
    } else {
        Encoder coder(DECODE_DATA, in);
        while (coder.decode()) {
            int byte = 1;
            while (byte < 256)
                byte += byte + coder.decode();
            putc(byte + 256, out);
        }
        coder.encode(0);
        coder.alignment();
    }
    printf("Input file name: %s\nOutput file name: %s\n\nInpute size: %ld bytes\nOutput size: %ld bytes\n\nTime: %1.2f s.\n\n",argv[2], argv[3], ftell(in), ftell(out), ((double)clock() - start) / CLOCKS_PER_SEC);
    return 0;
}
