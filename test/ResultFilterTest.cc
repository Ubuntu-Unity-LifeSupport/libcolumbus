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

#include "SearchParameters.hh"
#include "ResultFilter.hh"
#include "Word.hh"
#include "Document.hh"
#include "Corpus.hh"
#include "MatchResults.hh"
#include "Matcher.hh"
#include <cassert>
#include <cstdio>

using namespace Columbus;
using namespace std;

void testFiltering() {
    Word textField("text");
    const char *txt = "something";
    Word filterField1("field1");
    Word filterField2("field2");
    const char *val1str = "one";
    const char *val2str = "two";
    const char *val3str = "three";
    Word val1(val1str);
    Word val2(val2str);
    Word val3(val3str);
    Document d1(1);
    Document d2(2);
    Corpus c;
    Matcher m;
    SearchParameters emptyFilter;
    SearchParameters onlyTakeFirst, onlyTakeSecond, orTest, andTest;

    d1.addText(textField, txt);
    d1.addText(filterField1, val1str);
    d1.addText(filterField2, val3str);
    c.addDocument(d1);
    d2.addText(textField, txt);
    d2.addText(filterField1, val2str);
    d2.addText(filterField2, val3str);
    c.addDocument(d2);

    m.index(c);
    MatchResults r1 = m.match(txt, emptyFilter);
    assert(r1.size() == 2);

    onlyTakeFirst.getResultFilter().addNewSubTerm(filterField1, val1);
    MatchResults r2 = m.match(txt, onlyTakeFirst);
    assert(r2.size() == 1);
    assert(r2.getDocumentID(0) == 1);

    onlyTakeSecond.getResultFilter().addNewSubTerm(filterField1, val2);
    MatchResults r3 = m.match(txt, onlyTakeSecond);
    assert(r3.size() == 1);
    assert(r3.getDocumentID(0) == 2);

    orTest.getResultFilter().addNewSubTerm(filterField1, val1);
    orTest.getResultFilter().addNewTerm();
    orTest.getResultFilter().addNewSubTerm(filterField1, val2);
    MatchResults orResults = m.match(txt, orTest);
    assert(orResults.size() == 2);

    andTest.getResultFilter().addNewSubTerm(filterField2, val2);
    andTest.getResultFilter().addNewSubTerm(filterField1, val1);
    MatchResults andResults = m.match(txt, andTest);
    assert(andResults.size() == 0);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testFiltering();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}

