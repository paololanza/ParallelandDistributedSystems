#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <mutex>

#include <ff/ff.hpp>

using namespace std; 

namespace enc
{
    int total_task;

    mutex mtx; 

    typedef struct __task {
    string text;
    vector<string>* encoded_text;
    unordered_map<char, string> huffman_encoding;
    int id;
    } TASK; 

    class emitter : public ff::ff_monode_t<TASK> {
    private: 
        string pathfile;
        int nw;
        vector<string>* encoded_text;
        unordered_map<char, string> huffman_encoding;
    public:
        emitter(string pathfile, int nw, vector<string>* encoded_text, unordered_map<char,string> huffman_encoding)
                :pathfile(pathfile),nw(nw),encoded_text(encoded_text),huffman_encoding(huffman_encoding){}

        TASK * svc(TASK *) 
        {
            fstream readFile;
            string line, text;

            readFile.open(pathfile, ios::in);
            readFile.seekg(0, ios_base::end);
            int lenght = readFile.tellg();

            readFile.seekg(0);
            total_task = nw * 2;
            int interval = lenght/total_task;
            int id = 0;

            while(!readFile.eof())
            {
                getline(readFile, line);
                text += line;
                if(text.length() > interval || readFile.eof())
                {
                    //
                    auto t = new TASK(text, encoded_text, huffman_encoding, id);
                    ff_send_out(t);
                    text = "";
                    id++;
                }
            }
            return(EOS);
        }
    };

    class collector : public ff::ff_node_t<TASK> {
    private: 
    TASK * tt;

    public: 
    TASK * svc(TASK * t) {     
        free(t);
        return(GO_ON);
    }

    };

    TASK *  worker(TASK * t, ff::ff_node* nn) {
        auto text = t->text; 
        auto encoded_text = t->encoded_text; 
        auto huffman_encoding = t->huffman_encoding;
        auto id = t->id;
        string enc;
        for(char c : text)
        {         
            enc += huffman_encoding[c];
        }
        (*encoded_text)[id] = enc;
        return t;
    }
}