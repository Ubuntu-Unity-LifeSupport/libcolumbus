#!/usr/bin/python3 -tt
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Canonical, Ltd.

# Authors:
#    Jussi Pakkanen <jussi.pakkanen@canonical.com>

# This library is free software; you can redistribute it and/or modify it under
# the terms of version 3 of the GNU Lesser General Public License as published
# by the Free Software Foundation.

# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import unittest
import columbus

class TestWord(unittest.TestCase):
    
    def test_init(self):
        d = columbus.Word("hello")
        
    def test_encoding(self):
        str1 = "hello"
        str2 = 'abcåäö'
        
        w1 = columbus.Word(str1)
        self.assertEqual(str1, w1.get_string(), "plain ASCII string did not survive round trip")

        w2 = columbus.Word(str2)
        self.assertEqual(str2, w2.get_string(), 'non-ASCII string did not survive round trip')
        
    def test_exception(self):
        str1 = 'two words'
        with self.assertRaises(ValueError):
            w1 = columbus.Word(str1)

    def test_length(self):
        str1 = "hello"
        str2 = 'abcåäö'
        
        w1 = columbus.Word(str1)
        self.assertEqual(len(str1), len(w1), "plain ASCII Word has incorrect size")

        w2 = columbus.Word(str2)
        self.assertEqual(len(str2), len(w2), 'non-ASCII Word has incorrect size')

class TestWordList(unittest.TestCase):
    
    def test_init(self):
        l = columbus.WordList()
        
    def test_size(self):
        l = columbus.WordList()
        self.assertEqual(0, len(l), 'Incorrect size for empty list')
        w1 = columbus.Word('abc')
        w2 = columbus.Word('defg')
        
        l.add_word(w1)
        self.assertEqual(1, len(l))
        l.add_word(w2)
        self.assertEqual(2, len(l))
        l.add_word(w1)
        self.assertEqual(3, len(l))
        
    def test_split(self):
        l = columbus.split_to_words('this is  my text')
        self.assertEqual(4, len(l), 'text splitting fails')
        
    def test_indexing(self):
        l = columbus.split_to_words('this is  my text')
        self.assertEqual("this", l[0].get_string())
        self.assertNotEqual("is", l[0].get_string())
        self.assertEqual("is", l[1].get_string())
        self.assertEqual("my", l[2].get_string())
        self.assertEqual("text", l[3].get_string())

class TestDocument(unittest.TestCase):
    
    def test_init(self):
        d = columbus.Document(1)
        
    def test_doc(self):
        docid = 435
        field = columbus.Word('fieldname')
        text = 'ye olde butcherede englishe'
        d = columbus.Document(docid)
        
        self.assertEqual(d.get_id(), docid, 'Document ID got mangled.')
        self.assertEqual(d.field_count(), 0)
        
        d.add_text(field, text)
        self.assertEqual(d.field_count(), 1, 'field count did not increase')
        self.assertGreater(len(text), 0)
        self.assertEqual(len(d.get_text(field)), len(text.split()), 'stored text got mangled')

class TestCorpus(unittest.TestCase):
    
    def test_init(self):
        c = columbus.Corpus()
        
    def test_insertion(self):
        c = columbus.Corpus()
        d = columbus.Document(55)
        
        self.assertEqual(0, len(c))
        
        c.add_document(d)
        self.assertEqual(1, len(c))

class TestMatchResults():
    
    def test_init(self):
        mr = columbus.MatchResults()
        
    def test_basic(self):
        docid = 9
        relevancy = 1.3
        mr = columbus.MatchResults()
        self.assertEqual(len(mr), 0)
        
        mr.add_match(docid, relevancy)
        self.assertEqual(len(mr), 1)
        self.assertEqual(mr.get_id(0), docid)
        self.assertAlmostEqual(mr.get_relevancy(0), relevancy, 0.01)

class TestMatcher(unittest.TestCase):
    
    def test_init(self):
        m = columbus.Matcher()
        
    def test_simple_match(self):
        c = columbus.Corpus()
        m = columbus.Matcher()
        name1 = 0;
        name2 = 10;
        name3 = 1000;
        textName = columbus.Word("title")

        d1 = columbus.Document(name1)
        d1.add_text(textName, "abc def")
        d2 = columbus.Document(name2)
        d2.add_text(textName, "abe test")
        dFar = columbus.Document(name3)
        dFar.add_text(textName, "faraway donotmatchme")
        c.add_document(d1)
        c.add_document(d2)
        c.add_document(dFar)
        m.index(c)

        matches = m.match("abe")
        self.assertEqual(len(matches), 2)
        self.assertNotEqual(matches.get_document_id(0), name3);
        self.assertNotEqual(matches.get_document_id(1), name3);
        self.assertTrue(matches.get_document_id(0) == name1 or matches.get_document_id(1) == name1)
        self.assertTrue(matches.get_document_id(0) == name2 or matches.get_document_id(1) == name2)
    
    def test_errorvalues(self):
        m = columbus.Matcher()
        ev = m.get_errorvalues()
        ev.add_standard_errors()
    
    def test_indexweights(self):
        m = columbus.Matcher()
        iw = m.get_indexweights()
        field = columbus.Word("abc")
        self.assertAlmostEqual(iw.get_weight(field), 1.0, 0.0001)
        


class TestErrorValues(unittest.TestCase):
    
    def test_init(self):
        ev = columbus.ErrorValues()
        
    def test_values(self):
        small_error = 1;
        l1 = 16;
        l2 = 17;
        ev = columbus.ErrorValues()
        default_error = columbus.ErrorValues.get_default_error()
        self.assertLess(small_error, default_error)

        self.assertEqual(ev.get_substitute_error(l1, l2), default_error);
        ev.set_error(l1, l2, small_error);
        self.assertEqual(ev.get_substitute_error(l1, l2), small_error);
        self.assertEqual(ev.get_substitute_error(l2, l1), small_error);

        ev.clear_errors();
        self.assertEqual(ev.get_substitute_error(l1, l2), default_error);

class TestIndexWeights(unittest.TestCase):
    
    def test_init(self):
        w = columbus.IndexWeights()

    def test_weights(self):
        w = columbus.IndexWeights()
        original_weight = 1.0
        new_weight = 2.0
        accuracy = 4
        field = columbus.Word("abc")
        self.assertNotAlmostEqual(original_weight, new_weight, accuracy)

        self.assertAlmostEqual(w.get_weight(field), original_weight, accuracy)
        w.set_weight(field, 2.0)
        self.assertAlmostEqual(w.get_weight(field), new_weight, accuracy)

if __name__ == '__main__':
    unittest.main()
