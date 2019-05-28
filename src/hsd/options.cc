/*
 * options.cc
 *
 *  Created on: Aug 8, 2016
 *      Author: aign
 */

// system includes:
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <vector>
// local includes:
#include "options.hh"

using std::ostream;
using std::endl;

//
//=============================================================================
bool Options::parse(int argc, char *argv[])
{
	bool res = true;

	static const char option_string[] = "bce:hr:st:vx";
	static const struct option long_options[] = {
		{ "bootstrap", 0, 0, 'b' },
		{ "check-more", 0, 0, 'c' },
		{ "ctrim", 1, 0, 258 },
		{ "enum", 1, 0, 'e' },
		{ "help", 0, 0, 'h' },
		{ "reduce-coex", 1, 0, 'r' },
		{ "sorted", 0, 0, 's' },
		{ "trim", 1, 0, 263 },
		{ "timeout", 1, 0, 't' },
		{ "verb", 0, 0, 'v' },
		{ "ex-reduce", 0, 0, 'x' },
		{ 0, 0, 0, 0 }
	};

	while (res) {
		int option_index = 0;

		int c = getopt_long(argc, argv, option_string, long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			break;
		case 'b':
			bootstrap = true;
			break;
		case 'c':
			check_more = true;
			break;
		case 258:
			ctrimming = atoi(optarg);
			break;
		case 'e':
			enumerate = optarg;
			break;
		case 'h':
			help_flag = true;
			break;
		case 'r':
			reduce_coex = optarg;
			break;
		case 's':
			sorted = true;
			break;
		case 263:
			trimming = atoi(optarg);
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case 'v':
			++verb;
			break;
		case 'x':
			exhaustive_reduction = true;
			break;
		case '?':
			res = false;
			break;
		}
	}

	for (int i = optind; i < argc; ++i)
		rest.push_back(argv[i]);

	return res;
}

//
//=============================================================================
ostream& Options::print(ostream& out, char *prog_name) const
{
	out << "Usage: " << prog_name << " [options] file" << endl;
	out << "Options:" << endl;
	out << "        -b, --bootstrap               Bootstrap with disjoint sets" << endl;
	out << "        -c, --check-more              After each computed/reported diagnosis check if there are more" << endl;
	out << "        --ctrim=<int>                 Trim unsatisfiable cores at most k times while computing explanations" << endl;
	out << "                                      Available values: [0 .. INT_MAX] (default = 0)" << endl;
	out << "        -e, --enum=<string>           How many solutions to compute" << endl;
	out << "                                      Available values: [1 .. all], best (default = 1)" << endl;
	out << "        -h, --help                    Show this message" << endl;
	out << "        -r, --reduce-coex=<string>    Policy to reduce counterexamples" << endl;
	out << "                                      Available values: lin, none, qxp (default = none)" << endl;
	out << "        -s, --sorted                  Compute smallest hitting sets first (using MaxSAT)" << endl;
	out << "        --trim=<int>                  Trim unsatisfiable cores at most k times" << endl;
	out << "                                      Available values: [0 .. INT_MAX] (default = 0)" << endl;
	out << "        -t, --timeout=<int>           Specify a time limit" << endl;
	out << "        -v, --verb                    Set level of verbosity" << endl;
	out << "        -x, --ex-reduce               Reduce counterexamples exhaustively (taking into account all observations)" << endl;

	return out;
}
