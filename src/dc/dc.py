#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## dc.py
##
##  Created on: Dec 21, 2018
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import getopt
import itertools
import os
from pysat.examples.lbx import LBX
from pysat.examples.mcsls import MCSls
from pysat.formula import WCNF
from six.moves import reduce
import sys


#
#==============================================================================
def combine(diags):
    """
        Combine the sets of individual diagnoses.
    """

    def combine_recursive(i, prefix):
        """
            Recursive call to combine diagnoses.
        """

        if i == len(diags):
            cdiags.add(tuple(sorted(set(prefix))))
            return

        for d in diags[i]:
            combine_recursive(i + 1, prefix + d)

    # common diagnoses
    cdiags = set([])

    # it may be a bad idea to use recursion here
    combine_recursive(0, [])

    return sorted(cdiags, key=lambda d: len(d))


#
#==============================================================================
def combine_improved(diags):
    """
        First, apply the improvement step and then the combination.
        This procedure makes the check of Proposition 4 in the memo.
    """

    filt = [[] for dd in diags]
    diags = [[set(d) for d in dd] for dd in diags]

    cdiags1 = set([])

    for i, dd1 in enumerate(diags):
        for d1 in dd1:
            for j, dd2 in enumerate(diags):
                if i != j:
                    for d2 in dd2:
                        if d1.issuperset(d2):
                            break
                    else:
                        filt[i].append(list(d1))
                        break
            else:
                cdiags1.add(tuple(sorted(d1)))

    # some of the observation may end up having no diagnoses
    filt = list(filter(lambda dd: dd, filt))
    #
    # in this case, do no further combinations
    cdiags2 = combine(filt) if len(diags) == len(filt) else []

    return sorted(set(list(cdiags1) + cdiags2), key=lambda d: len(d))


#
#==============================================================================
def filter_garbage(cdiags):
    """
        Apply subsumption operations to get rid of garbage diagnoses.
    """

    def process_diag(processed_db, cl):
        for c in processed_db:
            if c <= cl:
                break
        else:
            processed_db.append(cl)
        return processed_db

    # applying subsumption to get a reduced set of diagnoses
    rdiags = reduce(process_diag, [set(d) for d in cdiags], [])

    return [sorted(d) for d in rdiags]


#
#==============================================================================
def parse_options():
    """
        Parses command-line options:
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'a:e:hiv',
                                   ['algo=',
                                    'enum=',
                                    'help',
                                    'improved',
                                    'solver=',
                                    'verbose'])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize())
        usage()
        sys.exit(1)

    algo = 'lbx'
    enum = 'all'
    improved = False
    solver = 'g3'
    verbose = 0

    for opt, arg in opts:
        if opt in ('-a', '--algo'):
            algo = str(arg)
        elif opt in ('-e', '--enum'):
            enum = str(arg)
        elif opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-i', '--improved'):
            improved = True
        elif opt in ('-s', '--solver'):
            solver = str(arg)
        elif opt in ('-v', '--verbose'):
            verbose += 1
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    # choosing the right MCS enumerator
    if algo == 'lbx':
        mcsls = LBX
    else:
        mcsls = MCSls

    # getting the correct integer number of individual diagnoses
    enum = 0 if enum == 'all' else int(enum)

    return mcsls, enum, improved, solver, verbose, args


#
#==============================================================================
def usage():
    """
        Prints usage message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -a, --algo=<string>      MCS enumeration algorithm to use')
    print('                                 Available values: lbx, mcsls (default: lbx)')
    print('        -e, --enum=<int>         Limit the number of individual diagnoses')
    print('                                 Available values: [0 .. INT_MAX], all (default: all)')
    print('        -h, --help')
    print('        -i, --improved           Apply improved combination')
    print('        -s, --solver=<string>    SAT solver to use')
    print('                                 Available values: g3, g4, lgl, m22, mgh (default: g3)')
    print('        -v, --verbose            Be verbose')


#
#==============================================================================
if __name__ == '__main__':
    # trying to set unbuffered output
    if sys.version_info.major == 2:
        sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    # parsing command-line options
    mcsls, to_enum, improved, solver, verbose, files = parse_options()

    if files:
        # input WCNF formula with multiple observations
        wcnf = WCNF(from_file=files[0], comment_lead=['c', 'o'])

        # multiple observations
        mobs = list(filter(lambda x: x[0] == 'o', wcnf.comments))

        # list of lists of individual diagnoses
        diags = [[] for o in mobs]

        # going through all observations
        for i, o in enumerate(mobs):
            # creating a copy of the formula
            wcnf_o = wcnf.copy()
            for l in o.split()[1:-1]:
                wcnf_o.append([int(l)])

            if verbose:
                print('c observation', i + 1)

            # enumerating individual diagnoses
            with mcsls(wcnf_o, use_cld=True, solver_name=solver) as oracle:
                for j, mcs in enumerate(oracle.enumerate()):
                    if verbose > 1:
                        print('c I:', ' '.join([str(cl_id) for cl_id in mcs]), '0')

                    if to_enum and j + 1 == to_enum:
                        break

                    diags[i].append(tuple(mcs))
                    oracle.block(mcs)

        print('c # of idiags:', sum([len(d) for d in diags]))

        # removing duplicates
        diags = list(set([tuple(sorted(dd)) for dd in diags]))
        diags = [[list(d) for d in dd] for dd in diags]

        # combining individual diagnoses
        if improved:
            # improved combination
            if verbose:
                print('c applying improved combination')

            cdiags = combine_improved(diags)
        else:
            # standard, expensive combination
            if verbose:
                print('c applying standard combination')

            cdiags = combine(diags)

        if verbose > 1:
            for d in cdiags:
                print('c C: {0} 0'.format(' '.join([str(i) for i in d])))

        print('c # of cdiags:', len(cdiags))

        rdiags = filter_garbage(cdiags)

        if verbose:
            for d in rdiags:
                print('c D: {0} 0'.format(' '.join([str(i) for i in d])))

        print('c # of diags:', len(rdiags))
        print('c # of garbage:', len(cdiags) - len(rdiags))
