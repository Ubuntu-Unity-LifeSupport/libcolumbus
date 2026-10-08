/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Matcher.hh"
#include "Corpus.hh"
#include "LevenshteinIndex.hh"
#include "Word.hh"
#include "Document.hh"
#include "WordList.hh"
#include "IndexMatches.hh"
#include "MatchResults.hh"
#include "ErrorValues.hh"
#include "ColumbusHelpers.hh"
#include "IndexWeights.hh"
#include "MatcherStatistics.hh"
#include "WordStore.hh"
#include "ResultFilter.hh"
#include "SearchParameters.hh"
#include <cassert>
#include <stdexcept>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#ifdef HAS_SPARSE_HASH
#include <google/sparse_hash_map>
#include <google/sparse_hash_set>
using google::sparse_hash_map;
using google::sparse_hash_set;
#define hashmap sparse_hash_map
#define hashset sparse_hash_set
#else
#include <unordered_map>
#include <unordered_set>
#define hashmap unordered_map
#define hashset unordered_set
#endif

COL_NAMESPACE_START
using namespace std;

struct idhasher : std::unary_function<const pair<WordID, WordID>, size_t> {
    size_t operator() ( const pair<WordID, WordID> &p) const {
        size_t w1 = p.first;
        size_t w2 = p.second;
        if(sizeof(size_t) > sizeof(WordID)) {
            return (w1 << 32) | w2; // Assuming size_t is 64 bits.
        } else {
            return w1 ^ w2;
        }
    }
};

typedef hashset<DocumentID> DocumentSet;
typedef hashmap<WordID, LevenshteinIndex*> IndexMap;
typedef hashmap<std::pair<WordID, WordID>, DocumentSet, idhasher > ReverseIndexData; // Index name, word, documents.

typedef IndexMap::iterator IndIterator;
typedef ReverseIndexData::iterator RevIndIterator;

typedef map<WordID, int> MatchErrorMap;

typedef map<WordID, MatchErrorMap> BestIndexMatches;

typedef BestIndexMatches::iterator MatchIndIterator;
typedef MatchErrorMap::iterator MatchIterator;


class ReverseIndex {
private:
    ReverseIndexData reverseIndex;
public:

    void add(const WordID wordID, const WordID indexID, const DocumentID id);
    bool documentHasTerm(const WordID wordID, const WordID indexID, DocumentID id);
    void findDocuments(const WordID wordID, const WordID indexID, std::vector<DocumentID> &result);
};

struct MatcherPrivate {
    IndexMap indexes;
    ReverseIndex reverseIndex;
    ErrorValues e;
    IndexWeights weights;
    MatcherStatistics stats;
    WordStore store;
    map<pair<DocumentID, WordID>, size_t> originalSizes; // Lengths of original documents.
};

void ReverseIndex::add(const WordID wordID, const WordID indexID, const DocumentID id) {
    pair<WordID, WordID> p;
    p.first = indexID;
    p.second = wordID;
    auto revIt = reverseIndex.find(p);
    if(revIt == reverseIndex.end()) {
        DocumentSet tmp;
        tmp.insert(id);
        reverseIndex[p] = tmp;
    } else {
        revIt->second.insert(id);
    }

}

bool ReverseIndex::documentHasTerm(const WordID wordID, const WordID indexID, DocumentID id) {
    pair<WordID, WordID> p;
    p.first = indexID;
    p.second = wordID;
    auto revIt = reverseIndex.find(p);
    if(revIt == reverseIndex.end())
        return false;
    return revIt->second.find(id) != revIt->second.end();
}

void ReverseIndex::findDocuments(const WordID wordID, const WordID indexID, std::vector<DocumentID> &result) {
    pair<WordID, WordID> p;
    p.first = indexID;
    p.second = wordID;
    auto revIt = reverseIndex.find(p);
    if(revIt == reverseIndex.end())
        return;
    DocumentSet &docSet = revIt->second;
    for(auto docIter = docSet.begin(); docIter != docSet.end(); docIter++) {
        result.push_back(*docIter);
    }
}

