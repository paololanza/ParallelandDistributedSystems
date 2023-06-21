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

void decodeCompressedText()
{
    //decode the string
    string decompressed_string;
    for(char c : res.substr(0,100000))
    {
        bitset<8> binary(c);
        decompressed_string += binary.to_string();
    }

    decodeText(decompressed_string);
}

// Builds Huffman Tree and decode given input text
void FFHuffmanEncoding(string text, int nw)
{
    utimer t3("Total computation", &usecs);
    unordered_map<char, int> mapper;
    {
        utimer t0("Reading file and compute statistics"); 
        auto e = mp::emitter(text, nw);
        auto c = mp::collector(&mapper); 
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
    ofstream writeFile("compressed_text.txt");
    long utime_encode_text;
    {
        utimer t1("Encoding text");
        auto e = enc::emitter(text, nw, huffmanCode);
        auto c = enc::collector(&encoded_text, &writeFile);
        // cout << "---> " << workers.size() << endl; 
        ff::ff_OFarm<enc::TASK> emf(enc::worker, nw);
        emf.add_emitter(e);
        emf.add_collector(c);
        emf.run_and_wait_end();
    }

    long utime_compress_text;
    {
        utimer t1("Compressing text");
        auto e = compression::emitter(encoded_text, nw);
        auto c = compression::collector(&writeFile);
        ff::ff_OFarm<compression::TASK> emf(compression::worker, nw);
        emf.add_emitter(e);
        emf.add_collector(c);
        emf.run_and_wait_end();
    }

    // ofstream writeFile("compressed_text.txt");
    // for(auto s : encoded_text)
    //     writeFile << s;





    // ofstream writeFile("compressed_text.txt");
    // string encodedText;
    // for(auto s : encoded_text)
    // {
    //     writeFile << s; 
    //     encodedText += s;
    // }

    // traverse the Huffman Tree again and this time
    // decode the encoded string
    //decodeText(encodedText);
}

int main(int argc, char * argv[])
{
    int i = 1;
    for(i; i <= 64; i=i*2)
    {
        cout << "--------------------------------------------------------------------------" <<endl;
        cout << "Computing Fast Flow Huffman implementation with " << i << " threads:" << endl;
        FFHuffmanEncoding("../test.txt", i);
        if(i == 1) 
        {
            seq_time = usecs;
        }
        cout << endl;
        float speedup = (float)seq_time/(float)usecs;
        cout << "SPEEDUP(" << i << ") = " << speedup << endl;
        cout << "EFFICIENCY(" << i << ") = " << speedup/i << endl;
    }

    decodeCompressedText();
}