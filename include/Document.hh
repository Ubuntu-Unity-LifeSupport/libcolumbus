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

#ifndef DOCUMENT_HH_
#define DOCUMENT_HH_

#include "ColumbusCore.hh"
#include<string>

COL_NAMESPACE_START

class Word;
class WordList;

struct DocumentPrivate;

class COL_PUBLIC Document final {
private:
    DocumentPrivate *p;

public:
    Document(DocumentID id);
    Document(const Document &d);
    ~Document();

    const Document& operator=(const Document &d);
    void addText(const Word &field, const WordList &words);
    void addText(const Word &field, const char *textAsUtf8);
    void addText(const Word &field, const std::string &textAsUtf8);
    const WordList& getText(const Word &field) const;
    size_t fieldCount() const;
    DocumentID getID() const;
    void getFieldNames(WordList &list) const;
    size_t wordCount(const Word &w, const Word field) const;
    size_t totalWordCount(const Word &w) const;

    // Needs iterators to browse through text names.
};

COL_NAMESPACE_END

#endif /* DOCUMENT_HH_ */
