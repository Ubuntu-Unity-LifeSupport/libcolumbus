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

#include "Word.hh"
#include "Trie.hh"
#include <cassert>

using namespace Columbus;

void testWordBuilding() {
    Trie t;
    Word w1("abc");
    Word w2("abd");
    Word result;
    TrieOffset node1, node2;

    WordID i1 = 1;
    WordID i2 = 2;

    assert(t.numWords() == 0);
    node1 = t.insertWord(w1, i1);
    assert(t.numWords() == 1);
    node2 = t.insertWord(w2, i2);
    assert(t.numWords() == 2);

    result = t.getWord(node1);
    assert(result == w1);

    result = t.getWord(node2);
    assert(result == w2);
}

void testHas() {
    Trie t;
    Word w1("abc");
    Word w2("abd");
    Word w3("a");
    Word w4("x");
    Word result;

    WordID i1 = 1;

    assert(t.numWords() == 0);
    t.insertWord(w1, i1);
    assert(t.hasWord(w1));
    assert(!t.hasWord(w2));
    assert(!t.hasWord(w3));
    assert(!t.hasWord(w4));
}

int main(int /*argc*/, char **/*argv*/) {
    // Move basic tests from levtrietest here.
    testWordBuilding();
    testHas();
    return 0;
}

