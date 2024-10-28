# Map projections utilities library

There are many map projections. Often, we have data in one projection but we need them in another.
This library offers a way how to convert data from one projection to another. 
The system works directly with projection equations that are mapped to intermediate coordinate system
that is mapped to a classic raster at the end.

## Simple usage

```c++
using namespace Projections;

unsigned w = 600;
unsigned h = 600;

std::vector<uint8_t> inputRawData;
std::vector<uint8_t> fileData;

//sample data loaded with lodepng library from png
lodepng::load_file(fileData, "input_file.png");
lodepng::decode(inputRawData, w, h, fileData, LodePNGColorType::LCT_GREY);


// Specify coordinates of corners for AABB
Coordinate bbMin, bbMax;
bbMin.lat = 21.140547_deg; bbMin.lon = -134.09548_deg;
bbMax.lat = 52.6132742_deg; bbMax.lon = -60.9365_deg;

//create input projection and set its visible frame
//sample of Lambert-Conic with input params
LambertConic * inputImage = new LambertConic(38.5_deg, -97.5_deg, 38.5_deg);

//set projection frame
//STEP_TYPE determines, where coordinate is located within the pixel
//it can be in its corner or center
inputImage->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);

//create output projection that uses the same frame as input
Equirectangular * outputImage = new Equirectangular();
outputImage->SetFrame(inputImage, false); 


//compute mapping from input -> output projection   
Reprojection reprojection = Reprojection<int>::CreateReprojection(inputImage, outputImage);

//initialize helper debug renderer
ProjectionRenderer pd(outputImage);
pd.Clear();
pd.DrawImage(&inputRawData[0], ProjectionRenderer::RenderImageType::GRAY, reprojection);
pd.DrawBorders();
pd.SaveToFile("output.png");
```

## Library description

Entire library uses `typedef MyRealType` for floating precision. 
The typedef is located in _GeoCoordinate.h_, because this class is included everywhere.

### GPS Projections

Folder _Projections_ contains main projection classes. 
Each class represents a single projection and is extended from `ProjectionInfo` using CRTP mechanism.

```c++
class Equirectangular : public ProjectionInfo<Equirectangular>
```

Is also must be made as a friend of its parent because parent can access its protected / private members.

```c++
friend class ProjectionInfo<Equirectangular>;
```

There are two static constants that determine some projection info

* `INDEPENDENT_LAT_LON` - determine, if lat / lon can be computed separatly. To compute one, we don't need the other.
* `ORTHOGONAL_LAT_LON` - determine if projection has lat / lon orthogonal to each other (for example Mercator)

The class must have implemented two methods for projections. 
They contain direct equations to convert from lat / lon to pseudo-pixels and its inverse formulas.

```c++
//ProjectedValue contains position of pseudo-pixel
ProjectedValue ProjectInternal(const Coordinate & c) const;
```

```c++
//x and y contains position of pseudo-pixel
ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const;
```

Every new projection should also be added to `enum Projections::PROJECTION`.

### Projection

Each projection is extended from interface `IProjectionInfo`. 
The intreface contains the type of projection in constant `curProjection`. 
It may be used to typecast interface to a correct type of projection.

User may specifiy macro `USE_VIRTUAL_INTERFACE`. With this macro, some methods are added to interface.

* `void SetFrame(const ProjectionFrame & frame)` - set existing frame
* `void SetFrame(const Coordinate & botLeft, const Coordinate & topRight, MyRealType w, MyRealType h, STEP_TYPE stepType, bool keepAR = true)` - set frame from minimal and maximal coordinate. They dont have to be AABB
* `void SetFrameFromAABB(const Coordinate & min, const Coordinate & max, MyRealType w, MyRealType h, STEP_TYPE stepType, bool keepAR = true)` - set frame fom AABB
* `const ProjectionFrame & GetFrame() const` - get vitual frame used for projection 

* `Coordinate GetTopLeftCorner() const` - get position of top left corner (eg. coordinate of pixel at [0, 0])
* `Coordinate GetDeltaStep() const` - get step between two neighbors coordinates (between two pixels)
* `void LineBresenham(Pixel<int> start, Pixel<int> end, std::function<void(int x, int y)> callback) const` - helper method that draws line from start to end pixel and for each pixel it "visits" callback is called
* `void ComputeAABB(Coordinate & min, Coordinate & max) const` - compute AABB around the frame and get frame min and max GPS coordinate

These methods are presented in the class `ProjectionInfo` and by extenstion they can be used in every GPS projection class.
With the macro defined, we can create an instance of `IProjectionInfo` and access these methods. 
Without it, `IProjectionInfo` has to be cast to a exact type.

The core of projection calculation is class `ProjectionInfo` (extended from interface `IProjectionInfo`).
The two main methods are

* `Pixel<PixelType> Project(const Coordinate & c) const` - project GPS to a pixel
* `Coordinate ProjectInverse(const Pixel<PixelType> & p) const` - project pixel to GPS

