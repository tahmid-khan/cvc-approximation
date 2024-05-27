#!/usr/bin/env python3

import csv
from typing import Mapping

import bs4
import requests

ALT_ATTR_TO_KEY = {
    "Nodes": "nodes",
    "Edges": "edges",
    "Maximum degree": "max_degree",
    "Average degree": "avg_degree",
    "Assortativity": "assortativity",
    "Total triangles": "total_triangles",
    "Average triangles": "avg_triangles",
    "Maximum triangles": "max_triangles",
    "Avg. clustering coef.": "avg_clustering_coef",
    "Frac. closed triangles": "frac_closed_triangles",
    "Maximum k-core": "max_k_core",
    "Max. clique (lb)": "max_clique_size_lb",
    "": "max_clique_size_lb",
}


def main() -> None:
    response = requests.get(
        "https://networkrepository.com/networks.php",
        headers={"User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:121.0)"},
    )
    assert (
        response.status_code == 200
    ), f"Failed to retrieve the webpage. Status code: {response.status_code}"

    soup = bs4.BeautifulSoup(response.text, features="lxml")

    table = soup.find("table", {"id": "myTable"})
    assert table is not None, "Failed to find the table"

    tbody = table.find("tbody")
    assert type(tbody) is bs4.Tag, "Failed to find the table body"

    rows = tbody.findChildren("tr", recursive=False)

    def validate_row_dict(row_dict: Mapping[str, str]) -> bool:
        url = row_dict["download_url"]
        return (
            not url.endswith(".php")
            and url != "https://nrvis.com/./download/data/bio/bio-MUTAG_g1.zip"
        )

    with open("network_repository_table.tsv", mode="w", newline="") as tsv_file:
        writer = csv.DictWriter(
            tsv_file,
            fieldnames=[
                "name",
                "type",
                "nodes",
                "edges",
                "max_degree",
                "avg_degree",
                "assortativity",
                "total_triangles",
                "avg_triangles",
                "max_triangles",
                "avg_clustering_coef",
                "frac_closed_triangles",
                "max_k_core",
                "max_clique_size_lb",
                "zip_size",
                "download_url",
            ],
            dialect=csv.excel_tab,
            # lineterminator="\n",  # default is "\r\n" for exel-tab
        )
        writer.writeheader()
        writer.writerows(filter(validate_row_dict, map(make_row_dict, rows)))


def make_row_dict(row) -> dict[str, str]:
    res: dict[str, str] = {}
    for td in row.findChildren("td", recursive=False):
        key: str
        value: str
        if td.has_attr("align"):
            key = (
                "name"
                if td.findChild("a", attrs={"class": "btn btn_gray_sm btn-xs"})
                else "type"
            )
            value = td_text(td)
        elif td.has_attr("alt"):
            key = ALT_ATTR_TO_KEY[td["alt"]]
            value = td_value(td)
        elif td.has_attr("class") and td["class"][0] == "{sortValue:":
            key = "zip_size"
            value = td_value(td)
        else:
            key = "download_url"
            value = td.findChild("a")["href"]
        res[key] = value
    return res


def td_text(td) -> str:
    return td.text.replace("\xA0", " ").strip()


def td_value(td) -> str:
    text = td_text(td)
    if text == "-":
        return text
    # the value is embedded in the clsas attribute as "{sortValue: <value>}"
    # i.e. the td element's class names are "{sortValue:" and "<value>}"
    # so we extract the value from the second class name by stripping the trailing "}"
    return td["class"][1][:-1]


if __name__ == "__main__":
    main()
