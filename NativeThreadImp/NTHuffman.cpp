#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <mutex>

#include "utimer.hpp"
#include "../ThreadPool/ThreadPool.hpp"
#include "Huffman.cpp"

using namespace std;

mutex mtx;

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
void buildHuffmanTree(string text)
{
    ThreadPool pool(64);
    fstream readFile;
    ofstream writeFile("compressed_text.txt");
    int character_num = 0;
    long usec;

    utimer t0("General Time: ", &usec);

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
    int interval = lenght/64;

    while(!readFile.eof())
    {
        getline(readFile, line);
        t += line;
        if(t.length() > interval || readFile.eof())
        {
            //submit the task to the threadpool
            tasks.push_back(pool.submit(map_line, t, ref(mapper), ref(character_num)));
            t = "";
        }
    }
    
    for(auto& task : tasks)
    {
        task.get();
    }

    readFile.close();


    auto huffmanCode = buildHuffmanEncoding(mapper);

    readFile.clear();
    readFile.open(text, ios::in);
    string str="";
    vector<future<string>> encoding_tasks;

    readFile.seekg(0);
    t = "";
    while(!readFile.eof())
    {
        getline(readFile, line);
        t += line;
        if(t.length() > interval || readFile.eof())
        {
            encoding_tasks.push_back(pool.submit(encoding_text, t, huffmanCode));
            t = "";
        }
    }

    for(auto& task : encoding_tasks)
    {
        const string enc = task.get();
        writeFile << enc;
        str += enc;
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
	string text_path = "../texttest.txt";

	buildHuffmanTree(text_path);

	return 0;
}