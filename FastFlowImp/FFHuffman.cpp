#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <vector>

#include "CompressFarm.cpp"
#include "MapFarm.cpp"
#include "EncodeFarm.cpp"
#include "../utils/Huffman.cpp"
#include "../utils/utimer.hpp"

using namespace std;
using namespace ff;

long usecs, seq_time;

void FFHuffmanEncoding(string text, int nw)
{
    utimer t3("Total computation", &usecs);
    unordered_map<char, int> mapper;
    string full_text, line;

    fstream readFile;

    readFile.open(text, ios::in);
    {
        utimer t0("Reading input txt file");
        while(!readFile.eof())
        {
            getline(readFile, line);
            full_text += line;
        }
    }

    {
        utimer t0("Compute statistics"); 
        auto e = mp::emitter(full_text, nw, &mapper);
        auto c = mp::collector(); 
        ff::ff_Farm<mp::TASK> mf(mp::worker, nw);
        mf.add_emitter(e);
        mf.add_collector(c);
        mf.run_and_wait_end();
    }

    unordered_map<char,string> huffmanCode;
    {
        utimer t2("Creating Huffman encoding");
        huffmanCode = buildHuffmanEncoding(mapper);
    }
    
    string encoded_text;
    vector<string> encoded_vector(nw);
    vector<string> out_vector(nw);
    ofstream writeFile("compressed_text.txt");
    long utime_encode_text;
    {
        utimer t1("Encoding text");
        auto e = enc::emitter(full_text, nw, huffmanCode, &encoded_vector);
        auto c = enc::collector();
        ff::ff_Farm<enc::TASK> emf(enc::worker, nw);
        emf.add_emitter(e);
        emf.add_collector(c);
        emf.run_and_wait_end();
    }

    long utime_compress_text;
    { 
        utimer t1("Compressing text");
        auto e = compression::emitter(nw, &encoded_vector, &out_vector);
        auto c = compression::collector();
        ff::ff_Farm<compression::TASK> cf(compression::worker, nw); 
        cf.add_emitter(e);
        cf.add_collector(c);
        cf.run_and_wait_end(); 
    }

    long utime_write_text;
    {
        utimer t0("Write out file");
        for(int i = 0; i < nw; i++)
        {
            writeFile << out_vector[i];
        }
    }
}

int main(int argc, char * argv[])
{
    string text = (argc > 1 ? argv[1] : "../test.txt");
    int i = 1;
    for(i; i <= 64; i=i*2)
    {
        cout << "--------------------------------------------------------------------------" <<endl;
        cout << "Computing Fast Flow Huffman implementation with " << i << " threads:" << endl;
        FFHuffmanEncoding(text, i);
        if(i == 1) 
        {
            seq_time = usecs;
        }
        cout << endl;
        float speedup = (float)seq_time/(float)usecs;
        cout << "SPEEDUP(" << i << ") = " << speedup << endl;
        cout << "EFFICIENCY(" << i << ") = " << speedup/i << endl;
    }
}