These methods project pixel to a GPS coordinate within the frame and vice versa.

Private method `std::tuple<double, double, double, double> GetFrameBotLeftTopRight(const Coordinate & botLeft, const Coordinate & topRight)` 
is used to determine corners of the projection in pseudo-pixels during the frame creation.
This method can be overriden in GPS projection class based on projection (for example see GOES). 
However, most of the time, default implementation is used.

#### Frame specification

Frame for projection is specifid in `SetFrame` method.
Set current data active frame based on image bottom left and top right coordinate
This assumes that data are plotted in 2D image and image has corners
These corners do not have to correspond to AABB of coordinates
(For example: Lambert - image is square but corners are not AABB)

Important is the `StepType` value. 
It determines, where coordinates botLeft and topRight are positioned within the pixel
It can be at pixel center or on its border.
First position [0, 0] is always correct, but maximal position [w, h] can be shifted by -1 based on mode:

*Border mode*
`image width = 3 pixels (a---b---c---d), GPS are located at a, b, c, d`
In this case, there are 3 "steps" (_`---`_) between GPS positions, but 4 positions in total
For this, max position is calculated directly from width that correspond to steps
`a + 3 * '---' => d`
 
*Center mode*
`image width = 3 pixels (|-a-|-b-|-c-|), GPS are located at a, b, c`
In this case, there are only 2 "steps" (_`-|-`_) between GPS positions, but 3 positions in total
For this, max position is calculated by subtract 1 from width to get correct last.
`a + (3 - 1) * '-x-' => c`

### Reprojections

The are methods in class `Reprojection`.

* Reprojection creation
```
template <typename FromProjection, typename ToProjection>
static Reprojection CreateReprojection(FromProjection * from, ToProjection * to)
```

Create reprojection to re-project data `from` -> `to`.
Calculates mapping: `toData[index] = fromData[reprojection[index]]`

* Reprojections using different filtering methods
```
template <typename DataType, typename Out = DataType*, size_t ChannelsCount = 1>
   Out ReprojectDataNerestNeighbor(const DataType* inputData, const DataType NO_VALUE) const 
```

```
template <typename DataType, typename Out = DataType*, size_t ChannelsCount = 1>
   Out ReprojectDataBilinear(const DataType* inputData, const DataType NO_VALUE) const
```

```
template <typename DataType, typename Out = DataType*, size_t ChannelsCount = 1>
   Out ReprojectDataBicubic(const DataType* inputData, const DataType NO_VALUE) const
```

Takes `inputData` and create output image. 
Copy data from `inputData` to the output based on reprojection mapping. 
In places, where no mapping is present, use NO_VALUE.


* Single pixel reprojection
```
template <typename InPixelType, typename OutPixelType,
          typename FromProjection, typename ToProjection>
Pixel<OutPixelType> ReProject(Pixel<InPixelType> p, const FromProjection * from, const ToProjection * to)
 ```

Reprojects single pixel `p` from projection `from` to projection `to`. 
Its basically one step from `CreateReprojection` method, that reprojects every pixel of input projection `from` to output projection `to`.

### Utilities

The are helper static methods in class `MapProjectionUtils`.

* `Coordinate CalcEndPointShortest(const Coordinate & start, const Angle & bearing, MyRealType dist)` - 
Calculate end point based on shortest path (on real earth surface).

* `Coordinate CalcEndPointDirect(const Coordinate & start, const Angle & bearing, MyRealType dist)` - 
Calculate end point based on direct path (straight line between two points in projected earth).
So called "Rhumb lines".

* `double Distance(const Coordinate & from, const Coordinate & to)` - 
Calculates Haversine distance between two GPS places

### Rendering

Class `ProjectionRenderer` is used mainly for debugging purposed. 
It can draw output data.

For a better debugging, use borders added with method `void AddBorders(const char * fileName, int useEveryNthPoint)`.
Borderd can be found in directory _TestData_ in a file _borders.zip_. 
The file must be decompressed.

### SIMD

In some casess, the speed-up can be achieved by using SIMD instructions 
(AVX can compute 8 float operations at once, NEON 4 float operations at once).
It offers calculation only in `float`, so `typedef MyRealType` should be `float`.
If not, the data are casted to `float`.
The support for this can be found in directory _simd_.
SIMD must be enabled by macro `ENABLE_SIMD` (for AVX) or `HAVE_NEON` (for NEON) during compilation.
Logic is similar to single instruction mode.

To define a new projection, see existing ones in _simd/avx/Projections_ or 
_simd/neon/Projections_.

SIMD versions are named same as single instructions oned. 
To distinguish them, a different namespace is used.

Simple example:

```c++
namespace avx = Projections::Avx;
namespace neon = Projections::Neon;

avx::Mercator mercAvx;
avx::Miller millerAvx;

neon::Mercator mercNeon;
neon::Equirectangular eqNeon;

Reprojection reprojectionAvx = avx::Reprojection<int>::CreateReprojection(&millerSimd, &mercSimd);

```