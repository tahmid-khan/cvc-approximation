#!/usr/bin/env python3

import csv
import pathlib
import shutil
import sys
import zipfile

import networkx as nx
import utils


ZIP_SIZE_LIMIT = 20 * 2**20
ZIP_SIZE_LIMIT_STR = "20 MiB"


def main() -> None:
    with open("network_repository_table.tsv", mode="r", newline="") as infile:
        with open("filtered_graphs.tsv", mode="w", newline="") as outfile:
            writer = csv.DictWriter(
                outfile,
                fieldnames=[
                    "name",
                    "type",
                    "nodes",
                    "edges",
                    "max_degree",
                    "avg_degree",
                    # "density",
                    "zip_size",
                    "unzipped_size",
                ],
                dialect=csv.excel_tab,
                lineterminator="\n",  # default is "\r\n" for exel-tab
            )
            writer.writeheader()

            for row in csv.DictReader(infile, dialect=csv.excel_tab):
                name = row['name']
                # filter
                if row["zip_size"] == "-":
                    continue
                if int(row["zip_size"]) > ZIP_SIZE_LIMIT:
                    print_err(f"Skipped {name}: stated zip size > {ZIP_SIZE_LIMIT_STR}")
                    continue

                # download
                url = row["download_url"]
                # if url == "https://nrvis.com/./download/data/bio/bio-MUTAG_g1.zip":
                #     print_err(f"Skipped {url}")
                #     continue
                zip_path, err = utils.download(url)
                assert zip_path is not None, f"Error downloading `{url}`: {err}"

                # unzip
                if not zipfile.is_zipfile(zip_path):
                    print_err(f"Skipped {name}: {zip_path} is not a zip")
                    delete_file_or_dir(zip_path)
                    continue
                dest_dir, err = utils.unzip(zip_path)
                assert dest_dir is not None, f"Error unzipping `{zip_path}`: {err}"
                zip_size = file_size(zip_path)
                delete_file_or_dir(zip_path)

                # filter
                if zip_size > ZIP_SIZE_LIMIT:
                    print_err(f"Skipped {name}: zip size > {ZIP_SIZE_LIMIT_STR}")
                    continue

                # read
                g = None
                for p in dest_dir.glob("**/*"):
                    if p.is_file() and p.suffix == ".edges":
                        print_err(f"skipping {name}: .edges format")
                        break
                    elif p.is_file() and p.suffix in (".mtx"):
                        g, err = utils.read_graph(p)
                        if g is not None:
                            break
                        print_err(f"Error reading `{p}`: {err}")
                if g is None:
                    # print_err(f"Skipped {name}: graph could not be read")
                    delete_file_or_dir(dest_dir)
                    continue
                unzipped_size = directory_size(dest_dir)
                delete_file_or_dir(dest_dir)

                # filter
                if g.order() < 3:
                    print_err(f"Skipped {name}: too small")
                if nx.is_empty(g):
                    print_err(f"Skipped {name}: no edges")
                if not nx.is_connected(g):
                    print_err(f"Skipped {name}: not a connected graph")
                    continue

                # process
                if g.is_directed():
                    g = g.to_undirected()

                # output
                max_deg, avg_deg = max_and_avg_degrees(g)
                writer.writerow({
                    "name": row["name"],
                    "type": row["type"],
                    "nodes": g.order(),
                    "edges": g.size(),
                    "max_degree": max_deg,
                    "avg_degree": avg_deg,
                    # "density": graph_density(g),
                    "zip_size": zip_size,
                    "unzipped_size": unzipped_size,
                })
                print_err(f'Written {row["name"]}')


def print_err(*values: object):
    print(*values, file=sys.stderr)


def max_and_avg_degrees(g: nx.Graph) -> tuple[int, float]:
    max_deg = 0
    total_deg = 0
    for _, deg in g.degree:
        d = int(deg)
        max_deg = max(max_deg, d)
        total_deg += d
    return max_deg, total_deg / g.order()


def graph_density(g: nx.Graph) -> float:
    return 2 * g.size() / (g.order() * (g.order() - 1))


def file_size(file_path: pathlib.Path) -> int:
    assert file_path.is_file()
    return file_path.stat().st_size


def directory_size(dir_path: pathlib.Path) -> int:
    assert dir_path.is_dir()
    return sum(f.stat().st_size for f in dir_path.glob("**/*") if f.is_file())


def delete_file_or_dir(path: pathlib.Path) -> None:
    if path.is_file():
        path.unlink()
    else:
        shutil.rmtree(path)


if __name__ == "__main__":
    main()
