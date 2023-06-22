#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <mutex>

#include <ff/ff.hpp>

bool pf = false; 

using namespace std; 

namespace mp
{  
  mutex mtx; 

  typedef struct __task {
    string text;
    unordered_map<char, int>* map;
    int i;
    int nw;
  } TASK; 

  class emitter : public ff::ff_monode_t<TASK> {
  private: 
      string full_text;
      int nw;
      unordered_map<char,int>* mapper;
  public:
    emitter(string full_text, int nw, unordered_map<char,int>* mapper):full_text(full_text),nw(nw),mapper(mapper){}

    TASK * svc(TASK *) 
      {
          int start = 0;
          int length = full_text.length();
          int interval = length/nw;
          for(int i = 0; i < nw; i++)
          {
            auto t = new TASK(full_text.substr(start, interval), mapper, i, nw);
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

    TASK * svc(TASK * t) {
      free(t);
      return(GO_ON);
    }

  };

  TASK *  worker(TASK * t, ff::ff_node* nn) {
      auto text = t->text; 
      auto occ = t->map;
      auto nw = t->nw;
      auto i = t->i;
      unordered_map<char, int> temp_map;

      for(char c : text) 
      {
          temp_map[c] += 1;
      }
      for(auto item : temp_map)
      {
        mtx.lock();
        (*occ)[item.first] += item.second;
        mtx.unlock();
      }
      return t;
    }
}