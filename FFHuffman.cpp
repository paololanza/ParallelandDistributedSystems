#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <vector>

#include "utimer.hpp"
#include "MapFarm.cpp"
#include "EncodeFarm.cpp"
#include "Huffman.cpp"

using namespace std;
using namespace ff;

// Builds Huffman Tree and decode given input text
void buildHuffmanTree(string text)
{
    unordered_map<char, int> mapper;
    int nw = 4;
    long usecs; 
    {
        utimer t0("Reading file and statistics",&usecs); 
        auto e = mp::emitter("texttest.txt", nw, &mapper);
        auto c = mp::collector(); 
        // cout << "---> " << workers.size() << endl; 
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
        auto e = enc::emitter("texttest.txt", nw, &encoded_text, huffmanCode);
        auto c = enc::collector();
        // cout << "---> " << workers.size() << endl; 
        ff::ff_Farm<enc::TASK> emf(enc::worker, nw);
        emf.add_emitter(e);
        emf.add_collector(c);
        emf.run_and_wait_end();
    }

    ofstream writeFile("compressed_text.txt");
    for(auto s : encoded_text)
        writeFile << s; 

    // traverse the Huffman Tree again and this time
    // decode the encoded string
    // int index = -1;
    // cout << "\nDecoded string is: \n";
    // while (index < (int)str.size() - 2) 
    // {
    //     decode(root, index, str);
    // }
    
}

// Huffman coding algorithm
int main(int argc, char * argv[])
{
	string text_path = "texttest.txt";

	buildHuffmanTree(text_path);

	return 0;
}