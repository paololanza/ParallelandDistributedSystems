#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <mutex>

#include "utimer.hpp"
#include "./ThreadPool/ThreadPool.hpp"

using namespace std;

mutex mtx;

// A Tree node
struct Node
{
	string ch;
	int freq;
	Node *left, *right;
};

// Function to allocate a new tree node
Node* getNode(string ch, int freq, Node* left, Node* right)
{
	Node* node = new Node();

	node->ch = ch;
	node->freq = freq;
	node->left = left;
	node->right = right;

	return node;
}

// Comparison object to be used to order the heap
struct comp
{
	bool operator()(Node* l, Node* r)
	{
		// highest priority item has lowest frequency
		return l->freq > r->freq;
	}
};

// traverse the Huffman Tree and store Huffman Codes
// in a map.
void encode(Node* root, string str,
			unordered_map<string, string> &huffmanCode)
{
	if (root == nullptr)
		return;

	// found a leaf node
	if (!root->left && !root->right) {
		huffmanCode[root->ch] = str;
	}

	encode(root->left, str + "0", huffmanCode);
	encode(root->right, str + "1", huffmanCode);
}

// traverse the Huffman Tree and decode the encoded string
void decode(Node* root, int &index, string str)
{
	if (root == nullptr) {
		return;
	}

	// found a leaf node
	if (!root->left && !root->right)
	{
		cout << root->ch << endl;
		return;
	}

	index++;

	if (str[index] =='0')
		decode(root->left, index, str);
	else
		decode(root->right, index, str);
}

void map_line(string line, 
              unordered_map<string,int> &mapper,
              int &character_num)
{
    // stringstream class check1
    stringstream check1(line);
     
    string intermediate;

    unordered_map<string,int> temp;
     
    // Tokenizing w.r.t. space ' '
    while(getline(check1, intermediate, ' '))
    {
        temp[intermediate] += 1;
    }

    for(auto item : temp)
    {
        mtx.lock();
        mapper[item.first] += item.second;
        mtx.unlock();
    }
}

string encoding_text(string text, unordered_map<string, string> encoding)
{
    stringstream check1(text);
    string intermediate;
    string encoded_text = "";
    while(getline(check1, intermediate, ' '))
    {
        encoded_text += encoding[intermediate];
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
    {
        utimer t0("General Time: ", &usec);

        readFile.open(text, ios::in);
        // count frequency of appearance of each character
        // and store it in a map
        unordered_map<string, int> mapper;
        vector<future<void>> tasks;
        {
            utimer t1("Reading Characters Time:");
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
        }
        
        // Create a priority queue to store live nodes of
        // Huffman tree;
        priority_queue<Node*, vector<Node*>, comp> leaf_nodes;
        priority_queue<Node*, vector<Node*>, comp> intermediary_nodes;

        // Create a leaf node for each character and add it
        // to the priority queue.
        long usec_create_nodes;
        {
            utimer t0("Creating nodes time: ", &usec_create_nodes);
            for (auto pair: mapper) 
            {
                leaf_nodes.push(getNode(pair.first, pair.second, nullptr, nullptr));
            }
        }

        long utime_create_tree;
        {   
            utimer t0("Creating Tree time: ", &utime_create_tree);

            // do till there is more than one node in the queue
            while ((leaf_nodes.size() + intermediary_nodes.size()) != 1)
            {
                // Remove the two nodes of highest priority
                // (lowest frequency) from the queue
                Node *left = leaf_nodes.top(); leaf_nodes.pop();
                Node *right = leaf_nodes.top();	leaf_nodes.pop();

                // Create a new internal node with these two nodes
                // as children and with frequency equal to the sum
                // of the two nodes' frequencies. Add the new node
                // to the priority queue.
                int sum = left->freq + right->freq;
                leaf_nodes.push(getNode("\0", sum, left, right));
            }
        }
        // root stores pointer to root of Huffman Tree
        Node* root = leaf_nodes.top();

        unordered_map<string, string> huffmanCode;
        long utime_encode_tree;
        {
            utimer t0("Encode Huffman tree", &utime_encode_tree);

            // traverse the Huffman Tree and store Huffman Codes
            // in a map. Also prints them
            encode(root, "", huffmanCode);
        }

        readFile.clear();
        readFile.open(text, ios::in);
        string str="";
        vector<future<string>> encoding_tasks;
        long utime_encode_text;
        {
            utimer t0("Encode text time", &utime_encode_text);
            readFile.seekg(0, ios_base::end);
            int lenght = readFile.tellg();

            readFile.seekg(0);
            int interval = lenght/64;
            string line;
            string t = "";
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

            //encoding_tasks.push_back(pool.submit(encoding_text, t, huffmanCode));

            for(auto& task : encoding_tasks)
            {
                const string enc = task.get();
                writeFile << enc;
                str += enc;
            }

            pool.shutdown();
        }

        // traverse the Huffman Tree again and this time
        // decode the encoded string
        // int index = -1;
        // cout << "\nDecoded string is: \n";
        // while (index < (int)str.size() - 2) 
        // {
        //     decode(root, index, str);
        // }
    }
}

// Huffman coding algorithm
int main()
{
	string text_path = "text.txt";

	buildHuffmanTree(text_path);

	return 0;
}