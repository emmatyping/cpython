project('python', 'c', version: '3.15.0a1', license: 'PSF-2.0', license_files: 'LICENSE')

# Calculate version info from version string above
python_version = meson.project_version()
version_array = python_version.split('.')
python_major = version_array[0]
python_minor = version_array[1]
python_pre = ''
foreach qualifier: ['a', 'b', 'rc']
  if version_array[-1].contains(qualifier)
    patch_array = version_array[2].split(qualifier)
    python_patch = patch_array[0]
    pre_version = patch_array[1]
    python_pre = f'@qualifier@@pre_version@'
    break
  endif
endforeach

if python_pre == ''
  python_patch = version_array[2]
endif

# TODO: non-debug builds
abi_flags = 'd'

ldversion = f'@python_major@.@python_minor@@abi_flags@'

conf = configuration_data()
configure_file(output : 'config.h',
               configuration : conf)

pegen_objs = [
  'Parser/pegen.c',
  'Parser/pegen_errors.c',
  'Parser/action_helpers.c',
  'Parser/parser.c',
  'Parser/string_parser.c',
  'Parser/peg_api.c',
]

tokenizer_objs = [
  'Parser/lexer/buffer.c',
  'Parser/lexer/lexer.c',
  'Parser/lexer/state.c',
  'Parser/tokenizer/file_tokenizer.c',
  'Parser/tokenizer/readline_tokenizer.c',
  'Parser/tokenizer/string_tokenizer.c',
  'Parser/tokenizer/utf8_tokenizer.c',
  'Parser/tokenizer/helpers.c',
]

# TODO: tokenizer headers

# TODO: pegen headers

pobjs = [
  'Parser/token.c'
]

parser_objs = [
  pobjs,
  pegen_objs,
  tokenizer_objs,
  'Parser/myreadline.c',
]

object_objs = [
  'Objects/abstract.c',
  'Objects/boolobject.c',
  'Objects/bytes_methods.c',
  'Objects/bytearrayobject.c',
  'Objects/bytesobject.c',
  'Objects/call.c',
  'Objects/capsule.c',
  'Objects/cellobject.c',
  'Objects/classobject.c',
  'Objects/codeobject.c',
  'Objects/complexobject.c',
  'Objects/descrobject.c',
  'Objects/enumobject.c',
  'Objects/exceptions.c',
  'Objects/genericaliasobject.c',
  'Objects/genobject.c',
  'Objects/fileobject.c',
  'Objects/floatobject.c',
  'Objects/frameobject.c',
  'Objects/funcobject.c',
  'Objects/interpolationobject.c',
  'Objects/iterobject.c',
  'Objects/listobject.c',
  'Objects/longobject.c',
  'Objects/dictobject.c',
  'Objects/odictobject.c',
  'Objects/memoryobject.c',
  'Objects/methodobject.c',
  'Objects/moduleobject.c',
  'Objects/namespaceobject.c',
  'Objects/object.c',
  'Objects/obmalloc.c',
  'Objects/picklebufobject.c',
  'Objects/rangeobject.c',
  'Objects/setobject.c',
  'Objects/sliceobject.c',
  'Objects/structseq.c',
  'Objects/templateobject.c',
  'Objects/tupleobject.c',
  'Objects/typeobject.c',
  'Objects/typevarobject.c',
  'Objects/unicode_format.c',
  'Objects/unicode_formatter.c',
  'Objects/unicode_writer.c',
  'Objects/unicodectype.c',
  'Objects/unicodeobject.c',
  'Objects/unionobject.c',
  'Objects/weakrefobject.c',
]

# TODO: add perf trampoline obj

