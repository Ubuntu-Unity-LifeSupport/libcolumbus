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
#include <cstdio>
#include <cstring>

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

/* Deleted files this process has mapped: their count and their total size.
 * A Trie maps a deleted temporary file; once destroyed, it must leave none. */
struct Deleted {
    int count;
    unsigned long bytes;
};

static Deleted deletedMappings() {
    FILE *maps = fopen("/proc/self/maps", "r");
    assert(maps); // a test that cannot see the mappings must not pass
    Deleted d = {0, 0};
    char line[4096];
    while(fgets(line, sizeof(line), maps)) {
        size_t len = strlen(line);
        if(len && line[len-1] == '\n')
            line[--len] = '\0';
        const char *suffix = " (deleted)";
        size_t slen = strlen(suffix);
        if(len >= slen && strcmp(line + len - slen, suffix) == 0) {
            unsigned long start = 0, end = 0;
            int fields = sscanf(line, "%lx-%lx", &start, &end);
            assert(fields == 2);
            d.count++;
            d.bytes += end - start;
        }
    }
    fclose(maps);
    return d;
}

void testNoMappingLeft() {
    Deleted before = deletedMappings();
    {
        Trie t;
        Word w("abc");
        t.insertWord(w, 1);
        Deleted alive = deletedMappings();
        assert(alive.count == before.count + 1);
    }
    assert(deletedMappings().count == before.count);
    for(int i = 0; i < 50; i++) {
        Trie t;
        Word w("abc");
        t.insertWord(w, 1);
    }
    assert(deletedMappings().count == before.count);
}

void testNoMappingLeftAfterExpand() {
    Deleted before = deletedMappings();
    {
        Trie t;
        char text[3] = {0, 0, 0};
        WordID id = 1;
        for(char a = 'a'; a <= 'z'; a++) {
            for(char b = 'a'; b <= 'z'; b++) {
                text[0] = a;
                text[1] = b;
                Word w(text);
                t.insertWord(w, id++);
            }
        }
        Deleted alive = deletedMappings();
        assert(alive.count == before.count + 1);
        assert(alive.bytes - before.bytes > 1024); // expand() ran
    }
    Deleted after = deletedMappings();
    assert(after.count == before.count);
    assert(after.bytes == before.bytes);
}

int main(int /*argc*/, char **/*argv*/) {
    // Move basic tests from levtrietest here.
    testWordBuilding();
    testHas();
    testNoMappingLeft();
    testNoMappingLeftAfterExpand();
    return 0;
}

