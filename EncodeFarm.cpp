#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <mutex>
//#include "utimer.hpp"

#include <ff/ff.hpp>		// change 1: include fastflow library code

bool pf = false; 

using namespace std; 

mutex mtx; 

typedef struct __task {
  string text;
  vector<string>* encoded_text;
  unordered_map<string, string> huffman_encoding;
} TASK; 

vector<double> v,r;                 // input and result vectors
					  
double f(double x) { return x*x; }   // function to be encoded_textped on v
inline double g(double x) { 
  for(int i=0; i<1000; i++) 
    x = sin(x); 
  return(x); 
} 


class emitter : public ff::ff_monode_t<TASK> {
private: 
    string pathfile;
	int nw;
    vector<string>* encoded_text;
    unordered_map<string, string> huffman_encoding;
public:
	enc_emitter(string pathfile, int nw, vector<string>* encoded_text, unordered_map<string,string> huffman_encoding)
            :pathfile(pathfile),nw(nw),encoded_text(encoded_text),huffman_encoding(huffman_encoding){}

	TASK * svc(TASK *) 
    {
        fstream readFile;
        string line, text;

        readFile.open(pathfile, ios::in);
        readFile.seekg(0, ios_base::end);
        int lenght = readFile.tellg();

        readFile.seekg(0);
        int interval = lenght/64;

        while(!readFile.eof())
        {
            getline(readFile, line);
            text += line;
            if(text.length() > interval || readFile.eof())
            {
                //
                auto t = new TASK(text, encoded_text, huffman_encoding);
                ff_send_out(t);
                text = "";
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
    auto huffman_encoding == t->huffman_encoding;
    string enc;
    for(char c : text)
    {         
        enc += huffman_encoding[c];
    }
    mtx.lock();
    (*encoded_text).push_back(enc);
    mtx.unlock();
    return t;
}