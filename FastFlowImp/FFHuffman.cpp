#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <vector>

#include "MapFarm.cpp"
#include "EncodeFarm.cpp"
#ifndef "Huffman.cpp"
#include "../utils/Huffman.cpp"
#include "../utils/utimer.hpp"

using namespace std;
using namespace ff;

// Builds Huffman Tree and decode given input text
void FFHuffmanEncoding(string text, int nw)
{
    utimer t3("Total computation");
    unordered_map<char, int> mapper;
    long usecs; 
    {
        utimer t0("Reading file and statistics",&usecs); 
        auto e = mp::emitter("../texttest.txt", nw, &mapper);
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
    
    vector<string> encoded_text(nw*3);
    long utime_encode_text;
    {
        utimer t1("Encoding text");
        auto e = enc::emitter("../texttest.txt", nw, &encoded_text, huffmanCode);
        auto c = enc::collector();
        // cout << "---> " << workers.size() << endl; 
        ff::ff_Farm<enc::TASK> emf(enc::worker, nw);
        emf.add_emitter(e);
        emf.add_collector(c);
        emf.run_and_wait_end();
    }

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

// Huffman coding algorithm
// int main(int argc, char * argv[])
// {
// 	string text_path = "/home/p.lanza1/SPM/texttest.txt";

// 	buildHuffmanTree(text_path);

// 	return 0;
// }