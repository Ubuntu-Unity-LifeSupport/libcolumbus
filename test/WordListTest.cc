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

#include "WordList.hh"
#include "Word.hh"
#include <cassert>
#include <stdexcept>

using namespace Columbus;

void testList() {
    WordList l;
    bool gotException;
    Word w1("abc");
    Word w2("def");
    Word *w2Shadow = new Word(w2);

    assert(l.size() == 0);
    try {
        Word wTmp = l[1];
        gotException = false;
    } catch(std::out_of_range &e) {
        gotException = true;
    }

    assert(gotException);

    l.addWord(w1);
    assert(l.size() == 1);
    assert(l[0] == w1);

    l.addWord(*w2Shadow);
    delete w2Shadow;
    assert(l.size() == 2);
    assert(l[1] == w2);
}

void testAssignment() {
    WordList *l1 = new WordList();
    WordList l2;
    Word w("abc");

    l1->addWord(w);
    assert(l1->size() == 1);
    l2 = *l1;
    assert(l2.size() == 1);
    assert(l2[0] == w);
    delete l1;
    assert(l2.size() == 1);
    assert(l2[0] == w);
}

void testEquality() {
    WordList l1, l2;
    Word w1("abc");
    Word w2("def");
    Word w3("ghi");

    assert(l1 == l2);
    assert(!(l1 != l2));

    l1.addWord(w1);
    assert(!(l1 == l2));
    assert(l1 != l2);

    l2.addWord(w1);
    assert(l1 == l2);
    assert(!(l1 != l2));

    l2.addWord(w2);
    assert(!(l1 == l2));
    assert(l1 != l2);

    l1.addWord(w3);
    assert(!(l1 == l2));
    assert(l1 != l2);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testList();
        testAssignment();
        testEquality();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
