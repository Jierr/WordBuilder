#ifndef WORDBUILDER_H
#define WORDBUILDER_H

#include <fstream>
#include <string>
#include <vector>

using namespace std;

class WordBuilder
{
public:
	WordBuilder();
	virtual ~WordBuilder();

	int open(string filename);
	bool hasLetters(const string& word, const string& letter);
	void setLetters(string letters);
	int scan();
	void setMinWordLength(size_t word_length);
	size_t createLetterSubSets(vector<string>& result, string whole_set, size_t sub_size, string built);
	vector<string>& getResult();
	vector<string>& getWords();
	void sortWords();
	size_t quickSortWords(vector<string>& words, long first, long last);
	bool isSorted();
	int sortedWordsCirteria(const string& w1, const string& w2);
	size_t deleteMultipleOccurence();

	string lower(const string& str);
	string upper(const string& str);


protected:
private:
	string letters;
	bool* letter_track;

	string dictionary_filename;
	fstream dictionary_file;
	size_t dictionary_size;
	char* dictionary;
	size_t min_word_length;
	size_t max_word_length;
	vector<string> words;
	vector<string> result;

	double progress;
	double done;

	bool sorted;

	size_t normalizeSet(vector<string>& set);

};

#endif // WORDBUILDER_H


