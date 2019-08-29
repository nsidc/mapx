# Points, Pixels, Grids, and Cells
A Mapping and Gridding Primer
_by Ken Knowles_

---

## Table of contents

---

## Introduction

Satellite remote sensing has made possible the collection of data over
large areas of the Earth. These data are often stored in "grids."  Grids
are an efficient means of storing data because the location of a value
within the grid is implicit--it is not explicitly stored in the grid. The
location is also constant, which makes it easy to compare data from
different sensors or different time periods.

The purpose of this document is to give a brief introduction to
mapping, gridding, and the associated terminology used in the mapx library
at NSIDC.  Along with comments in the source code, it also describes the
format of the map and grid parameter files.


## Concepts

### Maps

A map projection is a procedure or mathematical formula to transform a
curved surface onto a plane. The curved surface is usually the surface of
the Earth and the plane is what we call a "map." The derivation of a map
transformation requires a mathematical representation of the surface of
the Earth. The surface of constant geopotential referred to as mean sea
level or the geoid is quite complicated and its mathematical form is
correspondingly complex. When defining map projections then, it is common
to approximate the surface of the earth with a simpler surface such as an
ellipsoid or sphere.

The ellipsoid is flattened at the poles, relative to the sphere, by
about 1 part in 300. For maps of large areas (continents or bigger) using
the sphere introduces no significant error. When mapping smaller areas it
is not uncommon to choose an ellipsoid specifically for each area; each
area is mapped on an ellipsoid with a different radius and eccentricity.

Coordinates for points on the surface of the Earth are given in latitude
and longitude. Geodetic latitude is the angle between a vertical ray at the
point and the plane of the equator. Geocentric latitude is the angle between
a ray from the center of the Earth to the point and the plane of the equator.
On a sphere, the geodetic latitude is equivalent to the geocentric latitude.
It's more common to use geodetic latitude. There is only one definition of
longitude--the angle between a ray from the center of the Earth to the point
and the plane of the Greenwich Meridian (an arbitrary reference point). Coordinates
in the map plane (_x_,_y_) are defined in the usual manner with
 _x_ horizontal and increasing to the right and  _y_ vertical and
increasing upward.


### Projections

![Projection](images/mprojex.gif)

Map projections are specified as a set of equations giving _x_ and
_y_ in terms of _latitude_ and _longitude_. The manner in
which we wrap the paper around the globe determines the form of the
equations.  Cylindrical projections wrap the paper around in a tube,
conical projections use a cone, and azimuthal projections use a flat
surface. Several parameters are used in these equations to specify exactly
where the surface intersects the globe, the scale or size of the globe,
and the translation and rotation of the map on the plane surface.

There is no one best map projection. Each projection has different
properties and different "best" uses. Two of the most important
characteristics of maps are whether they are conformal or equal-area. No
map projection is both, and some are neither. If a map is conformal then
angles within a small area are reproduced accurately. This means that
shapes are preserved.  A small circle on the globe will look like a small
circle on the map. In formal terms, at any point on the map the scale
_h_ along a meridian of longitude is equal to the scale _k_
along a parallel of latitude.  If a map is equal-area then a small circle
placed anywhere on the map will always cover the same amount of area on
the globe and the product of _h_ and _k_ at any point is
one.

For maps that are not equal-area, _hk_ - 1 gives a measure of the
areal distortion. For maps that are not conformal, _k_/_h_ gives
the aspect ratio, which is a measure of shape distortion. For example, on
the Polar Stereographic map true at 70N (a conformal map) the areal
distortion varies from -6% at the pole to +29% at 45N and goes up to +276%
at the equator while, by definition, the aspect ratio remains 1:1
everywhere.  The Azimuthal Equal-Area map over the pole has, of course, no
areal distortion, while the aspect ratio varies from 1:1 at the pole to
1.17:1 at 45N and goes up to 2:1 at the equator.

