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

#include "Document.hh"
#include "Word.hh"
#include "WordList.hh"
#include <cassert>
#include <stdexcept>

using namespace Columbus;

void testDoc() {
    DocumentID docId = 42;
    Document d(docId);

    Word w1("abc");
    Word w2("def");
    Word textName("title");
    WordList *l = new WordList();
    l->addWord(w1);
    l->addWord(w2);

    d.addText(textName, *l);
    const WordList &l2 = d.getText(textName);
    assert(l2.size() == 2);
    assert(l2[0] == w1);
    assert(l2[1] == w2);

    delete l;
    const WordList &l3 = d.getText(textName);
    assert(l3.size() == 2);
    assert(l3[0] == w1);
    assert(l3[1] == w2);
}

void testIndexNames() {
    DocumentID docId = 102;
    Document d(docId);

    Word w1("abc");
    Word w2("def");
    Word text1Name("text1");
    Word text2Name("text2");
    WordList wl1;
    WordList wl2;
    WordList textNames;

    wl1.addWord(w1);
    wl2.addWord(w2);

    d.addText(text1Name, wl1);
    d.addText(text2Name, wl2);

    d.getFieldNames(textNames);
    for(size_t i=0; i<textNames.size(); i++) {
        bool gotException;
        try {
            d.getText(textNames[i]);
            gotException = false;
        } catch(std::invalid_argument &e) {
            gotException = true;
        }
        assert(!gotException);
    }

    bool gotException;
    try {
        Word nonexisting("foooooo");
        d.getText(nonexisting);
        gotException = false;
    } catch(std::invalid_argument &e) {
        gotException = true;
    }
    assert(gotException);

}

void testCounts() {
    Word w1("abc");
    Word w2("def");
    Word w3("geh");
    Word w4("ijk");
    Word f1("field1");
    Word f2("field2");
    Word f3("field3");
    WordList l1, l2, l3;
    Document d(2);

    l1.addWord(w1);
    l1.addWord(w2);
    l2.addWord(w2);
    l2.addWord(w3);

    assert(d.wordCount(w1, f1) == 0);
    assert(d.wordCount(w1, f2) == 0);
    assert(d.totalWordCount(w1) == 0);
    assert(d.totalWordCount(w2) == 0);
    assert(d.totalWordCount(w3) == 0);

    d.addText(f1, l1);
    assert(d.wordCount(w1, f1) == 1);
    assert(d.wordCount(w1, f2) == 0);
    assert(d.wordCount(w2, f1) == 1);
    assert(d.wordCount(w2, f2) == 0);
    assert(d.wordCount(w3, f1) == 0);
    assert(d.wordCount(w3, f2) == 0);
    assert(d.totalWordCount(w1) == 1);
    assert(d.totalWordCount(w2) == 1);
    assert(d.totalWordCount(w3) == 0);

    d.addText(f2, l2);
    assert(d.wordCount(w1, f1) == 1);
    assert(d.wordCount(w1, f2) == 0);
    assert(d.wordCount(w2, f1) == 1);
    assert(d.wordCount(w2, f2) == 1);
    assert(d.wordCount(w3, f1) == 0);
    assert(d.wordCount(w3, f2) == 1);
    assert(d.totalWordCount(w1) == 1);
    assert(d.totalWordCount(w2) == 2);
    assert(d.totalWordCount(w3) == 1);

    // Finally, test that a word appearing multiple
    // times in a list works properly.
    l3.addWord(w4);
    l3.addWord(w4);
    d.addText(f3, l3);
    assert(d.wordCount(w4, f3) == 2);
    assert(d.totalWordCount(w4) == 2);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testDoc();
        testIndexNames();
        testCounts();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}

