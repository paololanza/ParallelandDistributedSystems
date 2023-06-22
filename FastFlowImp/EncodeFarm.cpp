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
    typedef struct __task {
    string text;
    unordered_map<char, string> huffman_encoding;
    int id;
    int nw;
    vector<string>* encoded_vector;
    } TASK; 

    class emitter : public ff::ff_monode_t<TASK> {
    private: 
        string full_text;
        int nw;
        string encoded_text;
        unordered_map<char, string> huffman_encoding;
        vector<string>* encoded_vector;
    public:
        emitter(string full_text, int nw, unordered_map<char,string> huffman_encoding, vector<string>* encoded_vector)
                :full_text(full_text),nw(nw),huffman_encoding(huffman_encoding),encoded_vector(encoded_vector){}

        TASK * svc(TASK *) 
        {
            int start = 0;
            int interval = full_text.length()/nw;
            {
                for(int i = 0; i < nw; i++)
                {
                    auto t = new TASK(full_text.substr(start, interval), huffman_encoding, i, nw, encoded_vector);
                    ff_send_out(t);
                }
            }
            return(EOS);
        }
    };

    class collector : public ff::ff_node_t<TASK> {
    private: 
    TASK * tt;

    public: 
    collector(){}

    TASK * svc(TASK * t) {
        free(t);
        return(GO_ON);
    }

    };

    TASK *  worker(TASK * t, ff::ff_node* nn) {
        auto text = t->text; 
        //auto encoded_text = t->encoded_text;
        auto huffman_encoding = t->huffman_encoding;
        auto id = t->id;
        auto nw = t->nw;
        auto encoded_vector = t->encoded_vector;

        string enc;

        for(char c : text)
        {         
            enc += huffman_encoding[c];
        }
        (*encoded_vector)[id] = enc;
        return t;
    }
}