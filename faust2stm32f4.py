#!/usr/bin/env python

import re
import subprocess
import os

def interface2embedded_interface(interface_cpp, inputs=('acc_abs','acc_x','acc_y','acc_z')):
    """
    Convert Faust interface function to a faster hard-coded interface without strings.
    """
    add_re = re.compile(r'interface->add\w*\(("\w*", &\w*),')
    ch_lines = []
    for line in interface_cpp:
        line = line.strip()
        if line.startswith('interface->add'):
            par = add_re.findall(line)[0].split(',')
            name = par[0].strip('"')
            loc = par[1].strip()
            if name in inputs:
                ch_lines.append('synth_interface.{} = {};'.format(name, loc))

    return ch_lines

def faust_postprocess(source):

    # replace cpp templates with faster fixed defined functions
    source = source.replace('faustpower<2>', 'faustpower2')
    source = source.replace('faustpower<3>', 'faustpower3')

    # replace interface function with simpler version without strings
    start = source.find('virtual void buildUserInterface(UI* interface) {')
    stop = source.find('}',start)
    interface_embedded = interface2embedded_interface(source[start:stop].splitlines())

    new_source = ''.join((
        source[:start],
        'virtual void buildUserInterfaceEmbedded() {\n\t\t',
        '\n\t\t'.join(interface_embedded),
        '\n\t',
        source[stop:]))

    return new_source

def faust_replace_fixed_sliders(faust_in):
    """
    replace hsliders with fixed values, useful for quick testing
    of settings on the pc but compiling the as fixed on embedded platform.
    vsliders stay variable.
    """
    replace_slider = re.compile(r'hslider\(".*",([^,]*),[^)]*\)')
    return replace_slider.sub(r'\1',faust_in)

def main(faust_source='synth.dsp', faust_template='faust_synth_template.cpp'):

    tmp_file = faust_source + '.tmp'

    # preprocess: replace hsliders with fixed values, useful for quick testing
    # of settings on the pc but compiling the as fixed on embedded platform.
    # vsliders stay variable.
    with open(tmp_file,'w') as f:
        f.write(faust_replace_fixed_sliders(open(faust_source).read()))

    # run faust compiler
    faust_cpp = subprocess.check_output(['faust','-a',faust_template,tmp_file])

    # do some postprocessing for efficient embedded use
    faust_cpp = faust_postprocess(faust_cpp)
    with open('synth.cpp','w') as f:
        f.write(faust_cpp)

if __name__ == '__main__':
    main()
