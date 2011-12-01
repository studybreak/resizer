

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.cxxflags = (["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
      + ["-I/usr/include/GraphicsMagick", "-L${exec_prefix}/lib", "-lGraphicsMagick", "-llcms", "-ltiff", "-lfreetype", "-ljasper", "-ljpeg", "-lpng", "-lwmflite", "-lXext", "-lSM", "-lICE", "-lX11", "-lbz2", "-lxml2", "-lz", "-lm", "-lgomp", "-lpthread", "-lltdl"]) 
  obj.target = "resizer"
  obj.source = "resizer.cc"
