Import("env")
import gzip

def make_c_header(inName, outName):

   print('Writing ',inName,' to src/sensesp/net/web/',outName,'.h')

   infile = open('web/docroot/' + inName, "rb")
   inFileBytes = infile.read()
   print('Compressing into gzip...')
   inFileGziped = gzip.compress(inFileBytes)
   print('Non compressed size is ', str(len(inFileBytes)), ", gziped size is ", str(len(inFileGziped)), ".")
   outfile = open("src/sensesp/net/web/" + outName + ".h","w") 
   print('Generating header file...')
   outfile.write("#include <pgmspace.h>\n")
   outfile.write("const uint8_t PAGE_")
   outfile.write(outName)
   outfile.write("[] PROGMEM = {\n\t\t")
   lineBreak = 7
   for b in inFileGziped:
      outfile.write(hex(b))
      outfile.write(",")
      if lineBreak == 0:
         outfile.write("\n\t\t")
         lineBreak = 8
      lineBreak-=1

   #for line in infile:
   #   outfile.write(line)

   outfile.write("};\n\n")
   #const uint PAGE_index_size = 8;
   outfile.write("const uint PAGE_")
   outfile.write(outName)
   outfile.write("_size = ")
   outfile.write(str(len(inFileGziped)))
   outfile.write(";\n")
   infile.close()
   outfile.close()


def build_webUI(*args, **kwargs):
   env.Execute("terser --compress --output web/docroot/js/sensesp.min.js -- web/docroot/js/sensesp.js")
   make_c_header("js/sensesp.min.js", "js_sensesp")
   make_c_header("js/jsoneditor.min.js", "js_jsoneditor")
   make_c_header("css/bootstrap.min.css", "css_bootstrap")
   make_c_header("index.html", "index")

env.AlwaysBuild(env.Alias("webUI", None, build_webUI))
