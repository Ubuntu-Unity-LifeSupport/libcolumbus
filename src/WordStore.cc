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

#include "WordStore.hh"
#include "Word.hh"
#include "Trie.hh"
#include <vector>
#include <stdexcept>


COL_NAMESPACE_START
using namespace std;

/*
 * Wordstore turned out to be too slow when used with
 * sparse_maps or unordered_maps.
 */

struct hasher : std::unary_function<const Word&, std::size_t> {
    size_t operator() ( const Word &obj) const {
        return obj.hash();
    }
};

struct WordStorePrivate {
    Trie words;
    vector<TrieOffset> wordIndex; // The Word object is duplicated here. It should be fixed.
};

WordStore::WordStore() {
    p = new WordStorePrivate();

}

WordStore::~WordStore() {
    delete p;
}

WordID WordStore::getID(const Word &w) {
    if(p->words.hasWord(w)) {
        return p->words.getWordID(p->words.findWord(w));
    }
    TrieOffset node = p->words.insertWord(w, p->wordIndex.size());
    p->wordIndex.push_back(node);
    WordID result = p->wordIndex.size()-1;
    return result;
}

bool WordStore::hasWord(const Word &w) const {
    return p->words.hasWord(w);
}

Word WordStore::getWord(const WordID id) const {
    if(!hasWord(id)) {
        throw out_of_range("Tried to access non-existing WordID in WordStore.");
    }
    return p->words.getWord(p->wordIndex[id]);
}

bool WordStore::hasWord(const WordID id) const {
    return id < p->wordIndex.size();
}

COL_NAMESPACE_END
