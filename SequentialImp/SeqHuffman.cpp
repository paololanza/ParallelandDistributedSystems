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

// Builds Huffman Tree and decode given input text
void SeqHuffmanEncoding(string pathfile)
{
    string asciiString;
    {
        utimer t0("General Time: ", &usec);
        // count frequency of appearance of each character
        // and store it in a map
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
            utimer t0("Compute Statistics on Text", &usec_compute_statistics);
            for(char ch: text) 
            {
                freq[ch]++;
            }
        }

        unordered_map<char,string> huffmanCode;
        {
            utimer t0("Create the encoded tree");
            huffmanCode = buildHuffmanEncoding(freq);
        }
        ofstream writeFile("seq_compressed_text.txt");

        readFile.clear();
        readFile.open(pathfile, ios::in);
        
        readFile.seekg(0);

        string encoded_text;

        {
            utimer t0("Encode text time", &usec_encode_text);

            while(!readFile.eof())
            {
                getline(readFile, line);
                for (char ch: line) 
                {
                    encoded_text += huffmanCode[ch];
                }
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
            utimer t1("Write the encoded file");
            writeFile << asciiString;
        }

    }

    //decode the string
    string decompressed_string;
    for(char c : asciiString)
    {
        bitset<8> binary(c);
        decompressed_string += binary.to_string();
    }

    decodeText(decompressed_string);
}

// Huffman coding algorithm
int main()
{
	string text_path = "../test.txt";

	SeqHuffmanEncoding(text_path);

    float perc = (((float)usec_compute_statistics + (float)usec_encode_text + (float)usec_compress_string)/(float)usec);
    float ems = (1/(1 - perc));
    cout << "-----------------------------------------------" << endl;
    cout << "--               Amdahl's Law                --" << endl;
    cout << "-----------------------------------------------" << endl;
    cout << "- Parallelizable time: " << usec_compute_statistics + usec_encode_text + usec_compress_string <<  endl;
    cout << "- % Parallelizable code: " << perc << "%" <<  endl;
    cout << "- Total time: " << usec << endl << endl;
    cout << "Estimated Max Speedup: 1/(1-" << perc << ") = " << ems << endl;


	return 0;
}