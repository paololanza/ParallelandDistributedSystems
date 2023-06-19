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
    unordered_map<char, int> map;
  } TASK; 

  class emitter : public ff::ff_monode_t<TASK> {
  private: 
      string pathfile;
      int nw;
  public:
    emitter(string pathfile, int nw):pathfile(pathfile),nw(nw){}

    TASK * svc(TASK *) 
      {
          fstream readFile;
          string line, text;

          readFile.open(pathfile, ios::in);
          readFile.seekg(0, ios_base::end);
          int lenght = readFile.tellg();

          readFile.seekg(0);
          int interval = lenght/nw;
          int i;
          unordered_map<char,int> map;

          while(!readFile.eof())
          {
              getline(readFile, line);
              text += line;
              if(text.length() > interval || readFile.eof())
              {
                  //
                  auto t = new TASK(text, map);
                  ff_send_out(t);
                  text = "";
                  i++;
              }
          }
          return(EOS);
      }
  };

  class collector : public ff::ff_node_t<TASK> {
  private: 
    TASK * tt;
    unordered_map<char,int>* map;

  public: 
    collector(unordered_map<char,int>* map):map(map){}

    TASK * svc(TASK * t) {
      auto temp = t->map;
      for(auto item : temp)
      {
        (*map)[item.first] += item.second;
      }
      free(t);
      return(GO_ON);
    }

  };

  TASK *  worker(TASK * t, ff::ff_node* nn) {
      auto text = t->text; 
      auto occ = t->map; 
      unordered_map<char, int> temp_map;
      for(char c : text) 
      {
          (t->map)[c] += 1;
      }
      // for(auto item : temp_map)
      // {
      //   mtx.lock();
      //   (*occ)[item.first] += item.second;
      //   mtx.unlock();
      // }
      return t;
    }
}