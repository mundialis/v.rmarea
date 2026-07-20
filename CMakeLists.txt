cmake_minimum_required(VERSION 3.22)

project(v.rmarea)

include(build_addon)

set(sources
  copy_tab.c
  main.c
  proto.h
  remove_areas.c)

set(doc_files
  v.rmarea.html
  v.rmarea.md)

build_addon(
  ${PROJECT_NAME}
  GRASSLIBS gis vector dbmi
  SOURCES ${sources}
  DOCFILES ${doc_files})
