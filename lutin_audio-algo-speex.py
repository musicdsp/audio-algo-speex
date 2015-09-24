#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "speex algos wrapper"


def create(target):
	my_module = module.Module(__file__, 'audio-algo-speex', 'LIBRARY')
	my_module.add_src_file([
		'audio/algo/speex/debug.cpp',
		'audio/algo/speex/Resampler.cpp'
		])
	
	my_module.add_header_file([
		'audio/algo/speex/Resampler.h'
		])
	
	my_module.add_module_depend(['etk', 'audio'])
	
	my_module.add_optionnal_module_depend('speex-dsp', ["c++", "-DHAVE_SPEEX_DSP"])
	
	my_module.add_path(tools.get_current_path(__file__))
	# return module
	return my_module









