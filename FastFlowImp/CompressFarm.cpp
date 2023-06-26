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
    typedef struct __task {
        vector<string>* text;
        int id;
        vector<string>* out_vector;
    } TASK; 

    class emitter : public ff::ff_monode_t<TASK> {
    private: 
        int nw;
        vector<string>* encoded_vector;
        vector<string>* out_vector;
    public:
        emitter(int nw, vector<string>* encoded_vector, vector<string>* out_vector)
                :nw(nw),encoded_vector(encoded_vector), out_vector(out_vector){}

        TASK * svc(TASK *) 
        {
            string to_compress;
            int rem_character;

            for(int i = 0; i < nw; i++)
            {
                auto t = new TASK(encoded_vector, i, out_vector);
                ff_send_out(t);
            }

            return(EOS);
        }
    };

    class collector : public ff::ff_node_t<TASK> {
    private: 
        TASK * tt;

    public: 
        collector(){}

        TASK * svc(TASK * t) 
        {
            free(t);
            return(GO_ON);
        }

    };

    TASK *  worker(TASK * t, ff::ff_node* nn) {
        auto text_v = t->text;
        auto out_vector = t->out_vector;
        auto id = t->id;
        string rem_character;
        if(id != 0)
        {
            string prev = (*text_v)[id-1];
            rem_character = prev.substr((prev.length()/8)*8, prev.length());
        }
        string asciiString;
        string comp_text;
        string text = rem_character + (*text_v)[id];
        for (size_t i = 0; i < text.length(); i += 8)
        {
            bitset<8> bits(text.substr(i, 8));
            comp_text += static_cast<char>(bits.to_ulong());
        }
        (*out_vector)[id] = comp_text;
        return t;
    }
};