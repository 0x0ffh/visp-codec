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
typedef unsigned char  UC;
typedef unsigned short US;
typedef unsigned int   UI;
using namespace std;

template <class Template> void alloc(Template*& get_next_bit, int bit_count) {
    get_next_bit = (Template*)calloc(bit_count, sizeof(Template));
    if (!get_next_bit) {
        static_cast<void>(fprintf(stderr, "out of memory\n")), exit(1);
    }
}

std::ifstream::pos_type filesize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

class State_Machine {
protected:
    const int state_number;
    int previous_state;
    UI* state_count;
    static int general_table[256];

public:
    State_Machine(int bit_count = 256);
    int get_next_bit(int state) {
        assert(state >= 0 && state < state_number);
        return state_count[previous_state = state] >> 16;
    }

    void update(int soup_bit, int limit = 255) {
        int bit_count = state_count[previous_state] & 255, get_next_bit = state_count[previous_state] >> 14;
        if (bit_count < limit) {
            ++state_count[previous_state];
            state_count[previous_state] += ((soup_bit << 18) - get_next_bit) * general_table[bit_count] & 0xffffff00;
        }
    }
};

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

class Determinant {
    int previous_state;
    State_Machine bit_matrix;
    int state[256];
public:
    Determinant();
    int get_next_bit() {
        return bit_matrix.get_next_bit(previous_state << 8 | state[previous_state]);
    }

    void update(int soup_bit) {
        bit_matrix.update(soup_bit, 90);
        int& state_numeration = state[previous_state];
        (state_numeration += state_numeration + soup_bit) &= 255;
        if ((previous_state += previous_state + soup_bit) >= 256)
            previous_state = 0;
    }
};

Determinant::Determinant() : previous_state(0), bit_matrix(0x10000) {
    for (int i = 0; i < 0x100; ++i)
        state[i] = 0x66;
}


typedef enum { ENCODE_DATA, DECODE_DATA } Mode;
class Encoder {
private:
    Determinant determinant;
    const Mode codec_mode;
    FILE* soup;
    UI vector_x, vector_y;
    UI lambda;
public:
    Encoder(Mode io_mode, FILE* file_stream);
    void encode(int soup_bit);
    int decode();
    void alignment();

};

Encoder::Encoder(Mode io_mode, FILE* file_stream) : determinant(), codec_mode(io_mode), soup(file_stream), vector_x(0),
vector_y(0xffffffff), lambda(0) {
    if (codec_mode == DECODE_DATA) {
        for (int i = 0; i < 4; ++i) {
            int byte = getc(soup);
            if (byte == EOF) byte = 0;
            lambda = (lambda << 8) + (byte & 0xff);
        }
    }
}

inline void Encoder::encode(int soup_bit) {
    const UI get_next_bit = determinant.get_next_bit();
    assert(get_next_bit <= 0xffff);
    assert(soup_bit == 0 || soup_bit == 1);
    const UI soup_mix_byte = vector_x + ((vector_y - vector_x) >> 16) * get_next_bit + ((vector_y - vector_x & 0xffff) * get_next_bit >> 16);
    assert(soup_mix_byte >= vector_x && soup_mix_byte < vector_y);
    if (soup_bit) {
        vector_y = soup_mix_byte;
    }
    else
        vector_x = soup_mix_byte + 1;
        determinant.update(soup_bit);
        while (((vector_x ^ vector_y) & 0xff000000) == 0) {
            putc(vector_y >> 24, soup);
            vector_x <<= 8;
            vector_y = (vector_y << 8) + 255;
    }
}

inline int Encoder::decode() {
    const UI get_next_bit = determinant.get_next_bit();
    assert(get_next_bit <= 0xffff);
    const UI soup_mix_byte = vector_x + ((vector_y - vector_x) >> 16) * get_next_bit + ((vector_y - vector_x & 0xffff) * get_next_bit >> 16);
    assert(soup_mix_byte >= vector_x && soup_mix_byte < vector_y);
    int soup_bit = 0;
    if (lambda <= soup_mix_byte) {
        soup_bit = 1;
        vector_y = soup_mix_byte;
    }
    else
        vector_x = soup_mix_byte + 1;
    determinant.update(soup_bit);

    while (((vector_x ^ vector_y) & 0xff000000) == 0) {
        vector_x <<= 8;
        vector_y = (vector_y << 8) + 255;
        int byte = getc(soup);
        if (byte == EOF) byte = 0;
        lambda = (lambda << 8) + byte;
    }
    return soup_bit;
}

void Encoder::alignment() {
    if (codec_mode == ENCODE_DATA) {
        while (((vector_x ^ vector_y) & 0xff000000) == 0) {
            putc(vector_y >> 24, soup);
            vector_x <<= 8;
            vector_y = (vector_y << 8) + 255;
        }
        putc(vector_y >> 24, soup);
    }
}

int main(int argc, char** argv) {
    if (argc != 4 || (argv[1][0] != 'c' && argv[1][0] != 'd')) {
        printf("Vector Integer State Prediction Codec");
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
        if(filesize(argv[2]) < 300){
            cout << "\nVery small file\n";
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
