project(
  'libanim',
  'c',
  version: run_command('version.sh', 'get-vcs').stdout().strip(),
)

meson.add_dist_script('version.sh', 'set-dist', meson.project_version())

add_project_arguments(
  '-DLIBANIM_VERSION="@0@"'.format(meson.project_version()),
  '-DLIBANIM_STATIC',
  language: 'c',
)

deps = [
  dependency('libcurl'),
  dependency('libxml-2.0'),
  dependency('libssl'),
  dependency('libcrypto'),
]

subdir('src')

inc = include_directories('include')
libanim = static_library('anim', sources, include_directories: inc, dependencies: deps)

libanim_dep = declare_dependency(include_directories: inc, link_with: libanim)
