#!/usr/bin/env python3

# Author: Kasimir Gabert

import datetime
import argparse
import os

def write_header(o):
    o.write(f'''/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 *
 * Release <UNRELEASED VERSION FROM GIT>.
 *
 * Copyright (c) 2021-2022, GT-TDAlab (Umit V. Catalyurek)
 * Copyright (c) {datetime.date.today().year}, Kasimir Gabert
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef PIGO_HPP
#define PIGO_HPP

''')

def write_footer(o):
    o.write(r'''

#endif /* PIGO_HPP */
''')

seen_files = set()

def get_code(fn, root=True):
    if fn in seen_files:
        raise Exception('Loop detected')
    seen_files.add(fn)

    includes = set()
    code = ''
    path = os.path.dirname(fn)

    in_ns = False
    with open(fn) as f:
        for l in f:
            if l.startswith('#include '):
                if 'pigo' in l:
                    # This is another file, recurse
                    new_fn = os.path.join(path, l.split('"')[1])
                    print(f'Retrieving from: {new_fn}')
                    new_includes, new_code = get_code(new_fn, False)
                    includes = includes.union(new_includes)
                    code += new_code
                else:
                    includes.add(l)
            if in_ns:
                if l == '}\n':
                    in_ns = False
                else:
                    code += l
            elif 'namespace pigo {' in l:
                in_ns = True
                if root:
                    code += l

    if root:
        code += '\n}'

    return includes, code

def write_includes(includes, o):
    si = sorted(list(includes))
    for i in si:
        o.write(i)
    o.write('\n')

def create_single(fn, o):
    write_header(o)
    includes, code = get_code(fn)
    write_includes(includes, o)
    o.write(code)
    write_footer(o)

def main():
    parser = argparse.ArgumentParser(description='Create a single PIGO .hpp file')
    parser.add_argument('--start_file', default='include/pigo.hpp', help='The starting pigo.hpp file')
    parser.add_argument('output_file', help='The release single pigo.hpp file')

    args = parser.parse_args()

    with open(args.output_file, 'w') as o:
        create_single(args.start_file, o)

if __name__ == '__main__':
    main()
