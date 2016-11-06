#include "wordbuilder.h"
#include "config.h"

#include <algorithm>
#include <iostream>
#include <vector>


WordBuilder::WordBuilder()
{
	this->dictionary = NULL;
	this->dictionary_size = 0;
	this->result.clear();
	this->words.clear();
	this->min_word_length = 2;
	this->max_word_length = 0;
	this->letter_track = NULL;
	this->progress = 0.0f;
	this->done = 0.0f;
}

WordBuilder::~WordBuilder()
{
	if (dictionary != NULL)
	{
		delete[] dictionary;
	}

	if (letter_track != NULL)
	{
		delete[] letter_track;
	}

	dictionary = NULL;
	dictionary_size = 0;
	words.clear();
	result.clear();
	if (dictionary_file.is_open())
	{
		dictionary_file.close();
	}
}


int WordBuilder::open(string filename)
{
	string word;
	size_t b = 0;
	size_t wlen = 0;
	long bytes = 0;
	if (dictionary != NULL)
	{
		delete[] dictionary;
		dictionary = NULL;
		dictionary_size = 0;
		max_word_length = 0;
		words.clear();
	}

	dictionary_file.open(filename.c_str(), ios_base::in | ios_base::binary);
	if (!dictionary_file.is_open())
		return -1;


	dictionary_file.seekp(0, ios_base::end);
	bytes = dictionary_file.tellp();

	if (bytes > 0)
	{
		dictionary_size = bytes;
	}
	else
	{
		dictionary_file.close();
		return -2;
	}

	dictionary_file.seekp(0, ios_base::beg);



	dictionary = new char[dictionary_size + 1];
	if (dictionary == NULL)
	{
		dictionary_file.close();
		return -3;
	}

	dictionary_file.read(dictionary, dictionary_size);


	if (dictionary_file)
		cerr << dictionary_size << " Bytes read successfully." << endl;
	else
	{
		cerr << "error: only " << dictionary_file.gcount() << " of " << dictionary_size << " Bytes could be read" << endl;
		dictionary_file.close();
		return -4;
	}

	dictionary[dictionary_size] = 0;
	dictionary_file.close();

	b = 0;
	word = "";
	wlen = 0;
	this->max_word_length = 0;
	this->sorted = false;
	while (dictionary[b])
	{
		if ((dictionary[b] == '\r') || (dictionary[b] == '\n') || (dictionary[b] == ' ') || (dictionary[b] == '\t') || (dictionary[b] == '\0'))
		{
			if (word.length() >= min_word_length)
			{
				if (wlen > this->max_word_length)
					this->max_word_length = wlen;
				words.push_back(word);
			}
			word = "";
			wlen = 0;
		}
		else
		{
			word += dictionary[b];
			++wlen;
		}
		++b;
	}

	if (word.length() >= min_word_length)
	{
		if (wlen > this->max_word_length)
			this->max_word_length = wlen;
		words.push_back(word);
	}
	word = "";

	this->isSorted();

	if (sorted)
	{
		cerr << "Die wortliste ist sortiert." << endl;
	}
	else
	{
		cerr << "Die wortliste ist NICHT sortiert." << endl;
	}


	delete[] dictionary;
	dictionary = NULL;
	dictionary_size = 0;

#if (CONFIG_DEBUG == 1)

	cerr << ":open: Vollstaendige Wortliste" << endl;
	int w = 0;
	if (!words.empty())
	{
		for (w = 0; w < words.size(); ++w)
		{
			cerr << ":open: " << words[w] << endl;
		}
	}
#endif
	return 0;
}


bool WordBuilder::hasLetters(const string& word, const string& letter)
{
	int w = 0;
	int l = 0;
	string upper;
	string lower;


	if (word.length() != letter.length())
		return false;

	for (l = 0; l < letter.length(); ++l)
		letter_track[l] = false;

	upper = this->upper(letter);
	lower = this->lower(letter);



	for (w = 0; w < (int)word.length(); ++w)
	{
		for (l = 0; l < (int)letter.length(); ++l)
		{
			//TODO still double letters
			if ((letter_track[l] == false) && ((word[w] == upper[l]) || (word[w] == lower[l])))
			{
				letter_track[l] = true;
				break;
			}
		}
	}


	for (l = 0; l < (int)letter.length(); ++l)
	{
		if (letter_track[l] == false)
		{
			return false;
		}

	}
	//cerr<< word.length() << ">" << word << " mit " << letter.length() << ">" << letter << " == [" << upper << "] == [" << lower << "]" << endl;

	return true;
}


void WordBuilder::setLetters(string letters)
{
	this->letters = letters;
	if (letter_track)
	{
		delete[] letter_track;
		letter_track = NULL;
	}

	letter_track = new bool[letters.length()];
	if (letter_track == NULL)
	{
		cerr << ":setLetters:> letter_track == NULL" << endl;
		return;
	}
	for (int l = 0; l < letters.length(); ++l)
		letter_track[l] = false;
}

