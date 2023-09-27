import pathlib
import scripts.convert_html

Import("env")

# install brotli if it isn't installed
#try:
#    import brotli
#except ImportError:
#    env.Execute("$PYTHONEXE -m pip install brotli")



def convert_files(root_file_path: pathlib.Path, target):
    scripts.convert_html.convert_files(root_file_path, target)


def build_web_ui(*args, **kwargs) -> None:
    # install and build the web UI
    env.Execute("cd web && npm install && npm run build")

    scripts.convert_html.convert_files(
        pathlib.Path("web/dist/index.html"),
        "src/sensesp/net/web/autogen/web_ui_files.h",
        "kWebUIFiles",
        "/"
    )


env.AlwaysBuild(env.Alias("webUI", None, build_web_ui))
