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


#include "../utils/utimer.hpp"
#include "../utils/Huffman.cpp"
#include "ThreadPool/ThreadPool.hpp"

using namespace std;

mutex mtx;

long usecs, seq_time;

unordered_map<char,int> map_line(string line, unordered_map<char,int>& mapper)
{
    unordered_map<char,int> temp;

    for(char c : line)
    {
        temp[c] += 1;
    }

    for(auto item : temp)
    {
        mtx.lock();
        mapper[item.first] += item.second;
        mtx.unlock();
    }

    return temp;
}

string encoding_text(string text, unordered_map<char, string> encoding)
{
    stringstream check1(text);
    string intermediate;
    string encoded_text = "";
    for(char c : text)
    {
        encoded_text += encoding[c];
    }
    return encoded_text;
}

string encoding_text1(vector<string> full_text, unordered_map<char, string> encoding, int start, int end)
{
    // string text = full_text.substr(start, interval);
    string encoded_text = "";
    // for(char c : text)
    // {
    //     encoded_text += encoding[c];
    // }
    for(int i = start; i < end; i++)
    {
        for(char c : full_text[i])
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
    vector<future<unordered_map<char,int>>> tasks;
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
        utimer t0("Reading and Computing Statistics");
        while(!readFile.eof())
        {
            getline(readFile, line);
            t += line;
            //text_v.push_back(line);
        
            if(t.length() > interval || readFile.eof())
            {
                //submit the task to the threadpool
                tasks.push_back(pool.submit(map_line, t, ref(mapper)));
                //cout << t.length() << endl;
                t = "";
                i++;
                if(i == nw || i == nw + ((nw*load_balancing)/100))
                    interval = interval/2;
            }
        }

        for(auto& task : tasks)
        {
            //const auto temp_map = 
            task.get();
            // for(auto item : temp_map)
            // {
            //     mapper[item.first] += item.second;
            // }
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
        utimer t0("Encoding text and writing file");
        while(!readFile.eof())
        {
            getline(readFile, line);
            t += line;
            if(t.length() > interval || readFile.eof())
            {
                encoding_tasks[i] = pool.submit(encoding_text, t, huffmanCode);
                t = "";
                i++;
                if(i == nw || i == nw + ((nw*15)/100))
                    interval = interval/2;
            }
        }
        int total_task = i;
        // // int start = 0;
        // // int task_num = full_text.length()/8;
        // // int task_per_workers = task / nw;

        // int i = 0;
        // int start = 0;
        // int end = text_v.size()/nw;
        // for(int i = 0; i < nw; i++)
        // {
        //     encoding_tasks.push_back(pool.submit(encoding_text1, text_v, huffmanCode, start, end));
        // }

        //for(auto& task : encoding_tasks)
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
            writeFile << enc;
        }
    }

    pool.shutdown();


// traverse the Huffman Tree again and this time
// decode the encoded string
// int index = -1;
// cout << "\nDecoded string is: \n";
// while (index < (int)str.size() - 2) 
// {
//     decode(root, index, str);
// }
    
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
}