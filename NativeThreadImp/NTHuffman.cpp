#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <mutex>
#include <stdio.h>
#include <bitset>
#include <atomic>

#include "../utils/utimer.hpp"
#include "../utils/Huffman.cpp"
#include "ThreadPool/ThreadPool.hpp"

using namespace std;

long usecs, seq_time;

mutex mtx;

void count_characters(string text, unordered_map<char, int> &mapper)
{
    unordered_map<char, int> temp_map;

    for(char c : text)
    {
        temp_map[c] += 1;
    }

    for(auto item : temp_map)
    {
        mtx.lock();
        mapper[item.first] += item.second;
        mtx.unlock();
    }

}

string encoding_text(string text, unordered_map<char, string> encoding)
{
    string encoded_text = "";
    
    for(char c : text)
    {
        encoded_text += encoding[c];
    }
    
    return encoded_text;
}

string compress_byte(string text)
{
    string asciiString;
    for (size_t i = 0; i < text.length(); i += 8) 
    {
        string byte = text.substr(i, 8); 
        bitset<8> bits(byte);
        char asciiChar = static_cast<char>(bits.to_ulong()); 
        asciiString += asciiChar;
    }
    return asciiString;
}

auto NTHuffmanEncoding(string text, int nw)
{
    ThreadPool pool(nw);
    fstream readFile;
    ofstream writeFile("nt_compressed_text.txt");
    int character_num = 0;

    utimer t0("Total Time: ", &usecs);

    readFile.open(text, ios::in);

    string full_text;

    unordered_map<char, int> mapper;
    vector<future<void>> tasks;
    
    pool.init();

    string line;
    int i = 0;
    {
        utimer t0("Reading input txt file");
        while(!readFile.eof())
        {
            getline(readFile, line);
            full_text += line;
        }
        readFile.close();
    }

    {
        utimer t0("Computing Statistics");
        int start = 0;
        int interval = full_text.length()/nw;
        for(int i = 0; i < nw; i++)
        {
            if(i == nw - 1)
                interval = full_text.size() - start;
            tasks.push_back(pool.submit(count_characters, full_text.substr(start, interval), ref(mapper)));
            start += interval;
        }

        for(auto& task : tasks)
        {
            task.get();
        }
    }

    unordered_map<char,string> huffmanCode;
    {
        utimer t0("Huffman Encode");
        huffmanCode = buildHuffmanEncoding(mapper);
    }
    
    unordered_map<int,future<string>> encoding_tasks;

    vector<string> out_string(nw);
    vector<string> encoded_vector(nw);
    unordered_map<int, future<string>> compress_tasks;
    string rem_character = ""; 
    {
        utimer t0("Encoding text");

        int start = 0;
        int interval = full_text.length()/nw;
        for(int i = 0; i < nw; i++)
        {
            if(i == nw - 1)
                interval = full_text.size() - start;
            encoding_tasks[i] = pool.submit(encoding_text, full_text.substr(start, interval), huffmanCode);
            start += interval;
        }

        for(i = 0; i < nw; i++)
        {
            const string enc = encoding_tasks[i].get();
            encoded_vector[i] = enc;
        }
    }

    {
        utimer t0("Compression");
        for(i = 0; i < nw; i++)
        {
            string enc = encoded_vector[i];
            string to_compress = rem_character + enc.substr(0, ((enc.length()/8) * 8) - rem_character.length());
            rem_character = enc.substr((enc.length()/8) * 8, enc.length());
            
            compress_tasks[i] = pool.submit(compress_byte, to_compress);
        }

        for(i = 0; i < nw; i++)
        {
            const string compressed = compress_tasks[i].get();
            out_string[i] = compressed;
        }
    }

    {
        utimer t0("Write out file");
        for(int i = 0; i < nw; i++)
        {
            writeFile << out_string[i];
        }
    }

    pool.shutdown();

}

int main(int argc, char * argv[])
{
    string text = (argc > 1 ? argv[1] : "../test.txt");
    int i = 1;
    for(i; i <= 64; i=i*2)
    {
        cout << "--------------------------------------------------------------------------" <<endl;
        cout << "Computing Native Thread Huffman implementation with " << i << " threads:" << endl;
        NTHuffmanEncoding(text, i);
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