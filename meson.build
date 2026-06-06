project('chawa', 'c',
  version: '1.0',
  default_options: [
    'c_std=c11',
    'buildtype=release',
    'optimization=3',
    'warning_level=2',
    'strip=true',
  ]
)

deps = [
  dependency('gtk4'),
  dependency('webkitgtk-6.0'),
  dependency('libnotify'),
]

executable('chawa',
  files(
    'src/main.c',
    'src/window.c',
    'src/notify.c',
  ),
  dependencies: deps,
  install: true
)

install_data('data/chawa.desktop',
  install_dir: join_paths(get_option('datadir'), 'applications')
)

install_data('data/chawa.svg',
  install_dir: join_paths(get_option('datadir'), 'icons', 'hicolor', 'scalable', 'apps')
)