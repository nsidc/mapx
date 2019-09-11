![NSIDC logo](/images/NSIDC_logo_2018_poster-1.png)


# NSIDC Mapx

## Overview

Geographic coordinate transformations are used in many applications at the
National Snow & Ice Data Center, including gridded data display tools,
data processing systems, and online area search interfaces. These
applications require accurate, efficient, and reusable transformation
routines. The common packages GCTP and PROJ were difficult to use in our
modern multi-threaded event driven software architectures. The mapx
library was developed to fill this gap for our internal uses. We are now
offering the library to outside users.

### Features

The main benefit of an object-oriented design for the mapping library is
that each transformation is independent. Often, in the available standard
packages, only one instance of a given projection can be initialized at a
time. In mapx the creation and initialization of a new transformation has
no effect on existing mapx objects. This is an important feature where
concurrent processing is required. Examples include simultaneously
ingesting data from different maps, pixel-by-pixel conversion of one map
to another, or an online map server with multiple projection options.

### Projections:

The following projections are currently implemented:

- Azimuthal_Equal_Area
- Cylindrical_Equal_Area
- Mercator
- Mollweide
- Orthographic
- Sinusoidal
- Cylindrical_Equidistant
- Polar_Stereographic
- Polar_Stereographic_Ellipsoid
- Azimuthal_Equal_Area_Ellipsoid
- Cylindrical_Equal_Area_Ellipsoid
  - Lambert_Conic_Conformal_Ellipsoid
- Interupted_Homolosine_Equal_Area
  - Albers_Conic_Equal_Area
  - Albers_Conic_Equal_Area_Ellipsoid
- Integerized_Sinusoidal
  - Transverse_Mercator
  - Transverse_Mercator_Ellipsoid
  - Universal_Transverse_Mercator
- or anything reasonably similar

And, the library's design makes it easy to add new projections.

Many projections have both spherical and ellipsoidal versions. The
initialization of each transformation allows for the use of a different
datum to describe the spheroid. Even two transformations with the same
projection can have different datums. The datum is not restricted to the
Earth. You could easily use mapx on the moon or Mars, for example.

### Accuracy:

The coordinate transformations used in mapx are (for the most part) taken
from Snyder and tested against the numerical examples he gives in Appendix
A. Further, all transformations are tested against their inverse and are
consistent to within one meter.

### Application Program Interface (API):

This is the C version of the library. There is also a Java version
available at http://nsidc.org/data/tools/

Both versions provide constructors, a destructor, a forward transformation
(from latitude and longitude to map coordinates), an inverse
transformation (map coordinates to latitude and longitude), and a test to
determine if a point is within a map.

In addition the C version has a derived grid object that abstracts a map
projection and a matrix object for the storage and retrieval of gridded
data.

There are also a couple of two dimensional modeling objects--a polynomial
based model called pmodel and a cubic spline based model called smodel.

And, finally, an interface to a coastlines database derived from a
combination of WDBII and World Vector Shoreline.

### Applications:

This package also contains several standalone routines that serve as test
beds and coding examples but also as useful applications in their own
right. Some examples include:

- gridloc - output latitude and longitude for every cell in a grid
- regrid - interpolate data from one grid to another
- resamp - interpolate data from one grid to another (in a slightly different way)
- irregrid - interpolate irregularly sampled data (points) to a grid
- mapenum - enumerate (list) map feature vectors from a cdb file
- mtest - interactive command line map transformations
- gtest - interactive command line grid transformations

### Reference:

Snyder, John P., Map Projections Used by the U.S. Geological Survey, 1982.

## Level of Support

* This repository is not actively supported by NSIDC but we welcome issue submissions and pull requests in order to foster community contribution.

See the [LICENSE](LICENSE) for details on permissions and warranties. Please contact nsidc@nsidc.org for more information.

## Requirements

This package requires:
* C Compiler

## Installation

See the [INSTALL](INSTALL) file for details on installing the library and tools.

## Usage

Describe how to use the MyRepository application/tool, with platform-specific instructions if necessary.

## Troubleshooting

Describe any tips or tricks in case the user runs into problems.

## License

The mapx library is open source software. You can redistribute it and/or
modify it under the terms of the [GNU Lesser General Public License (LGPL)](LICENSE)
as published by the Free Software Foundation.

THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. THE
UNIVERSITY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE. See the GNU LGPL for more details.

See [LICENSE](LICENSE).

## Code of Conduct

See [Code of Conduct](CODE_OF_CONDUCT.md).

## Credit

This software was developed by the National Snow and Ice Data Center with funding from multiple sources.

Copyright (C) 1990-2004 University of Colorado.
