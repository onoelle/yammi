/**	  Unscharfe Suche nach Objektnamen
*
* 7-2000 bis 8-2000 by Oliver Nölle
*
* Version 1.0, last modified 18.8.2000
*

  - führt unscharfe Suche auf Objektnamen durch
  d.h. fehlertolerante Suche nach Objekten oder Strassen
  - basiert auf ngram-Zerlegung
  Artikel+Source in der c't (ich glaube 4-97)
  - Effizienz angepaßt an
    - kurzer Suchstring (ca. 2-20 Zeichen)
    - viele (ca. 10.000 - 100.000) aber dar kurze (ca. 10-30 Zeichen) Einträge in der Objektdatenbank
  - Qualität verbessert durch
    - Berechnung des Informationsgehalts eines ngrams (frequency=Auftretenshäufigkeit)

  Strategie:
  1. Suchstring + Objektnamen vorverarbeiten
  - to lowercase
  - phonetisch: ß -> ss
  - Umlaute in ae, oe, ue
  - Sonderzeichen -> Leerzeichen

  2. Suchstring in passende ngramme zerlegen, jeder bekommt eine Anfangsgewichtung nach Länge

  3. lineares durchsuchen der Textdatei, <NoBestMatches> besten Einträge werden in einer sortierten Liste gehalten



  4. Neubewertung der einzelnen ngramme nach frequency, Neusortierung der best matches liste

  folgende Verbesserungen denkbar:
  - diese 200 Einträge nach Levenshtein-Distanz neu bewerten/ordnen
  (z.B. fr suchstring "Heierweg" sollte "Haierweg" besser bewerten als "Eisweierweg"
  - anstatt den ersten beiden Stufen: Bewertung >1000 = exakter Anfangsmatch, Bewertung =1000 exaktes Vorkommen, <1000 fuzzy
  - anstatt fuzzy search und fuzzy phonetic getrennt: Bewertungen addieren / maximale Bewertung übernehmen
  - überdenken der Präsentation der Ergebnisse der verschiedenen Stufen => transparentere Bewertungen

*/

#ifndef FUZZSRCH_H
#define FUZZSRCH_H

#define  MaximumEntryLength 200		// maximum length of one searchable entry
#define  NoBestMatches 200			// how many hits to keep in best matches list
#define  MaximumNgramLen 10			// maximum length of used ngrams (more than 5 doesn't help a lot...)


// fuzzysearch returns an array of <NoBestMatches> entries of this class
// beware of null-pointers if checkNext is invoked less then <NoBestMatches> times!
// in this case (eg only five streets in db), evaluate the returned array until the first null-pointer!
// objPtr can be filled with anything when invoking FuzzySearch.checkNext(...)
class BestMatchEntry {
public:
	int sim;						// similarity coefficient, between 0(no match at all) and 1000 (all ngrams found)
	char* objName;					// original name of the object
	char* preparedName;				// preprocessed name
	void* objPtr;					// user-filled...whatever you like
	BestMatchEntry(const int sim_, const char* objName_, const char* preparedName_, void* objPtr_);
	~BestMatchEntry();
};



// NgramRep and NgramEntry are needed to represent the searchstring
// => only created once per search, so we don't worry too much about efficiency/space here

// represents a single ngram
class NgramEntry {
public:
	int length;
	int frequency;							// count how many times ngram appears
	int entropy;							// should give an information value to the appearance of this ngram
	int precondition[2];					// index of those ngrams that must be found for this ngram to be "appearable"
	bool found;								// flag for checking precondition
	char str[MaximumNgramLen];				// the ngram itself
	NgramEntry(int length_, const char* strStart, int pre1, int pre2);
};



// a searchstring is represented as a number of ngrams, organised by this class
class NgramRep {
public:
	int minNgramLen, maxNgramLen, noLevels;
	int noNgrams;							// no ngrams representing the searchstring
	int noNgramsInLevel[MaximumNgramLen];
	int lenNgramsInLevel[MaximumNgramLen];
	int maxScore;							// score of all ngrams added up = maximum score an entry can get
	NgramEntry** ngs;						// pointer to an array of ngrams
	NgramRep(char* searchStr, int length, int minNgramLen_, int maxNgramLen_);
	~NgramRep();
};




// main class for performing a fuzzy search
//
// normal use in 4 steps:
// step 1: create class + initialize with searchstring and params
// step 2: checkNext(...) for all entries in database
// step 3: (optional) newOrder(): reorder according to frequencies
// step 4: getBestMatchesList() and do sth. with the result
//
// good values: minNgramLen=2 and maxNgramLen=5
//
// class is robust against:
// - no matches at all (first found entries with sim=0 are returned)
// - searchstrings longer than MaximumEntryLength (=> cut off)
// - entries longer than MaximumEntryLength (=> cut off)
// - searchstrings with length 0 (no meaningful result, but no crash)
// - entries with length 0 (probably won't appear in result, which should be okay)
//
// beware of null-pointers in step 4 if best matches list is not completely filled
// (not enough objects checked to fill list)
class FuzzySearch {
public:
	// creates a not initialized class
	FuzzySearch();

	// creates and initializes class
	FuzzySearch(const char* searchstStr, int minNgramLen, int maxNgramLen);

	// frees all resources, including the best matches list
	~FuzzySearch();

	// initializes an existing class
	void initialize(const char* searchstStr, int minNgramLen, int maxNgramLen);

	// checks next entry from database, returns similarity value for that entry
	int checkNext(const char* nextObj, void* objPtr);

	// returns the result array of best matches, 0 if class was not initialized
	BestMatchEntry** getBestMatchesList();

	// performs the newOrder mechanism to take frequency of ngrams into account
	void newOrder();

	NgramEntry**	ngs;			// should be protected, but for testing we want to have access
	NgramRep*		ngr;			// same (we want to have a look at ngrams and their frequency/entropy)

protected:
	BestMatchEntry* bme[NoBestMatches];
	int		noObjects;
	int		minSim;
	int		searchStrLen;
	char	searchStr[MaximumEntryLength];
	char	preparedSearchStr[MaximumEntryLength];

	int prepareString(char* convStr, const char* originStr);
	// compares two best match entries, considering their similarity value
	static int bmeCompare(const void *elem1, const void *elem2)
	{
		int sim1=(*(BestMatchEntry**)elem1)->sim;
		int sim2=(*(BestMatchEntry**)elem2)->sim;
		return sim2-sim1;
	}
	int insertIntoOrderedList(int similarity, const char* insertObject, const char* prepObject, void* objPtr);
};

#endif