/*
 * These are helper functions for Matcher. They are not member functions to avoid polluting the header
 * with STL includes.
 */

static void addMatches(MatcherPrivate */*p*/, BestIndexMatches &bestIndexMatches, const Word &/*queryWord*/, const WordID indexID, IndexMatches &matches) {
    MatchIndIterator it = bestIndexMatches.find(indexID);
    map<WordID, int> *indexMatches;
    if(it == bestIndexMatches.end()) {
        map<WordID, int> tmp;
        bestIndexMatches[indexID] = tmp;
        it = bestIndexMatches.find(indexID);
    }
    indexMatches = &(it->second);
    for(size_t i=0; i < matches.size(); i++) {
        const WordID matchWordID = matches.getMatch(i);
        const int matchError = matches.getMatchError(i);
        MatchIterator mIt = indexMatches->find(matchWordID);
        if(mIt == indexMatches->end()) {
            (*indexMatches)[matchWordID] = matchError;
        } else {
            if(mIt->second > matchError)
                (*indexMatches)[matchWordID] = matchError;
        }

    }
}

/*
 * A simple relevancy calculator for matched word. Better ranking functions exist and should be examined:
 * http://en.wikipedia.org/wiki/TF_IDF
 * http://en.wikipedia.org/wiki/Okapi_BM25
 */
static double calculateRelevancy(MatcherPrivate *p, const WordID wID, const WordID indexID, int error) {
    const LevenshteinIndex * const ind = p->indexes[indexID];
    double errorMultiplier = 100.0/(100.0+error); // Should be adjusted for maxError or word length.
    size_t indexCount = ind->wordCount(wID);
    size_t indexMaxCount = ind->maxCount();
    assert(indexCount > 0);
    assert(indexMaxCount > 0);
    double frequencyMultiplier = 1.0 - double(indexCount)/(indexMaxCount+1);
    double indexWeightMultiplier = p->weights.getWeight(p->store.getWord(indexID));
    return errorMultiplier*frequencyMultiplier*indexWeightMultiplier;
}


static void matchIndexes(MatcherPrivate *p, const WordList &query, const SearchParameters &params, const int extraError, BestIndexMatches &bestIndexMatches) {
    for(size_t i=0; i<query.size(); i++) {
        const Word &w = query[i];
        int maxError;
        if(params.isDynamic())
            maxError = params.getDynamicError(w);
        else
            maxError = 2*LevenshteinIndex::getDefaultError();
        maxError += extraError;

        for(IndIterator it = p->indexes.begin(); it != p->indexes.end(); it++) {
            if(params.isNonsearchingField(p->store.getWord(it->first))) {
                continue;
            }
            IndexMatches m;
            it->second->findWords(w, p->e, maxError, m);
            addMatches(p, bestIndexMatches, w, it->first, m);
            debugMessage("Matched word %s in index %s with error %d and got %lu matches.\n",
                    w.asUtf8().c_str(), p->store.getWord(it->first).asUtf8().c_str(), maxError, (unsigned long) m.size());
        }
    }
}

static void gatherMatchedDocuments(MatcherPrivate *p,  BestIndexMatches &bestIndexMatches, map<DocumentID, double> &matchedDocuments) {
    for(MatchIndIterator it = bestIndexMatches.begin(); it != bestIndexMatches.end(); it++) {
        for(MatchIterator mIt = it->second.begin(); mIt != it->second.end(); mIt++) {
            vector<DocumentID> tmp;
            p->reverseIndex.findDocuments(mIt->first, it->first, tmp);
            debugMessage("Exact searched \"%s\" in field \"%s\", which was found in %lu documents.\n",
                    p->store.getWord(mIt->first).asUtf8().c_str(),
                    p->store.getWord(it->first).asUtf8().c_str(), (unsigned long)tmp.size());
            for(size_t i=0; i<tmp.size(); i++) {
                DocumentID curDoc = tmp[i];
                // At this point we know the matched word, and which index and field
                // it matched in. Now we can just increment the relevancy of said document.
                double relevancy = calculateRelevancy(p, mIt->first, it->first, mIt->second);
                auto doc = matchedDocuments.find(curDoc);
                if(doc == matchedDocuments.end())
                    matchedDocuments[curDoc] = relevancy;
                else
                    matchedDocuments[curDoc] += relevancy;
            }
        }
    }
}

