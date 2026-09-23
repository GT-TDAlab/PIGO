/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains tests for the FileReader class
 */

#include "tests.hpp"
#include "pigo.hpp"

#include <string>

using namespace std;
using namespace pigo;

int skip_comments() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n# Comment\n%Next comment\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    EQ(r.d-in_start, 0);
    r.skip_comments();
    EQ(r.d-in_start, 16);
    r.skip_comments();
    EQ(r.d-in_start, 16);

    r += 6;

    r.skip_comments();
    EQ(r.d-in_start, 46);
    
    return 0;
}

int read_int() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n# Comment\n%Next comment\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    r += 46;

    EQ(r.read_int<uint32_t>(), 9);
    EQ(r.read_int<uint32_t>(), 9);
    EQ(r.read_int<uint32_t>(), 3);
    EQ(r.read_int<uint32_t>(), 123);
    EQ(r.read_int<uint32_t>(), 321);

    return 0;
}

int at_eol() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n# Comment\n%Next co   \r\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    EQ(r.at_end_of_line(), false);
    r = r + 40;
    EQ(r.at_end_of_line(), true);

    return 0;
}

int move_to_non_int() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n# Comment\n%Next comment\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    EQ(r.d-in_start, 0);
    r.move_to_non_int();
    EQ(r.d-in_start, 0);

    r += 16;
    r.move_to_non_int();
    EQ(r.d-in_start, 17);

    r += (53-17);
    r.move_to_non_int();
    EQ(r.d-in_start, 55);

    return 0;
}

int move_to_next_int() {
            //  0         1          2          3          4          5          6          7
            //  0123456789012345 678901 2345678901 23456789012345 678901 234567890 12345678901
    string in {"# Comment field\n2 3 4\n# Comment\n%Next comment\n9 9 3\n123 #NEW\nmore   321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    EQ(r.d-in_start, 0);
    r.move_to_next_int();
    EQ(r.d-in_start, 16);
    r.move_to_next_int();
    EQ(r.d-in_start, 18);
    r.move_to_next_int();
    EQ(r.d-in_start, 20);

    r += (53-20);
    r.move_to_next_int();
    EQ(r.d-in_start, 68);

    return 0;
}

int copying() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n# Comment\n%Next comment\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    r += 10;

    FileReader r2 = r + 5;

    EQ(r2.d-in_start, 15);
    EQ(r.d-in_start, 10);

    return 0;
}

int good() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n# Co 4 5 \n%Next comment\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    r += 20;
    r.move_to_next_int();

    EQ(r.d-in_start, 46);

    return 0;
}

int move_in_comment() {
            //  0         1          2          3          4          5
            //  0123456789012345 678901 2345678901 23456789012345 678901 23456789 
    string in {"# Comment field\n2 3 4\n#2 999 5 \n%Next comment\n9 9 3\n123 321\n"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    r += 22;

    r.move_to_next_int();
    EQ(r.d-in_start, 46);

    EQ(r.size(), 59lu+1lu-(r.d-in_start));

    return 0;
}

/** at_str must match a string that ends exactly at end-of-input.
 *
 * Regression test: at_str compares the half-open range [d, d+s.size()), which
 * is in-bounds when d + s.size() == end. The bounds check used >= instead of
 * >, so a string filling the reader to its exact end was reported as absent --
 * e.g. a file whose entire content is a format header would fail detection.
 */
int at_str_exact_end() {
    string in {"PIGO"};

    FilePos in_start = in.c_str();
    FilePos in_end = in_start + in.size();

    FileReader r {in_start, in_end};

    // The whole input is exactly the string: it must match.
    EQ(r.at_str("PIGO"), true);
    // A longer string cannot fit and must not match.
    EQ(r.at_str("PIGONE"), false);
    // A prefix that leaves input remaining still matches.
    EQ(r.at_str("PIG"), true);

    return 0;
}

int main() {
    int pass = 0;

    TEST(skip_comments);
    TEST(read_int);
    TEST(at_eol);
    TEST(move_to_non_int);
    TEST(move_to_next_int);
    TEST(copying);
    TEST(good);
    TEST(move_in_comment);
    TEST(at_str_exact_end);

    return pass;
}

