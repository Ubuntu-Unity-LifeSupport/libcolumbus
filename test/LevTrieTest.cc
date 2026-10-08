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
 * This file tests the trie portion of the Levenshtein index.
 * That is, exact matches.
 */

#include <cstdio>
#include <cassert>
#include "LevenshteinIndex.hh"
#include "Word.hh"

using namespace Columbus;

void basicTest() {
    LevenshteinIndex ind;
    Word word1("word");
    Word word2("another");
    WordID w1ID = 1;
    WordID w2ID = 2;

    assert(!ind.hasWord(word1));
    assert(!ind.hasWord(word2));

    ind.insertWord(word1, w1ID);
    assert(ind.hasWord(word1));
    assert(!ind.hasWord(word2));

    ind.insertWord(word2, w2ID);
    assert(ind.hasWord(word1));
    assert(ind.hasWord(word2));
}

void shortTest() {
    LevenshteinIndex ind;
    Word word("a");

    assert(!ind.hasWord(word));

    ind.insertWord(word, 1);
    assert(ind.hasWord(word));
}

void prefixTest() {
    LevenshteinIndex ind;
    Word word("ab");
    Word prefix("a");

    assert(!ind.hasWord(word));
    assert(!ind.hasWord(prefix));

    ind.insertWord(word, 1);
    assert(ind.hasWord(word));
    assert(!ind.hasWord(prefix));

    ind.insertWord(prefix, 2);
    assert(ind.hasWord(word));
    assert(ind.hasWord(prefix));
}

void suffixTest() {
    LevenshteinIndex ind;
    Word word("abc");
    Word word2("abcd");

    assert(!ind.hasWord(word));
    assert(!ind.hasWord(word2));

    ind.insertWord(word, 1);
    assert(ind.hasWord(word));
    assert(!ind.hasWord(word2));

    ind.insertWord(word2, 2);
    assert(ind.hasWord(word));
    assert(ind.hasWord(word2));
}

void branchTest() {
    LevenshteinIndex ind;
    Word word("abc");
    Word word2("abcd");
    Word word3("abce");

    assert(!ind.hasWord(word));
    assert(!ind.hasWord(word2));
    assert(!ind.hasWord(word3));

    ind.insertWord(word, 1);
    assert(ind.hasWord(word));
    assert(!ind.hasWord(word2));
    assert(!ind.hasWord(word3));

    ind.insertWord(word2, 2);
    assert(ind.hasWord(word));
    assert(ind.hasWord(word2));
    assert(!ind.hasWord(word3));

    ind.insertWord(word3, 3);
    assert(ind.hasWord(word));
    assert(ind.hasWord(word2));
    assert(ind.hasWord(word3));
}

void countTest() {
    LevenshteinIndex ind;
    Word w1("abc");
    Word w2("def");
    Word w3("abce");
    WordID w1ID = 1;
    WordID w2ID = 2;
    WordID w3ID = 3;

    assert(ind.wordCount(w1ID) == 0);
    assert(ind.wordCount(w2ID) == 0);
    assert(ind.wordCount(w3ID) == 0);
    assert(ind.maxCount() == 0);
    assert(ind.numWords() == 0);

    ind.insertWord(w1, w1ID);
    assert(ind.wordCount(w1ID) == 1);
    assert(ind.wordCount(w2ID) == 0);
    assert(ind.wordCount(w3ID) == 0);
    assert(ind.maxCount() == 1);
    assert(ind.numWords() == 1);

    ind.insertWord(w2, w2ID);
    assert(ind.wordCount(w1ID) == 1);
    assert(ind.wordCount(w2ID) == 1);
    assert(ind.wordCount(w3ID) == 0);
    assert(ind.maxCount() == 1);
    assert(ind.numWords() == 2);

    ind.insertWord(w1, w1ID);
    assert(ind.wordCount(w1ID) == 2);
    assert(ind.wordCount(w2ID) == 1);
    assert(ind.wordCount(w3ID) == 0);
    assert(ind.maxCount() == 2);
    assert(ind.numWords() == 2);
}

int main(int /*argc*/, char **/*argv*/) {
#ifdef NDEBUG
    fprintf(stderr, "NDEBUG is defined, tests will not work!\n");
    return 1;
#else
    try {
        basicTest();
        shortTest();
        prefixTest();
        suffixTest();
        branchTest();
        countTest();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
#endif
}
