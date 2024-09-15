# SAILDB
<!-- badges: start -->
[![Project Status: WIP – Initial development is in progress, but there has not yet been a stable, usable release suitable for the public.](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](https://www.tldrlegal.com/license/mit-license)
<!-- badges: end -->

> [!IMPORTANT]  
> :construction: This is a work in progress :construction:  

An incomplete, unofficial Python package to interface with [SAIL Databank](https://saildatabank.com)'s database, supported by a C++20 backend.


## Project plan

### ODBC & DBI
- Attempt to conform to [PEP 249](https://peps.python.org/pep-0249/)
- Assess feasibility of using `nanodbc` as `ODBC` backend, or whether a handmade solution would be more perfomant - a C-side implementation of thread safe connection pooling might also be a nice addition

### Performance of Py/C API
- Utilise [Apache Arrow](https://arrow.apache.org/) for data access and to support batched data transfer to Python

### Utilities
- Handmade [DotEnv](https://dotenvx.com/docs/env-file) parser to aid user(s) in managing their workspace alongside profile & secret management
