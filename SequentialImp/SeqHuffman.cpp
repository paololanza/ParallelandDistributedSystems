#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>

#include "../utils/utimer.hpp"
#include "../utils/Huffman.cpp"

using namespace std;

// Builds Huffman Tree and decode given input text
void SeqHuffmanEncoding(string pathfile)
{
    long usec;
    {
        utimer t0("General Time: ", &usec);
        // count frequency of appearance of each character
        // and store it in a map
        unordered_map<char, int> freq;
        fstream readFile;
        string line, text;

        long usec_reading_ch; 
        {
            utimer t0("Reading Characters Time: ", &usec_reading_ch);

            readFile.open(pathfile, ios::in);

            readFile.seekg(0);

            while(!readFile.eof())
            {
                getline(readFile, line);
                for (char ch: line) 
                {
                    freq[ch]++;
                }
            }

            readFile.close();
        }
        
        auto huffmanCode = buildHuffmanEncoding(freq);

        long utime_encode_text;
        {
            utimer t0("Encode text time", &utime_encode_text);

            ofstream writeFile("seq_compressed_text.txt");

            readFile.clear();
            readFile.open(pathfile, ios::in);
            
            readFile.seekg(0);

            string encoded_text;

            while(!readFile.eof())
            {
                getline(readFile, line);
                for (char ch: line) 
                {
                    encoded_text += huffmanCode[ch];
                }
            }

            writeFile << encoded_text;
        }

        //cout << "\nEncoded string is :\n" << str << '\n';

        // traverse the Huffman Tree again and this time
        // decode the encoded string
        int index = -1;
        //cout << "\nDecoded string is: \n";
        // while (index < (int)str.size() - 2) {
        //     decode(root, index, str);
        // }
    }
}

// Huffman coding algorithm
// int main()
// {
// 	string text_path = "texttest.txt";

// 	buildHuffmanTree(text_path);

// 	return 0;
// }