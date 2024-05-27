#!/usr/bin/env python3

import csv
import math
from collections.abc import Callable
import subprocess
from typing import Any, Sequence

import utils


def split_intervals(
    data: Sequence[Any],
    *,
    key: Callable[[Any], float | int] = lambda x: x,
    size: float | int | None = None,
    divisions: int | None = None,
):
    """Divides the range of values in ``data`` into ``divisions`` equal divisions,
    if ``divisions`` is provided, or into maximally-sized intervals whose value-ranges
    don't exceed ``size``, if ``size`` is provided.

    ``divisions`` and ``size`` cannot both be given at the same time.

    :param data: The data of values to be split
    :param key: A function to apply to every element of the data to access the value
        based on which the seuqence is to be split. Defaults to the
        identity function
    :param size: The maximum range of values in each division of the split data; must be
        positive. Should not be provided if the size is to be determined from the number
        of divisions.
    :param divisions: The data will be divided into this many divisions; must be a
        positive integer. Should not be provided at the same time as size.
    :return: A seuqence of non-empty lists, each of which is a subarray of the sorted data
    """

    offset = 0
    if size is None:
        if divisions is None:
            raise ValueError("either size or number must be given")
        if divisions <= 0:
            raise ValueError("number must be positive")

        v = sorted(data, key=key)
        offset = key(v[0])
        size = (key(v[-1]) - offset) / divisions
    else:
        if divisions is not None:
            raise ValueError("only one of size and number can be given")
        if size <= 0:
            raise ValueError("size must be positive")

        v = sorted(data, key=key)
        offset = key(v[0])

    i = 0
    while i < len(v):
        upper_bound = (math.floor((key(v[i]) - offset) / size) + 1) * size
        j = i + 1
        while j < len(v) and key(v[j]) < upper_bound:
            j += 1
        yield v[i:j]
        i = j


def main():
    # data = [2, 3, 3, 3, 234, 2999, 1000, 2000, 2999, 2999.5, 2999.99999999999, 3000]
    # print(list(split_intervals(data, size=1000)))

    with open("filtered_graphs.tsv", mode="r", newline="") as f:
        rows = [row for row in csv.DictReader(f, dialect=csv.excel_tab)]

    def key(row):
        return float(row["density"])

    # rows = list(filter(lambda net: int(net["nodes"]) < 2**20, rows))
    intervals = list(split_intervals(rows, divisions=20, key=key))
    selected = intervals[0] + intervals[1]
    for interval in intervals[2:]:
        selected.append(interval[0])
        # print(len(interval))
    # print(selected)
    selected.sort(key=lambda row: int(row["nodes"]))

    def run(strategy: str, graph: str) -> tuple[int, int]:
        proc = subprocess.run(
            ["./cmake-build-release/ilst_cvc", strategy],
            input=graph,
            capture_output=True,
            text=True,
        )
        assert proc.returncode == 0, f"Returned {proc.returncode}: {proc.stderr}"
        fwd, rev = proc.stdout[:-1].split("\n")
        return len(fwd), len(rev)

    with open("results.tsv", mode="w", newline="") as outfile:
        writer = csv.DictWriter(
            outfile,
            fieldnames=[
                "name",
                "nodes",
                "edges",
                "max_degree",
                "avg_degree",
                "density",
                "dd_fwd",
                "dd_rev",
                "sl_fwd",
                "sl_rev",
                "sll_fwd",
                "sll_rev",
                "sd_fwd",
                "sd_rev",
            ],
            dialect=csv.excel_tab,
            lineterminator="\n",  # default is "\r\n" for exel-tab
        )
        writer.writeheader()
        for network in selected:
            name = network["name"]
            print(name)

            # download
            print("\tDownloading...")
            url = network["download_url"]
            zip_path, err = utils.download(url)
            assert zip_path is not None, f"Error downloading `{url}`: {err}"

            # unzip
            print("\tUnzipping...")
            dest_dir, err = utils.unzip(zip_path)
            # utils.delete_file_or_dir(zip_path)
            assert dest_dir is not None, f"Error unzipping `{zip_path}`: {err}"

            # read
            print("\tReading...")
            files = list(dest_dir.glob("**/*.mtx")) + list(dest_dir.glob("**/*.edges"))
            assert len(files) == 1
            g, err = utils.read_graph(files[0])
            # utils.delete_file_or_dir(dest_dir)
            assert g is not None

            # process and get string representation
            print("\tProcessing...")
            g = utils.preprocess_graph(g)
            g_str = utils.graph_to_string(g)
            with open(f"prepared_data/{name}.txt", mode="w") as f:
                f.write(g_str)

            # run
            max_deg, avg_deg = utils.max_and_avg_degrees(g)
            print("\tRunning... ", end="")
            row = {
                "name": name,
                "nodes": g.order(),
                "edges": g.size(),
                "max_degree": max_deg,
                "avg_degree": avg_deg,
                "density": utils.graph_density(g),
            }
            for strat in ("dd", "sl", "sll", "sd"):
                print(" " + strat, end="")
                fwd, rev = run(strat, g_str)
                row[strat + "_fwd"] = fwd
                row[strat + "_rev"] = rev
            writer.writerow(row)
            print()


if __name__ == "__main__":
    main()
