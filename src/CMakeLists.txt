# One folder and one QML module per feature, C++ and QML side by side. Dependencies point one way:
#   controls     the design system, knows no feature
#   canvas       drawing (Flux), uses libs/shapes # demo
#   sketchbook   the list of sketches (MVVM), uses canvas # demo
#   app          the executable: composition root, logging, crash handling

add_subdirectory(controls)
add_subdirectory(notify)
add_subdirectory(canvas) # demo
add_subdirectory(sketchbook) # demo
add_subdirectory(app)
