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

mutex mtx;

long usecs, seq_time;
string res;

// unordered_map<char,int> map_line(string line, unordered_map<char,int>& mapper)
// {
//     unordered_map<char,int> temp;

//     for(char c : line)
//     {
//         temp[c] += 1;
//     }

//     for(auto item : temp)
//     {
//         mtx.lock();
//         mapper[item.first].fetch_add(item.second);
//         mtx.unlock();
//     }

//     return temp;
// }

void count_characters(string full_text, int i, int nw, unordered_map<char, int> &mapper)
{
    int interval = full_text.length()/nw;
    int start = i * interval;

    unordered_map<char, int> temp_map;

    for(char c : full_text.substr(start, interval))
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

// string encoding_text(string text, unordered_map<char, string> encoding)
// {
//     // stringstream check1(text);
//     // string intermediate;
//     // string encoded_text = "";
//     // for(char c : text)
//     // {
//     //     encoded_text += encoding[c];
//     // }
//     // return encoded_text;
// }

string encoding_text1(string full_text, unordered_map<char, string> encoding, int i, int nw)
{
    int interval = full_text.length()/nw;
    int start = i * nw;
    string encoded_text = "";
    
    for(char c : full_text.substr(start, interval))
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

//template<typename T>
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
    vector<string> text_v;
    
    pool.init();

    string line;
    string t = "";
    int i = 0;

    readFile.seekg(0, ios_base::end);
    int lenght = readFile.tellg();
    cout << lenght;

    readFile.seekg(0);
    int load_balancing = 0;
    int interval = lenght/(nw+(nw*load_balancing)/100);

    {
        utimer t0("Reading input txt file");
        while(!readFile.eof())
        {
            getline(readFile, line);
            full_text += line;
        }
    }

    {
        utimer t0("Computing Statistics");
        // while(!readFile.eof())
        // {
        //     getline(readFile, line);
        //     t += line;
        //     //text_v.push_back(line);
        
        //     if(t.length() > interval || readFile.eof())
        //     {
        //         //submit the task to the threadpool
        //         tasks.push_back(pool.submit(map_line, t, ref(mapper)));
        //         //cout << t.length() << endl;
        //         t = "";
        //         i++;
        //         if(i == nw || i == nw + ((nw*load_balancing)/100))
        //             interval = interval/2;
        //     }
        // }
        for(int i = 0; i < nw; i++)
        {
            tasks.push_back(pool.submit(count_characters, full_text, i, nw, ref(mapper)));
        }

        for(auto& task : tasks)
        {
            task.get();
        }
    }

    readFile.close();

    auto huffmanCode = buildHuffmanEncoding(mapper);

    readFile.clear();
    readFile.open(text, ios::in);
    string str="";
    unordered_map<int,future<string>> encoding_tasks;
    int total_task = nw;//(nw+(nw*20/100));
    interval = lenght/total_task;

    readFile.seekg(0);
    t = "";
    i = 0;
    {
        utimer t0("Encoding text and compression");
        // while(!readFile.eof())
        // {
        //     getline(readFile, line);
        //     t += line;
        //     if(t.length() > interval || readFile.eof())
        //     {
        //         encoding_tasks[i] = pool.submit(encoding_text, t, huffmanCode);
        //         t = "";
        //         i++;
        //         if(i == nw || i == nw + ((nw*15)/100))
        //             interval = interval/2;
        //     }
        // }
        // int total_task = i;

        {
            utimer t0("-- Encoding Threads Overhead");
            for(int i = 0; i < nw; i++)
            {
                encoding_tasks[i] = pool.submit(encoding_text1, full_text, huffmanCode, i, nw);
            }
        }

        vector<byte> byte_array;
        unordered_map<int, future<string>> compress_tasks;
        string rem_character = ""; 
        for(i = 0; i < total_task; i++)
        {
            const string enc = encoding_tasks[i].get();
            string to_compress = rem_character + enc.substr(0, ((enc.length()/8) * 8) - rem_character.length());
            rem_character = enc.substr((enc.length()/8) * 8, enc.length());
            compress_tasks[i] = pool.submit(compress_byte, to_compress);
        }

        for(i = 0; i < total_task; i++)
        {
            const string enc = compress_tasks[i].get();
            res += enc;
        }
    }

    {
        utimer t0("Write out file");
        writeFile << res;
    }

    pool.shutdown();

}

int main()
{
    int i = 1;
    for(i; i <= 64; i=i*2)
    {
        cout << "--------------------------------------------------------------------------" <<endl;
        cout << "Computing Native Thread Huffman implementation with " << i << " threads:" << endl;
        NTHuffmanEncoding("../test.txt", i);
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