Import("env")

def make_c_header(inName, outName):

   print('Writing ',inName,' to src/net/web/',outName,'.h')

   infile = open('web/docroot/' + inName, "r")
   outfile = open("src/net/web/" + outName + ".h","w") 

   outfile.write("#include <pgmspace.h>\n")
   outfile.write("const char PAGE_")
   outfile.write(outName)
   outfile.write("[] PROGMEM = R\"=====(\n")

   for line in infile:
      outfile.write(line)

   outfile.write("\n)=====\";\n")

   infile.close()
   outfile.close()


def build_webUI(*args, **kwargs):
   env.Execute("terser --compress --output web/docroot/js/sensesp.min.js -- web/docroot/js/sensesp.js")
   make_c_header("js/sensesp.min.js", "js_sensesp")
   make_c_header("js/jsoneditor.min.js", "js_jsoneditor")
   make_c_header("index.html", "index")
   make_c_header("setup/index.html", "setup")

env.AlwaysBuild(env.Alias("webUI", None, build_webUI))
