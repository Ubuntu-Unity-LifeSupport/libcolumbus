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

#include "Matcher.hh"
#include "Corpus.hh"
#include "Word.hh"
#include "WordList.hh"
#include "Document.hh"
#include "MatchResults.hh"
#include "ColumbusHelpers.hh"
#include <cassert>

using namespace Columbus;
using namespace std;

Corpus * testCorpus() {
    Corpus *c = new Corpus();
    Word w1("abc");
    Word w2("def");
    Word w3("abe");
    Word w4("test");
    Word w5("faraway");
    Word w6("donotmatchme");
    DocumentID name1 = 0;
    DocumentID name2 = 10;
    DocumentID name3 = 1000;
    Word textName("title");

    WordList wl1, wl2, wlFar;
    wl1.addWord(w1);
    wl1.addWord(w2);
    wl2.addWord(w3);
    wl2.addWord(w4);
    wlFar.addWord(w5);
    wlFar.addWord(w6);

    Document d1(name1);
    d1.addText(textName, wl1);
    Document d2(name2);
    d2.addText(textName, wl2);
    Document dFar(name3);
    dFar.addText(textName, wlFar);
    c->addDocument(d1);
    c->addDocument(d2);
    c->addDocument(dFar);

    return c;
}

void testMatcher() {
    Corpus *c = testCorpus();
    Matcher m;
    MatchResults matches;
    WordList queryList;
    Word w1("abc");
    DocumentID dFarName = 1000;
    DocumentID name1 = 0;
    DocumentID name2 = 10;

    m.index(*c);
    delete(c);

    queryList.addWord(w1);
    matches = m.match(queryList);
    assert(matches.size() == 2);
    assert(matches.getDocumentID(0) != dFarName);
    assert(matches.getDocumentID(1) != dFarName);
    assert(matches.getDocumentID(0) == name1 ||
           matches.getDocumentID(1) == name1);
    assert(matches.getDocumentID(0) == name2 ||
           matches.getDocumentID(1) == name2);
}

void testRelevancy() {
    Corpus *c = testCorpus();
    Matcher m;
    MatchResults matches;
    WordList queryList;
    Word w1("abc");
    Word dFarName("distantdoc");
    DocumentID name1 = 0;

    m.index(*c);
    delete c;

    queryList.addWord(w1);
    matches = m.match(queryList);
    assert(matches.size() == 2);
    // Document doc1 has an exact match, so it should be the best match.
    assert(matches.getRelevancy(0) > matches.getRelevancy(1));
    assert(matches.getDocumentID(0) == name1);
}

void testMultiWord() {
    Corpus c;
    DocumentID correct = 1;
    DocumentID wrong = 0;
    Document d1(correct);
    Document d2(wrong);
    Word fieldName("name");
    Matcher m;
    MatchResults matches;

    d1.addText(fieldName, "Sarah Michelle Gellar");
    d2.addText(fieldName, "Sara Giller");

    c.addDocument(d1);
    c.addDocument(d2);
    m.index(c);

    matches = m.match("Sari Michell Geller");
    assert(matches.getDocumentID(0) == correct);
}

void testSentence() {
    Corpus c;
    DocumentID correct = 1;
    DocumentID wrong = 0;
    Document d1(correct);
    Document d2(wrong);
    Word fieldName("name");
    Word secondName("context");
    Matcher m;
    MatchResults matches;

    d1.addText(fieldName, "Fit Canvas to Layers");
    d1.addText(secondName, "View Zoom (100%)");
    d2.addText(fieldName, "Fit image in Window");
    d2.addText(secondName, "Image");

    c.addDocument(d1);
    c.addDocument(d2);

    m.index(c);
    matches = m.match("fit canvas to layers");
    assert(matches.getDocumentID(0) == correct);
}

