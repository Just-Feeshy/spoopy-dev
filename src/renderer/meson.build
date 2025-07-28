opt_kore = get_option('enable_kore')
# subdir('rhi')
#
# spoopy_r_opts = {
#     'kore': get_option('enable_kore'),
# }
#
# spoopy_r_deps = []
# spoopy_r_backend_path = ''
# spoopy_r_src = []
# spoopy_r_macros = []
#
# spoopy_needed_subs = [
#     'rhi'
# ]
#
# foreach sub : spoopy_needed_subs
#     subdir(sub)
#     spoopy_r_src += get_variable(f'spoopy_@sub@_src')
#     spoopy_r_deps += get_variable(f'spoopy_@sub@_deps')
# endforeach
#
# subdir('kore')
#
# foreach renderer, opt : spoopy_renderer_opts
#     if opt.allowed()
#         spoopy_r_src += get_variable(f'spoopy_@renderer@_src')
#         spoopy_r_macros += f'R(@renderer@)'
#         spoopy_r_deps += get_variable(f'spoopy_@renderer@_lipdeps')
#     endif
# endforeach
#
# spoopy_r_macros = ' '.join(spoopy_r_macros)
# config.set('SPOOPY_HAS_RENDERER_BACKENDS', spoopy_r_macros)
#
# spoopy_renderer_lib = static_library('spoopy_renderer',
#     sources: spoopy_r_src,
#     include_directories: spoopy_includes,
#     dependencies: spoopy_r_deps,
#     c_args: spoopy_c_args,
#     install: false
# )

if opt_kore.enabled()

endif
