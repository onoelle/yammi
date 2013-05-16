/**	  Unscharfe Suche nach Objektnamen
*
* 7-2000 - 8-2000 by Oliver Nölle
*
* Version 1.0, last modified 18.8.2000
*
* nur Methodenrümpfe, Klassendeklarationen+Doku in fuzzsrch.h
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fuzzsrch.h"


/*class BestMatchEntry {
public:
	int sim;
	char* objName;
	char* preparedName;
	void* objPtr;
*/

BestMatchEntry::BestMatchEntry(const int sim_, const char* objName_, const char* preparedName_, void* objPtr_) {
	sim=sim_;
	objPtr=objPtr_;

	// copy objName (if entry is too long, we simply cut it off to MaximumEntryLength)
	objName=new char[MaximumEntryLength];
//	strcpy(objName, objName_);
	char*  tmpPtr=objName;
	for ( ; *objName_ && (tmpPtr-objName < MaximumEntryLength-1); tmpPtr++, objName_++)
		*tmpPtr=*objName_;
	*tmpPtr='\0';

	// copy preparedName (shouldn't be too long because already cut off in FuzzySearch::prepareString, but to be sure...)
	preparedName=new char[MaximumEntryLength];
//	strcpy(preparedName, preparedName_);
	tmpPtr=preparedName;
	for ( ; *preparedName_ && (tmpPtr-preparedName < MaximumEntryLength-1); tmpPtr++, preparedName_++)
		*tmpPtr=*preparedName_;
	*tmpPtr='\0';
}


BestMatchEntry::~BestMatchEntry() {
	delete(objName);
	delete(preparedName);
}




// one ngram-entry associated with the searchstring
// keeps track of frequencies and, corresponding, entropy of an ngram
/*class NgramEntry {
public:
int length;
int frequency;
int entropy;					// should give an information value to the appearance of this ngram
int precondition[2];			// index of those ngrams that must be found for this ngram to be "appearable"
bool found;						// found in this objectname?
char str[MaximumNgramLen];
*/
NgramEntry::NgramEntry(int length_, const char* strStart, int pre1, int pre2) {
	int i;
	length=length_;
	frequency=0;
	if (length_==0) found=true;
	else found=false;
	precondition[0]=pre1;
	precondition[1]=pre2;
	entropy=20+length*10;
	for(i=0; i<length; i++) str[i]=strStart[i];
	str[i]='\0';
}

int max(int a, int b)
{
	if(a>=b) return a;
	else return b;
}

// a representation of a searchstring as an efficient ngram-datastructure
// this representation needs to be build only once per searchstring
/*class NgramRep {
public:
int minNgramLen, maxNgramLen, noLevels;
int noNgrams;
int noNgramsInLevel[MaximumNgramLen];
int lenNgramsInLevel[MaximumNgramLen];
NgramEntry** ngs;
*/
// constructor: divide searchstring into ngrams and build data structure
NgramRep::NgramRep(char* searchStr, int length, int minNgramLen_, int maxNgramLen_)
{
	minNgramLen=minNgramLen_;
	maxNgramLen=maxNgramLen_;
	noLevels=maxNgramLen-minNgramLen+1;

	int i, j;
	for (i=minNgramLen, j=0, noNgrams=0; i<= maxNgramLen; i++, j++) {
		noNgrams+=max(length-i+1, 0);
		noNgramsInLevel[j]=max(length-i+1, 0);
		lenNgramsInLevel[j]=i;
	}

	ngs=new NgramEntry*[noNgrams+1];
	// divide searchstring into ngrams
	int pre1, pre2;
	int total=1;		// keep in ngs[0] a pseudo ngram with found always =true
	ngs[0]=new NgramEntry(0, "x", -1, -1);
	maxScore=0;

	// for each level (=length of ngrams) get all ngrams
	for(int level=0; level<noLevels; level++)
	{
		for(i = 0; i < noNgramsInLevel[level]; i++)
		{
			if (level!=0) {
				pre1=total-noNgramsInLevel[level]-1;
				pre2=total-noNgramsInLevel[level];
			} else {
				pre1=0;
				pre2=0;
			}
			ngs[total]=new NgramEntry(lenNgramsInLevel[level], &searchStr[i], pre1, pre2);
			maxScore+=ngs[total]->entropy;
			total++;
		}
	}
}


NgramRep::~NgramRep() {
	for(int i=0; i<=noNgrams; i++)
		delete(ngs[i]);
	delete(ngs);
}



