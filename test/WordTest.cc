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
 * Tests the custom word class. Valgrind strongly recommended.
 */

#include <cstring>
#include <cassert>
#include <stdexcept>
#include <cstdio>
#include "Word.hh"

using namespace Columbus;
using namespace std;

void testEmpty() {
    Word w1;
    assert(w1.length() == 0);
}

void testIndexing() {
    Word w("abc");
    bool gotException = false;

    assert(w[0] == 'a');
    assert(w[1] == 'b');
    assert(w[2] == 'c');

    try {
        w[3];
    } catch(std::out_of_range &e) {
        gotException = true;
    }
    assert(gotException);

}

void shouldThrow(const char *str) {
    bool gotException;
    try {
        Word w(str);
        gotException = false;
    } catch(std::invalid_argument &e) {
        gotException = true;
    }
    assert(gotException);
}

void testWhitespace() {
    shouldThrow(" ");
    shouldThrow(" a");
    shouldThrow("a ");
    shouldThrow("a a");

    shouldThrow("\t");
    shouldThrow("a\t");
    shouldThrow("\ta");
    shouldThrow("a\ta");

    shouldThrow("\n");
    shouldThrow("a\n");
    shouldThrow("\na");
    shouldThrow("a\na");

    shouldThrow("\r");
    shouldThrow("a\r");
    shouldThrow("\ra");
    shouldThrow("a\ra");
}

void testCreation() {
    Word w1("abc");
    Word w2(w1);
    Word empty1;
    Word empty2(empty1);
    string s("xyz");
    Word strW(s);

    assert(empty1.length() == 0);
    assert(empty2.length() == 0);

    assert(w2.length() == 3);
    assert(w2[0] == 'a');
    assert(w2[1] == 'b');
    assert(w2[2] == 'c');

    assert(strW.length() == 3);
    assert(strW[0] == 'x');
    assert(strW[1] == 'y');
    assert(strW[2] == 'z');

    Word *w3 = new Word(w1);
    assert(w3->length() == 3);
    assert((*w3)[0] == 'a');
    assert((*w3)[1] == 'b');
    assert((*w3)[2] == 'c');
    delete w3;

    // Check that w1 did not get destroyed along with w3.
    assert(w1.length() == 3);
    assert(w1[0] == 'a');
    assert(w1[1] == 'b');
    assert(w1[2] == 'c');
}

void testComparison() {
    Word w1a;
    Word w1b;
    Word w2a("abc");
    Word w2b("abc");
    Word w2c("abd");
    Word different("different");

    assert(w1a == w1a);
    assert(w2a == w2a);
    assert(w2b == w2b);

    assert(!(w1a != w1b));
    assert(!(w2a != w2a));
    assert(!(w2b != w2b));

    assert(w1a != w2a);
    assert(w2a != w2c);
    assert(w1a != different);
    assert(w2a != different);

    assert(!(w1a == w2a));
    assert(!(w2a == w2c));
    assert(!(w1a == different));
    assert(!(w2a == different));
}

void testEncoding() {
    const unsigned char txt[5] = {0x61, 0xc3, 0xa4, 0x63, 0}; // "aäc" in UTF-8.
    char *text = (char*)txt;
    char returned[5];
    Word w1(text);

    assert(w1.length() == 3);
    assert(w1[0] == 'a');
    assert(w1[1] == 0xe4);
    assert(w1[2] == 'c');

    w1.toUtf8(returned, 5);
    assert(strcmp(text, returned) == 0);
    assert(strcmp(text, w1.asUtf8().c_str()) == 0);

    Word wAss("abc");
    assert(strcmp("abc", wAss.asUtf8().c_str()) == 0); // Make it allocate its internal things to check that they are released appropriately
    wAss = w1;
    assert(strcmp(text, w1.asUtf8().c_str()) == 0);
    assert(strcmp(text, wAss.asUtf8().c_str()) == 0);

    Word wInit(w1);
    assert(strcmp(text, w1.asUtf8().c_str()) == 0);
    assert(strcmp(text, wInit.asUtf8().c_str()) == 0);
}

void testLessThan() {
    Word w1("a");
    Word w2("b");
    Word w3("aa");
    Word w4("ab");

    assert(w1 < w2);
    assert(w1 < w3);
    assert(w3 < w2);
    assert(w3 < w4);
    assert(w4 < w2);
    assert(w1 < w4);
}

void testAutoLower() {
    const unsigned char txtUpper[4] = {0x41, 0xc3, 0x84, 0}; // "AÄ" in UTF-8.
    const unsigned char txtLower[4] = {0x61, 0xc3, 0xa4, 0}; // "aä" in UTF-8.

    const char *tu = (const char*) txtUpper;
    const char *tl = (const char*) txtLower;

    Word wUpper(tu);
    Word wLower(tl);

    assert(wUpper == wLower);
    assert(strcmp(wLower.asUtf8().c_str(), tl) == 0);
    assert(strcmp(wUpper.asUtf8().c_str(), tl) == 0);

}

void testJoin() {
    Word w1("abc");
    Word w2("def");
    Word r1("abcdef");
    Word r2("defabc");
    Word empty;
    Word result;

    result = w1.join(w2);
    assert(result == r1);
    // Test that it is properly null terminated.
    assert(strcmp(result.asUtf8().c_str(), "abcdef") == 0);

    result = w2.join(w1);
    assert(result == r2);

    result = empty.join(w1);
    assert(result == w1);
    result = w2.join(empty);
    assert(result == w2);

    result = empty.join(empty);
    assert(result == empty);
}

void testAssignment() {
    Word w;
    const char *txt = "abc";
    const char *txt2 = "defg";
    string txt3 = "xyz";
    string txt4 = "lmn";
    const char *txtError = "h h";
    bool gotAssertion;

    w = txt;
    assert(w == txt);
    assert(w != txt2);
    assert(strcmp(txt, w.asUtf8().c_str()) == 0);
    assert(w.length() == 3);

    w = txt2;
    assert(w != txt);
    assert(w == txt2);
    assert(strcmp(txt2, w.asUtf8().c_str()) == 0);
    assert(w.length() == 4);

    w = txt3;
    assert(w == txt3);
    assert(w != txt4);
    assert(txt3 == w.asUtf8());
    assert(w.length() == 3);

    w = txt4;
    assert(w != txt3);
    assert(w == txt4);
    assert(w == txt4);
    assert(w.length() == 3);

    try {
        w = txtError;
        gotAssertion = false;
    } catch(std::invalid_argument &e) {
        gotAssertion = true;
    }
    assert(gotAssertion);
}

int main(int /*argc*/, char **/* argv*/) {
    try {
        testEmpty();
        testIndexing();
        testWhitespace();
        testCreation();
        testComparison();
        testEncoding();
        testLessThan();
        testAutoLower();
        testJoin();
        testAssignment();
    } catch(const exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}

