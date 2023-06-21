#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <mutex>
#include <bitset>

#include <ff/ff.hpp>

using namespace std; 

namespace compression
{
    int total_task;

    mutex mtx; 

    typedef struct __task {
        string text;
        string compressed_text;
    } TASK; 

    class emitter : public ff::ff_monode_t<TASK> {
    private: 
        string text;
        int nw;
    public:
        emitter(string text, int nw)
                :text(text),nw(nw){}

        TASK * svc(TASK *) 
        {
            int character_task = text.length()/nw;
            int interval = (character_task/8)*8;
            for(int i = 0; i < nw; i++)
            {
                if(i == nw - 1)
                    interval = text.length() - interval * i;
                auto t = new TASK(text.substr(i*interval, interval), "");
                ff_send_out(t);
            }

            return(EOS);
        }
    };

    class collector : public ff::ff_node_t<TASK> {
    private: 
        TASK * tt;
        ofstream* out_stream;

    public: 
        collector(ofstream* out_stream):out_stream(out_stream){}

    TASK * svc(TASK * t) 
    {
        auto comp_text = t->compressed_text;
        (*out_stream) << comp_text;
        free(t);
        return(GO_ON);
    }

    };

    TASK *  worker(TASK * t, ff::ff_node* nn) {
        auto text = t->text;
        string asciiString;
        for (size_t i = 0; i < text.length(); i += 8) 
        {
            string byte = text.substr(i, 8); 
            bitset<8> bits(byte);
            char asciiChar = static_cast<char>(bits.to_ulong()); 
            asciiString += asciiChar;
        }
        t->compressed_text = asciiString;
        return t;
    }
};