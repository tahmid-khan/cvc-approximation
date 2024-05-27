#!/usr/bin/env python3


from io import StringIO
from os import PathLike
from pathlib import Path
import sys
from typing import TypeAlias
from urllib import request
from zipfile import ZipFile
import shutil

import networkx as nx
import scipy

_PathLike: TypeAlias = str | PathLike
RAW_DATA_ROOT: str = "./raw_data"


def download(
    url: str,
    destdir: _PathLike = Path(RAW_DATA_ROOT) / "downloaded",
    filename: str | None = None,
    *,
    overwrite: bool = False,
) -> tuple[Path | None, str | None]:
    try:
        destdir = Path(destdir)
        if not destdir.exists():
            destdir.mkdir(parents=True)
        elif not destdir.is_dir():
            return None, "Destination path is not a directory"

        if filename is None:
            filename = url.split("/")[-1]
        dest = destdir / Path(filename)

        path = dest
        if not dest.exists() or overwrite:
            path_str, _ = request.urlretrieve(url, dest)
            path = Path(path_str)
        err = "Destination path already exists" if dest.exists() else None
        return path, err
    except Exception as e:
        return None, f"Exception: `{e}`"


def unzip(
    file: _PathLike,
    destdir: _PathLike = Path(RAW_DATA_ROOT) / "unzipped",
) -> tuple[Path | None, str | None]:
    destdir = Path(destdir) / Path(file).stem
    try:
        with ZipFile(file, mode="r") as zip_file:
            zip_file.extractall(path=destdir)
            return destdir, None
    except Exception as e:
        return None, f"Exception: `{e}`"


# def correct(path: Path):
#     if path.name == "bio-human-gene2.edges":
#         lines = path.read_text().split("\n")
#         lines[17] = lines[17][2:]
#         path.write_text("\n".join(lines))
#         return read_mtx(path)
#     elif path.name in ("bio-mouse-gene.edges", "ca-IMDB.edges"):
#         lines = path.read_text().split("\n")
#         lines[2] = lines[2][1:]
#         path.write_text("\n".join(lines))
#         return read_mtx(path)


def read_graph(path: Path) -> tuple[nx.Graph | None, str | None]:
    if path.suffix == ".mtx":  # Matrix Market format
        return read_mtx(path)
    elif path.suffix == ".edges":  # edge list
        return read_edges(path)
    else:
        return None, "File extension not recognized"


SPECIAL = (
    "bio-celegans.mtx",
    "bio-diseasome.mtx",
    "bio-dmela.mtx",
    "ca-AstroPh.mtx",
    "ca-CondMat.mtx",
    "ca-GrQc.mtx",
    "ca-HepPh.mtx",
)

SPECIAL2 = {
    "ca-CSphd.mtx": 2,
    "ca-HepPh.mtx": 1,
    "ca-netscience.mtx": 1,
    "ca-sandi_auths.mtx": 2,
    "ca-Erdos992.mtx": 2,
}


def correct(path: Path):
    if path.name in SPECIAL2:
        with open(path, mode="r") as f:
            lines = f.readlines()
        line_num = SPECIAL2[path.name]
        lines[line_num] = lines[line_num][1:]
        with open(path, mode="w") as f:
            f.writelines(lines)

    if path.name == "DSJC1000-5.mtx":
        with open(path, mode="r") as f:
            lines = f.readlines()
        lines[1] = "1000 1000 249826\n"
        with open(path, mode="w") as f:
            f.writelines(lines)
    elif path.name == "DSJC500-5.mtx":
        with open(path, mode="r") as f:
            lines = f.readlines()
        lines[1] = "500 500 62624\n"
        with open(path, mode="w") as f:
            f.writelines(lines)
    elif path.name == "p-hat1000-3.mtx":
        with open(path, mode="r") as f:
            lines = f.readlines()
        lines[1] = "1000 1000 371746\n"
        with open(path, mode="w") as f:
            f.writelines(lines)
    elif path.name == "p-hat500-1.mtx":
        with open(path, mode="r") as f:
            lines = f.readlines()
        lines[1] = "500 500 31569\n"
        with open(path, mode="w") as f:
            f.writelines(lines)


