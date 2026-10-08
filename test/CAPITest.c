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

#include "columbus.h"
#include <assert.h>
#include <string.h>

void testWord() {
    const int bufSize = 10;
    char buf[bufSize];
    ColWord w = col_word_new("abc");
    assert(w);
    assert(col_word_length(w) == 3);
    col_word_as_utf8(w, buf, bufSize);
    assert(strcmp("abc", buf) == 0);
    col_word_delete(w);
}

void testDocument() {
    ColDocument d = col_document_new(55);
    ColWord w = col_word_new("abc");

    assert(d);
    assert(col_document_get_id(d) == 55);

    col_document_add_text(d, w, "this is just some text");

    col_word_delete(w);
    col_document_delete(d);
}

void testMatcher() {
    ColMatcher m = col_matcher_new();
    assert(m);
    col_matcher_delete(m);
}

void testMatchResults() {
    ColMatchResults mr = col_match_results_new();
    assert(mr);
    col_match_results_delete(mr);
}

void testCorpus() {
    ColCorpus c = col_corpus_new();
    ColDocument d = col_document_new(42);
    ColWord w = col_word_new("abc");
    col_document_add_text(d, w, "this is just some text");
    col_word_delete(w);

    assert(c);
    col_corpus_add_document(c, d);
    col_document_delete(d);
    col_corpus_delete(c);
}

ColCorpus buildCorpus() {
    ColCorpus c = col_corpus_new();
    DocumentID name1 = 0;
    DocumentID name2 = 10;
    DocumentID name3 = 1000;
    ColWord textName = col_word_new("title");

    ColDocument d1, d2, dFar;
    d1 = col_document_new(name1);
    col_document_add_text(d1, textName, "abc def");
    d2 = col_document_new(name2);
    col_document_add_text(d2, textName, "abe test");
    dFar = col_document_new(name3);
    col_document_add_text(dFar, textName, "faraway donotmatchme");
    col_corpus_add_document(c, d1);
    col_corpus_add_document(c, d2);
    col_corpus_add_document(c, dFar);

    col_word_delete(textName);
    col_document_delete(d1);
    col_document_delete(d2);
    col_document_delete(dFar);
    return c;
}

void testMatching() {
    ColCorpus c = buildCorpus();
    ColMatcher m = col_matcher_new();
    ColMatchResults matches;
    DocumentID dFarName = 1000;
    DocumentID name1 = 0;
    DocumentID name2 = 10;

    col_matcher_index(m, c);
    col_corpus_delete(c);

    matches = col_matcher_match(m, "abe");
    assert(col_match_results_size(matches) == 2);
    assert(col_match_results_get_id(matches, 0) != dFarName);
    assert(col_match_results_get_id(matches, 1) != dFarName);
    assert(col_match_results_get_id(matches, 0) == name1 ||
            col_match_results_get_id(matches, 1) == name1);
    assert(col_match_results_get_id(matches, 0) == name2 ||
            col_match_results_get_id(matches, 1) == name2);
    col_match_results_delete(matches);
    col_matcher_delete(m);
}

int main(int argc UNUSED_VAR, char **argv UNUSED_VAR) {
    testWord();
    testDocument();
    testMatcher();
    testMatchResults();
    testCorpus();
    testMatching();
    return 0;
}