/*
prepare a string:
- enclose with spaces (better matches with matching word boundaries)
- to lowercase
- replace special characters like '-', '.', ... with space
- return length
- phonetic similarities => ß -> ss, ö -> oe, ...
*/
int FuzzySearch::prepareString(char* convStr, const char* originStr)
{
	char*  tmpPtr=convStr;
	*tmpPtr++=' ';			// leading space

	// if entry is too long, we simply cut it off to MaximumEntryLength
	for ( ; *originStr && (tmpPtr-convStr < MaximumEntryLength-4); tmpPtr++, originStr++)
	{
// !!!		*tmpPtr = tolower((unsigned char)*originStr);
		*tmpPtr=*originStr;
		switch(*tmpPtr)
		{
			// pretty ugly, but in a standalone dos-shell program the entered characters are ASCII-umlaute,
			// in the db there are ANSI-umlaute, so we have to take care of both
		case -60: // ANSI-Umlaute
			*tmpPtr++ = 'a';
			*tmpPtr = 'e';
			break;
		case -28:
			*tmpPtr++ = 'a';

			*tmpPtr = 'e';
			break;
		case -114: // ASCII-Umlaute
			*tmpPtr++ = 'a';
			*tmpPtr = 'e';
			break;
		case -124:
			*tmpPtr++ = 'a';
			*tmpPtr = 'e';
			break;
		case -42:
			*tmpPtr++ = 'o';
			*tmpPtr = 'e';
			break;
		case -10:
			*tmpPtr++ = 'o';
			*tmpPtr = 'e';
			break;
		case -108:
			*tmpPtr++ = 'o';
			*tmpPtr = 'e';
			break;
		case -103:
			*tmpPtr++ = 'o';
			*tmpPtr = 'e';
			break;
		case -36:
			*tmpPtr++ = 'u';
			*tmpPtr = 'e';
			break;
		case -4:
			*tmpPtr++ = 'u';
			*tmpPtr = 'e';
			break;
		case -102:
			*tmpPtr++ = 'u';
			*tmpPtr = 'e';
			break;
		case -127:
			*tmpPtr++ = 'u';
			*tmpPtr = 'e';
			break;
		case ',': *tmpPtr = ' '; break; // so, diese Sonderzeichen kommen alle vor, alle in spaces umwandeln...
		case '-': *tmpPtr = ' '; break;
		case '.': tmpPtr--     ; break; // for yammi: discard them (R.E.M = REM)
		case '(': *tmpPtr = ' '; break;
		case ')': *tmpPtr = ' '; break;
		case '*': *tmpPtr = ' '; break;
		case '/': *tmpPtr = ' '; break;
		case '@': *tmpPtr = ' '; break; // ...bis hier
		case -33:	// scharfes s (im Textfile eingelesen)
		case -31:	// scharfes s (unter dos eingegeben)
			*tmpPtr++='s';
			*tmpPtr='s';
			break;
		case '\n':	// discard newline character
			tmpPtr--;
			break;
		case 13:	// discard linefeed? character
			tmpPtr--;
			break;
		}
	}

	*tmpPtr++=' ';		// finishing space
	*tmpPtr = '\0';
	return (tmpPtr - convStr);
}



// re-evaluate similarities (now with frequency) and re-order list of best matches
void FuzzySearch::newOrder() {
	if (noObjects==-1) return;
	NgramEntry** ngs;
	ngs=ngr->ngs;
	int i,j;

	// step 0: calculate new entropy for all ngrams (depending on frequency)
	ngr->maxScore=0;
	for(i = 1; i <= ngr->noNgrams; i++) {
		double prob=(double)ngs[i]->frequency/(double)noObjects;		// probability of appearance
		double loggi=log(prob*100+1)/log(2.0);							// clevere Formel...
		int entro = 20+(ngs[i]->length)*10;
		entro=(int) (entro*(1.0-loggi/10.0));
		if (entro<1) entro=1;

		ngr->maxScore+=entro;
		ngs[i]->entropy=entro;
	}

	// step 1: calculate new similarity value for all best matches
	for(j=0; j<NoBestMatches && bme[j]; j++) {
		int accuracy=0;
		// für jedes ngram match überprüfen
		for(i=1; i<=ngr->noNgrams; i++) {
			ngs[i]->found=false;
			if(ngs[ngs[i]->precondition[0]]->found==true && ngs[ngs[i]->precondition[1]]->found==true) {
				if(strstr(bme[j]->preparedName, ngs[i]->str)) {
					accuracy += (ngs[i]->entropy);
					ngs[i]->found=true;
				}
			}
		}
		accuracy=accuracy*1000/ngr->maxScore;
		bme[j]->sim=accuracy;
	}

	// step 2: re-order list
	qsort((void*)bme, j, sizeof(bme[0]), bmeCompare );
	return;
}



