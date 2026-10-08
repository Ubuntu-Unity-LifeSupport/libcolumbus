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
#include "Word.hh"
#include "Document.hh"
#include "Matcher.hh"
#include "MatchResults.hh"
#include "Corpus.hh"
#include "ErrorValues.hh"
#include "IndexWeights.hh"
#include <stdexcept>
#include <cstdio>

using namespace Columbus;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

ColWord col_word_new(const char *utf8_word) {
    try {
        Word *w = new Word(utf8_word);
        return reinterpret_cast<ColWord>(w);
    } catch(exception &e) {
        fprintf(stderr, "Error creating Word: %s\n", e.what());
    }
    return nullptr;
}

void col_word_delete(ColWord w) {
    try {
        delete reinterpret_cast<Word*>(w);
    } catch(exception &e) {
        fprintf(stderr, "Error deleting Word: %s\n", e.what());
    }
}

size_t col_word_length(ColWord w) {
    try {
        return reinterpret_cast<Word*>(w)->length();
    } catch(exception &e) {
        fprintf(stderr, "Error getting Word length: %s\n", e.what());
    }
    return 0;
}

void col_word_as_utf8(ColWord w, char *buf, unsigned int bufSize) {
    try {
        reinterpret_cast<Word*>(w)->toUtf8(buf, bufSize);
    } catch(exception &e) {
        fprintf(stderr, "Error converting to Utf-8: %s\n", e.what());
    }
}

ColDocument col_document_new(DocumentID id) {
    try {
        return reinterpret_cast<ColDocument>(new Document(id));
    } catch(exception &e) {
        fprintf(stderr, "Error creating Document: %s\n", e.what());
    }
    return nullptr;
}

void col_document_delete(ColDocument doc) {
    try {
        delete reinterpret_cast<Document*>(doc);
    } catch(exception &e) {
        fprintf(stderr, "Error deleting Document: %s\n", e.what());
    }
}

DocumentID col_document_get_id(ColDocument doc) {
    try {
        return reinterpret_cast<Document*>(doc)->getID();
    } catch(exception &e) {
        fprintf(stderr, "Error getting Document ID %s\n", e.what());
    }
    return INVALID_DOCID;
}

void col_document_add_text(ColDocument doc, ColWord field_name, const char *text_as_utf8) {
    try {
        Document *d = reinterpret_cast<Document*>(doc);
        Word *w = reinterpret_cast<Word*>(field_name);
        d->addText(*w, text_as_utf8);
    } catch(exception &e) {
        fprintf(stderr, "Error adding text: %s\n", e.what());
    }
}

ColMatcher col_matcher_new() {
    try {
        return reinterpret_cast<ColMatcher>(new Matcher());
    } catch(exception &e) {
        fprintf(stderr, "Error creating Matcher: %s\n", e.what());
    }
    return nullptr;
}

void col_matcher_delete(ColMatcher m) {
    try {
        delete reinterpret_cast<Matcher*>(m);
    } catch(exception &e) {
        fprintf(stderr, "Error deleting Matcher: %s\n", e.what());
    }
}

void col_matcher_index(ColMatcher m, ColCorpus c) {
    try {
        Matcher *matcher = reinterpret_cast<Matcher*>(m);
        Corpus *corp = reinterpret_cast<Corpus*>(c);
        matcher->index(*corp);
    } catch(exception &e) {
        fprintf(stderr, "Exception when indexing: %s\n", e.what());
    }
}

ColMatchResults col_matcher_match(ColMatcher m, const char *query_as_utf8) {
    try {
        Matcher *matcher = reinterpret_cast<Matcher*>(m);
        MatchResults *results =
                new MatchResults(matcher->match(query_as_utf8));
        return reinterpret_cast<ColMatchResults>(results);
    } catch(exception &e) {
        fprintf(stderr, "Exception when matching: %s\n", e.what());
        return nullptr;
    }
}

ColErrorValues col_matcher_get_error_values(ColMatcher m) {
    try {
        Matcher *matcher = reinterpret_cast<Matcher*>(m);
        return reinterpret_cast<ColErrorValues>(&matcher->getErrorValues());
    } catch(exception &e) {
        fprintf(stderr, "Error getting ErrorValues: %s\n", e.what());
    }
    return nullptr;
}

