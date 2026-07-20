# v.rmarea

## DESCRIPTION

*v.rmarea* removes small areas similar to *v.clean*. The main difference is that
small areas are merged with a neighboring area with identical attributes in the
specified *columns*. An error map is optionally written which stores the
erroneous geometries.

### Remove small areas

*tool=rmarea*

The *rmarea* tool attempts to remove areas <= *thresh*. The longest boundary
with an adjacent area with identical attributes is removed. If there is no
adjacent area with identical attributes, the area is left unchanged, even if it
is smaller than *thresh*.

Threshold must always be in square meters, also for latitude-longitude projects
or projects with units other than meters.

## NOTES

The user does **not** have to run *v.build* on the *output* vector, unless the
*-b* flag was used. The *-b* flag affects **only** the *output* vector -
topology is always built for *error* vector.

## EXAMPLES

### Remove areas smaller than 10 m^2

```bash
v.rmarea input=testmap output=cleanmap threshold=10 columns=label
```

## SEE ALSO

* [v.clean](v.clean.html)
* [v.info](v.info.html)
* [v.build](v.build.html)
* [g.gui.vdigit](g.gui.vdigit.html)
* [v.edit](v.edit.html)
* [v.fill.holes](v.fill.holes.html)
* [v.generalize](v.generalize.html)

## AUTHORS

David Gerdes, U.S. Army Construction Engineering Research Laboratory<br>
Radim Blazek, ITC-irst, Trento, Italy<br>
Martin Landa, FBK-irst (formerly ITC-irst), Trento, Italy<br>
Markus Metz, mundialis
