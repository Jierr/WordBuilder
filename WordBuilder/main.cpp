#include <stdint.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "config.h"
#include "wordbuilder.h"

using namespace std;

enum{
	OPTION_NONE = 0,
	OPTION_SORT = 0x01,
	OPTION_UNIQUE = 0x02,
	OPTION_SCRABBLE = 0x04,
};

//"G:\\Development\\wort_liste2.txt"
int main(int argc, char** argv)
{
	WordBuilder wordbuilder;

	int a = 1;
	string arg = "";
	string src = "";
	string letters = "";
	int min_word_length = 1;
	stringstream ss;
	uint32_t option = OPTION_NONE;

	while (a < argc)
	{
		arg = argv[a];
		if (arg.compare("-src") == 0)
		{
			++a;
			cerr << "Option: " << arg;
			if (a < argc)
			{
				src = argv[a];
				cerr << ": Datei: " << src << endl;
			}
			else
			{
				cerr << ": File missing!" << endl;
				return -1;
			}
		}
		else if ((arg.compare("-sort") == 0) || (arg.compare("-s") == 0))
		{
			option |= OPTION_SORT;
			cerr << "Option: " << arg << endl;
		}
		else if ((arg.compare("-unique") == 0) || (arg.compare("-u") == 0))
		{
			option |= OPTION_UNIQUE;
			cerr << "Option: " << arg << endl;
		}
		else if ((arg.compare("-su") == 0) || (arg.compare("-us") == 0))
		{
			option |= OPTION_UNIQUE | OPTION_SORT;
			cerr << "Option: " << arg << endl;
		}
		else if ((arg.compare("-min") == 0))
		{
			++a;
			cerr << "Option: " << arg;
			if (a < argc)
			{
				ss.str("");
				ss << string(argv[a]);
				ss >> min_word_length;
				cerr << ": Mindestwortlaenge: " << min_word_length << endl;
			}
		}
		else if ((arg.compare("-scrabble") == 0) || (arg.compare("-l") == 0))
		{
			++a;
			cerr << "Option: " << arg;
			if (a < argc)
			{
				option |= OPTION_SCRABBLE;
				letters = argv[a];
				cerr << ": Buchstaben: " << letters << endl;
			}
			else
			{
				cerr << ": Letters missing!" << endl;
				return -2;
			}
		}
		else
		{
			cerr << "Unbekannte Option: " << arg << endl;
		}
		++a;
	}


	cerr << "Oeffne Datei " << src << " ... "<<endl;
	wordbuilder.setMinWordLength(min_word_length);
	if (wordbuilder.open(src) != 0)
		return -1;

	if (option & OPTION_SORT)
	{
		cerr << "Sorting wordlist" << endl;
		wordbuilder.sortWords();
		if (wordbuilder.isSorted())
		{
			cerr << "Wortliste ist sortiert!" << endl;
		}
		else
		{
			cerr << "Wortliste ist NICHT sortiert!" << endl;
		}


		if (((option & OPTION_SCRABBLE) == 0) && ((option & OPTION_UNIQUE) == 0))
		{
			std::vector<string>::iterator word;
			vector<string>& matches = wordbuilder.getWords();

			if (!matches.empty())
			{
				for (word = matches.begin(); word != matches.end(); ++word)
				{
					cout << *word << endl;
				}
			}
		}
	}

	if (option & OPTION_UNIQUE)
	{
		cerr << "Deleting multiples of words" << endl;
		wordbuilder.deleteMultipleOccurence();


		if (((option & OPTION_SCRABBLE) == 0))
		{
			std::vector<string>::iterator word;
			vector<string>& matches = wordbuilder.getWords();

			if (!matches.empty())
			{
				for (word = matches.begin(); word != matches.end(); ++word)
				{
					cout << *word << endl;
				}
			}
		}
	}

	if (option & OPTION_SCRABBLE)
	{
		cerr << "Doing Scrabble..." << endl;
		wordbuilder.setLetters(letters);
		//wordbuilder.setLetters("AEEIGLLMORTY");
		//wordbuilder.setLetters("VEAOREHSBNMT");

		wordbuilder.scan();


		std::vector<string>::iterator word;
		vector<string>& matches = wordbuilder.getResult();

		if (!matches.empty())
		{
			for (word = matches.begin(); word != matches.end(); ++word)
			{
				cout << *word << endl;
			}
		}
	}
	return 0;

}