void testExactOrder() {
    Corpus c;
    DocumentID correct = 1;
    DocumentID wrong = 0;
    DocumentID moreWrong = 100;
    Document d1(correct);
    Document d2(wrong);
    Document d3(moreWrong);
    Word fieldName("name");
    Word secondName("context");
    Matcher m;
    MatchResults matches;
    WordList q = splitToWords("fit canvas to layers");
    d1.addText(fieldName, "Fit Canvas to Layers");
    d1.addText(secondName, "View Zoom (100%)");
    d2.addText(fieldName, "Fit image in Window");
    d2.addText(secondName, "Image");
    d3.addText(fieldName, "Not matching.");
    d3.addText(secondName, "fit canvas to layers");
    c.addDocument(d1);
    c.addDocument(d2);
    c.addDocument(d3);

    m.index(c);
    matches = m.onlineMatch(q, fieldName);
    assert(matches.size() >= 1);
    assert(matches.getDocumentID(0) == correct);
}

void testSmallestMatch() {
    Corpus c;
    DocumentID correct = 1;
    DocumentID wrong = 0;
    Document d1(correct);
    Document d2(wrong);
    Word fieldName("name");
    Word field2("dummy");
    Matcher m;
    MatchResults matches;
    WordList q = splitToWords("save");
    d1.addText(fieldName, "save");
    d1.addText(field2, "lots of text to ensure statistics of this field are ignored");
    d2.addText(fieldName, "save as");
    c.addDocument(d1);
    c.addDocument(d2);

    m.index(c);
    matches = m.onlineMatch(q, fieldName);
    assert(matches.size() == 2);
    assert(matches.getDocumentID(0) == correct);
}

void noCommonMatch() {
    Corpus c;
    DocumentID correct = 1;
    Document d1(correct);
    Word fieldName("name");
    Word field2("dummy");
    Matcher m;
    MatchResults matches;
    WordList q = splitToWords("fit canvas to selection");
    d1.addText(fieldName, "Preparing your Images for the Web");
    d1.addText(fieldName, "Help user manual");
    c.addDocument(d1);

    m.index(c);
    matches = m.onlineMatch(q, fieldName);
    assert(matches.size() == 0);
}

void emptyMatch() {
    Corpus c;
    DocumentID correct = 1;
    Document d1(correct);
    Word fieldName("name");
    Word field2("dummy");
    Matcher m;
    MatchResults matches;
    WordList q;
    d1.addText(fieldName, "Preparing your Images for the Web");
    d1.addText(fieldName, "Help user manual");
    c.addDocument(d1);

    m.index(c);
    matches = m.onlineMatch(q, fieldName);
    assert(matches.size() == 0);
}

void testMatchCount() {
    Corpus c;
    DocumentID correct = 1;
    DocumentID wrong = 0;
    Document d1(correct);
    Document d2(wrong);
    Word fieldName("name");
    Word secondName("context");
    Matcher m;
    MatchResults matches;
    WordList q = splitToWords("fit canvas to selection");
    d1.addText(fieldName, "Fit Canvas to Layers");
    d1.addText(secondName, "View Zoom (100%)");
    d2.addText(fieldName, "Selection editor");
    d2.addText(secondName, "Windows dockable dialogs");
    c.addDocument(d1);
    c.addDocument(d2);

    m.index(c);
    matches = m.onlineMatch(q, fieldName);
    assert(matches.size() == 2);
    assert(matches.getDocumentID(0) == correct);
}

void testPerfect() {
    Corpus c;
    DocumentID correct = 0;
    Document d1(1);
    Document d2(correct);
    Document d3(2);
    Document d4(3);
    Word fieldName("name");
    Matcher m;
    MatchResults matches;
    WordList q = splitToWords("save");
    d1.addText(fieldName, "Save as");
    d2.addText(fieldName, "Save");
    d3.addText(fieldName, "Save yourself");
    d4.addText(fieldName, "Save the whales");
    c.addDocument(d1);
    c.addDocument(d2);
    c.addDocument(d3);
    c.addDocument(d4);

    m.index(c);
    matches = m.onlineMatch(q, fieldName);
    assert(matches.size() >= 1);
    assert(matches.getDocumentID(0) == correct);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testMatcher();
        testRelevancy();
        testMultiWord();
        testSentence();
        testExactOrder();
        testSmallestMatch();
        noCommonMatch();
        emptyMatch();
        testMatchCount();
        testPerfect();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
