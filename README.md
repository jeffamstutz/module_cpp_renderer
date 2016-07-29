Run the GLUT viewer with...

```./ospGlutViewer [model file] -m cpp -c cpp_perspective --trianglemesh-type cpp_triangles -r cpp_ao_stream```

Where valid renderers are:

- cpp_raycast
- cpp_raycast_stream
- cpp_ao
- cpp_ao_stream

NOTE: Currently you must add the ```RTC_INTERSECT_STREAM``` flag to the model
inside OSPRay for the *_stream renderers to work.
