if host_machine.system() == 'darwin'
    spoopy_renderer_src += files('metal/spoopy_sokol.m')
else
    spoopy_renderer_src += files('spoopy_sokol.c')
endif
spoopy_renderer_deps += [sokol_dep]