def read_mtx(path: Path) -> tuple[nx.Graph | None, str | None]:
    # if path.name in SPECIAL:
    #     text = "%" + text
    # if path.name in SPECIAL2:
    #     lines = text.split("\n")
    #     lines[SPECIAL2[path.name]] = lines[SPECIAL2[path.name]][1:]  # strip the leading "% "
    #     text = "\n".join(lines)

    # print(f"Reading {path.name}...", end=" ", file=sys.stderr)
    correct(path)
    with open(path, mode="r") as f:
        text = f.read()
        # for field in ("real", "double", "complex", "integer"):
        #     if field in text:
        #         text = text.replace(field, "pattern")
        #         break
        if not text.startswith("%%"):
            text = "%" + text
        # try:
        matrix = scipy.io.mmread(StringIO(text))
        # except Exception as e:
        #     return None, f"Error reading Matrix Market file: `{e}`"

        try:
            return nx.from_scipy_sparse_array(matrix), None
        except Exception as e:
            err_msg = f"Failed to read as sparse matrix: `{e}`"
            # try:
            return nx.from_numpy_array(matrix), err_msg
            # except Exception as e:
            #     return None, f"{err_msg}... Also failed to read as square matrix: `{e}`"
    # with open(path, mode="w") as f:
    #     f.write(text)


def read_edges(path: Path) -> tuple[nx.Graph | None, str | None]:
    text = path.read_text().replace(",", " ")
    # if path.name == "bio-grid-yeast.edges":
    #     text = text[3:]  # strip the "%% " at the beginning
    return nx.parse_edgelist(text.split("\n"), comments="%", data=False), None
    # with open(path, mode="r") as f:
    #     text = f.read(1024).split("\n")[-1]
    #     delim = "," if "," in text else None
    #     return nx.read_edgelist(f, delimiter=delim, comments="%", data=False), None


def validate_graph(g: nx.Graph) -> list[str]:
    errors: list[str] = []

    if g.order() == 1:
        errors.append("Graph has only one vertex")

    self_loops = ", ".join(str(u) for u, v in g.edges() if u == v)
    if self_loops:
        errors.append(f"These vertices have self-loops: {self_loops}")

    if not nx.is_connected(g):
        errors.append("Graph is not connected")

    return errors


def preprocess_graph(g: nx.Graph, relabel: bool = True) -> nx.Graph:
    g = g.to_undirected()
    g.remove_edges_from(nx.selfloop_edges(g))
    if relabel:
        g = relabel_graph(g)
    return g


def relabel_graph(g: nx.Graph) -> nx.Graph:
    """Relabels the nodes using unsigned integers."""
    mapping = {}
    v = 0
    for node in g.nodes:
        mapping[node] = v
        v += 1
    return nx.relabel_nodes(g, mapping, copy=True)


def graph_to_string(g: nx.Graph) -> str:
    edges = "".join(sorted_edges(g))
    return f"""{g.order()}
{g.size()}
{edges}"""


def sorted_edges(g: nx.Graph) -> list[str]:
    return sorted(f"{u} {v}\n" for u in g.nodes() for v in g.adj[u] if u < v)


def file_size(file_path: Path) -> int:
    assert file_path.is_file()
    return file_path.stat().st_size


def directory_size(dir_path: Path) -> int:
    assert dir_path.is_dir()
    return sum(f.stat().st_size for f in dir_path.glob("**/*") if f.is_file())


def delete_file_or_dir(path: Path) -> None:
    if path.is_file():
        path.unlink()
    else:
        shutil.rmtree(path)


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


if __name__ == "__main__":
    g = nx.Graph()
    g.add_nodes_from(range(0, 5))
    g.add_edges_from([(0, 1), (3, 2), (3, 2), (3, 3), (3, 2), (3, 2)])
    assert sorted_unique_edges_str(g) == "0 1\n2 3\n"
