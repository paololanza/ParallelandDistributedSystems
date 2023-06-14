#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <mutex>
#include <stdio.h>


#include "../utils/utimer.hpp"
#include "../utils/Huffman.cpp"
#include "ThreadPool/ThreadPool.hpp"

using namespace std;

mutex mtx;

long usecs, seq_time;

void map_line(string line, 
              unordered_map<char,int> &mapper,
              int &character_num)
{
    // stringstream class check1
    stringstream check1(line);
     
    string intermediate;

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

// Builds Huffman Tree and decode given input text
void NTHuffmanEncoding(string text, int nw)
{
    ThreadPool pool(nw);
    fstream readFile;
    ofstream writeFile("compressed_text.txt");
    int character_num = 0;

    utimer t0("Total Time: ", &usecs);

    readFile.open(text, ios::in);

    unordered_map<char, int> mapper;
    vector<future<void>> tasks;
    
    pool.init();

    string line;
    string t = "";
    int i = 0;

    readFile.seekg(0, ios_base::end);
    int lenght = readFile.tellg();

    readFile.seekg(0);
    int interval = lenght/nw+(nw*20)/100;

    {
        utimer t0("Reading and Computing Statistics");
        while(!readFile.eof())
        {
            getline(readFile, line);
            t += line;
            if(t.length() > interval || readFile.eof())
            {
                //submit the task to the threadpool
                tasks.push_back(pool.submit(map_line, t, ref(mapper), ref(character_num)));
                //cout << t.length() << endl;
                t = "";
                i++;
                if(i == 64 || i == 116)
                    interval = interval/2;
            }
        }

        for(auto& task : tasks)
        {
            task.get();
        }
    }
    // char c[interval] = "";
    // while(!readFile.eof())
    // {
    //     readFile.read(c, sizeof(c));
    //     t += c;
    //     int a = readFile.tellg();
    //     //cout << a << endl;

    //     if(t.length() == interval || readFile.eof())
    //     {
    //         tasks.push_back(pool.submit(map_line, t, ref(mapper), ref(character_num)));
    //         t = "";
    //     }
    // }
    // cout << lenght << "-" << interval << endl;

    readFile.close();


    auto huffmanCode = buildHuffmanEncoding(mapper);

    readFile.clear();
    readFile.open(text, ios::in);
    string str="";
    vector<future<string>> encoding_tasks;
    interval = lenght/nw+(nw*20/100);

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
                encoding_tasks.push_back(pool.submit(encoding_text, t, huffmanCode));
                t = "";
                i++;
                if(i == 64 || i == 116)
                    interval = interval/2;
            }
        }

        for(auto& task : encoding_tasks)
        {
            const string enc = task.get();
            writeFile << enc;
            str += enc;
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
        NTHuffmanEncoding("../test1.txt", i);
        if(i == 1) 
        {
            seq_time = usecs;
        }
        cout << endl;
        float speedup = (float)seq_time/(float)usecs;
        cout << "SPEEDUP(" << i << ") = " << speedup << endl;
    }
}