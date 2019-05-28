# Model-Based Diagnosis with Multiple Observations

This repository contains a tool and benchmark distribution accompanying the paper *"Model-Based Diagnosis with Multiple Observations"* by [Alexey Ignatiev](https://reason.di.fc.ul.pt/~aign/), [Antonio Morgado](https://reason.di.fc.ul.pt/wiki/doku.php?id=antonio.morgado), [Georg Weissenbacher](http://www.georg.weissenbacher.name/), and [Joao Marques-Silva](http://www.di.fc.ul.pt/~jpms/) (to be presented at [IJCAI 2019](https://ijcai19.org/)).

## Tools

The distribution includes the tools implementing two approached MBD with multiple observations discussed in the paper:

1. HSD (*implicit* hitting set based approach)
2. DC and DC\* (DiagCombine and its improvement)

### Installation

1. HSD is written in C++ and so it should be compiled before using. For this, it should be enough to do the following:

	```
	$ cd src/hsd
	$ make
	```

2. DiagCombine (DC and DC\*) is written in Python. However, it exploits the original low-level implementations of SAT solvers *under the hood*. This is achieved by using [PySAT](https://pysathq.github.io/). Overall, the following dependancies should be installed:

* [PySAT](https://github.com/pysathq/pysat)
* [six](https://pythonhosted.org/six/)

Please, follow the installation instructions on these projects' websites to install them properly.

### Usage

Both tools have a list of command-line options, which can be seen when using the `-h` options:

```
$ hsd -h
$ dc.py -h
```

In the experiments presented in the paper, the tools were ran in the following way:

1. HSD:

	```
	$ hsd -e all -c -s -v instance.wcnf
	```

2. DC:

	```
	$ dc.py -vv instance.wcnf
	```

3. DC\*:

	```
	$ dc.py -i -vv instance.wcnf
	```

These configurations of HSD and DC/DC\* use [Glucose 3](https://www.labri.fr/perso/lsimon/glucose/) as an underlying SAT solver. All configurations target enumerating all aggregated diagnoses. Additionally, HSD is instructed to use MaxSAT-based enumeration of diagnoses (see option `-s`). Also, every iteration of HSD checks if more diagnoses exist (option `-c`). DC and DC\* are instructed to enumerate diagnoses by applying the LBX algorithm.

All tools were ran under [runsolver](http://www.cril.univ-artois.fr/en/software/runsolver.en.html) to enforce the time limit for each benchmark instance.

## Benchmarks

The set of benchmarks considered in the paper contains 144 instances and can be found in the [experiment/iscas85-mobs](experiment/iscas85-mobs) directory. They are encoded into maximum satisfiability (MaxSAT) and, thus, are in the [WCNF format for MaxSAT](https://maxsat-evaluations.github.io/2019/rules.html#input).

A MaxSAT formula of each instance encodes a *mutated* ISCAS85 circuit. 
The hard part of the formula encodes the flow of the target circuit while the soft clauses *activate* all the gates, i.e. enforce each gate **not** to be *abnormal*.

An observation is written as a comment line starting with `o` and followed by `0`. Each observation is treated as a conjunction of literals that correspond to the input and output of a circuit, i.e. `o -1 2 12 0` represents a conjunction of literals `-1`, `2`, and `12`.

Each circuit mutation represents a stuck-at fault injected in some of the gates. Mutated gates are reported in the comment line: `c mutated gates:`, i.e. `c mutated gates: 246gat` for [iscas85-mobs/c432/c432mut267p.wcnf.gz](iscas85-mobs/c432/c432mut267p.wcnf.gz). The corresonding faulty output of the mutated gate is reported as a Boolean literal (in the aforementioned instance the gate always outputs `True` as `c mutation: 267 0` sets gate's output variable 267 to `True`).

The original ISCAS85 circuits (in the ISCAS89 format) used for generating the benchmarks can be found in [experiment/orig-circuits](experiment/orig-circuits). However, note that they **are not augmented** with a list of observations.

## Citation

```
@inproceedings{imwms-ijcai19,
  author    = {Alexey Ignatiev and
               Antonio Morgado and
               Georg Weissenbacher and
               Joao Marques{-}Silva},
  title     = {Model-Based Diagnosis with Multiple Observations},
  booktitle = {IJCAI},
  year      = {2019},
  note      = {To Appear}
}
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
