project = 'TFC Framework'
copyright = '2023, Skaginn3X'
author = 'Skaginn3X'

extensions = [ "breathe", "myst_parser" ]
breathe_default_project = "tfc"

#templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', "CMakeLists.txt"]
include_patterns = [
    "**"
]

source_suffix = {
    '.rst': 'restructuredtext',
    '.txt': 'restructuredtext',
    '.md': 'markdown',
}

html_static_path = ['_static']

html_css_files = [
    'css/rtd_dark.css',
]

html_theme = 'sphinx_rtd_theme'
#html_static_path = ['_static']
