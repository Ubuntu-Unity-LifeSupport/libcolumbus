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

#include "Document.hh"
#include "Word.hh"
#include "WordList.hh"
#include "ColumbusHelpers.hh"
#include <map>
#include <stdexcept>

COL_NAMESPACE_START
using namespace std;

struct DocumentPrivate {
    DocumentID id;
    map<Word, WordList> texts;
};

typedef map<Word, WordList>::iterator TextIter;
typedef map<Word, WordList>::const_iterator TextIterC;

Document::Document(DocumentID id) {
    p = new DocumentPrivate();
    p->id = id;
}

Document::Document(const Document& d) {
    p = new DocumentPrivate();
    p->id = d.p->id;
    p->texts = d.p->texts;
}

Document::~Document() {
    delete p;
}

void Document::addText(const Word &field, const WordList &words) {
    p->texts[field] = words;
}

void Document::addText(const Word &field, const char *textAsUtf8) {
    addText(field, splitToWords(textAsUtf8));
}

void Document::addText(const Word &field, const std::string &textAsUtf8) {
    addText(field, textAsUtf8.c_str());
}

const WordList& Document::getText(const Word &field) const {
    TextIter res = p->texts.find(field);
    if(res == p->texts.end()) {
        throw invalid_argument("Tried to access nonexisting text field in Document.");
    }
    return res->second;
}

size_t Document::fieldCount() const {
    return p->texts.size();
}

DocumentID Document::getID() const {
    return p->id;
}

void Document::getFieldNames(WordList &list) const {
    for(TextIter it=p->texts.begin(); it != p->texts.end(); it++) {
        list.addWord(it->first);
    }
}

const Document& Document::operator=(const Document&d) {
    if(this == &d)
        return *this;
    p->id = d.p->id;
    p->texts = d.p->texts;
    return *this;
}

size_t Document::wordCount(const Word &w, const Word field) const {
    TextIterC it = p->texts.find(field);
    size_t count = 0;
    if(it == p->texts.end())
        return count;
    for(size_t i = 0; i < it->second.size(); i++) {
        if(it->second[i] == w)
            count++;
    }
    return count;
}

size_t Document::totalWordCount(const Word &w) const {
    size_t count = 0;
    for(TextIterC it = p->texts.begin(); it != p->texts.end(); it++) {
        count += wordCount(w, it->first);
    }
    return count;
}

COL_NAMESPACE_END
