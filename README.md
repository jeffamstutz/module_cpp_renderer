Run the GLUT viewer with...

```./ospGlutViewer [model file] -m cpp -c cpp_perspective --trianglemesh-type cpp_triangles -r cpp_ao_stream```

Where valid renderers are:

- cpp_raycast
- cpp_raycast_stream
- cpp_ao
- cpp_ao_stream

NOTE: Currently you must use the latest 'devel' branch of OSPRay _and_ enable
the ```OSPRAY_USE_EXTERNAL_EMBREE``` CMake option with Embree v2.9.0 or higher
in order to enable the ```RTC_INTERSECT_STREAM``` flag for the cpp_*_stream
renderers.
