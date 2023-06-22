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
        string text;
        int id;
        vector<string>* out_vector;
    } TASK; 

    class emitter : public ff::ff_monode_t<TASK> {
    private: 
        int nw;
        vector<string> encoded_vector;
        vector<string>* out_vector;
    public:
        emitter(int nw, vector<string> encoded_vector, vector<string>* out_vector)
                :nw(nw),encoded_vector(encoded_vector), out_vector(out_vector){}

        TASK * svc(TASK *) 
        {
            string to_compress;
            int rem_character;

            for(int i = 0; i < nw; i++)
            {
                // if(i > 0)
                // {
                //     rem_character = encoded_vector[i-1].length() - (encoded_vector[i-1] * 8)/8;
                //     to_compress = encoded_vector[i-1].substr((encoded_vector[i-1] * 8)/8, rem_character) + (encoded_vector[i] * 8)/8;
                // }
                // else
                // {
                //     to_compress = (encoded_vector[i] * 8)/8;
                // }
                auto t = new TASK(encoded_vector[i], i, out_vector);
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
        auto text = t->text;
        auto out_vector = t->out_vector;
        auto id = t-> id;
        string asciiString;
        string comp_text;
        for (size_t i = 0; i < text.length(); i += 8) 
        {
            bitset<8> bits(text.substr(i, 8));
            comp_text += static_cast<char>(bits.to_ulong()); 
        }
        (*out_vector)[id] = comp_text;
        return t;
    }
};