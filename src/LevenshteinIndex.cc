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

/*
 * This file implements a fast Levenshtein matcher for a dictionary of
 * words. It is a re-implementation of code placed in the public domain
 * here:
 *
 * http://stevehanov.ca/blog/index.php?id=114
 */

#include <stdio.h>
#include <cassert>
#include <map>
#include <vector>
#include "LevenshteinIndex.hh"
#include "ErrorValues.hh"
#include "Word.hh"
#include "ErrorMatrix.hh"
#include "Trie.hh"

#ifdef HAS_SPARSE_HASH
#include <google/sparse_hash_map>
using google::sparse_hash_map;
#define hashmap sparse_hash_map
#else
#include <unordered_map>
#define hashmap unordered_map
#endif

COL_NAMESPACE_START
using namespace std;

typedef vector<pair<Letter, TrieNode*> > ChildList;
typedef ChildList::iterator ChildListIter;
typedef ChildList::const_iterator ChildListConstIter;

typedef hashmap<WordID, size_t> WordCount;


struct LevenshteinIndexPrivate {
    WordCount wordCounts; // How many times the word has been added to this index.
    size_t maxCount; // How many times the most common word has been added.
    size_t numNodes;
    size_t numWords; // How many words are in this index in total.
    size_t longestWordLength; // Longest word that has been added. Same as tree depth.
    Trie trie;
};


LevenshteinIndex::LevenshteinIndex() {
    p = new LevenshteinIndexPrivate();
    p->maxCount = 0;
    p->longestWordLength = 0;
}

LevenshteinIndex::~LevenshteinIndex() {
    delete p;
}

int LevenshteinIndex::getDefaultError() {
    return ErrorValues::getDefaultError();
}

void LevenshteinIndex::insertWord(const Word &word, const WordID wordID) {
    if(word.length() == 0)
        return;
    auto it = p->wordCounts.find(wordID);
    size_t newCount;
    if(it != p->wordCounts.end()) {
        newCount = p->wordCounts[wordID] + 1;
    } else {
        newCount = 1;
    }
    p->trie.insertWord(word, wordID);
    p->wordCounts[wordID] = newCount;
    if(word.length() > p->longestWordLength)
        p->longestWordLength = word.length();
    if(p->maxCount < newCount)
        p->maxCount = newCount;
    return;
}

bool LevenshteinIndex::hasWord(const Word &word) const {
    return p->trie.hasWord(word);
}

void LevenshteinIndex::findWords(const Word &query, const ErrorValues &e, const int maxError, IndexMatches &matches) const {
    TrieOffset root;
    TrieOffset sibling;
    ErrorMatrix em(p->longestWordLength+1, query.length()+1,
            e.getDeletionError(), e.getStartInsertionError(query.length()));

    assert(em.get(0, 0) == 0);
    if(query.length() > 0)
        assert(em.get(0, 1) == e.getInsertionError());
    root = p->trie.getRoot();
    sibling = p->trie.getSiblingList(root);
    while(sibling != 0) {
        Letter l = p->trie.getLetter(sibling);
        TrieOffset nextNode = p->trie.getChild(sibling);
        searchRecursive(query, nextNode, e, l, (Letter)0, 1, em, matches, maxError);
        sibling = p->trie.getNextSibling(sibling);
    }
    matches.sort();
}

int LevenshteinIndex::findOptimalError(const Letter letter, const Letter previousLetter, const Word &query,
        const size_t i, const size_t depth, const ErrorMatrix &em, const ErrorValues &e) const {
    int insertError = em.get(depth, i-1) + e.getInsertionError();
    int deleteError;
    if(i >= query.length())
        deleteError = em.get(depth-1, i) + e.getEndDeletionError();
    else
        deleteError = em.get(depth-1, i) + e.getDeletionError();

    int substituteError = em.get(depth-1, i-1) + e.getSubstituteError(query.text[i-1], letter);

    int transposeError;
    if(i > 1 && query.text[i - 1] == previousLetter && query.text[i - 2] == letter) {
        transposeError = em.get(depth-2, i-2) + e.getTransposeError();
    } else {
        transposeError = insertError + 10000; // Ensures this will not be chosen.
    }
    return min(insertError, min(deleteError, min(substituteError, transposeError)));
}

void LevenshteinIndex::searchRecursive(const Word &query, TrieOffset node, const ErrorValues &e,
        const Letter letter, const Letter previousLetter, const size_t depth, ErrorMatrix &em,
        IndexMatches &matches, const int maxError) const {

    for(size_t i = 1; i < query.length()+1; i++) {
        int minError = findOptimalError(letter, previousLetter, query, i, depth, em, e);
        em.set(depth, i, minError);
    }

    // Error row evaluated. Now check if a word was found and continue recursively.
    if(em.totalError(depth) <= maxError && p->trie.getWordID(node) != INVALID_WORDID) {
        matches.addMatch(query, p->trie.getWordID(node), em.totalError(depth));
    }
    if(em.minError(depth) <= maxError) {
        TrieOffset sibling = p->trie.getSiblingList(node);
        while(sibling != 0) {
            Letter l = p->trie.getLetter(sibling);
            TrieOffset nextNode = p->trie.getChild(sibling);
            searchRecursive(query, nextNode, e, l, letter, depth+1, em, matches, maxError);
            sibling = p->trie.getNextSibling(sibling);
        }
    }
}

size_t LevenshteinIndex::wordCount(const WordID queryID) const {
    auto i = p->wordCounts.find(queryID);
    if(i == p->wordCounts.end())
        return 0;
    return i->second;
}

size_t LevenshteinIndex::maxCount() const {
    return p->maxCount;
}

size_t LevenshteinIndex::numNodes() const {
    return p->trie.numNodes();
}

size_t LevenshteinIndex::numWords() const {
    return p->trie.numWords();
}

COL_NAMESPACE_END