python_objs = [
  'Python/_contextvars.c',
  'Python/_warnings.c',
  'Python/Python-ast.c',
  'Python/Python-tokenize.c',
  'Python/asdl.c',
  'Python/assemble.c',
  'Python/ast.c',
  'Python/ast_preprocess.c',
  'Python/ast_unparse.c',
  'Python/bltinmodule.c',
  'Python/brc.c',
  'Python/ceval.c',
  'Python/codecs.c',
  'Python/codegen.c',
  'Python/compile.c',
  'Python/context.c',
  'Python/critical_section.c',
  'Python/crossinterp.c',
  'Python/dynamic_annotations.c',
  'Python/errors.c',
  'Python/flowgraph.c',
  'Python/frame.c',
  'Python/frozenmain.c',
  'Python/future.c',
  'Python/gc.c',
  'Python/gc_free_threading.c',
  'Python/gc_gil.c',
  'Python/getargs.c',
  'Python/getcompiler.c',
  'Python/getcopyright.c',
  'Python/getplatform.c',
  'Python/getversion.c',
  'Python/ceval_gil.c',
  'Python/hamt.c',
  'Python/hashtable.c',
  'Python/import.c',
  'Python/importdl.c',
  'Python/index_pool.c',
  'Python/initconfig.c',
  'Python/interpconfig.c',
  'Python/instrumentation.c',
  'Python/instruction_sequence.c',
  'Python/intrinsics.c',
  'Python/jit.c',
  'Python/legacy_tracing.c',
  'Python/lock.c',
  'Python/marshal.c',
  'Python/modsupport.c',
  'Python/mysnprintf.c',
  'Python/mystrtoul.c',
  'Python/object_stack.c',
  'Python/optimizer.c',
  'Python/optimizer_analysis.c',
  'Python/optimizer_symbols.c',
  'Python/parking_lot.c',
  'Python/pathconfig.c',
  'Python/preconfig.c',
  'Python/pyarena.c',
  'Python/pyctype.c',
  'Python/pyfpe.c',
  'Python/pyhash.c',
  'Python/pylifecycle.c',
  'Python/pymath.c',
  'Python/pystate.c',
  'Python/pystats.c',
  'Python/pythonrun.c',
  'Python/pytime.c',
  'Python/qsbr.c',
  'Python/bootstrap_hash.c',
  'Python/specialize.c',
  'Python/stackrefs.c',
  'Python/structmember.c',
  'Python/symtable.c',
  'Python/sysmodule.c',
  'Python/thread.c',
  'Python/traceback.c',
  'Python/tracemalloc.c',
  'Python/uniqueid.c',
  'Python/getopt.c',
  'Python/pystrcmp.c',
  'Python/pystrtod.c',
  'Python/pystrhex.c',
  'Python/dtoa.c',
  'Python/fileutils.c',
  'Python/suggestions.c',
  'Python/perf_trampoline.c',
  'Python/perf_jit_trampoline.c',
  'Python/remote_debugging.c',
  # TODO: replace below dynload file based on platform
  'Python/dynload_shlib.c',
]

# TODO: LIBOBJS: where does this come from?
# TODO: MACHDEP OBJS: added via flag
# TODO: DTRACE_OBJS (from configure)
# TODO: PLATFORM_OBJS (from configure)

module_objs = [
  'Modules/config.c',
  'Modules/main.c',
  'Modules/gcmodule.c',
]

# these were set by makesetup
mod_objs = [
  'Modules/_asynciomodule.c',
  'Modules/_bisectmodule.c',
  'Modules/_csv.c',
  'Modules/_datetimemodule.c',
  'Modules/_decimal/_decimal.c',
  'Modules/_heapqmodule.c',
  'Modules/_interpchannelsmodule.c',
  'Modules/_interpqueuesmodule.c',
  'Modules/_interpretersmodule.c',
  'Modules/_json.c',
  'Modules/_lsprof.c',
  'Modules/rotatingtree.c',
  'Modules/_multiprocessing/multiprocessing.c',
  'Modules/_multiprocessing/semaphore.c',
  'Modules/_opcode.c',
  'Modules/_pickle.c',
  'Modules/_queuemodule.c',
  'Modules/_randommodule.c',
  'Modules/socketmodule.c',
  'Modules/_statisticsmodule.c',
  'Modules/_struct.c',
  'Modules/_typesmodule.c',
  'Modules/_typingmodule.c',
  'Modules/_zoneinfo.c',
  'Modules/arraymodule.c',
  'Modules/binascii.c',
  'Modules/cmathmodule.c',
  'Modules/mathmodule.c',
  'Modules/mathintegermodule.c',
  'Modules/mmapmodule.c',
  'Modules/selectmodule.c',
  'Modules/_sysconfig.c',
]

# TODO: go through makesetup and re-implement here

library_objs_omit_frozen = [
  'Modules/getbuildinfo.c',
  parser_objs,
  object_objs,
  python_objs,
  module_objs,
  mod_objs,
]

library_objs = [
  library_objs_omit_frozen,
  'Modules/getpath.c',
  # TODO: generate frozen.c
  #'Modules/frozen.c',
]

# TODO: config.c.in to config.c (handled by makesetup)
inst_so_name = f'python@ldversion@'

inst_lib = shared_library(
  inst_so_name, library_objs,
  link_args: f'-Wl,-h@inst_so_name@',
  install: true,
)

shared_library(
  f'python@python_major@',
  link_args: f'-Wl,-hpython@python_major@',
  link_with: inst_lib,
  install: true,
)