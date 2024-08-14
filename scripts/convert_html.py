#!/usr/bin/env python3

from dataclasses import dataclass
import gzip
import itertools
import pathlib
import logging
import sys

# import brotli

logging.basicConfig(level=logging.DEBUG)


def gather_files(root_dir: pathlib.Path, root_file_name: str) -> list[pathlib.Path]:
    """
    Gather all files that are required by the root file.

    Assume that the presence of the filename in the root file means that the file is required.
    """
    # get all files under root_dir
    all_files = list(root_dir.glob("**/*"))
    files = [f for f in all_files if f.is_file()]

    required_files = set([root_dir / root_file_name])

    traverse_files = [root_dir / root_file_name]

    # search for each file in the root file
    for traversing in traverse_files:
        with traversing.open() as f:
            contents = f.read()
            for file in files:
                if file.name in contents:
                    if file not in required_files:
                        traverse_files.append(file)
                        required_files.add(file)

    return sorted(list(required_files))


@dataclass
class StaticPage:
    path: str
    content: bytes
    length: int
    content_type: str | None
    # Content-encoding doesn't have to be defined because everything is Brotli

    @classmethod
    def from_path(cls, path: pathlib.Path, omit_root: str = None, base_path: str = "/"):
        path_ = str(path)
        if omit_root is not None:
            path_ = path_.replace(omit_root, "", 1)
        # treat index.html as the root
        if path_.endswith("index.html"):
            path_ = path_.replace("index.html", "", 1)
        if path_.startswith("/"):
            path_ = path_[1:]
        path_ = base_path + path_
        with path.open("rb") as f:
            content = f.read()
            # compressed_content = brotli.compress(content)
            compressed_content = gzip.compress(content)
            ext = path.suffix
            if ext == ".js":
                content_type = "application/javascript"
            elif ext == ".css":
                content_type = "text/css"
            elif ext == ".svg":
                content_type = "image/svg+xml"
            elif ext == ".html":
                content_type = "text/html"
            else:
                content_type = None

        return StaticPage(
            path_, compressed_content, len(compressed_content), content_type
        )

    def export(self):
        """
        Output the static page as a C++ PageData initializer.
        """
        hex_content = ("\\x{:02x}".format(b) for b in self.content)

        chunk_size = 16

        def chunks(lst, chk_size):
            lst_it = iter(lst)
            return iter(lambda: tuple(itertools.islice(lst_it, chk_size)), ())

        wrapped_content = "\n".join(
            '    "' + "".join(g) + '"' for g in chunks(hex_content, chunk_size)
        )

        content_type_output = (
            f'"{self.content_type}",' if self.content_type is not None else "  nullptr,"
        )

        output = f"""{{
  "{self.path}",

{wrapped_content},
  {self.length},
  {content_type_output}
  "gzip",
}}"""

        return output


@dataclass
class PageData:
    static_pages: list[StaticPage]

    def export(self, target: str, constant_name: str):
        page_data_list = ",\n".join(sp.export() for sp in self.static_pages)

        define_guard = target.upper().replace(".", "_").replace("/", "_")

        output = f"""
#ifndef {define_guard}
#define {define_guard}
#include "sensesp/net/web/static_file_data.h"

namespace sensesp {{

const StaticFileData {constant_name}[] = {{
{page_data_list},
  {{nullptr, nullptr, 0, nullptr, nullptr}}
}};

}}  // namespace sensesp

#endif  // {define_guard}
        """

        with open(target, "w") as f:
            f.write(output)


def convert_files(
    root_file_path: pathlib.Path, target: str, constant_name: str, base_path: str = "/"
):
    required_files = gather_files(root_file_path.parent, root_file_path.name)

    # compress and generate header files

    static_pages = [
        StaticPage.from_path(
            rf, omit_root=str(root_file_path.parent), base_path=base_path
        )
        for rf in required_files
    ]

    page_data = PageData(static_pages)
    page_data.export(target, constant_name)


if __name__ == "__main__":
    convert_files(pathlib.Path(sys.argv[1]), sys.argv[2], sys.argv[3], sys.argv[4])