Tissot's indicatrix combines the two previous measures. Tissot's
indicatrix is a small ellipse which shows both the scale distortion and
the maximum angular deformation at a particular point on the map. See
[Maling](#reference) or [Snyder](#references) for a complete discussion.


### Grids

A grid is a rectangular array of points. Grids record regularly spaced
samples over an area. When sampling over the surface of the earth a grid
is determined by a map projection, a sampling interval, an origin, and the
number of rows and columns. So called "lat/lon" or "equal angle" grids are
equivalent to grids sampled on a Cylindrical Equidistant map projection.

A grid coordinate system is defined in the map plane with axes parallel
to the rows and columns of the grid and units equal to the sampling
interval.  The grid sample locations are at the whole integer grid
coordinate points.  To conform to mathematical array conventions, the grid
coordinates (_r_,_s_) start at (0,0) in the upper left corner
with _r_ increasing to the right, and _s_ increasing downward
(this also conforms to digital image processing conventions). A grid cell
(_i_,_j_) is defined as the area between grid coordinates
_i_-.5 and _i_+.5, and _j_-.5 and _j_+.5.  To conform
to rounding conventions, the lower bound is included in the cell while the
upper bound is not (round up at .5).  Note that the _r_ coordinate
corresponds to the grid column number _j_, and the _s_
coordinate corresponds to the grid row number _i_.

Think of the grid as a map drawn on graph paper. To find a grid cell
location, count graph lines over from the left and down from the top, then
read the latitude and longitude off the map.

!(images/coordef.gif)


### Binning

The process of sampling data on a grid is sometimes referred to as
binning, especially when the sampling method is to average all data that
falls into the grid cell. Other methods used to sample data are: take the
closest value to the grid point (nearest neighbor) or interpolate between
surrounding grid points.  Also, binning doesn't need to be the average of
multiple data points; it could be the minimum, the maximum, the median, or
the latest value in the grid cell. The characteristics of the data should
determine the sampling method.


### Pictures

Pixels are analogous to grid cells. To display gridded data as a
picture, define a transformation from grid coordinates to pixel
coordinates and then sample the gridded data at the whole integer pixel
coordinate points.  The most common technique is to map grid cells onto
pixels one for one. More sophisticated techniques, often referred to as
"resampling", allow for scrolling, zooming, and rotating.


## Parameters

Many programs we use at NSIDC depend on the "mapx" module to handle
 map  projections. These include: GISMO, PSQ, regrid, gridomatic, and the
EASE-Grid  processing software. Most of these tools provide access to predefined
maps,  but, it is relatively easy to define your own map with a map projection
 parameters  (.mpp) file.


### Map Projection Parameters (.mpp) File Format

Each parameter is described by a "keyword: value" pair on a single
line.  A colon must delimit the end of a keyword without any intervening
spaces, tabs, or newlines. Keywords are not case sensitive. Parameters can
be followed by comments, which begin with a semi-colon (;) or a pound sign
(#): all text following a semi-colon or a pound sign on a particular line
is ignored.  Parameters can appear in any order. The default value is used
when the corresponding keyword is not found. Unrecognized keywords are
silently ignored. This means that if you misspell a keyword you'll get the
default value without warning.  The following parameters define the map
projection for all projections except [Integerized Sinusoidal](#isin)
and [Universal Transverse Mercator (UTM)](#utm).

<table border="1" width="100%" nosave="">
       <tbody>
          <tr>
       <th>Keyword</th>
        <th>Format</th>
        <th>Default</th>
        <th>Description</th>
       </tr>
        <tr>
       <td>Map Projection</td>
        <td>string</td>
        <td>required field</td>
        <td>projection name (see [list ](#projection_names)below)</td>
       </tr>
        <tr nosave="">
       <td>Map Reference Latitude</td>
        <td>decimal degrees</td>
        <td>required field</td>
        <td nosave="">reference latitude for map projection</td>
       </tr>
        <tr>
       <td>Map Reference Longitude</td>
        <td>decimal degrees</td>
        <td>required field</td>
        <td>reference longitude for map projection</td>
       </tr>
        <tr>
       <td>Map Second Reference Latitude</td>
        <td>decimal degrees</td>
        <td>none</td>
        <td>used by some projections (see [explanation](#second_latitude) below)</td>
       </tr>
        <tr>
       <td>Map Rotation</td>
        <td>decimal degrees</td>
        <td>0.0</td>
        <td>rotation counter-clockwise</td>
       </tr>
        <tr>
       <td>Map Scale</td>
        <td>float</td>
        <td>1.0</td>
        <td>map scale factor (radius units per map unit)</td>
       </tr>
        <tr>
       <td>Map Origin Latitude</td>
        <td>decimal degrees</td>
        <td>reference latitude</td>
        <td>translated map origin latitude</td>
       </tr>
        <tr>
       <td>Map Origin Longitude</td>
        <td>decimal degrees</td>
        <td>reference longitude</td>
        <td>translated map origin longitude</td>
       </tr>
        <tr>
       <td>Map Origin X</td>
        <td>float</td>
        <td>x coordinate of origin latitude-longitude transformed to map
coordinates</td>
        <td>translated map x origin in map units</td>
       </tr>
        <tr>
       <td>Map Origin Y</td>
        <td>float</td>
        <td>y coordinate of origin latitude-longitude transformed to map
coordinates</td>
        <td>translated map y origin in map units</td>
       </tr>
        <tr>
       <td>Map False Easting</td>
        <td>float</td>
        <td>0.0</td>
        <td>offset map x origin in map units</td>
       </tr>
        <tr>
       <td>Map False Northing</td>
        <td>float</td>
        <td>0.0</td>
        <td>offset map y origin in map units</td>
       </tr>
        <tr>
       <td>Map Eccentricity</td>
        <td>float</td>
        <td>Clark 1866 = 0.082271673</td>
        <td>eccentricity of ellipsoid</td>
       </tr>
        <tr>
       <td>Map Eccentricity Squared</td>
        <td>float</td>
        <td>Clark 1866 = 0.00676862817822</td>
        <td>square of eccentricity of ellipsoid</td>
       </tr>
        <tr>
       <td>Map Equatorial Radius</td>
        <td>float</td>
        <td>Clark 1866 ellipsoid = 6378.2064 km or equivalent authallic sphere
  = 6371.228 km</td>
        <td>equatorial radius of ellipsoid or radius of sphere</td>
       </tr>
        <tr>
       <td>Map Polar Radius</td>
        <td>float</td>
        <td>Clark 1866 ellipsoid = 6356.5838 km</td>
        <td>polar radius of ellipsoid</td>
       </tr>
        <tr>
       <td>Map Center Scale</td>
        <td>float</td>
        <td>1.0</td>
        <td>scale factor at central meridian <br>
      for Transverse Mercator</td>
       </tr>
         <tr>
           <td valign="top">Map Maximum Error<br>
           </td>
           <td valign="top">float<br>
           </td>
           <td valign="top">0.0<br>
           </td>
           <td valign="top">maximum allowed error in map units (0 disables
 map  error checking)<br>
           </td>
         </tr>

  </tbody>      
</table>

The following parameters are used only by programs that actually draw maps.

<table border="1" width="100%" nosave="">
       <tbody>
          <tr>
       <th>Keyword</th>
        <th>Format</th>
        <th>Default</th>
        <th>Description</th>
       </tr>
        <tr>
       <td>Map Southern Bound</td>
        <td>decimal degrees</td>
        <td>90.00S</td>
        <td>bottom of map, starting point for latitude graticule</td>
       </tr>
        <tr>
       <td>Map Northern Bound</td>
        <td>decimal degrees</td>
        <td>90.00N</td>
        <td>top of map</td>
       </tr>
        <tr>
       <td>Map Western Bound</td>
        <td>decimal degrees</td>
        <td>180.00W</td>
        <td>left side of map, starting point for longitude graticule</td>
       </tr>
        <tr>
       <td>Map Eastern Bound</td>
        <td>decimal degrees</td>
        <td>180.00E</td>
        <td>right side of map</td>
       </tr>
        <tr>
       <td>Map Graticule Latitude Interval</td>
        <td>decimal degrees</td>
        <td>30.</td>
        <td>graticule spacing</td>
       </tr>
        <tr>
       <td>Map Graticule Longitude Interval</td>
        <td>decimal degrees</td>
        <td>30.</td>
        <td>graticule spacing</td>
       </tr>
        <tr>
       <td>Map Graticule Label Latitude</td>
        <td>decimal degrees</td>
        <td>0.00N</td>
        <td>where to label meridians</td>
       </tr>
        <tr>
       <td>Map Graticule Label Longitude</td>
        <td>decimal degrees</td>
        <td>0.00E</td>
        <td>where to label parallels</td>
       </tr>
        <tr>
       <td>Map CIL Detail Level</td>
        <td>integer</td>
        <td>1</td>
        <td>level of detail for coastlines, islands, and lakes</td>
       </tr>
        <tr>
       <td>Map BDY Detail Level</td>
        <td>integer</td>
        <td>0</td>
        <td>level of detail for political boundaries</td>
       </tr>
        <tr>
       <td>Map RIV Detail Level</td>
        <td>integer</td>
        <td>0</td>
        <td>level of detail for rivers</td>
       </tr>

  </tbody>      
</table>

note: For Integerized Sinusoidal, set scale to size of each row.


### Map Projection Names

- Albers Conic Equal-Area
- Azimuthal Equal-Area
- Azimuthal Equal-Area (ellipsoid)
- Cylindrical Equal-Area
- Cylindrical Equal-Area (ellipsoid)
- Cylindrical Equidistant
- [Integerized Sinusoidal](#asin)
- Interrupted Homolosine Equal-Area
- Lambert Conic Conformal (ellipsoid)
- Mercator
- Mollweide
- Orthographic
- Polar Stereographic
- Polar Stereographic (ellipsoid)
- Sinusoidal
- Transverse Mercator
- Transverse Mercator (ellipsoid)
- [Universal Transverse Mercator](#utm)

Keywords must be entered verbatim. Capitalization doesn't matter but
white space does. Unrecognized (or mispelled) keywords are silently
ignored.  Map names are not case sensitive, white space and dashes are
ignored, and most word permutations are acceptable. For example,
"Equal-Area Cylindrical" is equivalent to "CYLINDRICAL EQUALAREA".

The reference latitude and longitude specify the original location and
orientation of the map projection. The origin of the rectangular map
coordinate system can be rotated and translated with the Rotation and
Origin keywords. Note that a positive rotation value will rotate the
coordinate system _counter-clockwise_ around the translated map
origin. This will have the effect of rotating the resulting grid of pixel
values _clockwise_ with respect to the grid that would be obtained if
the rotation value were 0.

<a name="second_latitude"></a>For the Cylindrical Equal-Area, and Polar
Stereographic projections, the second reference latitude specifies the
latitude of "true" scale. Conceptually, this is where the projection plane
intersects the surface. For the Albers Conic Equal-Area and Lambert Conic
Conformal projections, the conic plane is secant to the surface between
the reference latitude and the second reference latitude.

The equatorial radius and eccentricity specify the surface to be mapped
(usually the Earth's surface). Alternatively, the square of the
eccentricity rather than the eccentricity may be specified; or both the
equatorial radius and the polar radius may be specified rather than either
the eccentricity or the square of the eccentricity; or the polar radius
and either the eccentricity or the square of the eccentricity map be
specified rather than the equatorial radius. The scale and the equatorial
radius determine the map units. For example if the radius is in kilometers
(as is the case with the default) and the scale is 1.0, then the map units
will be kilometers. For spherical projections, the eccentricity should not
be specified, and the default equatorial radius is 6371.228, i.e. the
authalic sphere equivalent to the Clark ellipsoid in km. For elliptical
projections, the default eccentricity is 0.082271673 and the default
equatorial radius is 6378.2064, i.e. the Clark 1866 ellipsoid in km. For
the WGS-84 ellipsoid in meters, use 0.081819190843 for the eccentricity
and 6378137.0 for the equatorial radius. PostScript units are inches, so
for PostScript output you would specify the scale in kilometers per inch.
For a grid you can define the scale in terms of kilometers per grid cell.
The gridding module "grids" also has parameters which specify grid cells
per map unit or, alternatively, map units per grid cell. This allows the
same .mpp file to be used for multiple nested grids.

Map Origin Latitude and Longitude specify the translated map origin. By
default the map origin is the same as the reference latitude and
longitude.  Alternatively, the translated map origin can be specified by
Map Origin X and Y, which take precedence over any specification of the
Map Origin Latitude or Longitude. Both Map Origin X and Y must be
specified if either is specified.  If Map Rotation is specified, then the
rotation is centered on the translated map origin. Furthermore, the
specification of Grid Map Column and Row values in the
[Grid Parameter Definition File](#gpd) will locate the translated map origin at the
specified column and row location within the grid.

False easting and false northing offsets are applied to the x and y map
coordinates, respectively, with respect to the original map origin
specified by the reference latitude and longitude. Note that Map Origin X
and Y specify values _after_ false easting and false northing offsets
have been applied.  

The center scale value is used only by the Transverse Mercator
projection (and for the [UTM](#utm) projection as well). It
specifies the scale factor along the central meridian.

When the maximum error value is non-zero, then error checking, is
enabled.  For a forward transformation (lat-lon to x-y), the computed x-y
pair is inverse transformed back into a second lat-lon pair. For an
inverse transformation (x-y to lat-lon), the computed lat-lon pair is
forward transformed into a second x-y pair. In both cases, the
distance in map units between the first and the second pairs is
computed. If it is greater than the maximum error value, then both values
of the returned pair (x-y for forward and lat-lon for inverse) are set to
not-a-number (nan). If the maximum error value is 0 (the default), then no
such error checking is performed. Note that setting maximum error to a
non-zero value will slow down forward and inverse transformations by about
a factor of 2.      

The remaining parameters are used by programs that actually draw maps
(e.g. psmap, mapenum). The southern, northern, western, and eastern bounds
of the map are the starting point for the graticule and are used to speed
up the search in the map outline database. They cannot be counted on to
clip the map accurately. All longitudes should be in the range -180 to
+360. West to east should not span more than 360 degrees. West specifies
the left side of the map and east the right, not necessarily the minimum
and maximum longitudes.  

The latitude and longitude graticule intervals specify the spacing
between graticule lines. The graticule is the overlay of latitude and
longitude lines on the map. By default, a parallel is drawn every 30
degrees starting at the bottom of the map (90.00S) and a meridian is drawn
every 30 degrees starting at the left side of the map (180.00W). Label
latitude and longitude specify the parallel and meridian along which to
draw the graticule line labels.  

The last three numbers are used by the database search routine to
specify the level of detail for map lines. The level refers to the number
and size of features which will be selected, as opposed to resolution of
each feature. The higher the number, the more detail will be included.
For example, level 1 will include only the largest features, such as whole
continents, while level 2 would also include some smaller features like
large lakes.  In either case the continental outline will appear with the
same amount of detail (the same jaggedness).


### Map Projection Parameters (.mpp) File Format for Integerized Sinusoidal

The following parameters define the Integerized Sinusoidal (ISin)
projection.

<table border="1" width="100%" nosave="">
       <tbody>
          <tr>
       <th>Keyword</th>
        <th>Format</th>
        <th>Default</th>
        <th>Description</th>
       </tr>
        <tr>
       <td>Map Projection</td>
        <td>string</td>
        <td>required</td>
        <td>Integerized Sinusoidal</td>
       </tr>
        <tr>
       <td>Map Reference Longitude</td>
        <td>decimal degrees</td>
        <td>0.0</td>
        <td>Longitude of the central meridian</td>
       </tr>
        <tr>
       <td>Map Rotation</td>
        <td>decimal degrees</td>
        <td>0.0</td>
        <td>rotation counter-clockwise</td>
       </tr>
        <tr>
       <td>Map Scale</td>
        <td>float</td>
        <td>1.0</td>
        <td>map scale factor (meters per map unit)</td>
       </tr>
        <tr>
       <td>Map ISin NZone</td>
        <td>int</td>
        <td>86400</td>
        <td>Number of equally spaced latitudinal zones</td>
       </tr>
        <tr>
       <td>Map ISin Justify</td>
        <td>int</td>
        <td>1.0=left</td>
        <td>justify flag for ISin map</td>
       </tr>
        <tr>
       <td>Map Origin Latitude</td>
        <td>decimal degrees</td>
        <td>0.0 (= effective reference latitude)</td>
        <td>translated map origin latitude</td>
       </tr>
        <tr>
       <td>Map Origin Longitude</td>
        <td>decimal degrees</td>
        <td>effective reference longitude</td>
        <td>translated map origin longitude</td>
       </tr>
        <tr>
       <td>Map Origin X</td>
        <td>float</td>
        <td>x coordinate of origin latitude-longitude transformed to meters</td>
        <td>translated map x origin in meters</td>
       </tr>
        <tr>
       <td>Map Origin Y</td>
        <td>float</td>
        <td>y coordinate of origin latitude-longitude transformed to meters</td>
        <td>translated map y origin in meters</td>
       </tr>
        <tr>
       <td>Map False Easting</td>
        <td>float</td>
        <td>0.0</td>
        <td>offset map x origin in meters</td>
       </tr>
        <tr>
       <td>Map False Northing</td>
        <td>float</td>
        <td>0.0</td>
        <td>offset map y origin in meters</td>
       </tr>
        <tr>
       <td>Map Equatorial Radius</td>
        <td>float</td>
        <td>6371007.181</td>
        <td>radius of sphere in meters</td>
       </tr>

  </tbody>      
</table>

For the ISin projection all map projection parameters have the same definitions
as described [above](#mpp) with the following exceptions.

For the ISin projection, the Map Reference Latitude parameter value is
forced to 0.0; any Map Reference Latitude value supplied by the user is
ignored.  The Map Reference Longitude parameter is not required for the
ISin projection; the default value is 0.0.

The Map ISin NZone parameter is used only by the ISin projection; it
specifies the number of equally spaced latitudinal zones (i.e. "rows"),
and must be 2 or larger and even. Note that this value is a characteristic
of the projection only and is independent of the actual number of rows in
any grid that utilizes the ISin projection. Also note that currently only
the default value of 86400 is used by all ECS MODIS products (including
all 1000 m, 500 m, and 250 m products) based on the ISin projection.

The Map ISin Justify parameter is used only by the ISin projection; it
is used to indicate what to do with zones with an odd number of columns.
If it has a value of 0 or 1, it indicates the extra column is on the right
(zero) or left (one) of the projection y-axis. If the flag is set to 2,
the number of columns is calculated so there is always an even number of
columns in each zone. Note that currently only the default value of 1 is
used by all ECS MODIS products based on the ISin projection.

By default, setting Map Projection to Integerized Sinusoidal specifies
a spherical projection using a radius of 6371007.181 meters. Note that
this value is currently used for all ECS MODIS products based on the ISin
projection.  Only a spherical projection is supported for the ISin
projection.


### Map Projection Parameters (.mpp) File Format for Universal Transverse Mercator

<table border="1" width="100%" nosave="">
       <tbody>
          <tr>
       <th>Keyword</th>
        <th>Format</th>
        <th>Default</th>
        <th>Description</th>
       </tr>
        <tr>
       <td>Map Projection</td>
        <td>string</td>
        <td>required</td>
        <td>Universal Transverse Mercator</td>
       </tr>
        <tr>
       <td>Map UTM Zone</td>
        <td>integer</td>
        <td>0</td>
        <td>specifies value for UTM zone</td>
       </tr>
        <tr nosave="">
       <td>Map Reference Latitude</td>
        <td>decimal degrees</td>
        <td>determined by UTM zone</td>
        <td nosave="">determines sign of UTM zone</td>
       </tr>
        <tr>
       <td>Map Reference Longitude</td>
        <td>decimal degrees</td>
        <td>determined by UTM zone</td>
        <td>determines absolute value of UTM zone</td>
       </tr>
        <tr>
       <td>Map Rotation</td>
        <td>decimal degrees</td>
        <td>0.0</td>
        <td>rotation counter-clockwise</td>
       </tr>
        <tr>
       <td>Map Scale</td>
        <td>float</td>
        <td>1.0</td>
        <td>map scale factor (meters per map unit)</td>
       </tr>
        <tr>
       <td>Map Origin Latitude</td>
        <td>decimal degrees</td>
        <td>0 (= effective reference latitude)</td>
        <td>translated map origin latitude</td>
       </tr>
        <tr>
       <td>Map Origin Longitude</td>
        <td>decimal degrees</td>
        <td>effective reference longitude</td>
        <td>translated map origin longitude</td>
       </tr>
        <tr>
       <td>Map Origin X</td>
        <td>float</td>
        <td>x coordinate of origin latitude-longitude transformed to meters</td>
        <td>translated map x origin in meters</td>
       </tr>
        <tr>
       <td>Map Origin Y</td>
        <td>float</td>
        <td>y coordinate of origin latitude-longitude transformed to meters</td>
        <td>translated map y origin in meters</td>
       </tr>
        <tr>
       <td>Map False Easting</td>
        <td>float</td>
        <td>500000</td>
        <td>offset map x origin in meters</td>
       </tr>
        <tr>
       <td>Map False Northing</td>
        <td>float</td>
        <td>0 for northern hemisphere; <br>
      10000000 for southern hemisphere</td>
        <td>offset map y origin in meters</td>
       </tr>
        <tr>
       <td>Map Eccentricity</td>
        <td>float</td>
        <td>WGS-84 = 0.081819190843</td>
        <td>eccentricity of ellipsoid</td>
       </tr>
        <tr>
       <td>Map Eccentricity Squared</td>
        <td>float</td>
        <td>WGS-84 = 0.0066943799902</td>
        <td>square of eccentricity of ellipsoid</td>
       </tr>
        <tr>
       <td>Map Equatorial Radius</td>
        <td>float</td>
        <td>WGS-84 =&nbsp; 6378137.0 meters</td>
        <td>equatorial radius of ellipsoid (or radius of sphere if eccentricity
   = 0) in meters</td>
       </tr>
        <tr>
       <td>Map Polar Radius</td>
        <td>float</td>
        <td>WGS-84 = 6356752.314245 meters</td>
        <td>polar radius of ellipsoid in meters</td>
       </tr>
        <tr>
       <td>Map Center Scale</td>
        <td>float</td>
        <td>0.9996</td>
        <td>scale factor at central meridian</td>
       </tr>
         <tr>
           <td valign="top">Map Maximum Error<br>
           </td>
           <td valign="top">float<br>
           </td>
           <td valign="top">100.0 meters<br>
           </td>
           <td valign="top">maximum allowed error in meters (0 disables map
 error  checking)<br>
           </td>
         </tr>

  </tbody>      
</table>

For the UTM projection all map projection parameters have the same definitions
as described [above](#mpp) with the following exceptions.

The UTM zone value is used only by the UTM projection. Zones are in the
range 1 to 60 for the northern hemisphere, and -1 to -60 for the southern
hemisphere. If the UTM zone is 0, then the hemisphere and absolute value
of the zone are determined by the reference latitude and longitude,
respectively.  If the UTM zone is not 0, then the reference latitude and
longitude are not required and are ignored if present. The effective
reference latitude value for every UTM zone is 0 (the equator); the
effective reference longitude value is the central meridian of the zone
starting at 177W for zones 1 and -1, 171W for zones 2 and -2, and so on
until 177E for zones 60 and -60. Note that _either_ Map UTM Zone must
be non-zero _or both_ Map Reference Latitude _and_ Map Reference
Longitude _must_ be specified.

The default Map Origin Latitude and Longitude values are determined by
the effective reference latitude value, which is always 0, and the
effective reference longitude value, which is determined from the UTM
zone, either as supplied in the Map UTM Zone value or as derived from the
Map Reference Longitude value.

By default, setting Map Projection to Universal Transverse Mercator
specifies an ellipsoidal projection using the WGS-84 ellipsoid. To specify
a spherical UTM projection, set Map Eccentricity to 0, and set Map
Equatorial Radius to the radius of the sphere in meters.

For the UTM projection, the default false easting is 500000 (five
hundred thousand) meters. For northern hemisphere UTM zones, the default
false northing is 0 meters, and for southern hemisphere UTM zones, the
default false northing is 10000000 (10 million) meters. Note that these
values assume that Map Equatorial Radius is in meters and that Map Scale
is 1.0. Note also that Map Origin X and Y specify values _after_
false easting and false northing offsets have been applied. For example,
for the UTM projection for a southern hemisphere UTM zone, specifying a
Map Origin X value of 175000 will set the translated map origin to 325000
(500000 - 175000) meters west of the central meridian for the UTM zone,
and specifying a Map Origin Y value of 5325000 will set the translated map
origin to 4675000 (10000000 - 5325000) meters south of the equator.

The center scale value specifies the scale factor along the central meridian;
the default for the UTM projection is 0.9996.

When the maximum error value is non-zero, then error checking, is
enabled; the default for the UTM projection is 100.0 meters. For a forward
transformation (lat-lon to x-y), the computed x-y pair is inverse
transformed back into a second lat-lon pair. For an inverse transformation
(x-y to lat-lon), the computed lat-lon pair is forward transformed
into a second x-y pair.  In both cases, the distance in map units between
the first and the second pairs is computed. If it is greater than the
maximum error value, then both values of the returned pair (x-y for
forward and lat-lon for inverse) are set to not-a-number (nan). Note that
when error checking is enabled, it will slow down forward and inverse
transformations by about a factor of 2. To disable error checking, set the
maximum error value to 0. Error checking is turned on by default for the
UTM projection because the equations used for the projection can produce
errors of more than 100 meters for points that are at a latitude of +- 45
degrees and more than about 17.5 degrees of longitude beyond the
center of the specified 6 degree-wide UTM Zone; and in some cases, forward
transformations of a lat-lon pair that is located far outside of the
specified UTM Zone can produce an x-y pair that resides within the
specified UTM Zone.


### Grid Parameter Definition (.gpd) File Format

Grids are defined in grid parameter definition (.gpd) files.

<table border="1" width="100%" nosave="">
       <tbody>
          <tr>
       <th>Keyword</th>
        <th>Format</th>
        <th>Default</th>
        <th>Description</th>
       </tr>
        <tr>
       <td>Grid Width</td>
        <td>integer</td>
        <td>required field</td>
        <td>number of columns in grid</td>
       </tr>
        <tr>
       <td>Grid Height</td>
        <td>integer</td>
        <td>required field</td>
        <td>number of rows in grid</td>
       </tr>
        <tr>
       <td>Grid Map Origin Column</td>
        <td>float</td>
        <td>0.0</td>
        <td>column coordinate of map origin</td>
       </tr>
        <tr>
       <td>Grid Map Origin Row</td>
        <td>float</td>
        <td>0.0</td>
        <td>row coordinate of map origin</td>
       </tr>
        <tr>
       <td>Grid Cells per Map Unit</td>
        <td>float</td>
        <td>1.0</td>
        <td>sets both column and row scale</td>
       </tr>
        <tr>
       <td>Grid Columns per Map Unit</td>
        <td>float</td>
        <td>1.0</td>
        <td>sets column scale if not already set</td>
       </tr>
        <tr>
       <td>Grid Rows per Map Unit</td>
        <td>float</td>
        <td>1.0</td>
        <td>sets row scale if not already set</td>
       </tr>
        <tr>
       <td>Grid Map Units per Cell</td>
        <td>float</td>
        <td>1.0</td>
        <td>alternative method to set both column and row scale</td>
       </tr>
        <tr>
       <td>Grid Map Units per Column</td>
        <td>float</td>
        <td>1.0</td>
        <td>sets column scale if not already set</td>
       </tr>
        <tr>
       <td>Grid Map Units per Row</td>
        <td>float</td>
        <td>1.0</td>
        <td>sets row scale if not already set</td>
       </tr>
        <tr>
       <td>Grid MPP File</td>
        <td>string</td>
        <td>none</td>
        <td>name of .mpp file, map projection parameters can also be specified
  in the same file with the grid definition parameters</td>
       </tr>

  </tbody>      
</table>


### Limitations

The mapx library is very flexible but it does impose certain
restrictions on the kinds of grids that can be defined. All grids must be
rectangular arrays. Rectangular arrays are easy to use with existing
scientific visualization tools, like IDL, but they may be inefficient in
terms of storage space for non-rectangular data sets. If some flag value
(for example zero) is stored in the unused cells, then this wasted space
can be compacted by any good data compression scheme. This same invalid
data flag can be used within the grid as well, leading to further storage
savings. Another restriction is that each grid must be based on a map
projection defined in mapx. Many of the most useful projections have
already been implemented and fortunately it is easy, given a mathematical
formulation of the forward and inverse transformations, to add a new
projection to mapx (see source code for instructions).  The mathematical
formulations of most common map projections can be found in the references
below.


### Reference

**American Society of Photogrammetry. 1983.** _Manual of Remote
Sensing._ Robert N. Colwell, editor. Second edition. Falls Church, VA.

**Gonzalez, Rafael C. and Paul Wintz. 1987.**_Digital Image
Processing._Second edition. Reading, MA: Addison-Wesley.

**Maling, D. H. 1992.** _Coordinate Systems and Map
Projections._ Second edition. Elmsford, N.Y.: Pergamon Press.

**Snyder, John P. 1987.**_Map Projections, A Working
Manual_. U.  S. Geological Survey Professional Paper 1395. Department
of the Interior.  Washington, D. C.


---

Mapx is maintained at the National Snow & Ice Data Center in Boulder, CO.

Please direct questions or comments to [NSIDC User Services](mailto:nsidc@nsidc.org)
