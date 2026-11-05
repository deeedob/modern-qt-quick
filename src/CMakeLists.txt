# One folder and one QML module per feature, C++ and QML side by side. Dependencies point one way:
#   style        the design system, knows no feature
#   canvas       drawing (Flux), uses libs/shapes
#   sketchbook   the list of sketches (MVVM), uses canvas
#   app          the executable: composition root, logging, crash handling

add_subdirectory(style)
add_subdirectory(canvas)
add_subdirectory(sketchbook)
add_subdirectory(app)
