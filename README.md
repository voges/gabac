# GABAC

Genomics-oriented Context-Adaptive Binary Arithmetic Coding

[![Build Status](https://travis-ci.org/mitogen/gabac.svg?branch=master)](https://travis-ci.org/mitogen/gabac)

**First off, we refer the reader to ``USAGE_POLICY.md``.**

---

## Quick start on Linux

Clone the GABAC repository:

    git clone https://github.com/mitogen/gabac.git

Build the GABAC library and the gabacify application:

    mkdir build
    cd build
    cmake ..
    make

This will generate the gabacify application at ``build/bin/gabacify``.

Perform test roundtrips with ``ci/test_roundtrips.sh``.

## Continuous integration

Commits to this repository are continuously tested on **Travis CI** (https://travis-ci.org/voges/gabac). Take a look at the file ``.travis.yml`` to see what is being done on Travis' (virtual) machines.

## Build system

We use **CMake** (https://cmake.org) as build system.

## Version control system

### Branching

We use **Git** and we use the **Gitflow** workflow (https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow).

That means:

* The ``master`` branch contains only *release* commits.
* Every commit on the master branch is *tagged* according to **Semantic Versioning 2.0.0** (see below).
* Development generally takes place on the ``develop`` branch.
* Actual development takes place in *feature* branches, e.g., ``feature/my_fancy_feature``.
* Once a *feature* is completed, its branch can be merged back into the ``develop`` branch.

### Version numbers

We use the Semantic Versioning 2.0.0 (https://semver.org).

That means:

* The **release** version number format is: MAJOR.MINOR.PATCH
* We increment the
  * MAJOR version when making incompatible API changes,
  * MINOR version when adding functionality in a backwards-compatible manner, and
  * PATCH version when making backwards-compatible bug fixes.
* **Pre-release** versions are denoted by appending a hyphen and a series of dot separated identifiers immediately following the patch version.
  * Example 1: 1.0.0-alpha.1 ("alpha version 1 of the planned major release 1")
  * Example 2: 1.0.0-beta.1 ("beta version 1 of the planned major release 1")
  * Example 3: 1.0.0-rc.1 ("release candidate (rc) 1 of the planned major release 1")
  * Note: 1.0.0-alpha.1 < 1.0.0-beta.1 < 1.0.0-rc.1 by definition (see https://semver.org)

## Who do I talk to?

Jan Voges <[voges@tnt.uni-hannover.de](mailto:voges@tnt.uni-hannover.de)>

Tom Paridaens <[tom.paridaens@ugent.be](mailto:tom.paridaens@ugent.be)>

Mikel Hernaez <[mhernaez@illinois.edu](mhernaez@illinois.edu)>
