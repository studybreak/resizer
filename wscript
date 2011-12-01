

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

  libpath = ['/lib', '/usr/lib', '/usr/local/lib', '/opt/local/lib']
  includes = ['/usr/include/GraphicsMagick', '/usr/include', '/usr/includes', '/usr/local/includes', '/opt/local/includes'];

  libmagick = conf.check(lib="GraphicsMagick",
             header_name='magick/api.h',
             includes=includes,
             libpath=libpath,
             function_name='InitializeMagick',
             uselib_store='MAGICK')

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.cxxflags = (["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
      + ["-llcms", "-ltiff", "-lfreetype", "-ljasper", "-ljpeg", "-lpng", "-lwmflite", "-lXext", "-lSM", "-lICE", "-lX11", "-lbz2", "-lxml2", "-lz", "-lm", "-lgomp", "-lpthread", "-lltdl"])
  obj.target = "resizer"
  obj.source = "resizer.cc"
  uselib = "MAGICK"
  obj.uselib = uselib