// insert into best matches list (ordered by similarity)
int FuzzySearch::insertIntoOrderedList(int similarity, const char* insertObject, const char* prepObject, void* objPtr)
{
	int i, j;
	for (i=0; i<NoBestMatches; i++) {
		if (bme[i]==NULL) {
			bme[i]=new BestMatchEntry(similarity, insertObject, prepObject, objPtr);
			break;
		}
		else
			if (bme[i]->sim<similarity) {
				// delete last entry coz it will be dropped
				if (bme[NoBestMatches-1]!=NULL)
					delete(bme[NoBestMatches-1]);
				for(j=NoBestMatches-1; j>i; j--) {
					bme[j]=bme[j-1];
				}
				bme[i]=new BestMatchEntry(similarity, insertObject, prepObject, objPtr);
				break;
			}
	}
	if (bme[NoBestMatches-1]!=NULL)
		return bme[NoBestMatches-1]->sim;
	else
		return -1;
}


void FuzzySearch::initialize(const char* searchStr_, int minNgramLen, int maxNgramLen)
{
	if (noObjects!=-1) {						// object was already initialized => we have to clean up first
		// this is exactly the same as what the destructor does (but we can't call it, can we?)
		// clean up bme-list
		for(int i=0; i<NoBestMatches; i++) {
			if (bme[i]!=0)
				delete(bme[i]);
		}
		delete ngr;
	}

	// cut off searchstring to MaximumEntryLength
	//	strcpy(searchStr, searchStr_);
	char*  tmpPtr=searchStr;
	for ( ; *searchStr_ && (tmpPtr-searchStr < MaximumEntryLength-1); tmpPtr++, searchStr_++)
		*tmpPtr=*searchStr_;
	*tmpPtr='\0';

	// initialize list of best matches
	for (int i=0; i<NoBestMatches; i++)
		bme[i]=NULL;
	noObjects=0;
	minSim=-1;

	//	printf("searchstring vorbereiten...");
	searchStrLen = prepareString(preparedSearchStr, searchStr);
	//	printf(" result: <%s>\n", preparedSearchStr);

	//	printf("erzeuge ngramrep...\n");
	ngr=new NgramRep(preparedSearchStr, searchStrLen, minNgramLen, maxNgramLen);
	ngs=ngr->ngs;
}


// default-constructor
FuzzySearch::FuzzySearch()
{
	noObjects=-1;
}


// init-constructor
FuzzySearch::FuzzySearch(const char* searchStr_, int minNgramLen, int maxNgramLen)
{
	initialize(searchStr_, minNgramLen, maxNgramLen);
}


// destructor
FuzzySearch::~FuzzySearch()
{
	if (noObjects==-1) return;
	// clean up bme-list
	for(int i=0; i<NoBestMatches; i++) {
		if (bme[i]!=0)
			delete(bme[i]);
	}
	delete ngr;
}


// perform searchstep
// returns: similarity coefficient between 0 and 1, -1 if error
int FuzzySearch::checkNext(const char* nextEntry, void* objPtr)
{
	if (noObjects==-1) return -1;
	char	preparedNextEntry[MaximumEntryLength];
	int		accuracy=0;
	int		i;

	if (*nextEntry==0)
		return -1;

	noObjects++;
	// prepare the string (leading space, lowercase, ...)
	prepareString(preparedNextEntry, nextEntry);

	// für jedes ngram match überprüfen
	for(i=1; i<=ngr->noNgrams; i++) {
		ngs[i]->found=false;
		if(ngs[ngs[i]->precondition[0]]->found==true && ngs[ngs[i]->precondition[1]]->found==true) {
			if(strstr(preparedNextEntry, ngs[i]->str)) {
				accuracy += ngs[i]->entropy;
				ngs[i]->frequency++;
				ngs[i]->found=true;
			}
		}
	}
	accuracy=(accuracy*1000)/ngr->maxScore;
	// Wenn Guete >= minimaler Wert in best matches liste: insert into list
	if(accuracy > minSim) {
		minSim=insertIntoOrderedList(accuracy, nextEntry, preparedNextEntry, objPtr);
	}
	return accuracy;
}

// returns a sorted array of the best matches
// returns 0 if class was not initialized
BestMatchEntry** FuzzySearch::getBestMatchesList() {
	if (noObjects==-1) return 0;
		else return bme;
}
