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

#include"SearchParameters.hh"
#include"Word.hh"
#include"Matcher.hh"
#include"Document.hh"
#include"Corpus.hh"
#include"MatchResults.hh"
#include<cassert>

using namespace Columbus;

void testDynamic() {
    SearchParameters sp;
    assert(sp.isDynamic());

    sp.setDynamic(false);
    assert(!sp.isDynamic());

    sp.setDynamic(true);
    assert(sp.isDynamic());
}

void testNosearch() {
    SearchParameters sp;
    Word w1("abc");
    Word w2("def");

    assert(!sp.isNonsearchingField(w1));
    assert(!sp.isNonsearchingField(w2));

    sp.addNonsearchingField(w1);
    assert(sp.isNonsearchingField(w1));
    assert(!sp.isNonsearchingField(w2));

    sp.addNonsearchingField(w2);
    assert(sp.isNonsearchingField(w1));
    assert(sp.isNonsearchingField(w2));
}

void testNosearchMatching() {
    Word textField("text");
    Word search("field1");
    Word nonSearch("field2");
    const char *val1str = "one";
    Corpus c;
    Matcher m;
    SearchParameters sp;
    MatchResults r;
    Document d1(1);
    Document d2(2);

    sp.addNonsearchingField(nonSearch);
    d1.addText(search, val1str);
    d2.addText(nonSearch, val1str);
    c.addDocument(d1);
    c.addDocument(d2);
    m.index(c);

    r = m.match(val1str, sp);
    assert(r.size() == 1);
    assert(r.getDocumentID(0) == 1);
}

int main(int /*argc*/, char **/*argv*/) {
    testDynamic();
    testNosearch();
    testNosearchMatching();
}
