# `v.rmarea` - Removing small areas
`v.rmarea` removes small areas similar to official GRASS Module [`v.clean`](https://grass.osgeo.org/grass-stable/manuals/v.clean.html). 
The main difference is that small areas are merged with a neighboring area 
with identical attributes in the specified `columns`. 
An error map is optionally written which stores the erroneous
geometries.
