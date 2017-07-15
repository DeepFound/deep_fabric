#!/usr/bin/env python

__author__ = "Eric Mann, Grant Mills -> Deep Information Sciences."

import argparse
import sys

def main():
    _parser    = argparse.ArgumentParser(description='Deep make SQL Inserts generation.')
    _parser.add_argument('--rows',
                         action='store',
                         dest='rows',
                         default=100000000,
                         help='Number of ROWS to insert.',
                         type=int)

    _parser.add_argument('--chunk',
                         action='store',
                         dest='chunk',
                         default=1000,
                         help='Number of rows per chunk.',
                         type=int)

    _parser.add_argument('--output',
                         action='store',
                         dest='output',
                         default='/usr/local/deep-demo/inserts.sql',
                         help='Filename to write the insert statements into.')

    _args       = _parser.parse_args()

    _file       = open(_args.output, 'w')

    _statements = _args.rows / _args.chunk
    _value      = 0

    for _statement in xrange(_statements):
        for _row in xrange(_args.chunk):
            if _row == 0:
                _file.write('/* 1475006882.51 */ INSERT IGNORE  INTO `table1` (`id`) VALUES ')

            _file.write('(%d)' % (_value,))
            _value += 1

            if _row == (_args.chunk - 1):
                _file.write(';\n')
            else:
                _file.write(',');

if __name__ == "__main__":
    main()
