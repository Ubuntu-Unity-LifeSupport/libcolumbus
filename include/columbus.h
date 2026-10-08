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
 * This file defines the C API of Columbus. If you can, it is strongly
 * recommended to use the C++ API of columbus.hh instead.
 */

#ifndef COLUMBUS_H_
#define COLUMBUS_H_

#ifdef COLUMBUS_HH_
#error "Mixing C and C++ public header includes. You can only use one or the other."
#endif

#include "ColumbusCore.hh"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ColWord;
typedef void* ColDocument;
typedef void* ColMatcher;
typedef void* ColMatchResults;
typedef void* ColCorpus;
typedef void* ColErrorValues;
typedef void* ColIndexWeights;

COL_PUBLIC ColWord col_word_new(const char *utf8_word);
COL_PUBLIC void col_word_delete(ColWord w);
COL_PUBLIC size_t col_word_length(ColWord w);
COL_PUBLIC void col_word_as_utf8(ColWord w, char *buf, unsigned int bufSize);

COL_PUBLIC ColDocument col_document_new(DocumentID id);
COL_PUBLIC void col_document_delete(ColDocument doc);
COL_PUBLIC DocumentID col_document_get_id(ColDocument doc);
COL_PUBLIC void col_document_add_text(ColDocument doc, ColWord field_name, const char *text_as_utf8);

COL_PUBLIC ColMatcher col_matcher_new();
COL_PUBLIC void col_matcher_delete(ColMatcher m);
COL_PUBLIC void col_matcher_index(ColMatcher m, ColCorpus c);
COL_PUBLIC ColMatchResults col_matcher_match(ColMatcher m, const char *query_as_utf8);
COL_PUBLIC ColErrorValues col_matcher_get_error_values(ColMatcher m);
COL_PUBLIC ColIndexWeights col_matcher_get_index_weights(ColMatcher m);

COL_PUBLIC ColMatchResults col_match_results_new();
COL_PUBLIC void col_match_results_delete(ColMatchResults mr);
COL_PUBLIC size_t col_match_results_size(ColMatchResults mr);
COL_PUBLIC DocumentID col_match_results_get_id(ColMatchResults mr, size_t i);
COL_PUBLIC double col_match_results_get_relevancy(ColMatchResults mr, size_t i);

COL_PUBLIC ColCorpus col_corpus_new();
COL_PUBLIC void col_corpus_delete(ColCorpus c);
COL_PUBLIC void col_corpus_add_document(ColCorpus c, ColDocument d);

COL_PUBLIC void col_index_weights_set_weight(ColIndexWeights weights, const ColWord field, const double new_weight);
COL_PUBLIC double col_index_weights_get_weight(ColIndexWeights weights, const ColWord field);

COL_PUBLIC void col_error_values_add_standard_errors(ColErrorValues ev);
COL_PUBLIC void col_error_values_set_substring_mode(ColErrorValues ev);

#ifdef __cplusplus
}
#endif

#endif
