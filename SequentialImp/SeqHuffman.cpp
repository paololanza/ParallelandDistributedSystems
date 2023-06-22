#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <bitset>

#include "../utils/utimer.hpp"
#include "../utils/Huffman.cpp"

using namespace std;

long usec, usec_reading_ch, usec_compute_statistics, usec_encode_text, usec_compress_string;

void SeqHuffmanEncoding(string pathfile)
{
    string asciiString;
    {
        utimer t0("General Time: ", &usec);
        unordered_map<char, int> freq;
        fstream readFile;
        string line, text;

        {
            utimer t0("Reading Characters Time", &usec_reading_ch);

            readFile.open(pathfile, ios::in);

            readFile.seekg(0);

            while(!readFile.eof())
            {
                getline(readFile, line);
                text += line;
            }

            readFile.close();
        }

        {
            utimer t0("Statistics", &usec_compute_statistics);
            for(char ch: text) 
            {
                freq[ch]++;
            }
        }

        unordered_map<char,string> huffmanCode;
        {
            utimer t0("Huffman encoding");
            huffmanCode = buildHuffmanEncoding(freq);
        }
        ofstream writeFile("seq_compressed_text.txt");

        readFile.clear();
        readFile.open(pathfile, ios::in);
        
        readFile.seekg(0);

        string encoded_text;

        {
            utimer t0("Encode text", &usec_encode_text);
            
            for (char ch: text) 
            {
                encoded_text += huffmanCode[ch];
            }
            
        }

        {
            utimer t1("Compress text", &usec_compress_string);

            for (size_t i = 0; i < encoded_text.length(); i += 8) 
            {
                string byte = encoded_text.substr(i, 8); 
                bitset<8> bits(byte);
                char asciiChar = static_cast<char>(bits.to_ulong()); 
                asciiString += asciiChar;
            }
        }

        {
            utimer t1("Write out file");
            writeFile << asciiString;
        }

    }
}

int main(int argc, char * argv[])
{
    string text = (argc > 1 ? argv[1] : "../test.txt");
    
	SeqHuffmanEncoding(text);

	return 0;
}