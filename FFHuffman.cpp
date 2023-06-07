#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <map>
#include <future>
#include <vector>

#include "utimer.hpp"
#include "MapFarm.cpp"

using namespace std;
using namespace ff;


// A Tree node
struct Node
{
	string ch;
	int freq;
	Node *left, *right;
};

// Function to allocate a new tree node
Node* getNode(char ch, int freq, Node* left, Node* right)
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

// void map_line(string line, 
//               unordered_map<string,int> &mapper,
//               int &character_num)
// {
//     // stringstream class check1
//     stringstream check1(line);
     
//     string intermediate;

//     unordered_map<string,int> temp;
     
//     // Tokenizing w.r.t. space ' '
//     while(getline(check1, intermediate, ' '))
//     {
//         temp[intermediate] += 1;
//     }
//     transform(// execution::par,
// 	    input.begin(), input.end(), 
// 	    mapperOut.begin(),   
// 	    mapper);

//     for(auto item : temp)
//     {
//         mtx.lock();
//         mapper[item.first] += item.second;
//         mtx.unlock();
//     }
// }

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
    unordered_map<char, int> map;
    int nw = 4;
    long usecs; 
    {
        utimer t0("Reading file and compute statistics ",&usecs); 
        auto e = emitter("texttest.txt", nw, &map);
        auto c = collector(); 
        // cout << "---> " << workers.size() << endl; 
        ff::ff_Farm<TASK> mf(worker, nw);
        mf.add_emitter(e);
        mf.add_collector(c);
        mf.run_and_wait_end();
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
            for (auto pair: map) 
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
                leaf_nodes.push(getNode('\0', sum, left, right));
            }
        }
        //root stores pointer to root of Huffman Tree
        Node* root = leaf_nodes.top();

        unordered_map<string, string> huffmanCode;
        long utime_encode_tree;
        {
            utimer t0("Encode Huffman tree", &utime_encode_tree);

            // traverse the Huffman Tree and store Huffman Codes
            // in a map. Also prints them
            encode(root, "", huffmanCode);
        }

        // readFile.clear();
        // readFile.open(text, ios::in);
        vector<string> encoded_text;
        long utime_encode_text;
        {
            auto e = emitter("texttest.txt", nw, &encoded_text, huffmanCode);
            auto c = collector();
            // cout << "---> " << workers.size() << endl; 
            ff::ff_Farm<TASK> mf(worker, nw);
            mf.add_emitter(e);
            mf.add_collector(c);
            mf.run_and_wait_end();
        }

        // traverse the Huffman Tree again and this time
        // decode the encoded string
        // int index = -1;
        // cout << "\nDecoded string is: \n";
        // while (index < (int)str.size() - 2) 
        // {
        //     decode(root, index, str);
        // }
        // long a;
        // {
        //     utimer t0("read test: ", &a);
        //     readFile.clear();
        //     readFile.open(text, ios::in);
        //     vector<char> characters;
        //     while(!readFile.eof())
        //     {
        //         getline(readFile, line);
        //         for(char c : line)
        //         {
        //             characters.push_back(c);
        //         }
        //     }
        // }
    
}

// Huffman coding algorithm
int main(int argc, char * argv[])
{
	string text_path = "texttest.txt";

	buildHuffmanTree(text_path);

	return 0;
}