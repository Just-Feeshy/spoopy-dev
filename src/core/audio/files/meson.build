audio_files_src = files(
	'spoopy_audio_files.c'
)

files_backend_opts = {
	'opus' : get_option('support_opus_files'),
}

files_macro = []

foreach file_backend, fopt : files_backend_opts
	if fopt.allowed()
		subdir(file_backend)
		files_macro += ['AUDIO_FILE(@0@)'.format(file_backend.to_upper())]

		audio_files_src += get_variable(f'file_@file_backend@_src')
		spoopy_audio_deps += get_variable(f'file_@file_backend@_dep')
	endif
endforeach

files_macro = ' '.join(files_macro)
config.set('SPOOPY_HAS_AUDIO_FILES', files_macro)
