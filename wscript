#!/usr/bin/env python

def set_options(ctx):
  ctx.tool_options('compiler_cxx')

def configure(ctx):
  ctx.check_tool('compiler_cxx')
  ctx.check_tool('node_addon')
#  ctx.check_cfg(package='taglib', uselib_store='taglib', args='-ltag')

def build(ctx):
  t = ctx.new_task_gen('cxx', 'shlib', 'node_addon', uselib = 'taglib')
#  t.cxxflags = ['-ltag']
  ctx.env.append_value('LINKFLAGS', '-ltag'.split())
  t.target = 'tagnode'
  t.source = [
      'src/tagnode.cpp'
  ]
#  t.uselib = ['taglib']
