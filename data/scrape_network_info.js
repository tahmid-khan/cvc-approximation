// page: https://networkrepository.com/networks.php

const rows = document.querySelectorAll("#myTable>tbody tr");

function scrapeVertexCount(row) {
	const val = row.children[2].getAttribute("class").substr(12);
	return val.substr(0, val.indexOf("}"));
}

function scrapeDownloadUrl(row) {
	const a = row.lastChild.lastChild;
	return a.getAttribute("href");
}

list = [];
for (row of rows) {
	const s = scrapeDownloadUrl(row) + "\t" + scrapeVertexCount(row);
	list.push(s);
}

console.log("zip_url\tgraph_order\n");
console.log(list.join("\n"));
