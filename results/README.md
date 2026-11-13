# Results

This directory is a placeholder for generated benchmark outputs. The suite
does not ship a fixed set of measurements; the results depend on the host,
the compiler, the options, and the load.

Suggested layout:

```
results/
  csv/       raw CSV blocks from `make run` or `make bench`
  figures/   generated figures from the CSV blocks
```

The benchmark programs print one CSV block per operation to stdout. Redirect
stdout to a file to keep the raw samples:

```sh
make run > results/csv/run.csv
```

The raw samples are always retained, including invalid or migrated rows. The
`valid` column and the `aux_start`/`aux_stop` columns are the basis for the
filtering described in [docs/METHODOLOGY.md](../docs/METHODOLOGY.md).

The `results/` contents are not part of the source tree; the generated CSV
and figure files are ignored by `.gitignore`.
