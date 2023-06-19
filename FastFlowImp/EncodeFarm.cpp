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
    string encoded_text;
    unordered_map<char, string> huffman_encoding;
    int id;
    } TASK; 

    class emitter : public ff::ff_monode_t<TASK> {
    private: 
        string pathfile;
        int nw;
        string encoded_text;
        unordered_map<char, string> huffman_encoding;
    public:
        emitter(string pathfile, int nw, unordered_map<char,string> huffman_encoding)
                :pathfile(pathfile),nw(nw),huffman_encoding(huffman_encoding){}

        TASK * svc(TASK *) 
        {
            fstream readFile;
            string line, text;

            readFile.open(pathfile, ios::in);
            readFile.seekg(0, ios_base::end);
            int lenght = readFile.tellg();

            readFile.seekg(0);
            int load_balancing = 0;
            total_task = nw + (nw * load_balancing)/ 100;
            int interval = lenght/total_task;
            int id = 0;

            while(!readFile.eof())
            {
                getline(readFile, line);
                text += line;
                if(text.length() > interval || readFile.eof())
                {
                    //
                    auto t = new TASK(text, "", huffman_encoding, id);
                    ff_send_out(t);
                    text = "";
                    id++;
                    if (id == nw || id == nw + (nw*load_balancing)/100)
                        interval = interval / 2;
                }
            }
            return(EOS);
        }
    };

    class collector : public ff::ff_node_t<TASK> {
    private: 
    TASK * tt;
    vector<string>* encoded_vector;
    ofstream* out_stream;

    public: 
    collector(vector<string>* encoded_vector, ofstream* out_stream):encoded_vector(encoded_vector), out_stream(out_stream){}

    TASK * svc(TASK * t) {     
        auto id = t->id;
        auto text = t->encoded_text;
        (*out_stream) << text;
        free(t);
        return(GO_ON);
    }

    };

    TASK *  worker(TASK * t, ff::ff_node* nn) {
        auto text = t->text; 
        //auto encoded_text = t->encoded_text; 
        auto huffman_encoding = t->huffman_encoding;
        auto id = t->id;
        string enc;
        for(char c : text)
        {         
            enc += huffman_encoding[c];
        }
        //(*encoded_text)[id] = enc;
        t->encoded_text = enc;
        return t;
    }
}