void WordBuilder::setMinWordLength(size_t word_length)
{
	this->min_word_length = word_length;
}


string WordBuilder::lower(const string& str)
{
	string result = "";
	for (int s = 0; s < str.length(); ++s)
	{
		if ((str[s] >= 65) && (str[s] <= 90))
		{
			result += str[s] - 65 + 97;
		}
		else
		{
			result += str[s];
		}
	}
	return result;
}

string WordBuilder::upper(const string& str)
{
	string result = "";
	for (int s = 0; s < str.length(); ++s)
	{
		if ((str[s] >= 97) && (str[s] <= 122))
		{
			result += str[s] - 97 + 65;
		}
		else
		{
			result += str[s];
		}
	}
	return result;

}

int WordBuilder::scan()
{
	size_t sub_size;
	size_t elems = 0;
	int count = 0;
	vector<string> subset;
	size_t sub_letters = 0;
	size_t word = 0;

	for (sub_size = min_word_length; sub_size <= letters.length(); ++sub_size)
	{
		subset.clear();
		elems = createLetterSubSets(subset, letters, sub_size, "");
		normalizeSet(subset);

		if (!subset.empty())
		{
#if CONFIG_DEBUG == 1
			string upper;
			string lower;
			cerr << ":scan: Subsets der Groesse " << sub_size << " mit " << elems << " == " << subset.size() << " Elementen." << endl;
			for (sub_letters = 0; sub_letters < subset.size(); ++sub_letters)
			{
				upper = this->upper(subset[sub_letters]);
				lower = this->lower(subset[sub_letters]);
				cerr << ":scan:     [" << subset[sub_letters] << "] == [" << upper << "] == [" << lower << "]" << endl;
			}
#endif
#if 1
			if (!words.empty())
			{
				for (sub_letters = 0; sub_letters < subset.size(); ++sub_letters)
				{
					for (word = 0; word < words.size(); ++word)
					{
						if (hasLetters(words[word], subset[sub_letters]))
						{
							result.push_back(words[word]);
							++count;
						}
					}
				}
			}
#endif
		}
	}
	return count;
}


size_t WordBuilder::normalizeSet(vector<string>& set)
{
	size_t count = 0;
	std::vector<string>::iterator unique;
	std::vector<string>::iterator entry;


	if (!set.empty())
	{
		for (unique = set.begin(); unique != set.end(); ++unique)
		{
			if (unique->compare("") != 0)
			{

				++count;
				entry = unique;
				++entry;
				while (entry != set.end())
				{
					if (unique->compare(*entry) == 0)
					{
						//after erase entry already points to next element
						*entry = "";
					}

					//compare next element
					++entry;
				}
			}

		}

		unique = set.begin(); ++unique;
		while ((unique != set.end()) && (unique->compare("") == 0))
		{
			++unique;
		}


		for (entry = set.begin(); (entry != set.end()) && (unique != set.end()); ++entry)
		{
			if (entry->compare("") == 0)
			{
				*entry = *unique;
				*unique = "";
				++unique;
				while ((unique != set.end()) && (unique->compare("") == 0))
				{
					++unique;
				}
			}
			else if (entry == unique)
			{
				++unique;
				while ((unique != set.end()) && (unique->compare("") == 0))
				{
					++unique;
				}
			}

		}

		set.resize(count);

	}
	return count;
}


vector<string>& WordBuilder::getResult()
{
	return result;
}


vector<string>& WordBuilder::getWords()
{
	return words;
}

void WordBuilder::sortWords()
{
	if (!sorted)
	{
		quickSortWords(words, 0, words.size() - 1);
	}
	this->sorted = true;
}



