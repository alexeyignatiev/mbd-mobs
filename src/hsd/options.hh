/*
 * options.hh
 *
 *  Created on: Aug 8, 2016
 *      Author: aign
 */

#ifndef OPTIONS_HH_
#define OPTIONS_HH_

// system includes:
#include <ostream>
#include <string>
#include <vector>

// local includes:
// local defines:

using std::string;

class Options {
public:
	bool parse(int argc, char *argv[]);
	Options()
		: bootstrap            (false)
		, check_more           (false)
		, ctrimming            (0)
		, enumerate            ("1")
		, help_flag            (false)
		, reduce_coex          ("none")
		, sorted               (false)
		, trimming             (0)
		, timeout              (0)
		, verb                 (0)
		, exhaustive_reduction (false)
	{}

	bool get_bootstrap() const { return bootstrap; }
	bool get_check_more() const { return check_more; }
	int get_ctrimming() const { return ctrimming; }
	string get_enumerate() const { return enumerate; }
	bool get_help_flag() const { return help_flag; }
	string get_reduce_coex() const { return reduce_coex; }
	bool get_sorted() const { return sorted; }
	int get_trimming() const { return trimming; }
	int get_timeout() const { return timeout; }
	int get_verb() const { return verb; }
	bool get_exhaustive_reduction() const { return exhaustive_reduction; }

	const std::vector<string>& get_rest() const { return rest; }
	std::ostream& print(std::ostream& out, char *prog_name) const;
private:
	std::vector<string> rest;
	bool bootstrap;
	bool check_more;
	int ctrimming;
	string enumerate;
	bool help_flag;
	string reduce_coex;
	bool sorted;
	int trimming;
	int timeout;
	int verb;
	bool exhaustive_reduction;
protected:
};

#endif // OPTIONS_HH_