ColIndexWeights col_matcher_get_index_weights(ColMatcher m) {
    try {
        Matcher *matcher = reinterpret_cast<Matcher*>(m);
        return reinterpret_cast<ColIndexWeights>(&matcher->getIndexWeights());
    } catch(exception &e) {
        fprintf(stderr, "Error getting IndexWeights: %s\n", e.what());
    }
    return nullptr;
}

ColMatchResults col_match_results_new() {
    try {
        return reinterpret_cast<ColMatchResults>(new MatchResults());
    } catch(exception &e) {
        fprintf(stderr, "Error creating MatchResults: %s\n", e.what());
    }
    return nullptr;
}
void col_match_results_delete(ColMatchResults mr) {
    try {
        delete reinterpret_cast<MatchResults*>(mr);
    } catch(exception &e) {
        fprintf(stderr, "Error deleting MatchResults: %s\n", e.what());
    }
}

size_t col_match_results_size(ColMatchResults mr) {
    try {
        return reinterpret_cast<MatchResults*>(mr)->size();
    } catch(exception &e) {
        fprintf(stderr, "Error getting match size: %s\n", e.what());
    }
    return 0;
}

DocumentID col_match_results_get_id(ColMatchResults mr, size_t i) {
    try {
        MatchResults *results = reinterpret_cast<MatchResults*>(mr);
        return results->getDocumentID(i);
    } catch(exception &e) {
        fprintf(stderr, "Exception when getting result document ID: %s\n", e.what());
    }
    return INVALID_DOCID;
}

double col_match_results_get_relevancy(ColMatchResults mr, size_t i) {
    try {
        MatchResults *results = reinterpret_cast<MatchResults*>(mr);
        return results->getDocumentID(i);
    } catch(exception &e) {
        fprintf(stderr, "Exception when getting result document ID: %s\n", e.what());
    }
    return -1.0;
}

ColCorpus col_corpus_new() {
    try {
        return reinterpret_cast<ColCorpus>(new Corpus());
    } catch(exception &e) {
        fprintf(stderr, "Error creating Corpus: %s\n", e.what());
    }
    return nullptr;
}


void col_corpus_delete(ColCorpus c) {
    try {
        delete reinterpret_cast<Corpus*>(c);
    } catch(exception &e) {
        fprintf(stderr, "Error deleting Corpus: %s\n", e.what());
    }
}

void col_corpus_add_document(ColCorpus c, ColDocument d) {
    try {
        Corpus *corp = reinterpret_cast<Corpus*>(c);
        Document *doc = reinterpret_cast<Document*>(d);
        corp->addDocument(*doc);
    } catch(exception &e) {
        fprintf(stderr, "Error adding document: %s\n", e.what());
    }
}

void col_error_values_add_standard_errors(ColErrorValues ev) {
    try {
        ErrorValues *results = reinterpret_cast<ErrorValues*>(ev);
        results->addStandardErrors();
    } catch(exception &e) {
        fprintf(stderr, "Error adding standard errors: %s\n", e.what());
    }
}

void col_error_values_set_substring_mode(ColErrorValues ev) {
    try {
        ErrorValues *results = reinterpret_cast<ErrorValues*>(ev);
        results->setSubstringMode();
    } catch(exception &e) {
        fprintf(stderr, "Error setting substring mode: %s\n", e.what());
    }
}

void col_index_weights_set_weight(ColIndexWeights weights, const ColWord field, const double new_weight) {
    try {
        IndexWeights *cweight = reinterpret_cast<IndexWeights*>(weights);
        Word *w = reinterpret_cast<Word*>(field);
        cweight->setWeight(*w, new_weight);
    } catch(exception &e) {
        fprintf(stderr, "Error setting weight: %s\n", e.what());
    }
}

double col_index_weights_get_weight(ColIndexWeights weights, const ColWord field) {
    try {
        IndexWeights *cweight = reinterpret_cast<IndexWeights*>(weights);
        Word *w = reinterpret_cast<Word*>(field);
        return cweight->getWeight(*w);
    } catch(exception &e) {
        fprintf(stderr, "Error getting weight: %s\n", e.what());
    }
    return 1.0;
}


#ifdef __cplusplus
}
#endif
