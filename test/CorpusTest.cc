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

#include "Corpus.hh"
#include "Word.hh"
#include "Document.hh"
#include "WordList.hh"
#include <cassert>
#include <cstdio>

using namespace Columbus;

void testCorpus() {
    Corpus c;
    Word w1("abc");
    Word w2("def");
    Word w3("test1");
    Word w4("test2");
    DocumentID name1 = 0;
    DocumentID name2 = 0;
    Word textName("title");

    WordList wl1, wl2;
    wl1.addWord(w1);
    wl1.addWord(w2);
    wl2.addWord(w3);
    wl2.addWord(w4);

    Document d1(name1);
    d1.addText(textName, wl1);
    Document *d2 = new Document(name2);
    d2->addText(textName, wl2);

    assert(c.size() == 0);
    c.addDocument(d1);
    assert(c.size() == 1);
    c.addDocument(*d2);
    assert(c.size() == 2);

    assert(c.getDocument(0).getID() == name1);
    const Document &dNew = c.getDocument(1);
    assert(dNew.getID() == name2);

    delete d2;
    assert(c.size() == 2);
    const Document &dNew2 = c.getDocument(1);
    assert(dNew2.getID() == name2);
    const WordList &lNew = dNew.getText(textName);
    assert(lNew[0] == w3);
    assert(lNew[1] == w4);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testCorpus();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