static bool subtermsMatch(MatcherPrivate *p, const ResultFilter &filter, size_t term, DocumentID id) {
    for(size_t subTerm=0; subTerm < filter.numSubTerms(term); subTerm++) {
        const Word &filterName = filter.getField(term, subTerm);
        const Word &value = filter.getWord(term, subTerm);
        bool termFound = p->reverseIndex.documentHasTerm(
                p->store.getID(value), p->store.getID(filterName), id);
        if(!termFound) {
            return false;
        }
    }
    return true;
}

Matcher::Matcher() {
    p = new MatcherPrivate();
}

void Matcher::index(const Corpus &c) {
    double buildStart, buildEnd;
    buildStart = hiresTimestamp();
    buildIndexes(c);
    buildEnd = hiresTimestamp();
    debugMessage("Added %lu documents to matcher. It now has %lu indexes. Index population took %.2f seconds.\n",
            (unsigned long) c.size(), (unsigned long) p->indexes.size(), buildEnd - buildStart);
    for(IndIterator it = p->indexes.begin(); it != p->indexes.end(); it++) {
        debugMessage("Index \"%s\" has %lu words and %lu nodes.\n",
                p->store.getWord(it->first).asUtf8().c_str(), (unsigned long) it->second->numWords(),
                (unsigned long) it->second->numNodes());
    }
}

Matcher::~Matcher() {
    for(IndIterator it = p->indexes.begin(); it != p->indexes.end(); it++) {
        delete it->second;
    }
    delete p;
}

void Matcher::buildIndexes(const Corpus &c) {
    for(size_t ci = 0; ci < c.size(); ci++) {
        const Document &d = c.getDocument(ci);
        WordList textNames;
        d.getFieldNames(textNames);
        for(size_t ti=0; ti < textNames.size(); ti++) {
            const Word &fieldName = textNames[ti];
            const WordID fieldID = p->store.getID(fieldName);
            const WordList &text = d.getText(fieldName);
            pair<DocumentID, WordID> lengths;
            lengths.first = d.getID();
            lengths.second = fieldID;
            p->originalSizes[lengths] = text.size();
            for(size_t wi=0; wi<text.size(); wi++) {
                const Word &word = text[wi];
                const WordID wordID = p->store.getID(word);
                p->stats.wordProcessed(wordID);
                addToIndex(word, wordID, fieldID);
                p->stats.addedWordToIndex(wordID, fieldName);
                p->reverseIndex.add(wordID, fieldID, d.getID());
            }
        }
    }
}

void Matcher::addToIndex(const Word &word, const WordID wordID, const WordID indexID) {
    LevenshteinIndex *target;
    IndIterator it = p->indexes.find(indexID);
    if(it == p->indexes.end()) {
        target = new LevenshteinIndex();
        p->indexes[indexID] = target;
    } else {
        target = it->second;
    }
    target->insertWord(word, wordID);
}


void Matcher::relevancyMatch(const WordList &query, const SearchParameters &params, const int extraError, MatchResults &matchedDocuments) {
    map<DocumentID, double> docs;
    BestIndexMatches bestIndexMatches;
    double start, indexMatchEnd, gatherEnd, finish;

    start = hiresTimestamp();
    matchIndexes(p, query, params, extraError, bestIndexMatches);
    indexMatchEnd = hiresTimestamp();
    // Now we know all matched words in all indexes. Gather up the corresponding documents.
    gatherMatchedDocuments(p, bestIndexMatches, docs);
    gatherEnd = hiresTimestamp();
    for(auto it=docs.begin(); it != docs.end(); it++) {
        matchedDocuments.addResult(it->first, it->second);
    }
    debugMessage("Found a total of %lu documents.\n", (unsigned long) matchedDocuments.size());
    finish = hiresTimestamp();
    debugMessage("Query finished. Index lookups took %.2fs, result gathering %.2fs, result building %.2fs.\n",
            indexMatchEnd - start, gatherEnd - indexMatchEnd, finish - gatherEnd);
}

