#!/usr/bin/env python3

import concurrent.futures
import os
import requests
import sys
import time

# Functions

def usage(status=0):
    progname = os.path.basename(sys.argv[0])
    print(f'''Usage: {progname} [-h HAMMERS -t THROWS] URL
    -h  HAMMERS     Number of hammers to utilize (1)
    -t  THROWS      Number of throws per hammer  (1)
    -v              Display verbose output
    ''')
    sys.exit(status)

def hammer(url, throws, verbose, hid):
    ''' Hammer specified url by making multiple throws (ie. HTTP requests).

    - url:      URL to request
    - throws:   How many times to make the request
    - verbose:  Whether or not to display the text of the response
    - hid:      Unique hammer identifier

    Return the average elapsed time of all the throws.
    '''
    start = time.time()

    r = requests.get(url)
    if verbose:
        print(r.text)



    end = time.time() - start
    return end

def do_hammer(args):
    ''' Use args tuple to call `hammer` '''
    return hammer(*args)

def main():
    hammers = 1
    throws  = 1
    verbose = False
    arguments = sys.argv[1:]
    URL = None

    if len(sys.argv) == 1:
        usage()

    # Parse command line arguments
    while arguments:
        arg = arguments.pop(0)

        if arg == '-h':
            hammers = arguments.pop(0)
        elif arg == '-t':
            throws = arguments.pop(0)
        elif arg == '-v':
            verbose = True
        else:
            URL = arg
    t = hammer(URL, throws, verbose, hammers)
    print(f'total time: {round(t, 2)}')
   
    # Create pool of workers and perform throws
    pass

# Main execution

if __name__ == '__main__':
    main()

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
