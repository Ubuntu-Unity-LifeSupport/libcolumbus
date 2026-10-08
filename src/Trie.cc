/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 * This class implements a trie as an array. It uses a sparse memory mapped
 * file for backing storage. This makes it possible to grow the allocation
 * efficiently with ftruncate.
 *
 * The offsets have 32 bits to save memory. 4 gigs of trie should be enough
 * for everybody.
 *
 * This low level bit fiddling makes the code slightly hard to read. It
 * should still be understandable, though.
 */

#include"Trie.hh"
#include"Word.hh"
#include<sys/mman.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdexcept>
#include<string>
#include<vector>
#include<cassert>
#include<cstdint>
#include<memory>

using namespace std;

COL_NAMESPACE_START

struct TrieHeader {
    TrieOffset totalSize;
    TrieOffset firstFree;
    uint32_t numWords;
    uint32_t numNodes;
};

struct TriePtrs {
    Letter l;
    TrieOffset child;
    TrieOffset sibling;
};

struct TrieNode {
    WordID word;
    TrieOffset parent;
};

struct TriePrivate {
    FILE *f = nullptr;
    char *map = nullptr; // never MAP_FAILED
    TrieOffset mapSize = 0; // the size of map, kept outside it
    TrieHeader *h = nullptr;
    TrieOffset root = 0;

    ~TriePrivate() {
        // The mapping keeps the deleted temporary file and its pages alive
        // until it is unmapped, even after the file is closed.
        if(map && munmap(map, mapSize) != 0) {
            fprintf(stderr, "Trie: munmap failed: %s\n", strerror(errno));
        }
        if(f) {
            fclose(f);
        }
    }
};

Trie::Trie() {
    // Owned by holder until the Trie is complete: a throw below never runs
    // ~Trie, and holder releases the file and the mapping.
    unique_ptr<TriePrivate> holder(new TriePrivate());
    p = holder.get();
    p->f = tmpfile();
    if(!p->f) {
        string msg("Could not create temporary file: ");
        msg += strerror(errno);
        throw runtime_error(msg);
    }
    expand();
    p->h->firstFree = sizeof(TrieHeader);
    p->root = p->h->firstFree;
    p->h->numWords = 0;
    addNewNode(0);
    holder.release();
}


Trie::~Trie() {
    delete p;
}

// The old mapping stays valid until the new one exists, so a failure leaves
// the Trie as it was. The file only grows: pages past the mapping are unused.
void Trie::expand() {
    TrieOffset newSize;
    if(p->mapSize) {
        if(p->mapSize > UINT32_MAX / 2) {
            throw runtime_error("Trie too large");
        }
        newSize = p->mapSize*2;
    } else {
        newSize = 1024;
    }
    if(ftruncate(fileno(p->f), newSize) != 0) {
        string err = "Truncate failed: ";
        err += strerror(errno);
        throw runtime_error(err);
    }
    char *newMap = (char*)mmap(NULL, newSize, PROT_READ | PROT_WRITE, MAP_SHARED,
                  fileno(p->f), 0);
    if(newMap == MAP_FAILED) {
        string err = "MMap failed: ";
        err += strerror(errno);
        throw runtime_error(err);
    }
    if(p->map && munmap(p->map, p->mapSize) != 0) {
        fprintf(stderr, "Trie: munmap failed: %s\n", strerror(errno));
    }
    p->map = newMap;
    p->mapSize = newSize;
    if(madvise(p->map, newSize, MADV_RANDOM | MADV_WILLNEED) != 0) {
        fprintf(stderr, "Problem with madvise: %s\n", strerror(errno));
    }
    p->h = (TrieHeader*)p->map;
    p->h->totalSize = newSize;
    assert(p->h->totalSize > p->h->firstFree);
}

TrieOffset Trie::append(const char *data, const int size) {
    TrieOffset result;
    assert(p->h->totalSize > p->h->firstFree);
    while(p->h->firstFree + size >= p->h->totalSize) {
        expand();
    }
    memcpy(p->map + p->h->firstFree, data, size);
    result = p->h->firstFree;
    p->h->firstFree += size;
    assert(p->h->totalSize > p->h->firstFree);
    return result;
}

TrieOffset Trie::addNewNode(const TrieOffset parent) {
    TrieNode n;
    TriePtrs ptr;
    TrieOffset nodeoffset;
    n.word = INVALID_WORDID;
    n.parent = parent;
    ptr.child = ptr.sibling = ptr.l = 0;
    nodeoffset = append((char*)&n, sizeof(n));
    append((char*)&ptr, sizeof(ptr));
    p->h->numNodes++;
    return nodeoffset;
}

