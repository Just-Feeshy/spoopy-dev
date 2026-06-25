spoopy_renderer_src = []
spoopy_renderer_deps = []

spoopy_renderer_src += files('spoopy_backend_stub.c')

sokol_dep = dependency('sokol', fallback: ['sokol', 'sokol_dep'], required: true)
sokol_dep = declare_dependency(
	dependencies: [sokol_dep],
	compile_args: sokol_compile_args,
)

subdir('sokol')
