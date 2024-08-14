import pathlib
import scripts.convert_html

Import("env")

# try:
#    import brotli
# except ImportError:
#    env.Execute("$PYTHONEXE -m pip install brotli")


def convert_files(root_file_path: pathlib.Path, target):
    scripts.convert_html.convert_files(root_file_path, target)


def frontend(*args, **kwargs) -> None:
    # install and build the web UI

    scripts.convert_html.convert_files(
        pathlib.Path("frontend/dist/index.html"),
        "src/sensesp/net/web/autogen/frontend_files.h",
        "kFrontendFiles",
        "/",
    )


env.AlwaysBuild(env.Alias("frontend", None, frontend))