TrieOffset Trie::addNewSibling(const TrieOffset node, const TrieOffset sibling, Letter l) {
    TriePtrs *last; // Assign only at the end so remappings won't invalidate it.
    TriePtrs ptr;
    TrieOffset newSibling;
    ptr.l = l;
    ptr.child = addNewNode(node);
    ptr.sibling = 0;
    newSibling = append((char*) &ptr, sizeof(ptr));
    last = (TriePtrs*)(p->map + sibling);
    assert(last->sibling == 0);
    last->sibling = newSibling;
    return ptr.child;
}

TrieOffset Trie::insertWord(const Word &word, const WordID wordID) {
    size_t i=0;
    TrieOffset node = p->root;
    while(word.length() > i) {
        Letter l = word[i];
        TrieOffset searcher = node;
        //TrieNode *n = (TrieNode*)(p->map + searcher);
        TrieOffset sibl = searcher + sizeof(TrieNode);
        TriePtrs *ptrs = (TriePtrs*)(p->map + sibl);
        while(ptrs->sibling != 0 && ptrs->l != l) {
            sibl = ptrs->sibling;
            ptrs = (TriePtrs*)(p->map + sibl);
        }

        if(ptrs->l == l) {
            node = ptrs->child;
        } else {
            node = addNewSibling(node, sibl, l);
        }
        i++;
    }
    TrieNode *final = (TrieNode*)(p->map + node);
    if (final->word == INVALID_WORDID) {
        final->word = wordID;
        p->h->numWords++;
    }
    /*
     * Theoretically there is nothing wrong with adding the same word with
     * different IDs. In our case it probably means that the word deduplicator
     * is not working and there is a leak somewhere. So check explicitly.
     */
    assert(final->word == wordID);
    return node;
}

bool Trie::hasWord(const Word &word) const {
    TrieOffset node = findWord(word);
    if(!node)
        return false;
    TrieNode *n = (TrieNode*)(p->map+node);
    if(n->word != INVALID_WORDID)
        return true;
    return false;
}

TrieOffset Trie::findWord(const Word &word) const {
    TrieOffset node = p->root;
    for(size_t i=0; word.length() > i; i++) {
        Letter l = word[i];
        TrieOffset searcher = node;
        TrieOffset sibl = searcher + sizeof(TrieNode);
        TriePtrs *ptrs = (TriePtrs*)(p->map + sibl);
        while(ptrs->sibling != 0 && ptrs->l != l) {
            sibl = ptrs->sibling;
            ptrs = (TriePtrs*)(p->map + sibl);
        }

        if(ptrs->l != l)
            return 0;
        node = ptrs->child;
    }
    return node;
}

TrieOffset Trie::getRoot() const {
    return p->root;
}


TrieOffset Trie::getSiblingList(TrieOffset node) const {
    TriePtrs *ptrs = (TriePtrs*)(p->map + node + sizeof(TrieNode));
    return ptrs->sibling;

}

TrieOffset Trie::getNextSibling(TrieOffset sibling) const {
    TriePtrs *ptrs = (TriePtrs*)(p->map + sibling);
    return ptrs->sibling;
}

Letter Trie::getLetter(TrieOffset sibling) const {
    TriePtrs *ptrs = (TriePtrs*)(p->map + sibling);
    return ptrs->l;
}

TrieOffset Trie::getChild(TrieOffset sibling) const {
    TriePtrs *ptrs = (TriePtrs*)(p->map + sibling);
    return ptrs->child;
}

WordID Trie::getWordID(TrieOffset node) const {
    TrieNode *n = (TrieNode*)(p->map + node);
    return n->word;
}

bool Trie::hasSibling(TrieOffset sibling) const {
    TriePtrs *ptrs = (TriePtrs*)(p->map + sibling);
    return ptrs->sibling != 0;
}

size_t Trie::numWords() const {
    return p->h->numWords;
}

size_t Trie::numNodes() const {
    return p->h->numNodes;
}

TrieOffset Trie::getParent(TrieOffset node) const {
    TrieNode *n = (TrieNode*)(p->map + node);
    return n->parent;
}

TrieOffset Trie::getSiblingTo(const TrieOffset node, const TrieOffset child) const {
    TrieOffset sibling = getSiblingList(node);
    while(getChild(sibling) != child) {
        sibling = getNextSibling(sibling);
        if(!sibling)
            throw runtime_error("Trie is corrupted");
    }
    return sibling;
}

Word Trie::getWord(const TrieOffset startNode) const {
    vector<Letter> letters;
    vector<Letter> res;
    TrieOffset node = startNode;
    if(node == 0) {
        return Word();
    }
    TrieOffset parent = getParent(node);
    letters.push_back(0);
    while(parent) {
        TrieOffset newParent;
        letters.push_back(getLetter(getSiblingTo(parent, node)));
        newParent = getParent(parent);
        node = parent;
        parent = newParent;
    }
    res.insert(res.begin(), letters.rbegin(), letters.rend());
    return Word(&(res[0]), res.size());
}

COL_NAMESPACE_END
