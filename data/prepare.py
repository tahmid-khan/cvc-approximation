import csv
import io
import zipfile
from pathlib import Path
from urllib import request

import networkx as nx
import scipy

_DATA_ROOT = Path(".")
REPOSITORY_INDEX_FILE = _DATA_ROOT / "network_info.tsv"
ZIPS_ROOT = _DATA_ROOT / "raw" / "zips"
EXTRACTED_ROOT = _DATA_ROOT / "raw" / "extracted"
GRAPHS_ORDER_02_32_ROOT = _DATA_ROOT / "prepared" / "order_02-32"
GRAPHS_ORDER_33_64_ROOT = _DATA_ROOT / "prepared" / "order_33-64"
GRAPHS_ORDER_65_99_ROOT = _DATA_ROOT / "prepared" / "order_65-99"


def main():
    prepare()


def prepare(already_downloaded=False, already_extracted=False):
    if not already_downloaded:
        ZIPS_ROOT.mkdir(parents=True, exist_ok=True)
        download_zips()

    if not already_downloaded or not already_extracted:
        EXTRACTED_ROOT.mkdir(exist_ok=True)
        extract_zips()

    GRAPHS_ORDER_02_32_ROOT.mkdir(parents=True, exist_ok=True)
    GRAPHS_ORDER_33_64_ROOT.mkdir(exist_ok=True)
    GRAPHS_ORDER_65_99_ROOT.mkdir(exist_ok=True)
    for path in EXTRACTED_ROOT.iterdir():
        if path.is_file():
            prepare_graph(path)


def download_zips():
    for row in csv.DictReader(open(REPOSITORY_INDEX_FILE), delimiter="\t"):
        order = int(row["graph_order"])
        if not (2 <= order <= 99):
            continue
        url = row["zip_url"]
        filename = url.split("/")[-1]
        print(f"Downloading {url} -- stated graph order: {order}")
        dest_path = ZIPS_ROOT / filename
        if dest_path not in ZIPS_ROOT.iterdir():
            request.urlretrieve(url, dest_path)
            if not zipfile.is_zipfile(dest_path):
                dest_path.unlink()


def extract_zips():
    for path in ZIPS_ROOT.iterdir():
        if path.suffix != ".zip":
            continue
        print(f"Extracting {path}")
        with zipfile.ZipFile(path, "r") as zip_file:
            zip_file.extractall(EXTRACTED_ROOT)


def prepare_graph(path: Path):
    print(f"Processing {path}: ", end="")
    if path.suffix == ".mtx":
        text = path.read_text()
        for field in ("real", "double", "complex", "integer"):
            if field in text:
                text = text.replace(field, "pattern")
                break

        try:
            matrix = scipy.io.mmread(io.StringIO(text))
        except Exception as err:
            print(f"skipped because of error reading/parsing Matrix Market file: {err}")
            return

        try:
            g = nx.from_scipy_sparse_array(matrix)
        except Exception as err:
            print(
                f"Failed to read as sparse matrix (exception: {err}), trying square matrix.",
                end=" ",
            )
            try:
                g = nx.from_numpy_array(matrix)
            except Exception as err:
                print(f"Failed to read as square matrix (exception: {err}) -- skipped.")
                return

    elif path.suffix == ".edges":
        try:
            g = nx.Graph()
            for line in open(path).read().split("\n"):
                line = line.split("%", maxsplit=1)[0]
                if line == "":
                    continue
                line = line.replace(",", " ")
                nodes = tuple(map(int, line.split()[:2]))
                if len(nodes) < 2:
                    continue
                g.add_edge(*nodes)
        except Exception as err:
            print(f"skipped because of error: {err}")
            return
    else:
        print("skipped because file extension not recognized")
        return

    if not nx.is_connected(g):
        print("skipped because graph not connected")
        return

    if g.order() < 2:
        print("skipped because graph order < 2")
        return
    elif g.order() <= 32:
        dest_path = GRAPHS_ORDER_02_32_ROOT
    elif g.order() <= 64:
        dest_path = GRAPHS_ORDER_33_64_ROOT
    elif g.order() <= 99:
        dest_path = GRAPHS_ORDER_65_99_ROOT
    else:
        print("skipped because graph order > 99")
        return

    print(f"graph order: {g.order()}")
    g = relabel(g)

    with open(dest_path / f"{Path(path).stem}.txt", "w") as f:
        f.write(nx_graph_to_text(g))


def relabel(g: nx.Graph) -> nx.Graph:
    mapping = {}
    v = 0
    for node in g.nodes:
        mapping[node] = v
        v += 1
    return nx.relabel_nodes(g, mapping, copy=True)


def nx_graph_to_text(g: nx.Graph) -> str:
    return f"{g.order()}\n{g.size()}\n" + "\n".join(f"{e[0]} {e[1]}" for e in g.edges())


if __name__ == "__main__":
    main()
