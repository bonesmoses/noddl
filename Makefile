MODULES = noddl

PGFILEDESC = "Extension to block DDL for no reason."
EXTENSION = noddl
DATA = noddl--1.0.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
