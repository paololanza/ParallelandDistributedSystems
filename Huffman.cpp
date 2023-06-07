#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <map>

using namespace std;

// A Tree node
struct Node
{
	char ch;
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
			unordered_map<char, string> &huffmanCode)
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
		cout << root->ch;
		return;
	}

	index++;

	if (str[index] =='0')
		decode(root->left, index, str);
	else
		decode(root->right, index, str);
}

// Builds Huffman Tree and return a map that contains a characters with its encoding
unordered_map<char,string> buildHuffmanEncoding(unordered_map<char, int> mapper)
{  
    // Create a priority queue to store live nodes of
    // Huffman tree;
    priority_queue<Node*, vector<Node*>, comp> nodes;

    // Create a leaf node for each character and add it
    // to the priority queue.
    for (auto pair: mapper) 
    {
        nodes.push(getNode(pair.first, pair.second, nullptr, nullptr));
    }
    

    // do till there is more than one node in the queue
    while (nodes.size() != 1)
    {
        // Remove the two nodes of highest priority
        // (lowest frequency) from the queue
        Node *left = nodes.top(); nodes.pop();
        Node *right = nodes.top();	nodes.pop();

        // Create a new internal node with these two nodes
        // as children and with frequency equal to the sum
        // of the two nodes' frequencies. Add the new node
        // to the priority queue.
        int sum = left->freq + right->freq;
        nodes.push(getNode('\0', sum, left, right));
    }
        
    // root stores pointer to root of Huffman Tree
    Node* root = nodes.top();

    unordered_map<char, string> huffmanCode;

    // traverse the Huffman Tree and store Huffman Codes
    // in a map.
    encode(root, "", huffmanCode);

    return huffmanCode;
}