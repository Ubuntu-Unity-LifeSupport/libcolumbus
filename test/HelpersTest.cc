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

#include "ColumbusHelpers.hh"
#include "Word.hh"
#include "WordList.hh"
#include <cassert>

using namespace Columbus;

bool splitCorrectly(const char *txt, const WordList &l) {
    WordList result = splitToWords(txt);
    return result == l;
}

void testSplitter() {
    Word w1("abc");
    Word w2("def");
    WordList l1;
    l1.addWord(w1);
    l1.addWord(w2);

    assert(splitCorrectly("abc def", l1));
    assert(splitCorrectly("abc\tdef", l1));
    assert(splitCorrectly("abc\ndef", l1));
    assert(splitCorrectly("abc\rdef", l1));
    assert(splitCorrectly(" abc def", l1));
    assert(splitCorrectly("abc def ", l1));
    assert(splitCorrectly(" abc def ", l1));

    WordList empty;
    assert(splitCorrectly("", empty));
    assert(splitCorrectly(" ", empty));
    assert(splitCorrectly("\t", empty));
    assert(splitCorrectly("\n", empty));
    assert(splitCorrectly("\r", empty));
    assert(splitCorrectly(" \t\n\r\n\t ", empty));
}

void testWeirdWord() {
    const unsigned char txt[] = {0x42, 0x6c, 0x75, 0x65, 0x73, 0x20, 0xe2, 0x80, 0x9a, 0xc3, 0x84, 0xc3, 0xb2, 0x6e, 0xe2, 0x80,
                                   0x9a, 0xc3, 0x84, 0xc3, 0xb4, 0x20, 0x54, 0x72, 0x6f, 0x75, 0x62, 0x6c, 0x65, 0x0d, 0x0a, 0};
    WordList l = splitToWords((const char*)txt);
    assert(l.size() == 3);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testSplitter();
        testWeirdWord();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
