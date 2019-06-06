Import("env")

def build_webUI(*args, **kwargs):
   env.Execute("terser --compress --output data/js_sensesp.js -- web/docroot/js/sensesp.js")
   env.Execute("cp web/docroot/js/jsoneditor.min.js data/js_jsoneditor.min.js")
   env.Execute("cp web/docroot/index.html data/index.html")
   env.Execute("cp web/docroot/setup/index.html data/setup.html")

env.AlwaysBuild(env.Alias("webUI", None, build_webUI))
