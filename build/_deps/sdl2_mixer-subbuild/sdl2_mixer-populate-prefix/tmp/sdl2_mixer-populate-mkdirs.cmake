# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-src"
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-build"
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix"
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix/tmp"
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix/src/sdl2_mixer-populate-stamp"
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix/src"
  "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix/src/sdl2_mixer-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix/src/sdl2_mixer-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/cucha/Desktop/Andres/facultad/Taller/stardew_valley_tp/build/_deps/sdl2_mixer-subbuild/sdl2_mixer-populate-prefix/src/sdl2_mixer-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