size_t WordBuilder::quickSortWords(vector<string>& words, long first, long last)
{
	string key; 
	long left = first;
	long right = last;
	int lsorted = 0;
	int rsorted = 0;
	string tmp;
	size_t done_words = 0;


	if ((left >= right) || (left < 0) || (right < 0) || (left >= words.size()) || (right >= words.size()))
		return 0;

	if ((last - first) == 1)
	{
		if (sortedWordsCirteria(words[first], words[last]) < 0)
		{
			tmp = words[left];
			words[left] = words[right];
			words[right] = tmp;
		}
		return 2;
	}

	key = words[first + ((last - first) / 2)];

#if CONFIG_DEBUG_PROGRESS == 1
	cerr << ":quickSortWords:> Key ist [" << (first + ((last - first) / 2)) << "] " << key << " in [" << first << "," << last << "]" <<  endl;
	size_t overall = last - first + 1;
	float percent_step = (float)(overall) / 100.0;
	float percent = 0;
	float done = 0;
#endif // CONFIG_DEBUG_PROGRESS

	while (left < right)
	{
		lsorted = sortedWordsCirteria(words[left], key);
		rsorted = sortedWordsCirteria(key, words[right]);
		while ((left < right) && ((lsorted > 0) || (rsorted>0)))
		{
			if (lsorted>0)
				++left;
			else if (rsorted>0)
				--right;
			lsorted = sortedWordsCirteria(words[left], key);
			rsorted = sortedWordsCirteria(key, words[right]);

		}
		if (left < right)
		{
			tmp = words[left];
			words[left] = words[right];
			words[right] = tmp;

			++left;
		}
#if CONFIG_DEBUG_PROGRESS == 1
		done = overall - (right - left + 1);
		if ((done / percent_step) >= (percent + 4.99))
		{
			percent = done / percent_step;
			cerr << ":quickSortWords:> Fortschritt: " << percent << "%" << endl;
		}
#endif // CONFIG_DEBUG_PROGRESS
	}

	if (left > first)
		done_words += quickSortWords(words, first, left - 1);
	else
		done_words += 1;


	if (right < last)
		done_words += quickSortWords(words, right, last);
	else
		done_words += 1;

	if (((done_words * 100) / (words.size())) > 5)
	{
		cerr << done_words << " von " << words.size() << " Worten erledigt" << endl;
	}

	return done_words;
}

bool WordBuilder::isSorted()
{
	size_t w1, w2;

	if (!words.empty())
	{
		for (w1 = 0, w2 = 1; w2 < words.size(); ++w2, ++w1)
		{
			if (sortedWordsCirteria(words[w1], words[w2]) < 0)
			{
				cerr << ":isSorted:> Zeile " << w1 << " und "  << w2 << " unsortiert!" << endl;
				this->sorted = false;
				return false;
			}
		}
	}
	this->sorted = true;
	return true;
}


int WordBuilder::sortedWordsCirteria(const string& w1, const string& w2)
{
	string lower1, lower2;
	size_t len = 0;

	lower1 = this->lower(w1);
	lower2 = this->lower(w2);

	if (w1.length() < w2.length())
		len = w1.length();
	else
		len = w2.length();

	for (int c = 0; c < len; ++c)
	{
		if (lower1[c] > lower2[c]) return -1;
		else if (lower1[c] < lower2[c]) return 1;
	}

	if (w1.length() == w2.length())
		return 0;
	else if (w1.length() > w2.length())
		return -1;
	else
		return 1;

}

size_t WordBuilder::deleteMultipleOccurence(bool case_sensitive)
{
	size_t count = 0;
	std::vector<string>::iterator unique;
	std::vector<string>::iterator entry;

	string ulower;
	string elower;

	if (!this->sorted)
		isSorted();

	done = 0;
	progress = 0;

	if (sorted)
	{
		if (!words.empty())
		{
			for (unique = words.begin(); unique != words.end(); ++unique)
			{
				done = done + 1.0;
				if ((done / words.size()) > (progress + 0.001))
				{
					progress = done / words.size();
					cerr << progress * 100 << "%" << endl;
				}
				if (unique->compare("") != 0)
				{
					if (!case_sensitive)
						ulower = this->lower(*unique);
					else
						ulower = *unique;

					++count;
					entry = unique;
					++entry;
					if (entry != words.end())
					{

						if (!case_sensitive)
							elower = this->lower(*entry);
						else
							elower = *entry;
					}
					while ((entry != words.end()) && ((ulower.compare(elower) == 0)))
					{
						*entry = "";
						++entry;
						if (!case_sensitive)
							elower = this->lower(*entry);
						else
							elower = *entry;
					}
				}

			}

			unique = words.begin(); ++unique;			
			while ((unique != words.end()) && (unique->compare("") == 0))
			{
				++unique;
			}


			for (entry = words.begin(); (entry != words.end()) && (unique != words.end()); ++entry)
			{
				if (entry->compare("") == 0)
				{
					*entry = *unique;
					*unique = "";
					++unique;
					while ((unique != words.end()) && (unique->compare("") == 0))
					{
						++unique;
					}
				}
				else if (entry == unique)
				{
					++unique;
					while ((unique != words.end()) && (unique->compare("") == 0))
					{
						++unique;
					}
				}
			}

			words.resize(count);
		}
	}
	else
	{
		count = normalizeSet(words);
	}

	return count;
}


size_t WordBuilder::createLetterSubSets(vector<string>& result, string whole_set, size_t sub_size, string built)
{
	string remain;
	int last = built.length();
	int count = 0;
	int pos = 0;


	if (sub_size > whole_set.length())
		return 0;

	if (sub_size == 0)
	{
		result.push_back(built);
		return 1;
	}

	built += ' ';
	for (pos = 0; pos <= (whole_set.length() - sub_size); ++pos)
	{
		built[last] = whole_set[pos];
		remain = whole_set.substr(pos + 1);
		count += createLetterSubSets(result, remain, sub_size - 1, built);
	}

	return count;
}