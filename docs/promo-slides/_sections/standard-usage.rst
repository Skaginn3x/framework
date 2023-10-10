Standard usage
==============

Installation
------------

You can install stable version of ``sphinx-revealjs`` from PyPI.

.. code-block:: console

   pip install sphinx-revealjs

Set up
------

Edit ``conf.py`` to use this extension

.. code-block:: python

   extensions = [
       "sphinx_revealjs",
   ]

You can customize behabior. Please see `documentation <https://sphinx-revealjs.readthedocs.io/en/latest/configurations.html>`_.

Write source
------------

Write plain reStructuredText.

.. code-block:: rst

   My Reveal.js presentation
   =========================

   Agenda
   ------

   * Author
   * Feature

   Author: Who am I
   ================

   Own self promotion

   Content
   =======

Build presentation
------------------

This extension has custom builder name ``revealjs``.
If you make docs as Reveal.js presentation, you call ``make revealjs``.

.. code-block:: console

   make revealjs

This presentation is made from `source <https://github.com/attakei/sphinx-revealjs/blob/master/demo/index.rst>`_.