MatchResults Matcher::match(const WordList &query, const SearchParameters &params) {
    MatchResults matchedDocuments;
    const int maxIterations = 1;
    const int increment = LevenshteinIndex::getDefaultError();
    const size_t minMatches = 10;
    MatchResults allMatches;

    if(query.size() == 0)
        return matchedDocuments;
    // Try to search with ever growing error until we find enough matches.
    for(int i=0; i<maxIterations; i++) {
        MatchResults matches;
        relevancyMatch(query, params, i*increment, matches);
        if(matches.size() >= minMatches || i == maxIterations-1) {
            allMatches.addResults(matches);
            break;
        }
    }

    /* Filter results into final set. */
    auto &filter = params.getResultFilter();
    for(size_t i=0; i<allMatches.size(); i++) {
        DocumentID id = allMatches.getDocumentID(i);
        for(size_t term=0; term < filter.numTerms(); term++) {
            if(subtermsMatch(p, filter, term, id)) {
                matchedDocuments.copyResult(allMatches, i);
                break;
            }
        }
    }
    return matchedDocuments;
}

MatchResults Matcher::match(const char *queryAsUtf8) {
    return match(splitToWords(queryAsUtf8));
}

MatchResults Matcher::match(const std::string &queryAsUtf8) {
    return match(queryAsUtf8.c_str());
}


MatchResults Matcher::match(const WordList &query) {
    SearchParameters defaults;
    return match(query, defaults);
}

ErrorValues& Matcher::getErrorValues() {
    return p->e;
}

MatchResults Matcher::match(const char *queryAsUtf8, const SearchParameters &params) {
    return match(splitToWords(queryAsUtf8), params);
}

IndexWeights& Matcher::getIndexWeights() {
    return p->weights;
}

static map<DocumentID, size_t> countExacts(MatcherPrivate *p, const WordList &query, const WordID indexID) {
    map<DocumentID, size_t> matchCounts;
    for(size_t i=0; i<query.size(); i++) {
        const Word &w = query[i];
        if(w.length() == 0 || !p->store.hasWord(w)) {
            continue;
        }
        WordID curWord = p->store.getID(w);
        vector<DocumentID> exacts;
        p->reverseIndex.findDocuments(curWord, indexID, exacts);
        for(const auto &i : exacts) {
            matchCounts[i]++; // Default is zero initialisation.
        }
    }
    return matchCounts;
}

struct DocCount {
    DocumentID id;
    size_t matches;
};

MatchResults Matcher::onlineMatch(const WordList &query, const Word &primaryIndex) {
    MatchResults results;
    set<DocumentID> exactMatched;
    map<DocumentID, double> accumulator;
    if(!p->store.hasWord(primaryIndex)) {
        string msg("Index named ");
        msg += primaryIndex.asUtf8();
        msg += " is not known";
        throw invalid_argument(msg);
    }
    WordID indexID = p->store.getID(primaryIndex);
    // How many times each document matched with zero error.
    vector<DocCount> stats;
    for(const auto &i : countExacts(p, query, indexID)) {
        DocCount c;
        pair<DocumentID, WordID> key;
        exactMatched.insert(i.first);
        key.first = i.first;
        key.second = indexID;
        c.id = i.first;
        c.matches = i.second;
        stats.push_back(c);
    }
    for(const auto &i: stats) {
        accumulator[i.id] = 2*i.matches;
        if(i.matches == query.size()
                && i.matches == p->originalSizes[make_pair(i.id, indexID)]) { // Perfect match.
            accumulator[i.id] += 100;
        }
    }
    // Merge in fuzzy matches.
    MatchResults fuzzyResults = match(query);
    for(size_t i = 0; i<fuzzyResults.size(); i++) {
        DocumentID docid = fuzzyResults.getDocumentID(i);
        accumulator[docid] += fuzzyResults.getRelevancy(i);
    }
    for(const auto &i : accumulator) {
        results.addResult(i.first, i.second);
    }
    return results;
}

COL_NAMESPACE_END

